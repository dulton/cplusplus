/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD.
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD.

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPServedUserHeader.h                       *
 *                                                                            *
 * This file defines the methods of the PServedUser header object:            *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change its parameters.                                                     *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jan.2008                                             *
 ******************************************************************************/
#ifndef RVSIPPSERVEDUSERHEADER_H
#define RVSIPPSERVEDUSERHEADER_H

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
/*                   DECLARATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipPServedUserHeaderStringName
 * ----------------------------
 * Defines all PServedUser header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_SERVED_USER_DISPLAYNAME,
    RVSIP_P_SERVED_USER_OTHER_PARAMS,
    RVSIP_P_SERVED_USER_BAD_SYNTAX
}RvSipPServedUserHeaderStringName;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPServedUserHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PServedUser header object inside a given message object.
 *          The header is kept in the header list of the message. You can choose
 *          to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - The handle to the message object.
 *         pushHeaderAtHead - A Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE) or to the
 *                            tail (RV_FALSE).
 * output: hHeader          - The handle to the newly constructed PServedUser
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderConstructInMsg(
                                       IN  RvSipMsgHandle			hSipMsg,
                                       IN  RvBool					pushHeaderAtHead,
                                       OUT RvSipPServedUserHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPServedUserHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PServedUser Header object.
 *          The header is constructed on a given page taken from a specified
 *          pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - The handle to the Message manager.
 *         hPool   - The handle to the memory pool that the object will use.
 *         hPage   - The handle to the memory page that the object will use.
 * output: hHeader - The handle to the newly constructed P-Served-User header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderConstruct(
                                           IN  RvSipMsgMgrHandle        hMsgMgr,
                                           IN  HRPOOL                   hPool,
                                           IN  HPAGE                    hPage,
                                           OUT RvSipPServedUserHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPServedUserHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PServedUser header object to a
 *          destination PServedUser header object. You must construct the
 *          destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - The handle to the destination PServedUser header object.
 *    hSource      - The handle to the destination PServedUser header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderCopy(
                                         INOUT RvSipPServedUserHeaderHandle hDestination,
                                         IN    RvSipPServedUserHeaderHandle hSource);


/***************************************************************************
 * RvSipPServedUserHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Served-User header object to a textual PServedUser
 *          header. The textual header is placed on a page taken from a
 *          specified pool. To copy the textual header from the page to a
 *          consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - The handle to the P-Served-User header object.
 *        hPool    - The handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderEncode(
                                          IN    RvSipPServedUserHeaderHandle	hHeader,
                                          IN    HRPOOL                  hPool,
                                          OUT   HPAGE*                  hPage,
                                          OUT   RvUint32*               length);

/***************************************************************************
 * RvSipPServedUserHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PServedUser header, such as
 *          "P-Served-User:sip:172.20.5.3:5060",into a P-Served-User header
 *          object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PServedUser header object.
 *    buffer    - The buffer containing a textual PServedUser header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderParse(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipPServedUserHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PServedUser header value into an PServedUser
 *          header object. A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value
 *          part as a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPServedUserHeaderParse() function to parse
 *          strings that also include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PServedUser header object.
 *    buffer    - The buffer containing a textual PServedUser header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderParseValue(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
                                     IN    RvChar*					buffer);

/***************************************************************************
 * RvSipPServedUserHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PServedUser header with bad-syntax information.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as
 *          it was.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderFix(
                                     IN RvSipPServedUserHeaderHandle	hHeader,
                                     IN RvChar*                 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPServedUserHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PServedUser header fields are kept in string format,
 *          such as the P-Served-User header display name. To get such a
 *          field from the PServedUser header object, your application should
 *          supply an adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the Get
 *          function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - The handle to the PServedUser header object.
 *   stringName  - The enumeration of the string name for which you require
 *                 the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPServedUserHeaderGetStringLength(
                                      IN  RvSipPServedUserHeaderHandle     hHeader,
                                      IN  RvSipPServedUserHeaderStringName stringName);

/***************************************************************************
 * RvSipPServedUserHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the PServedUser header into a given
 *          buffer. If the bufferLen is adequate, the function copies the
 *          requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the
 *          required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen	 - The length of the requested parameter + 1, to
 *                     include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetDisplayName(
                                               IN RvSipPServedUserHeaderHandle	hHeader,
                                               IN RvChar*               strBuffer,
                                               IN RvUint                bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPServedUserHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - The handle to the header object.
 *    strDisplayName - The display name to be set in the PServedUser header.
 *                     If NULL is supplied, the existing display name is
 *                     removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetDisplayName(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
                                     IN    RvChar*					strDisplayName);

/***************************************************************************
 * RvSipPServedUserHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the P-Served-User header object
 *          as an Address object. This function returns the handle to the
 *          address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if
 *               the Address Spec object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - The handle to the PServedUser header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPServedUserHeaderGetAddrSpec(
                                    IN RvSipPServedUserHeaderHandle	hHeader);

/***************************************************************************
 * RvSipPServedUserHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PServedUser header object.
 *    hAddrSpec - The handle to the Address Spec Address object. If NULL is supplied,
 *				  the existing, Address Spec is removed from the PServedUser header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetAddrSpec(
                                        IN    RvSipPServedUserHeaderHandle hHeader,
                                        IN    RvSipAddressHandle	pAddrSpec);

/***************************************************************************
 * RvSipPServedUserHeaderSetSessionCaseType
 * ------------------------------------------------------------------------
 * General:Sets the SessionCase Type in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *    eSessionCaseType - The SessionCase Type to be set in the PServedUser
 *                   header. If UNDEFINED is supplied, the
 *                   existing SessionCase Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetSessionCaseType(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
									 IN	   RvSipPServedUserSessionCaseType	eSessionCaseType);

/***************************************************************************
 * RvSipPServedUserHeaderGetSessionCaseType
 * ------------------------------------------------------------------------
 * General: Returns the SessionCase Type value in the header.
 * Return Value: Returns RvSipPServedUserSessionCaseType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - The handle to the header object.
 ***************************************************************************/
