 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipPChargingVectorHeader.h                 *
 *                                                                            *
 * The file defines the methods of the PChargingVector header object:       *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/
#ifndef RVSIPPCHARGINGVECTORHEADER_H
#define RVSIPPCHARGINGVECTORHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipMsgTypes.h"
#include "RvSipCommonList.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipPChargingVectorHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PChargingVector header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
	RVSIP_P_CHARGING_VECTOR_ICID_VALUE,
	RVSIP_P_CHARGING_VECTOR_ICID_GEN_ADDR,
	RVSIP_P_CHARGING_VECTOR_ORIG_IOI,
	RVSIP_P_CHARGING_VECTOR_TERM_IOI,
	RVSIP_P_CHARGING_VECTOR_GGSN,
	RVSIP_P_CHARGING_VECTOR_GPRS_AUTH_TOKEN,
	RVSIP_P_CHARGING_VECTOR_BRAS,
	RVSIP_P_CHARGING_VECTOR_XDSL_AUTH_TOKEN,
	RVSIP_P_CHARGING_VECTOR_BCID,
    RVSIP_P_CHARGING_VECTOR_OTHER_PARAMS,
    RVSIP_P_CHARGING_VECTOR_BAD_SYNTAX
}RvSipPChargingVectorHeaderStringName;

