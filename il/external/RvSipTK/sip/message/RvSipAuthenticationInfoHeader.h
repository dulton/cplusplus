/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     RvSipAuthenticationInfoHeader.h                        *
 *                                                                            *
 * The file defines the methods of the AuthenticationInfo header object:      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *    Tamar Barzuza     Mar 2005                                              *
 ******************************************************************************/

#ifndef RVSIPAUTHENTICATIONINFOHEADER_H
#define RVSIPAUTHENTICATIONINFOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/


/*
 * RvSipAuthenticationInfoHeaderStringName
 * ----------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all AuthenticationInfo header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_AUTH_INFO_NEXT_NONCE,
	RVSIP_AUTH_INFO_CNONCE,
	RVSIP_AUTH_INFO_RSP_AUTH,
	RVSIP_AUTH_INFO_MSG_QOP,
    RVSIP_AUTH_INFO_BAD_SYNTAX
}RvSipAuthenticationInfoHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipAuthenticationInfoHeaderFieldName
 * ----------------------------
 * This enum defines all the Authentication-Info header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_AUTH_INFO_FIELD_NEXT_NONCE   = 0,
	RVSIP_AUTH_INFO_FIELD_CNONCE       = 1,
	RVSIP_AUTH_INFO_FIELD_NONCE_COUNT  = 2,
	RVSIP_AUTH_INFO_FIELD_RSP_AUTH     = 3,
	RVSIP_AUTH_INFO_FIELD_MSG_QOP      = 4,
	RVSIP_AUTH_INFO_FIELD_BAD_SYNTAX   = 5
}RvSipAuthenticationInfoHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAuthenticationInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs an AuthenticationInfo header object inside a given message
 *          object. The header is kept in the header list of the message. You
 *          can choose to insert the header either at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE), or to
 *                            the tail (RV_FALSE).
 * output: hHeader          - Handle to the newly constructed AuthenticationInfo
 *                            header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderConstructInMsg(
                                 IN  RvSipMsgHandle                       hSipMsg,
                                 IN  RvBool                               pushHeaderAtHead,
                                 OUT RvSipAuthenticationInfoHeaderHandle *hHeader);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone AuthenticationInfo Header
 *          object. The header is constructed on a given page taken from a
 *          specified pool. The handle to the new header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderConstruct(
                                         IN  RvSipMsgMgrHandle                    hMsgMgr,
                                         IN  HRPOOL                               hPool,
                                         IN  HPAGE                                hPage,
                                         OUT RvSipAuthenticationInfoHeaderHandle *hHeader);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies the informative field from a source AuthenticationInfo header
 *          to a destination AuthenticationInfo header. The informative field is
 *          the field indicated by the RvSipAuthInfoType enumeration.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination AuthenticationInfo header object.
 *    hSource      - Handle to the source AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderCopy
                                    (IN    RvSipAuthenticationInfoHeaderHandle hDestination,
                                     IN    RvSipAuthenticationInfoHeaderHandle hSource);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes an AuthenticationInfo header object to a textual AuthenticationInfo header. The
 *          textual header is placed on a page taken from a specified pool. In order to copy
 *          the textual header from the page to a consecutive buffer, use
 *          RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle to the AuthenticationInfo header object.
 *        hPool             - Handle to the specified memory pool.
 * output: hPage            - The memory page allocated to contain the encoded header.
 *         length           - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderEncode(
                                          IN    RvSipAuthenticationInfoHeaderHandle  hHeader,
                                          IN    HRPOOL                               hPool,
                                          OUT   HPAGE*                               hPage,
                                          OUT   RvUint32*                            length);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Authentication-Info header into an
 *          Authentication-Info header object.
 *          All the textual fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the AuthenticationInfo header object.
 *    buffer    - Buffer containing a textual AuthenticationInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderParse(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar*                              buffer);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual AuthenticationInfo header value into an AuthenticationInfo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipAuthenticationInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the AuthenticationInfo header object.
 *    buffer    - The buffer containing a textual AuthenticationInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderParseValue(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar*                              buffer);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an AuthenticationInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderFix(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar*                             pFixedBuffer);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the AuthenticationInfo header fields are kept in a string format, for
 *          example, the AuthenticationInfo header next-nonce string. In order to get such a field
 *          from the AuthenticationInfo header object, your application should supply an
 *          adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the AuthenticationInfo header object.
 *    eStringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAuthenticationInfoHeaderGetStringLength(
                                      IN  RvSipAuthenticationInfoHeaderHandle     hHeader,
                                      IN  RvSipAuthenticationInfoHeaderStringName eStringName);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetNextNonce
 * ------------------------------------------------------------------------
 * General: Copies the next-nonce string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetNextNonce(
                                       IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                       IN RvChar*                              strBuffer,
                                       IN RvUint                               bufferLen,
                                       OUT RvUint*                             actualLen);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetNextNonce
 * ------------------------------------------------------------------------
 * General:Sets the next-nonce string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the AuthenticationInfo header object.
 *    pNextNonce - The next-nonce string to be set in the AuthenticationInfo header.
 *                 If NULL is supplied, the existing next-nonce is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetNextNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar                              *pNextNonce);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General: Copies the cnonce string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetCNonce(
                                       IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                       IN RvChar*							   strBuffer,
                                       IN RvUint							   bufferLen,
                                       OUT RvUint*							   actualLen);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General:Sets the cnonce string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the AuthenticationInfo header object.
 *    pCNonce  - The cnonce string to be set in the AuthenticationInfo header. If a NULL value is
 *             supplied, the existing cnonce string in the header object is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetCNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar *                            pCNonce);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetResponseAuth
 * ------------------------------------------------------------------------
 * General: Copies the response-auth string from the AuthenticationInfo header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the AuthenticationInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetResponseAuth(
                                        IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                        IN RvChar*                             strBuffer,
                                        IN RvUint                              bufferLen,
                                        OUT RvUint*                            actualLen);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetResponseAuth
 * ------------------------------------------------------------------------
 * General:Sets the response-auth string in the AuthenticationInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle to the AuthenticationInfo header object.
 *    pResponseAuth  - The response-autt string to be set in the AuthenticationInfo header. If a NULL value is
 *                     supplied, the existing response-auth string in the header is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetResponseAuth(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar *                            pResponseAuth);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetNonceCount
 * ------------------------------------------------------------------------
 * General: Gets the nonce count value from the AuthenticationInfo Header object.
 * Return Value: Returns the nonce count value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAuthenticationInfoHeaderGetNonceCount(
                                    IN RvSipAuthenticationInfoHeaderHandle hSipAuthInfoHeader);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetNonceCount
 * ------------------------------------------------------------------------
 * General: Sets the nonce count value in the AuthenticationInfo Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAuthInfoHeader          - Handle to the AuthenticationInfo header object.
 *    nonceCount              - The nonce count value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetNonceCount(
                                 IN    RvSipAuthenticationInfoHeaderHandle hSipAuthInfoHeader,
                                 IN    RvInt32                             nonceCount);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetQopOption
 * ------------------------------------------------------------------------
 * General: Gets the Qop option enumeration from the AuthenticationInfo object.
 * Return Value: The qop type from the object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Returns the Qop option enumeration from the object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipAuthenticationInfoHeaderGetQopOption(
                                      IN  RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General: Copies the Qop string value of the Authentication-Info object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Authentication-Info header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetStrQop(
                                       IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                       IN RvChar*                             strBuffer,
                                       IN RvUint                              bufferLen,
                                       OUT RvUint*                            actualLen);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetQopOption
 * ------------------------------------------------------------------------
 * General:Sets the Qop string in the Authentication-Info header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Authentication-Info header object.
 *    eQop     - The qop option enumeration value to be set in the object.
 *    strQop   - You can use this parameter only if the eQop parameter is set to
 *               RVSIP_AUTH_QOP_OTHER. In this case you can supply the qop
 *               option as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetQopOption(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvSipAuthQopOption                  eQop,
									 IN RvChar *                            strQop);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad AuthenticationInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetStrBadSyntax(
                                     IN  RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN  RvChar*							  strBuffer,
                                     IN  RvUint								  bufferLen,
                                     OUT RvUint*							  actualLen);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal
 *          AuthenticationInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the AuthenticationInfo header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 *           eStringName    - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the AuthenticationInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderGetRpoolString(
                             IN    RvSipAuthenticationInfoHeaderHandle      hSipAuthInfoHeader,
                             IN    RvSipAuthenticationInfoHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                                *pRpoolPtr);

/***************************************************************************
 * RvSipAuthenticationInfoHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the AuthenticationInfo header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipAuthInfoHeader - Handle to the AuthenticationInfo header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticationInfoHeaderSetRpoolString(
                             IN RvSipAuthenticationInfoHeaderHandle      hSipAuthInfoHeader,
                             IN RvSipAuthenticationInfoHeaderStringName  eStringName,
                             IN RPOOL_Ptr                                *pRpoolPtr);


#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPAUTHENTICATIONINFOHEADER_H*/
