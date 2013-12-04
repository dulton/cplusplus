 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPDCSOSPSHeader.h						  *
 *                                                                            *
 * The file defines the methods of the PDCSOSPS header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jan.2006                                             *
 ******************************************************************************/
#ifndef RVSIPPDCSOSPSHEADER_H
#define RVSIPPDCSOSPSHEADER_H

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
 * RvSipPDCSOSPSHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PDCSOSPS header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_DCS_OSPS_TAG,
    RVSIP_P_DCS_OSPS_BAD_SYNTAX
}RvSipPDCSOSPSHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSOSPSHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSOSPS header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderConstructInMsg(
                               IN  RvSipMsgHandle				hSipMsg,
                               IN  RvBool						pushHeaderAtHead,
                               OUT RvSipPDCSOSPSHeaderHandle*	hHeader);

/***************************************************************************
 * RvSipPDCSOSPSHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSOSPS Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderConstruct(
                                   IN  RvSipMsgMgrHandle          hMsgMgr,
                                   IN  HRPOOL                     hPool,
                                   IN  HPAGE                      hPage,
                                   OUT RvSipPDCSOSPSHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPDCSOSPSHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSOSPS header object to a destination PDCSOSPS
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSOSPS header object.
 *    hSource      - Handle to the destination PDCSOSPS header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderCopy(
                                 INOUT	RvSipPDCSOSPSHeaderHandle hDestination,
                                 IN		RvSipPDCSOSPSHeaderHandle hSource);


/***************************************************************************
 * RvSipPDCSOSPSHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a PDCSOSPS header object to a textual PDCSOSPS header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the PDCSOSPS header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderEncode(
                              IN    RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN    HRPOOL						hPool,
                              OUT   HPAGE*						hPage,
                              OUT   RvUint32*					length);

/***************************************************************************
 * RvSipPDCSOSPSHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSOSPS header-for example,
 *          "P-DCS-OSPS: BLV"-into a PDCSOSPS header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSOSPS header object.
 *    buffer    - Buffer containing a textual PDCSOSPS header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderParse(
							 IN    RvSipPDCSOSPSHeaderHandle hHeader,
							 IN    RvChar*					 buffer);

/***************************************************************************
 * RvSipPDCSOSPSHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSOSPS header value into an PDCSOSPS header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSOSPSHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSOSPS header object.
 *    buffer    - The buffer containing a textual PDCSOSPS header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderParseValue(
                                 IN    RvSipPDCSOSPSHeaderHandle hHeader,
                                 IN    RvChar*					 buffer);

/***************************************************************************
 * RvSipPDCSOSPSHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSOSPS header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderFix(
                             IN RvSipPDCSOSPSHeaderHandle	hHeader,
                             IN RvChar*						pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSOSPS header fields are kept in a string format-for example, the
 *          PDCSOSPS header OSPSTag name. In order to get such a field from the PDCSOSPS header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSOSPS header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSOSPSHeaderGetStringLength(
                              IN  RvSipPDCSOSPSHeaderHandle     hHeader,
                              IN  RvSipPDCSOSPSHeaderStringName stringName);

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStrOSPSTag
 * ------------------------------------------------------------------------
 * General: Copies the StrOSPSTag from the PDCSOSPS header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderGetStrOSPSTag(
                                               IN RvSipPDCSOSPSHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPDCSOSPSHeaderSetOSPSTag
 * ------------------------------------------------------------------------
 * General:Sets the OSPSTag Type in the PDCSOSPS header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader		- Handle to the header object.
 *	eOSPSTag	- Enumeration for the type of the OSPSTag.
 *	strOSPSTag	- The OSPSTag Type to be set in the PDCSOSPS header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderSetOSPSTag(
                             IN    RvSipPDCSOSPSHeaderHandle	hHeader,
							 IN	   RvSipOSPSTagType				eOSPSTag,
                             IN    RvChar*						strOSPSTag);

/***************************************************************************
 * RvSipPDCSOSPSHeaderGetOSPSTag
 * ------------------------------------------------------------------------
 * General: Returns the RvSipOSPSTagType enumeration value for the OSPSTag of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvSipOSPSTagType RVCALLCONV RvSipPDCSOSPSHeaderGetOSPSTag(
									IN RvSipPDCSOSPSHeaderHandle hHeader);


/***************************************************************************
 * RvSipPDCSOSPSHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSOSPS header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderGetStrBadSyntax(
                                       IN RvSipPDCSOSPSHeaderHandle hHeader,
                                       IN RvChar*                 strBuffer,
                                       IN RvUint                  bufferLen,
                                       OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPDCSOSPSHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSOSPS header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSOSPSHeaderSetStrBadSyntax(
                              IN RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN RvChar*					strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPDCSOSPSHEADER_H */