RVAPI RvSipPServedUserSessionCaseType RVCALLCONV RvSipPServedUserHeaderGetSessionCaseType(
                                               IN RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * RvSipPServedUserHeaderSetRegistrationStateType
 * ------------------------------------------------------------------------
 * General:Sets the RegistrationState Type in the PServedUser header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *    eRegistrationStateType - The RegistrationState Type to be set in the
 *                   PServedUser header. If UNDEFINED is supplied, the
 *                   existing RegistrationState Type is removed from
 *                   the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetRegistrationStateType(
                                     IN    RvSipPServedUserHeaderHandle	hHeader,
									 IN	   RvSipPServedUserRegistrationStateType	eRegistrationStateType);

/***************************************************************************
 * RvSipPServedUserHeaderGetRegistrationStateType
 * ------------------------------------------------------------------------
 * General: Returns the RegistrationState Type value in the header.
 * Return Value: Returns RvSipPServedUserRegistrationStateType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - The handle to the header object.
 ***************************************************************************/
RVAPI RvSipPServedUserRegistrationStateType RVCALLCONV RvSipPServedUserHeaderGetRegistrationStateType(
                                               IN RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * RvSipPServedUserHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PServedUser header other params field of the
 *          PServedUser header object into a given buffer.
 *          Not all the PServedUser header parameters have separated fields
 *          in the PServedUser header object. Parameters with no specific
 *          fields are referred to as other params. They are kept in the
 *          object in one concatenated string in the form,
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, this function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the PServedUser header object.
 *        strBuffer  - The buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen -  The length of the requested parameter + 1, to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetOtherParams(
                                               IN RvSipPServedUserHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPServedUserHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PServedUser header object.
 *         Not all the PServedUser header parameters have separated
 *         fields in the PServedUser header object. Parameters with no
 *         specific fields are referred to as "other params".
 *         They are kept in the object in one concatenated string in
 *         the form, "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - The handle to the PServedUser header object.
 *    strPServedUserParam	- The extended parameters field to be set in the
 *                            PServedUser header. If NULL is supplied, the
 *                            existing extended parameters field is removed
 *                            from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetOtherParams(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar*				 strPServedUserParam);

/***************************************************************************
 * RvSipPServedUserHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer. A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the
 *          required buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad PServedUser header,
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
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderGetStrBadSyntax(
                                               IN RvSipPServedUserHeaderHandle	hHeader,
                                               IN RvChar*				strBuffer,
                                               IN RvUint				bufferLen,
                                              OUT RvUint*				actualLen);

/***************************************************************************
 * RvSipPServedUserHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object. A SIP header has the
 *          following grammar: header-name:header-value. When a header
 *          contains a syntax error, the header-value is kept as a separate
 *          "bad-syntax" string. By using this function you can create a
 *          header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header. You can use his function when you want
 *          to send an illegal PServedUser header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- The handle to the header object.
 *  strBadSyntax	- The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPServedUserHeaderSetStrBadSyntax(
                                  IN RvSipPServedUserHeaderHandle	 hHeader,
                                  IN RvChar*                 strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPSERVEDUSERHEADER_H */


