 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPDCSLAESHeader.h						  *
 *                                                                            *
 * The file defines the methods of the PDCSLAES header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jan.2006                                             *
 ******************************************************************************/
#ifndef RVSIPPDCSLAESHEADER_H
#define RVSIPPDCSLAESHEADER_H

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
 * RvSipPDCSLAESHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PDCSLAES header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
	RVSIP_P_DCS_LAES_SIG_HOST,
	RVSIP_P_DCS_LAES_CONTENT_HOST,
	RVSIP_P_DCS_LAES_KEY,
    RVSIP_P_DCS_LAES_OTHER_PARAMS,
    RVSIP_P_DCS_LAES_BAD_SYNTAX
}RvSipPDCSLAESHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSLAESHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSLAES header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSLAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderConstructInMsg(
                                       IN  RvSipMsgHandle				hSipMsg,
                                       IN  RvBool						pushHeaderAtHead,
                                       OUT RvSipPDCSLAESHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSLAESHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSLAES Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-DCS-LAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderConstruct(
                                           IN  RvSipMsgMgrHandle          hMsgMgr,
                                           IN  HRPOOL                     hPool,
                                           IN  HPAGE                      hPage,
                                           OUT RvSipPDCSLAESHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPDCSLAESHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSLAES header object to a destination PDCSLAES
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSLAES header object.
 *    hSource      - Handle to the destination PDCSLAES header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderCopy(
                                         INOUT RvSipPDCSLAESHeaderHandle hDestination,
                                         IN    RvSipPDCSLAESHeaderHandle hSource);


/***************************************************************************
 * RvSipPDCSLAESHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-DCS-LAES header object to a textual PDCSLAES header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-DCS-LAES header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderEncode(
                                          IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                          IN    HRPOOL						hPool,
                                          OUT   HPAGE*						hPage,
                                          OUT   RvUint32*					length);

/***************************************************************************
 * RvSipPDCSLAESHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSLAES header-for example,
 *          "P-DCS-LAES: radvision.com:5060"-into a P-DCS-LAES header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSLAES header object.
 *    buffer    - Buffer containing a textual PDCSLAES header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderParse(
                                     IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                     IN    RvChar*						buffer);

/***************************************************************************
 * RvSipPDCSLAESHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSLAES header value into an PDCSLAES header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSLAESHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSLAES header object.
 *    buffer    - The buffer containing a textual PDCSLAES header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderParseValue(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar*					 buffer);

/***************************************************************************
 * RvSipPDCSLAESHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSLAES header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderFix(
                                     IN RvSipPDCSLAESHeaderHandle	hHeader,
                                     IN RvChar*						pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPDCSLAESHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSLAES header fields are kept in a string format-for example, the
 *          P-DCS-LAES header LaesSigHost. In order to get such a field from the PDCSLAES header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSLAES header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSLAESHeaderGetStringLength(
                                      IN  RvSipPDCSLAESHeaderHandle     hHeader,
                                      IN  RvSipPDCSLAESHeaderStringName stringName);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesSigHost field of the PDCSLAES header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesSigHost(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General:Sets the LaesSigHost field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	 - Handle to the PDCSLAES header object.
 *    strLaesSig - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *				   supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesSigHost(
                                 IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                 IN    RvChar *						pLaesSig);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetLaesSigPort
 * ------------------------------------------------------------------------
 * General: Gets the LaesSigPort parameter from the PDCSLAES header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSLAESHeaderGetLaesSigPort(
                                         IN RvSipPDCSLAESHeaderHandle hPDCSLAESHeader);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetLaesSigPort
 * ------------------------------------------------------------------------
 * General:  Sets the LaesSigPort parameter of the PDCSLAES header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 *    portNum         - The port number value to be set in the object.
 *						An UNDEFINED value would clear the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetLaesSigPort(
                                         IN    RvSipPDCSLAESHeaderHandle hPDCSLAESHeader,
                                         IN    RvInt32					 portNum);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesContentHost field of the PDCSLAES
 *			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesContentHost(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General:Sets the LaesContentHost field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesContentHost(
                                     IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                     IN    RvChar*						pLaesContentHost);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetLaesContentPort
 * ------------------------------------------------------------------------
 * General: Gets LaesContentPort parameter from the PDCSLAES header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPDCSLAESHeaderGetLaesContentPort(
									IN RvSipPDCSLAESHeaderHandle hPDCSLAESHeader);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetLaesContentPort
 * ------------------------------------------------------------------------
 * General:  Sets LaesContentPort parameter of the PDCSLAES header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hPDCSLAESHeader - Handle to the PDCSLAES header object.
 *    portNum         - The port number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetLaesContentPort(
                                     IN    RvSipPDCSLAESHeaderHandle hPDCSLAESHeader,
                                     IN    RvInt32					 portNum);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrLaesKey
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header LaesSig field of the PDCSLAES header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrLaesKey(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrLaesKey
 * ------------------------------------------------------------------------
 * General:Sets the LaesKey field in the PDCSLAES header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    strLaesSig - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrLaesKey(
                                     IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                     IN    RvChar*						pLaesKey);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSLAES header other params field of the PDCSLAES header object into a
 *          given buffer.
 *          Not all the PDCSLAES header parameters have separated fields in the PDCSLAES
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSLAES header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetOtherParams(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSLAES header object.
 *         Not all the PDCSLAES header parameters have separated fields in the PDCSLAES
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSLAES header object.
 *    OtherParams - The extended parameters field to be set in the PDCSLAES header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetOtherParams(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *					 strOtherParams);

/***************************************************************************
 * RvSipPDCSLAESHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSLAES header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderGetStrBadSyntax(
                                               IN RvSipPDCSLAESHeaderHandle hHeader,
                                               IN RvChar*					strBuffer,
                                               IN RvUint					bufferLen,
                                               OUT RvUint*					actualLen);

/***************************************************************************
 * RvSipPDCSLAESHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSLAES header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSLAESHeaderSetStrBadSyntax(
                                  IN RvSipPDCSLAESHeaderHandle	hHeader,
                                  IN RvChar*					strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPDCSLAESHEADER_H */


