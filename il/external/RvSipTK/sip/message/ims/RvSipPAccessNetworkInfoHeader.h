 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *					RvSipPAccessNetworkInfoHeader.h							  *
 *                                                                            *
 * The file defines the methods of the PAccessNetworkInfo header object:      *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/
#ifndef RVSIPPACCESSNETWORKINFOHEADER_H
#define RVSIPPACCESSNETWORKINFOHEADER_H

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
 * RvSipPAccessNetworkInfoHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PAccessNetworkInfo header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_ACCESS_NETWORK_INFO_ACCESS_TYPE,
	RVSIP_P_ACCESS_NETWORK_INFO_CGI_3GPP,
	RVSIP_P_ACCESS_NETWORK_INFO_UTRAN_CELL_ID_3GPP,
	RVSIP_P_ACCESS_NETWORK_INFO_I_WLAN_NODE_ID,
	RVSIP_P_ACCESS_NETWORK_INFO_DSL_LOCATION,
	RVSIP_P_ACCESS_NETWORK_INFO_CI_3GPP2,
    RVSIP_P_ACCESS_NETWORK_INFO_OTHER_PARAMS,
    RVSIP_P_ACCESS_NETWORK_INFO_BAD_SYNTAX
}RvSipPAccessNetworkInfoHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PAccessNetworkInfo header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PAccessNetworkInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderConstructInMsg(
                                       IN  RvSipMsgHandle						hSipMsg,
                                       IN  RvBool								pushHeaderAtHead,
                                       OUT RvSipPAccessNetworkInfoHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PAccessNetworkInfo Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Access-Network-Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderConstruct(
                                           IN  RvSipMsgMgrHandle					hMsgMgr,
                                           IN  HRPOOL								hPool,
                                           IN  HPAGE								hPage,
                                           OUT RvSipPAccessNetworkInfoHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PAccessNetworkInfo header object to a destination PAccessNetworkInfo
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PAccessNetworkInfo header object.
 *    hSource      - Handle to the destination PAccessNetworkInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderCopy(
                                         INOUT RvSipPAccessNetworkInfoHeaderHandle hDestination,
                                         IN    RvSipPAccessNetworkInfoHeaderHandle hSource);


/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Access-Network-Info header object to a textual PAccessNetworkInfo header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Access-Network-Info header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderEncode(
                                          IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                          IN    HRPOOL								hPool,
                                          OUT   HPAGE*								hPage,
                                          OUT   RvUint32*							length);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAccessNetworkInfo header-for example,
 *          "PAccessNetworkInfo:IEEE-802.11a"-into a P-Access-Network-Info header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PAccessNetworkInfo header object.
 *    buffer    - Buffer containing a textual PAccessNetworkInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderParse(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                     IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAccessNetworkInfo header value into an PAccessNetworkInfo header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPAccessNetworkInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PAccessNetworkInfo header object.
 *    buffer    - The buffer containing a textual PAccessNetworkInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderParseValue(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                     IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PAccessNetworkInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderFix(
                                 IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                 IN RvChar*								pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PAccessNetworkInfo header fields are kept in a string format-for example, the
 *          P-Access-Network-Info header display name. In order to get such a field from the PAccessNetworkInfo header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PAccessNetworkInfo header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStringLength(
                                  IN  RvSipPAccessNetworkInfoHeaderHandle     hHeader,
                                  IN  RvSipPAccessNetworkInfoHeaderStringName stringName);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrAccessType
 * ------------------------------------------------------------------------
 * General: Copies the StrAccessType from the PAccessNetworkInfo header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrAccessType(
                                           IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                           IN RvChar*								strBuffer,
                                           IN RvUint								bufferLen,
                                           OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetAccessType
 * ------------------------------------------------------------------------
 * General:Sets the Access Type in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader		  - Handle to the header object.
 *	eAccessType	  -	enumeration for the access type.
 *	strAccessType - The Access Type to be set in the PAccessNetworkInfo header,
 *					when eAccessType == RVSIP_ACCESS_TYPE_OTHER.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetAccessType(
									IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
									IN	   RvSipPAccessNetworkInfoAccessType	eAccessType,
									IN    RvChar*								strAccessType);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetAccessType
 * ------------------------------------------------------------------------
 * General: Returns the enumeration value for the access type.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 * output:RvSipPAccessNetworkInfoAccessType - enumeration for the access type.
 ***************************************************************************/
RVAPI RvSipPAccessNetworkInfoAccessType RVCALLCONV RvSipPAccessNetworkInfoHeaderGetAccessType(
									IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetNetworkProvided
 * ------------------------------------------------------------------------
 * General: Returns value NetworkProvided parameter in the header.
 * Return Value: Returns NetworkProvided boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPAccessNetworkInfoHeaderGetNetworkProvided(
                                    IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetNetworkProvided
 * ------------------------------------------------------------------------
 * General:Sets the NetworkProvided parameter in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader          - Handle to the header object.
 *    bNetworkProvided - The NetworkProvided to be set in the PAccessNetworkInfo header. 
 *                       If RV_FALSE is supplied, the parameter is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetNetworkProvided(
                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
					 IN	   RvBool                               bNetworkProvided);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header Cgi3gpp field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrCgi3gpp(
                                       IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                       IN RvChar*								strBuffer,
                                       IN RvUint								bufferLen,
                                       OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General:Sets the Cgi3gpp field in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strCgi3gpp - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrCgi3gpp(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                     IN    RvChar *								pCgi3gpp);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General:Sets the UtranCellId3gpp field in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pUtranCellId3gpp);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header UtranCellId3gpp field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(
                                       IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                       IN RvChar*								strBuffer,
                                       IN RvUint								bufferLen,
                                       OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General:Sets the IWlanNodeID field in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strIWlanNodeID - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrIWlanNodeID(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pIWlanNodeID);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header IWlanNodeID field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrIWlanNodeID(
                                       IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                       IN RvChar*								strBuffer,
                                       IN RvUint								bufferLen,
                                       OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrDslLocation
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header DslLocation field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrDslLocation(
                                       IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                       IN RvChar*								strBuffer,
                                       IN RvUint								bufferLen,
                                       OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrDslLocation
 * ------------------------------------------------------------------------
 * General:Sets the DslLocation field in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strDslLocation - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrDslLocation(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pDslLocation);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header Ci3gpp2 field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrCi3gpp2(
                                               IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                               IN RvChar*								strBuffer,
                                               IN RvUint								bufferLen,
                                               OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General:Sets the Ci3gpp2 field in the PAccessNetworkInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    strCi3gpp2 - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrCi3gpp2(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								pCi3gpp2);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PAccessNetworkInfo header other params field of the PAccessNetworkInfo header object into a
 *          given buffer.
 *          Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAccessNetworkInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetOtherParams(
                                               IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                               IN RvChar*								strBuffer,
                                               IN RvUint								bufferLen,
                                               OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PAccessNetworkInfo header object.
 *         Not all the PAccessNetworkInfo header parameters have separated fields in the PAccessNetworkInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAccessNetworkInfo header object.
 *    OtherParams - The extended parameters field to be set in the PAccessNetworkInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetOtherParams(
                                 IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                 IN    RvChar *								strOtherParams);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PAccessNetworkInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderGetStrBadSyntax(
                                   IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                                   IN RvChar*								strBuffer,
                                   IN RvUint								bufferLen,
                                   OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAccessNetworkInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PAccessNetworkInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAccessNetworkInfoHeaderSetStrBadSyntax(
                              IN RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                              IN RvChar*								strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPACCESSNETWORKINFOHEADER_H */


