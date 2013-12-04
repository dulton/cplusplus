 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *					RvSipPAnswerStateHeader.h						    	  *
 *                                                                            *
 * The file defines the methods of the PAnswerState header object:            *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jul.2007                                             *
 ******************************************************************************/
#ifndef RVSIPPANSWERSTATEHEADER_H
#define RVSIPPANSWERSTATEHEADER_H

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
 * RvSipPAnswerStateHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PAnswerState header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_ANSWER_STATE_ANSWER_TYPE,
    RVSIP_P_ANSWER_STATE_OTHER_PARAMS,
    RVSIP_P_ANSWER_STATE_BAD_SYNTAX
}RvSipPAnswerStateHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPAnswerStateHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PAnswerState header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PAnswerState header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderConstructInMsg(
                                       IN  RvSipMsgHandle						hSipMsg,
                                       IN  RvBool								pushHeaderAtHead,
                                       OUT RvSipPAnswerStateHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPAnswerStateHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PAnswerState Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Answer-State header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderConstruct(
                                           IN  RvSipMsgMgrHandle					hMsgMgr,
                                           IN  HRPOOL								hPool,
                                           IN  HPAGE								hPage,
                                           OUT RvSipPAnswerStateHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPAnswerStateHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PAnswerState header object to a destination PAnswerState
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PAnswerState header object.
 *    hSource      - Handle to the destination PAnswerState header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderCopy(
                                         INOUT RvSipPAnswerStateHeaderHandle hDestination,
                                         IN    RvSipPAnswerStateHeaderHandle hSource);

/***************************************************************************
 * RvSipPAnswerStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Answer-State header object to a textual PAnswerState header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Answer-State header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderEncode(
                                          IN    RvSipPAnswerStateHeaderHandle hHeader,
                                          IN    HRPOOL								hPool,
                                          OUT   HPAGE*								hPage,
                                          OUT   RvUint32*							length);

/***************************************************************************
 * RvSipPAnswerStateHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAnswerState header-for example,
 *          "PAnswerState:IEEE-802.11a"-into a P-Answer-State header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PAnswerState header object.
 *    buffer    - Buffer containing a textual PAnswerState header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderParse(
                                     IN    RvSipPAnswerStateHeaderHandle	hHeader,
                                     IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPAnswerStateHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PAnswerState header value into an PAnswerState header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPAnswerStateHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PAnswerState header object.
 *    buffer    - The buffer containing a textual PAnswerState header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderParseValue(
                                     IN    RvSipPAnswerStateHeaderHandle	hHeader,
                                     IN    RvChar*								buffer);

/***************************************************************************
 * RvSipPAnswerStateHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PAnswerState header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderFix(
                                 IN RvSipPAnswerStateHeaderHandle hHeader,
                                 IN RvChar*								pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPAnswerStateHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PAnswerState header fields are kept in a string format-for example, the
 *          P-Answer-State header display name. In order to get such a field from the PAnswerState header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PAnswerState header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPAnswerStateHeaderGetStringLength(
                                  IN  RvSipPAnswerStateHeaderHandle     hHeader,
                                  IN  RvSipPAnswerStateHeaderStringName stringName);

/***************************************************************************
 * RvSipPAnswerStateHeaderGetStrAnswerType
 * ------------------------------------------------------------------------
 * General: Copies the StrAnswerType from the PAnswerState header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetStrAnswerType(
                                           IN RvSipPAnswerStateHeaderHandle	hHeader,
                                           IN RvChar*								strBuffer,
                                           IN RvUint								bufferLen,
                                           OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAnswerStateHeaderSetAnswerType
 * ------------------------------------------------------------------------
 * General:Sets the Answer Type in the PAnswerState header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader		  - Handle to the header object.
 *	eAnswerType	  -	enumeration for the access type.
 *	strAnswerType - The Answer Type to be set in the PAnswerState header,
 *					when eAnswerType == RVSIP_ANSWER_TYPE_OTHER.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetAnswerType(
									IN    RvSipPAnswerStateHeaderHandle	hHeader,
									IN	   RvSipPAnswerStateAnswerType	eAnswerType,
									IN    RvChar*								strAnswerType);

/***************************************************************************
 * RvSipPAnswerStateHeaderGetAnswerType
 * ------------------------------------------------------------------------
 * General: Returns the enumeration value for the access type.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 * output:RvSipPAnswerStateAnswerType - enumeration for the access type.
 ***************************************************************************/
RVAPI RvSipPAnswerStateAnswerType RVCALLCONV RvSipPAnswerStateHeaderGetAnswerType(
									IN RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * RvSipPAnswerStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PAnswerState header other params field of the PAnswerState header object into a
 *          given buffer.
 *          Not all the PAnswerState header parameters have separated fields in the PAnswerState
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PAnswerState header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetOtherParams(
                                               IN RvSipPAnswerStateHeaderHandle	hHeader,
                                               IN RvChar*								strBuffer,
                                               IN RvUint								bufferLen,
                                               OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAnswerStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PAnswerState header object.
 *         Not all the PAnswerState header parameters have separated fields in the PAnswerState
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PAnswerState header object.
 *    OtherParams - The extended parameters field to be set in the PAnswerState header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetOtherParams(
                                 IN    RvSipPAnswerStateHeaderHandle	hHeader,
                                 IN    RvChar *								strOtherParams);

/***************************************************************************
 * RvSipPAnswerStateHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PAnswerState header,
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
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderGetStrBadSyntax(
                                   IN RvSipPAnswerStateHeaderHandle	hHeader,
                                   IN RvChar*								strBuffer,
                                   IN RvUint								bufferLen,
                                   OUT RvUint*								actualLen);

/***************************************************************************
 * RvSipPAnswerStateHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PAnswerState header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPAnswerStateHeaderSetStrBadSyntax(
                              IN RvSipPAnswerStateHeaderHandle	hHeader,
                              IN RvChar*								strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPANSWERSTATEHEADER_H */


