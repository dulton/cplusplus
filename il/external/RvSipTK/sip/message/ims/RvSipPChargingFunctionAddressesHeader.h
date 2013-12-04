 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                 RvSipPChargingFunctionAddressesHeader.h					  *
 *                                                                            *
 * The file defines the methods of the PChargingFunctionAddresses header      *
 * object:construct, destruct, copy, encode, parse and the ability to		  *
 * access and change it's parameters.                                         *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Nov.2005                                             *
 ******************************************************************************/
#ifndef RVSIPPCHARGINGFUNCTIONADDRESSESHEADER_H
#define RVSIPPCHARGINGFUNCTIONADDRESSESHEADER_H

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
 * RvSipPChargingFunctionAddressesHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PChargingFunctionAddresses header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_OTHER_PARAMS,
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_BAD_SYNTAX
}RvSipPChargingFunctionAddressesHeaderStringName;

/*
 * RvSipPChargingFunctionAddressesListElementStringName
 * ----------------------------
 * This enum defines all the Element's strings (for getting it's size)
 * Defines all PChargingFunctionAddressesListElement object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
	RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_ELEMENT_VALUE
}RvSipPChargingFunctionAddressesListElementStringName;
/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingFunctionAddresses header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PChargingFunctionAddresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructInMsg(
                           IN  RvSipMsgHandle								hSipMsg,
                           IN  RvBool										pushHeaderAtHead,
                           OUT RvSipPChargingFunctionAddressesHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingFunctionAddresses Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-Charging-Function-Addresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstruct(
                           IN  RvSipMsgMgrHandle							hMsgMgr,
                           IN  HRPOOL										hPool,
                           IN  HPAGE										hPage,
                           OUT RvSipPChargingFunctionAddressesHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PChargingFunctionAddresses header object to a 
 *          destination PChargingFunctionAddresses header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingFunctionAddresses header object.
 *    hSource      - Handle to the destination PChargingFunctionAddresses header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderCopy(
                                 INOUT RvSipPChargingFunctionAddressesHeaderHandle hDestination,
                                 IN    RvSipPChargingFunctionAddressesHeaderHandle hSource);


/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-Charging-Function-Addresses header object to a textual PChargingFunctionAddresses header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-Charging-Function-Addresses header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderEncode(
                          IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                          IN    HRPOOL										hPool,
                          OUT   HPAGE*										hPage,
                          OUT   RvUint32*									length);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingFunctionAddresses header-for example,
 *          "P-Charging-Function-Addresses: ccf=192.1.1.1"-into a P-Charging-Function-Addresses 
 *          header object. All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PChargingFunctionAddresses header object.
 *    buffer    - Buffer containing a textual PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderParse(
                             IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                             IN    RvChar*										buffer);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PChargingFunctionAddresses header value into an PChargingFunctionAddresses header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPChargingFunctionAddressesHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PChargingFunctionAddresses header object.
 *    buffer    - The buffer containing a textual PChargingFunctionAddresses header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderParseValue(
							 IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
							 IN    RvChar*										buffer);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PChargingFunctionAddresses header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderFix(
							 IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
							 IN RvChar*										pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingFunctionAddresses header fields are kept in a string format-for example, the
 *          P-Charging-Function-Addresses header ccf parameter. In order to get such a field from the PChargingFunctionAddresses header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PChargingFunctionAddresses header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetStringLength(
                          IN  RvSipPChargingFunctionAddressesHeaderHandle     hHeader,
                          IN  RvSipPChargingFunctionAddressesHeaderStringName stringName);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstructCcfList
 * ------------------------------------------------------------------------
 * General: Constructs a RvSipCommonList object on the header's page, into 
 *			the given handle. Also placing the list in the CcfList parameter
 *			of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader	- Handle to the header object.
 * output: hList	- Handle to the constructed list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructCcfList(
                                       IN	RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
									   OUT	RvSipCommonListHandle*						hList);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetCcfList
 * ------------------------------------------------------------------------
 * General: Returns the ccf list of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetCcfList(
							IN RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetCcfList
 * ------------------------------------------------------------------------
 * General: Sets the ccfList in the PChargingFunctionAddresses header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingFunctionAddressesHeaderConstructCcfList(),
 *			and than RvSipPChargingFunctionAddressesListElementConstructInHeader() to construct each element.
 ** Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PChargingFunctionAddresses header object.
 *    hList   - Handle to the ccfList object. If NULL is supplied, the existing
 *              ccfList is removed from the PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetCcfList(
                            IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                            IN    RvSipCommonListHandle							hList);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderConstructEcfList
 * ------------------------------------------------------------------------
 * General: Constructs a RvSipCommonList object on the header's page, into 
 *			the given handle. Also placing the list in the EcfList parameter
 *			of the header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader	- Handle to the header object.
 * output: hList	- Handle to the constructed list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderConstructEcfList(
                               IN	RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
							   OUT	RvSipCommonListHandle*				hList);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetEcfList
 * ------------------------------------------------------------------------
 * General: Returns the ecf list of the header.
 * Return Value: Returns RvSipCommonListHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipCommonListHandle RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetEcfList(
								IN RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetEcfList
 * ------------------------------------------------------------------------
 * General: Sets the ecfList in the PChargingFunctionAddresses header object.
 *			This function should be used when copying a list from another header, for example.
 *			If you want to create a new list, use RvSipPChargingFunctionAddressesHeaderConstructEcfList(),
 *			and than RvSipPChargingFunctionAddressesListElementConstructInHeader() to construct each element.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PChargingFunctionAddresses header object.
 *    hList - Handle to the ecfList object. If NULL is supplied, the existing
 *              ecfList is removed from the PChargingFunctionAddresses header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetEcfList(
                                IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                                IN    RvSipCommonListHandle							hList);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PChargingFunctionAddresses header other params field of the PChargingFunctionAddresses header object into a
 *          given buffer.
 *          Not all the PChargingFunctionAddresses header parameters have separated fields in the PChargingFunctionAddresses
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PChargingFunctionAddresses header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetOtherParams(
                           IN RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                           IN RvChar*										strBuffer,
                           IN RvUint										bufferLen,
                           OUT RvUint*										actualLen);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PChargingFunctionAddresses header object.
 *         Not all the PChargingFunctionAddresses header parameters have separated fields in the PChargingFunctionAddresses
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PChargingFunctionAddresses header object.
 *    OtherParams - The extended parameters field to be set in the PChargingFunctionAddresses header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetOtherParams(
                             IN    RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                             IN    RvChar *										strOtherParams);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PChargingFunctionAddresses header,
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
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderGetStrBadSyntax(
                           IN RvSipPChargingFunctionAddressesHeaderHandle	hHeader,
                           IN RvChar*										strBuffer,
                           IN RvUint										bufferLen,
                           OUT RvUint*										actualLen);

/***************************************************************************
 * RvSipPChargingFunctionAddressesHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PChargingFunctionAddresses header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesHeaderSetStrBadSyntax(
                              IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                              IN RvChar*									 strBadSyntax);


/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementConstructInHeader
 * ------------------------------------------------------------------------
 * General: Constructs a PChargingFunctionAddressesListElement object inside a given Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader			- Handle to the header object.
 * output: hElement			- Handle to the newly constructed PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementConstructInHeader(
                               IN	RvSipPChargingFunctionAddressesHeaderHandle		hHeader,
                               OUT	RvSipPChargingFunctionAddressesListElemHandle*	hElement);

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PChargingFunctionAddressesListElement object. The element is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          element object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hElement - Handle to the newly constructed PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementConstruct(
							   IN  RvSipMsgMgrHandle								hMsgMgr,
							   IN  HRPOOL											hPool,
							   IN  HPAGE											hPage,
							   OUT RvSipPChargingFunctionAddressesListElemHandle*	hElement);

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source PChargingFunctionAddressesListElement object to a 
 *		   destination PChargingFunctionAddressesListElement object.
 *         You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PChargingFunctionAddressesListElement object.
 *    hSource      - Handle to the source PChargingFunctionAddressesListElement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementCopy(
								INOUT	RvSipPChargingFunctionAddressesListElemHandle	hDestination,
								IN		RvSipPChargingFunctionAddressesListElemHandle	hSource);

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PChargingFunctionAddressesListElement fields are kept in a string
 *			format-for example, the value parameter. In order to get such a field from
 *			the element object, your application should supply an adequate buffer to 
 *			where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader	- Handle to the PChargingFunctionAddresses header object.
 *  stringName	- Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPChargingFunctionAddressesListElementGetStringLength(
                              IN  RvSipPChargingFunctionAddressesListElemHandle		hElement,
                              IN  RvSipPChargingFunctionAddressesListElementStringName stringName);

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementGetStrValue
 * ------------------------------------------------------------------------
 * General: Copies the PChargingFunctionAddressesListElement value field into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hElement	- Handle to the PChargingFunctionAddressesListElement object.
 *        strBuffer	- Buffer to fill with the requested parameter.
 *        bufferLen	- The length of the buffer.
 * output:actualLen	- The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementGetStrValue(
                               IN  RvSipPChargingFunctionAddressesListElemHandle	hElement,
                               IN  RvChar*											strBuffer,
                               IN  RvUint											bufferLen,
                               OUT RvUint*											actualLen);

/***************************************************************************
 * RvSipPChargingFunctionAddressesListElementSetStrValue
 * ------------------------------------------------------------------------
 * General:Sets the Value field in the PChargingFunctionAddressesListElement object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement	- Handle to the PChargingFunctionAddressesListElement object.
 *    pValue		- The value to be set in the PChargingFunctionAddressesListElement. If NULL is
 *				  supplied, the existing value field is removed from the element.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPChargingFunctionAddressesListElementSetStrValue(
                             IN    RvSipPChargingFunctionAddressesListElemHandle	hElement,
                             IN    RvChar *											pValue);

#endif /*#ifdef RV_SIP_IMS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPCHARGINGFUNCTIONADDRESSESHEADER_H */


