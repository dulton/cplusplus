/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipSecurityHeader.h						  *
 *                                                                            *
 * The file defines the methods of the Security header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Dec.2005                                             *
 ******************************************************************************/
#ifndef RVSIPSECURITYHEADER_H
#define RVSIPSECURITYHEADER_H

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
 * RvSipSecurityHeaderType
 * ----------------------
 * This enum defines whether the Security header represents a
 * SecurityClient, SecurityServer or SecurityVerify header.
 */
typedef enum
{
    RVSIP_SECURITY_UNDEFINED_HEADER = -1,
    RVSIP_SECURITY_CLIENT_HEADER,
	RVSIP_SECURITY_SERVER_HEADER,
    RVSIP_SECURITY_VERIFY_HEADER
}RvSipSecurityHeaderType;

/*
 * RvSipSecurityHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all Security header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_SECURITY_MECHANISM_TYPE,
	RVSIP_SECURITY_PREFERENCE,
	RVSIP_SECURITY_DIGEST_ALGORITHM,
    RVSIP_SECURITY_DIGEST_QOP,
	RVSIP_SECURITY_DIGEST_VERIFY,
	RVSIP_SECURITY_OTHER_PARAMS,
    RVSIP_SECURITY_BAD_SYNTAX
}RvSipSecurityHeaderStringName;

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipSecurityHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Security header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderConstructInMsg(
                                   IN  RvSipMsgHandle				hSipMsg,
                                   IN  RvBool						pushHeaderAtHead,
                                   OUT RvSipSecurityHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipSecurityHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Security Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderConstruct(
                                       IN  RvSipMsgMgrHandle			hMsgMgr,
                                       IN  HRPOOL						hPool,
                                       IN  HPAGE						hPage,
                                       OUT RvSipSecurityHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipSecurityHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Security header object to a destination Security
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Security header object.
 *    hSource      - Handle to the destination Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderCopy(
                                         INOUT RvSipSecurityHeaderHandle hDestination,
                                         IN    RvSipSecurityHeaderHandle hSource);


/***************************************************************************
 * RvSipSecurityHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Security header object to a textual Security header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Security header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderEncode(
                                          IN    RvSipSecurityHeaderHandle hHeader,
                                          IN    HRPOOL                    hPool,
                                          OUT   HPAGE*                    hPage,
                                          OUT   RvUint32*                 length);

/***************************************************************************
 * RvSipSecurityHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Security header-for example,
 *          "Security-Verify: ipsec-ike;q=0.1"-into a Security header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Security header object.
 *    buffer    - Buffer containing a textual Security header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderParse(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar*					 buffer);

/***************************************************************************
 * RvSipSecurityHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Security header value into an Security header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipSecurityHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Security header object.
 *    buffer    - The buffer containing a textual Security header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderParseValue(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar*					 buffer);

/***************************************************************************
 * RvSipSecurityHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Security header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderFix(
                                     IN RvSipSecurityHeaderHandle hHeader,
                                     IN RvChar*                   pFixedBuffer);

/***************************************************************************
 * RvSipSecurityHeaderIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two Security header objects.
 *         Security headers are considered equal if all parameters, excluding 
 *         extension parameters, are equal (case insensitive).
 *         The string parameters are compared byte-by-byte,
 *         A header containing a parameter never matches a header without that parameter.
 * Return Value: Returns RV_TRUE if the Security header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - A handle to the Security Header object.
 *    hOtherHeader - Handle to the Security header object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipSecurityHeaderIsEqual(
                                 IN  const RvSipSecurityHeaderHandle  hHeader,
                                 IN  const RvSipSecurityHeaderHandle  hOtherHeader);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSecurityHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Security header fields are kept in a string format-for example, the
 *          Security header DigestAlgorithm name. In order to get such a field from the Security header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Security header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipSecurityHeaderGetStringLength(
                                      IN  RvSipSecurityHeaderHandle     hHeader,
                                      IN  RvSipSecurityHeaderStringName stringName);

/***************************************************************************
 * RvSipSecurityHeaderGetSecurityHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Security Header object.
 * Return Value: Returns the Security header type enumeration from the Security
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipSecurityHeaderType RVCALLCONV RvSipSecurityHeaderGetSecurityHeaderType(
                                   IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetSecurityHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Security Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader			  - Handle to the Security header object.
 *    eSecurityHeaderType - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSecurityHeaderType(
                                 IN    RvSipSecurityHeaderHandle hHeader,
                                 IN    RvSipSecurityHeaderType   eSecurityHeaderType);

/***************************************************************************
 * RvSipSecurityHeaderGetStrMechanismType
 * ------------------------------------------------------------------------
 * General: Copies the StrMechanismType from the Security header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrMechanismType(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetMechanismType
 * ------------------------------------------------------------------------
 * General:Sets the Mechanism Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strMechanismType - The Mechanism Type to be set in the Security header. If NULL is supplied, the
 *                 existing Mechanism Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetMechanismType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityMechanismType	eMechanismType,
                                     IN    RvChar*						strMechanismType);

/***************************************************************************
 * RvSipSecurityHeaderGetMechanismType
 * ------------------------------------------------------------------------
 * General: Returns the RvSipSecurityMechanismType enumeration value for the mechanism type of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvSipSecurityMechanismType RVCALLCONV RvSipSecurityHeaderGetMechanismType(
                                               IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderGetStrPreference
 * ------------------------------------------------------------------------
 * General: Copies the Security header Preference field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrPreference(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetStrPreference
 * ------------------------------------------------------------------------
 * General:Sets the Preference field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *    strPreference	- The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrPreference(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *					 pPreference);

/***************************************************************************
 * RvSipSecurityHeaderGetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Gets the Digest algorithm enumeration from the Security header
 *          object.
 *          if this function returns RVSIP_AUTH_ALGORITHM_OTHER, call
 *          RvSipSecurityHeaderGetStrDigestAlgorithm() to get the algorithm in a
 *          string format.
 * Return Value: Returns the digest algorithm enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipAuthAlgorithm RVCALLCONV RvSipSecurityHeaderGetDigestAlgorithm(
                                      IN  RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Sets the Digest algorithm in Security header object.
 *          If eDigestAlgorithm is RVSIP_AUTH_SCHEME_OTHER, strDigestAlgorithm is
 *          copied to the header. Otherwise, strDigestAlgorithm is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader				- Handle to the Security header object.
 *  eDigestAlgorithm	- The Digest algorithm to be set in the object.
 *	strDigestAlgorithm	- Text string giving the Digest algorithm to be set in the object. You can
 *							use this parameter if eDigestAlgorithm is set to
 *							RVSIP_AUTH_ALGORITHM_OTHER. Otherwise, the parameter is set to
 *							NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetDigestAlgorithm(
                                  IN  RvSipSecurityHeaderHandle	hHeader,
                                  IN  RvSipAuthAlgorithm		eDigestAlgorithm,
                                  IN  RvChar					*strDigestAlgorithm);

/***************************************************************************
 * RvSipSecurityHeaderGetDigestAlgorithmAKAVersion
 * ------------------------------------------------------------------------
 * General: Gets the AKAVersion from the Security header object.
 * Return Value: Returns the AKAVersion, or UNDEFINED if the AKAVersion
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetDigestAlgorithmAKAVersion(
                                         IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetDigestAlgorithmAKAVersion
 * ------------------------------------------------------------------------
 * General:  Sets the AKAVersion number of the Security header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- Handle to the Security header object.
 *    AKAVersion	- The AKAVersion number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetDigestAlgorithmAKAVersion(
                                         IN    RvSipSecurityHeaderHandle	hHeader,
                                         IN    RvInt32						AKAVersion);
									  
/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestAlgorithm field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestAlgorithm(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetStrDigestAlgorithm
 * ------------------------------------------------------------------------
 * General:Sets the DigestAlgorithm field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strDigestAlgorithm - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrDigestAlgorithm(
                                 IN    RvSipSecurityHeaderHandle hHeader,
                                 IN    RvChar *					 pDigestAlgorithm);

/***************************************************************************
 * RvSipSecurityHeaderGetDigestQopOption
 * ------------------------------------------------------------------------
 * General: Gets the DigestQop option from the Security header object.
 * Return Value: The DigestQop option from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipSecurityHeaderGetDigestQopOption(
                                      IN  RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestQop
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestQop field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestQop(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetStrDigestQop
 * ------------------------------------------------------------------------
 * General:Sets the DigestQop field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *    strDigestQop	- The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrDigestQop(
                                     IN	RvSipSecurityHeaderHandle	hHeader,
									 IN	RvSipAuthQopOption			eDigestQop,
                                     IN	RvChar *					pDigestQop);

/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestVerify
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestVerify field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestVerify(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetStrDigestVerify
 * ------------------------------------------------------------------------
 * General:Sets the DigestVerify field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strDigestVerify - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrDigestVerify(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pDigestVerify);

/***************************************************************************
 * RvSipSecurityHeaderSetAlgorithmType
 * ------------------------------------------------------------------------
 * General:Sets the Algorithm Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eAlgorithmType - The Algorithm Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Algorithm Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetAlgorithmType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityAlgorithmType	eAlgorithmType);

/***************************************************************************
 * RvSipSecurityHeaderGetAlgorithmType
 * ------------------------------------------------------------------------
 * General: Returns the value of Algorithm Type in the header.
 * Return Value: Returns RvSipSecurityAlgorithmType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityAlgorithmType RVCALLCONV RvSipSecurityHeaderGetAlgorithmType(
                                               IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetProtocolType
 * ------------------------------------------------------------------------
 * General:Sets the Protocol Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eProtocolType - The Protocol Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Protocol Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetProtocolType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityProtocolType	eProtocolType);

/***************************************************************************
 * RvSipSecurityHeaderGetProtocolType
 * ------------------------------------------------------------------------
 * General: Returns value Protocol Type in the header.
 * Return Value: Returns RvSipSecurityProtocolType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityProtocolType RVCALLCONV RvSipSecurityHeaderGetProtocolType(
                                               IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetModeType
 * ------------------------------------------------------------------------
 * General:Sets the Mode Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eModeType - The Mode Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Mode Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetModeType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityModeType		eModeType);

/***************************************************************************
 * RvSipSecurityHeaderGetModeType
 * ------------------------------------------------------------------------
 * General: Returns value Mode Type in the header.
 * Return Value: Returns RvSipSecurityModeType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityModeType RVCALLCONV RvSipSecurityHeaderGetModeType(
									IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General:Sets the EncryptAlgorithm Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eEncryptAlgorithmType - The EncryptAlgorithm Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing EncryptAlgorithm Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetEncryptAlgorithmType(
                                     IN    RvSipSecurityHeaderHandle			hHeader,
									 IN	   RvSipSecurityEncryptAlgorithmType	eEncryptAlgorithmType);

/***************************************************************************
 * RvSipSecurityHeaderGetEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General: Returns value EncryptAlgorithm Type in the header.
 * Return Value: Returns RvSipSecurityEncryptAlgorithmType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityEncryptAlgorithmType RVCALLCONV RvSipSecurityHeaderGetEncryptAlgorithmType(
                                               IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * RvSipSecurityHeaderGetSpiC
 * ------------------------------------------------------------------------
 * General: return spi value and boolean stating if it is initialized.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader		- Handle to the Security header object.
 * output:isInitialized - is the spi value initialized.
 *		  spiC			- The spi value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetSpiC(
                                               IN RvSipSecurityHeaderHandle hHeader,
											  OUT RvBool*					isInitialized,
                                              OUT RvUint32*	                spiC);

/***************************************************************************
 * RvSipSecurityHeaderSetSpiC
 * ------------------------------------------------------------------------
 * General:Sets the Spi Value field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *	  isInitialized - is the spi value initialized.
 *	  spiC			- The spi value. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSpiC(
                                     IN RvSipSecurityHeaderHandle	hHeader,
                                     IN RvBool						isInitialized,
                                     IN RvUint32	                spiC);

/***************************************************************************
 * RvSipSecurityHeaderGetSpiS
 * ------------------------------------------------------------------------
 * General: return spi value and boolean stating if it is initialized.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader		- Handle to the Security header object.
 * output:isInitialized - is the spi value initialized.
 *		  spiS			- The spi value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetSpiS(
                                               IN RvSipSecurityHeaderHandle hHeader,
											  OUT RvBool*					isInitialized,
                                              OUT RvUint32*	                spiS);

/***************************************************************************
 * RvSipSecurityHeaderSetSpiS
 * ------------------------------------------------------------------------
 * General:Sets the Spi Value field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *	  isInitialized - is the spi value initialized.
 *	  spiS			- The spi value. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSpiS(
                                     IN RvSipSecurityHeaderHandle	hHeader,
                                     IN RvBool						isInitialized,
                                     IN RvUint32	                spiS);

/***************************************************************************
 * RvSipSecurityHeaderGetPortC
 * ------------------------------------------------------------------------
 * General: Gets portC parameter from the Security header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetPortC(
                                         IN RvSipSecurityHeaderHandle hSecurityHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetPortC
 * ------------------------------------------------------------------------
 * General:  Sets portC parameter of the Security header object.
 *           Notice: Supplying port larger than 65535 will lead to unexpected results.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 *    portNum         - The port number value to be set in the object (must be
 *                      smaller than 65535).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetPortC(
                                         IN    RvSipSecurityHeaderHandle hSecurityHeader,
                                         IN    RvInt32					 portNum);

/***************************************************************************
 * RvSipSecurityHeaderGetPortS
 * ------------------------------------------------------------------------
 * General: Gets portS parameter from the Security header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetPortS(
                                         IN RvSipSecurityHeaderHandle hSecurityHeader);

/***************************************************************************
 * RvSipSecurityHeaderSetPortS
 * ------------------------------------------------------------------------
 * General:  Sets portS parameter of the Security header object.
 *           Notice: Supplying port larger than 65535 will lead to unexpected results.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 *    portNum         - The port number value to be set in the object (must be
 *                      smaller than 65535).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetPortS(
                                         IN    RvSipSecurityHeaderHandle hSecurityHeader,
                                         IN    RvInt32				 	 portNum);

/***************************************************************************
 * RvSipSecurityHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Security header other params field of the Security header object into a
 *          given buffer.
 *          Not all the Security header parameters have separated fields in the Security
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetOtherParams(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Security header object.
 *         Not all the Security header parameters have separated fields in the Security
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strOtherParams - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetOtherParams(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *					 pOtherParams);

/***************************************************************************
 * RvSipSecurityHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad Security header,
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
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrBadSyntax(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipSecurityHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Security header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrBadSyntax(
                                  IN RvSipSecurityHeaderHandle	hHeader,
                                  IN RvChar*					strBadSyntax);

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* RVSIPSECURITYHEADER_H */