/*
 * RvSipPChargingVectorInfoListElementStringName
 * ----------------------------
 * This enum defines all the Element's strings (for getting it's size)
 * Defines all PChargingVectorInfoListElement object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
	RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_CID,
	RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_FLOW_ID
}RvSipPChargingVectorInfoListElementStringName;
/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingVectorHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingVector header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PChargingVector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipPChargingVectorHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingVector Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Charging-Vector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipPChargingVectorHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PChargingVector header object to a destination PChargingVector
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingVector header object.
 *    hSource      - Handle to the destination PChargingVector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderCopy(
                                         INOUT RvSipPChargingVectorHeaderHandle hDestination,
                                         IN    RvSipPChargingVectorHeaderHandle hSource);


/***************************************************************************
 * RvSipPChargingVectorHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Charging-Vector header object to a textual PChargingVector header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Charging-Vector header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderEncode(
                                          IN    RvSipPChargingVectorHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length);

/***************************************************************************
 * RvSipPChargingVectorHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingVector header-for example,
 *          "PChargingVector:sip:172.20.5.3:5060"-into a P-Charging-Vector header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PChargingVector header object.
 *    buffer    - Buffer containing a textual PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderParse(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipPChargingVectorHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingVector header value into an PChargingVector header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPChargingVectorHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PChargingVector header object.
 *    buffer    - The buffer containing a textual PChargingVector header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderParseValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar*                 buffer);

/***************************************************************************
 * RvSipPChargingVectorHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PChargingVector header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderFix(
                                     IN RvSipPChargingVectorHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPChargingVectorHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingVector header fields are kept in a string format-for example, the
 *          P-Charging-Vector header display name. In order to get such a field from the PChargingVector header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PChargingVector header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingVectorHeaderGetStringLength(
                                      IN  RvSipPChargingVectorHeaderHandle     hHeader,
                                      IN  RvSipPChargingVectorHeaderStringName stringName);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrIcidValue
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header IcidValue field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrIcidValue(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrIcidValue
 * ------------------------------------------------------------------------
 * General:Sets the IcidValue field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strIcidValue - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrIcidValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidValue);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header IcidGenAddr field of the PChargingVector
			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrIcidGenAddr(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General:Sets the IcidGenAddr field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrIcidGenAddr(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidGenAddr);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrOrigIoi
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header IcidValue field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrOrigIoi(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrOrigIoi
 * ------------------------------------------------------------------------
 * General:Sets the OrigIoi field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strIcidValue - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrOrigIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOrigIoi);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrTermIoi
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header TermIoi field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrTermIoi(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrTermIoi
 * ------------------------------------------------------------------------
 * General:Sets the IcidValue field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    strTermIoi - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrTermIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pTermIoi);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrGgsn
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header Ggsn field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrGgsn(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrGgsn
 * ------------------------------------------------------------------------
 * General:Sets the Ggsn field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGgsn	- The Ggsn field to be set in the PChargingVector header. If NULL is
 *					supplied, the existing Ggsn field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrGgsn(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGgsn);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header GprsAuthToken field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrGprsAuthToken(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General:Sets the GprsAuthToken field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGprsAuthToken - The GprsAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the GprsAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrGprsAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGprsAuthToken);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBras
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header Bras field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBras(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrBras
 * ------------------------------------------------------------------------
 * General:Sets the Bras field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strGprsAuthToken - The GprsAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the GprsAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBras(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pBras);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header XdslAuthToken field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrXdslAuthToken(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General:Sets the XdslAuthToken field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    strXdslAuthToken - The XdslAuthToken field to be set in the PChargingVector header. If NULL is
 *                    supplied, the XdslAuthToken parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrXdslAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pXdslAuthToken);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetIWlanChargingInfo
 * ------------------------------------------------------------------------
 * General: Returns value IWlanChargingInfo parameter in the header.
 * Return Value: Returns IWlanChargingInfo boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorHeaderGetIWlanChargingInfo(
                                               IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetIWlanChargingInfo
 * ------------------------------------------------------------------------
 * General:Sets the IWlanChargingInfo parameter in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    bIWlanChargingInfo - The IWlanChargingInfo to be set in the PChargingVector header. If RV_FALSE is supplied, the
 *                 parameter ("pdg") is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetIWlanChargingInfo(
                     IN    RvSipPChargingVectorHeaderHandle	hHeader,
					 IN	   RvBool							bIWlanChargingInfoType);


/***************************************************************************
 * RvSipPChargingVectorHeaderGetPacketcableChargingInfo
 * ------------------------------------------------------------------------
 * General: Returns value PacketcableChargingInfo parameter in the header.
 * Return Value: Returns PacketcableChargingInfo boolean Parameter.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorHeaderGetPacketcableChargingInfo(
										IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetPacketcableChargingInfo
 * ------------------------------------------------------------------------
 * General:Sets the PacketcableChargingInfo parameter in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    bPacketcableChargingInfo - The PacketcableChargingInfo to be set in the PChargingVector header. If RV_FALSE is supplied, the
 *                 parameter ("packetcable-multimedia") is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetPacketcableChargingInfo(
                     IN    RvSipPChargingVectorHeaderHandle	hHeader,
					 IN	   RvBool							bPacketcableChargingInfoType);
					 
/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBCid
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header BCid field of the PChargingVector header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBCid(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrBCid
 * ------------------------------------------------------------------------
 * General:Sets the BCid field in the PChargingVector header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *    pBCid		- The BCid field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing BCid field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBCid(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pBCid);
											   
/***************************************************************************
 * RvSipPChargingVectorHeaderConstructPdpInfoList
 * ------------------------------------------------------------------------
 * General: Returns the PdpInfoList of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructPdpInfoList(
                                       IN	RvSipPChargingVectorHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*				hList);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetPdpInfoList
 * ------------------------------------------------------------------------
 * General: Returns the PdpInfoList of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingVectorHeaderGetPdpInfoList(
                                               IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetPdpInfoList
 * ------------------------------------------------------------------------
 * General: Sets the PdpInfoList in the PChargingVector header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingVectorHeaderConstructPdpInfoList(),
 *			and than RvSipPChargingVectorInfoListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PChargingVector header object.
 *    hList - Handle to the PdpInfoList object. If NULL is supplied, the existing
 *              PdpInfoList is removed from the PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetPdpInfoList(
                                    IN    RvSipPChargingVectorHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle				hList);

/***************************************************************************
 * RvSipPChargingVectorHeaderConstructDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Returns the DslBearerInfoList of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderConstructDslBearerInfoList(
                                       IN	RvSipPChargingVectorHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*				hList);
									   
/***************************************************************************
 * RvSipPChargingVectorHeaderGetDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Returns the dslBearerInfoList of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingVectorHeaderGetDslBearerInfoList(
                                               IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetDslBearerInfoList
 * ------------------------------------------------------------------------
 * General: Sets the DslBearerInfoList in the PChargingVector header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingVectorHeaderConstructDslBearerInfoList(),
 *			and than RvSipPChargingVectorInfoListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PChargingVector header object.
 *    hList	  - Handle to the DslBearerInfoList object. If NULL is supplied, the existing
 *              DslBearerInfoList is removed from the PChargingVector header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetDslBearerInfoList(
                                    IN    RvSipPChargingVectorHeaderHandle	hHeader,
                                    IN    RvSipCommonListHandle				hList);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVector header other params field of the PChargingVector header object into a
 *          given buffer.
 *          Not all the PChargingVector header parameters have separated fields in the PChargingVector
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingVector header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetOtherParams(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PChargingVector header object.
 *         Not all the PChargingVector header parameters have separated fields in the PChargingVector
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingVector header object.
 *    OtherParams - The extended parameters field to be set in the PChargingVector header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetOtherParams(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                strOtherParams);

/***************************************************************************
 * RvSipPChargingVectorHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PChargingVector header,
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
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderGetStrBadSyntax(
                                               IN RvSipPChargingVectorHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen);

/***************************************************************************
 * RvSipPChargingVectorHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PChargingVector header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorHeaderSetStrBadSyntax(
                                  IN RvSipPChargingVectorHeaderHandle	hHeader,
                                  IN RvChar*							strBadSyntax);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementConstructInHeader
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingVectorInfoListElement object inside a given Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader			- Handle to the header object.
 * output: hElement			- Handle to the newly constructed PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementConstructInHeader(
                               IN	RvSipPChargingVectorHeaderHandle		hHeader,
                               OUT	RvSipPChargingVectorInfoListElemHandle*	hElement);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingVectorInfoListElement object. The element is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          element object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Charging-Vector header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementConstruct(
							   IN  RvSipMsgMgrHandle						hMsgMgr,
							   IN  HRPOOL									hPool,
							   IN  HPAGE									hPage,
							   OUT RvSipPChargingVectorInfoListElemHandle*	hElement);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source PChargingVectorInfoListElement object to a 
 *		   destination PChargingVectorInfoListElement object.
 *         You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingVectorInfoListElement object.
 *    hSource      - Handle to the source PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementCopy(
								INOUT	RvSipPChargingVectorInfoListElemHandle	hDestination,
								IN		RvSipPChargingVectorInfoListElemHandle	hSource);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingVectorInfoListElement fields are kept in a string
 *			format-for example, the cid parameter. In order to get such a field from
 *			the element object, your application should supply an adequate buffer to 
 *			where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingVector header object.
 *  stringName	- Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingVectorInfoListElementGetStringLength(
                              IN  RvSipPChargingVectorInfoListElemHandle		hElement,
                              IN  RvSipPChargingVectorInfoListElementStringName stringName);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStrCid
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVectorInfoListElement cid field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingVectorInfoListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementGetStrCid(
                               IN  RvSipPChargingVectorInfoListElemHandle	hElement,
                               IN  RvChar*									strBuffer,
                               IN  RvUint									bufferLen,
                               OUT RvUint*									actualLen);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetStrCid
 * ------------------------------------------------------------------------
 * General:Sets the Cid field in the PChargingVectorInfoListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    pCid		- The cid to be set in the PChargingVectorInfoListElement. If NULL is
 *				  supplied, the existing cid field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetStrCid(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pCid);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetStrFlowID
 * ------------------------------------------------------------------------
 * General: Copies the PChargingVectorInfoListElement FlowID field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingVectorInfoListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementGetStrFlowID(
                               IN  RvSipPChargingVectorInfoListElemHandle	hElement,
                               IN  RvChar*									strBuffer,
                               IN  RvUint									bufferLen,
                               OUT RvUint*									actualLen);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetStrFlowID
 * ------------------------------------------------------------------------
 * General:Sets the FlowID field in the PChargingVectorInfoListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    pFlowID	- The FlowID to be set in the PChargingVectorInfoListElement. If NULL is
 *				  supplied, the existing FlowID field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetStrFlowID(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pFlowID);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetItem
 * ------------------------------------------------------------------------
 * General: Gets Item parameter from the PChargingVectorInfoListElement object.
 * Return Value: Returns the Item number, or UNDEFINED if the Item number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle to the PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipPChargingVectorInfoListElementGetItem(
                                         IN RvSipPChargingVectorInfoListElemHandle hElement);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetItem
 * ------------------------------------------------------------------------
 * General:  Sets Item parameter of the PChargingVectorInfoListElement object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    item		- The item number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetItem(
                                 IN    RvSipPChargingVectorInfoListElemHandle hElement,
                                 IN    RvInt32								  item);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementGetSig
 * ------------------------------------------------------------------------
 * General: Gets Sig parameter from the PChargingVectorInfoListElement object.
 * Return Value: Returns the Sig bool.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle to the PChargingVectorInfoListElement object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipPChargingVectorInfoListElementGetSig(
                                         IN RvSipPChargingVectorInfoListElemHandle hElement);

/***************************************************************************
 * RvSipPChargingVectorInfoListElementSetSig
 * ------------------------------------------------------------------------
 * General:  Sets Sig parameter of the PChargingVectorInfoListElement object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingVectorInfoListElement object.
 *    sig		- The item number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingVectorInfoListElementSetSig(
                                 IN    RvSipPChargingVectorInfoListElemHandle hElement,
                                 IN    RvBool								  sig);

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPCHARGINGVECTORHEADER_H */


