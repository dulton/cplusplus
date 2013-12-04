/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipContactHeader.h                           *
 *                                                                            *
 * The file defines the methods of the Contact header object:                 *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/
#ifndef RVSIPCONTACTHEADER_H
#define RVSIPCONTACTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipContactHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Contact header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_CONTACT_DISPLAYNAME,
    RVSIP_CONTACT_OTHER_PARAMS,
    RVSIP_CONTACT_QVAL,
    RVSIP_CONTACT_TEMP_GRUU,
    RVSIP_CONTACT_PUB_GRUU,
    RVSIP_CONTACT_AUDIO,
    RVSIP_CONTACT_AUTOMATA,
    RVSIP_CONTACT_CLASS,
    RVSIP_CONTACT_DUPLEX,
    RVSIP_CONTACT_DATA,
    RVSIP_CONTACT_CONTROL,
    RVSIP_CONTACT_MOBILITY,
    RVSIP_CONTACT_DESCRIPTION,
    RVSIP_CONTACT_EVENTS,
    RVSIP_CONTACT_PRIORITY,
    RVSIP_CONTACT_METHODS,
    RVSIP_CONTACT_SCHEMES,
    RVSIP_CONTACT_APPLICATION,
    RVSIP_CONTACT_VIDEO,
    RVSIP_CONTACT_LANGUAGE,
    RVSIP_CONTACT_TYPE,
    RVSIP_CONTACT_ISFOCUS,
    RVSIP_CONTACT_ACTOR,
    RVSIP_CONTACT_TEXT,
    RVSIP_CONTACT_EXTENSIONS,
    RVSIP_CONTACT_SIP_INSTANCE,
    RVSIP_CONTACT_BAD_SYNTAX
}RvSipContactHeaderStringName;

#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipContactHeaderFieldName
 * ----------------------------
 * This enum defines all the Contact header fields.
 * It is used for getting and setting via RvSipHeader interface.
 */
typedef enum
{
	RVSIP_CONTACT_FIELD_ADDR_SPEC     = 0,
	RVSIP_CONTACT_FIELD_EXPIRES       = 1,
	RVSIP_CONTACT_FIELD_QVAL          = 2,
    RVSIP_CONTACT_FIELD_ACTION        = 3,
	RVSIP_CONTACT_FIELD_OTHER_PARAMS  = 4,
	RVSIP_CONTACT_FIELD_STAR          = 5,
	RVSIP_CONTACT_FIELD_BAD_SYNTAX    = 6
}RvSipContactHeaderFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipContactHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Contact header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipContactHeaderHandle* hHeader);

/***************************************************************************
 * RvSipContactHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Contact Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipContactHeaderHandle* hHeader);

/***************************************************************************
 * RvSipContactHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Contact header object to a destination Contact
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Contact header object.
 *    hSource      - Handle to the destination Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderCopy(
                                         INOUT RvSipContactHeaderHandle hDestination,
                                         IN    RvSipContactHeaderHandle hSource);


/***************************************************************************
 * RvSipContactHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a contact header object to a textual Contact header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the contact header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderEncode(
                                          IN    RvSipContactHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length);

/***************************************************************************
 * RvSipContactHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Contact header-for example,
 *          "Contact:sip:172.20.5.3:5060"-into a contact header object. All the textual
 *          fields are placed inside the object.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Contact header object.
 *    buffer    - Buffer containing a textual Contact header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderParse(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipContactHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Contact header value into an Contact header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContactHeaderParse() function to parse strings that also
 *          include the header-name.
 *          Note: Before performing the parse operation the stack 
 *          resets all the header fields. Therefore, if the parse 
 *          function fails, you will not be able to get the previous 
 *          header field values.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Contact header object.
 *    buffer    - The buffer containing a textual Contact header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderParseValue(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipContactHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Contact header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderFix(
                                     IN RvSipContactHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipContactHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Contact header fields are kept in a string format-for example, the
 *          contact header display name. In order to get such a field from the Contact header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Contact header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipContactHeaderGetStringLength(
                                      IN  RvSipContactHeaderHandle     hHeader,
                                      IN  RvSipContactHeaderStringName stringName);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipContactHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetDisplayName(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the Contact header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetDisplayName(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 strDisplayName);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipContactHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the contact header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipContactHeaderGetAddrSpec(
                                    IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * RvSipContactHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Contact header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the Contact header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetAddrSpec(
                                        IN    RvSipContactHeaderHandle hHeader,
                                        IN    RvSipAddressHandle       pAddrSpec);


/***************************************************************************
 * RvSipContactHeaderGetExpires
 * ------------------------------------------------------------------------
 * General: The Contact header contains an Expires header object. This function returns the
 *          handle to the Expires header object.
 * Return Value: Returns the handle to the Expires header object, or NULL if the Expires header
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipExpiresHeaderHandle RVCALLCONV RvSipContactHeaderGetExpires(
                                         IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * RvSipContactHeaderSetExpires
 * ------------------------------------------------------------------------
 * General: Sets the Expires header object in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    hExpires - Handle to the Expires header object to be set in the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetExpires(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvSipExpiresHeaderHandle hExpires);

/***************************************************************************
 * RvSipContactHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the 'q' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetQVal(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: Sets the 'q' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strQVal  - The value of the offset of the 'q' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetQVal(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strQVal);

/***************************************************************************
 * RvSipContactHeaderGetAction
 * ------------------------------------------------------------------------
 * General: The Contact header contains the Action type. This function
 *          returns this type.
 * Return Value: Returns the type of the action in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipContactAction RVCALLCONV RvSipContactHeaderGetAction(
                                         IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * RvSipContactHeaderSetAction
 * ------------------------------------------------------------------------
 * General: Sets the action type in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    action   - The action type to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetAction(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvSipContactAction       action);

/***************************************************************************
 * RvSipContactHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Contact header other params field of the Contact header object into a
 *          given buffer.
 *          Not all the Contact header parameters have separated fields in the Contact
 *          header object. Parameters with no specific fields are refered to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Contact header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetOtherParams(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Contact header object.
 *         Not all the Contact header parameters have separated fields in the Contact
 *         header object. Parameters with no specific fields are refered to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Contact header object.
 *    strContactParam - The extended parameters field to be set in the Contact header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetOtherParams(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                strContactParam);


/***************************************************************************
 * RvSipContactHeaderSetStar
 * ------------------------------------------------------------------------
 * General: A Contact header can be in the form of "Contact: * ". This function defines the
 *          Contact header as an Asterisk Contact header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle of the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStar(
                                     IN    RvSipContactHeaderHandle hHeader);


/***************************************************************************
 * RvSipContactHeaderIsStar
 * ------------------------------------------------------------------------
 * General:Determines whether or not the contact header object contains an Asterisk (*).
 *         This means that the header is of the form "Contact: * ".
 * Return Value: Returns RV_TRUE if the Contact header is of the form "Contact: * ". Otherwise,
 *               the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle of the Contact header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipContactHeaderIsStar(
                                     IN RvSipContactHeaderHandle hHeader);


/***************************************************************************
 * RvSipContactHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Contact header,
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetStrBadSyntax(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Contact header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStrBadSyntax(
                                  IN RvSipContactHeaderHandle hHeader,
                                  IN RvChar*                 strBadSyntax);


/***************************************************************************
 * RvSipContactHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Contact header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipContactHeader - Handle to the Contact header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output patameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetRpoolString(
                             IN    RvSipContactHeaderHandle      hSipContactHeader,
                             IN    RvSipContactHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr);


/***************************************************************************
 * RvSipContactHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Contact header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipContactHeader - Handle to the Contact header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetRpoolString(
                             IN    RvSipContactHeaderHandle      hSipContactHeader,
                             IN    RvSipContactHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr);


/***************************************************************************
 * RvSipContactHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Contact header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetCompactForm(
                                   IN    RvSipContactHeaderHandle hHeader,
                                   IN    RvBool                bIsCompact);

/***************************************************************************
 * RvSipContactHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Contact header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetCompactForm(
                                   IN    RvSipContactHeaderHandle hHeader,
                                   IN    RvBool                *pbIsCompact);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * RvSipContactHeaderGetPubGruu
 * ------------------------------------------------------------------------
 * General: Copies the 'pub-gruu' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetPubGruu(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetPubGruu
 * ------------------------------------------------------------------------
 * General: Sets the 'pub-gruu' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strPubGruu  - The value of the offset of the 'pub-gruu' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetPubGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strPubGruu);

/***************************************************************************
 * RvSipContactHeaderGetTempGruu
 * ------------------------------------------------------------------------
 * General: Copies the 'temp-gruu' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetTempGruu(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipContactHeaderSetTempGruu
 * ------------------------------------------------------------------------
 * General: Sets the 'temp-gruu' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strTempGruu  - The value of the offset of the 'temp-gruu' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetTempGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strTempGruu);

/***************************************************************************
 * RvSipContactHeaderGetRegIDNum
 * ------------------------------------------------------------------------
 * General: Gets the reg-id from the Contact header object.
 * Return Value: Returns the reg-id number, or UNDEFINED if the reg-id
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipContactHeaderGetRegIDNum(
                                         IN RvSipContactHeaderHandle hHeader);


/***************************************************************************
 * RvSipContactHeaderSetRegIDNum
 * ------------------------------------------------------------------------
 * General: Sets the reg-id field in the Contact header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Contact header object.
 *    regIDNum  - The reg-id value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetRegIDNum(
                                         IN    RvSipContactHeaderHandle hHeader,
                                         IN    RvInt32                  regIDNum);
                                               
/***************************************************************************
 * RvSipContactHeaderIsFeatureTagActive
 * ------------------------------------------------------------------------
 * General: Check if the requested feature tag is active.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        eType     - Enumeration denoting type of feature tag.
 *        
 * output:bIsFeatureActive - Boolean denoting whether the requested tag is active.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderIsFeatureTagActive(
                                               IN  RvSipContactHeaderHandle   hHeader,
                                               IN  RvSipContactFeatureTagType eType,
                                               OUT RvBool*                    bIsFeatureActive);

/***************************************************************************
 * RvSipContactHeaderGetStrFeatureTag
 * ------------------------------------------------------------------------
 * General: Copies the Feature Tag from the Contact header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        eType      - Enumeration denoting type of feature tag.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetStrFeatureTag(
                                               IN RvSipContactHeaderHandle   hHeader,
                                               IN RvSipContactFeatureTagType eType,
                                               IN RvChar*                    strBuffer,
                                               IN RvUint                     bufferLen,
                                               OUT RvUint*                   actualLen);

/***************************************************************************
 * RvSipContactHeaderSetStrFeatureTag
 * ------------------------------------------------------------------------
 * General:Sets a Feature Tag in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    eType       - Enumeration denoting type of feature tag.
 *    pFeatureTag - The Feature Tag to be set in the Contact header. If NULL is supplied, the
 *                  existing Feature Tag is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStrFeatureTag(
                                     IN RvSipContactHeaderHandle   hHeader,
                                     IN RvSipContactFeatureTagType eType,
                                     IN RvChar*                    pFeatureTag);

/***************************************************************************
 * RvSipContactHeaderSetFeatureTagActivation
 * ------------------------------------------------------------------------
 * General: Activates/deactivates a specific Feature Tag in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the header object.
 *    eType   - The Feature Tag to be activated in the Contact header. 
 *    bIsActivated - boolean denoting should the feature be activated or deactivated.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetFeatureTagActivation(
                                     IN RvSipContactHeaderHandle   hHeader,
                                     IN RvSipContactFeatureTagType eType,
                                     IN RvBool                     bIsActivated);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPCONTACTHEADER_H */


