 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *					RvSipPDCSBillingInfoHeader.h			                  *
 *                                                                            *
 * The file defines the methods of the PDCSBillingInfo header object:         *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Jan.2006                                             *
 ******************************************************************************/
#ifndef RVSIPPDCSBILLINGINFOHEADER_H
#define RVSIPPDCSBILLINGINFOHEADER_H

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
 * RvSipPDCSBillingInfoHeaderStringName
 * ----------------------------
 * This enum defines all the header's strings (for getting it's size)
 * Defines all PDCSBillingInfo header object fields that are kept in the
 * object in a string format.
 */
typedef enum
{
	RVSIP_P_DCS_BILLING_INFO_BILLING_CORRELATION_ID,
	RVSIP_P_DCS_BILLING_INFO_FEID,
	RVSIP_P_DCS_BILLING_INFO_FEID_HOST,
	RVSIP_P_DCS_BILLING_INFO_RKS_GROUP_ID,
    RVSIP_P_DCS_BILLING_INFO_OTHER_PARAMS,
    RVSIP_P_DCS_BILLING_INFO_BAD_SYNTAX
}RvSipPDCSBillingInfoHeaderStringName;


/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a PDCSBillingInfo header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed PDCSBillingInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderConstructInMsg(
                                   IN  RvSipMsgHandle					 hSipMsg,
                                   IN  RvBool							 pushHeaderAtHead,
                                   OUT RvSipPDCSBillingInfoHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone PDCSBillingInfo Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed P-DCS-Billing-Info header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderConstruct(
                                   IN  RvSipMsgMgrHandle				 hMsgMgr,
                                   IN  HRPOOL							 hPool,
                                   IN  HPAGE							 hPage,
                                   OUT RvSipPDCSBillingInfoHeaderHandle* hHeader);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source PDCSBillingInfo header object to a destination PDCSBillingInfo
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination PDCSBillingInfo header object.
 *    hSource      - Handle to the destination PDCSBillingInfo header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderCopy(
                                 INOUT RvSipPDCSBillingInfoHeaderHandle hDestination,
                                 IN    RvSipPDCSBillingInfoHeaderHandle hSource);


/***************************************************************************
 * RvSipPDCSBillingInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a P-DCS-Billing-Info header object to a textual PDCSBillingInfo header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the P-DCS-Billing-Info header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderEncode(
                                  IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                  IN    HRPOOL							 hPool,
                                  OUT   HPAGE*							 hPage,
                                  OUT   RvUint32*						 length);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSBillingInfo header-for example,
 *          "P-DCS-Billing-Info:1234567890abcdef1234567890abcdef1234567890abcdef/1234567890abcdef@radvision.com; rksgroup=radvision;charge="sip:bob@radvision.com""
 *			-into a P-DCS-Billing-Info header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the PDCSBillingInfo header object.
 *    buffer    - Buffer containing a textual PDCSBillingInfo header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderParse(
                                 IN		RvSipPDCSBillingInfoHeaderHandle hHeader,
                                 IN		RvChar*							 buffer);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual PDCSBillingInfo header value into an PDCSBillingInfo header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipPDCSBillingInfoHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the PDCSBillingInfo header object.
 *    buffer    - The buffer containing a textual PDCSBillingInfo header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderParseValue(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar*							buffer);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an PDCSBillingInfo header with bad-syntax information.
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
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderFix(
                             IN RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN RvChar*							 pFixedBuffer);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the PDCSBillingInfo header fields are kept in a string format-for example, the
 *          P-DCS-Billing-Info header Billing-Correlation-ID. In order to get such a field from the PDCSBillingInfo header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the PDCSBillingInfo header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipPDCSBillingInfoHeaderGetStringLength(
                              IN  RvSipPDCSBillingInfoHeaderHandle     hHeader,
                              IN  RvSipPDCSBillingInfoHeaderStringName stringName);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header BillingCorrelationID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrBillingCorrelationID(
                                   IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                                   IN RvChar*							strBuffer,
                                   IN RvUint							bufferLen,
                                   OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General:Sets the BillingCorrelationID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strBillingCorrelationID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrBillingCorrelationID(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pBillingCorrelationID);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrFEID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header FEID field of the PDCSBillingInfo
			header object into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrFEID(
                                   IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                                   IN RvChar*							strBuffer,
                                   IN RvUint							bufferLen,
                                   OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrFEID
 * ------------------------------------------------------------------------
 * General:Sets the FEID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strUtranCellId3gpp - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrFEID(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pFEID);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrFEIDHost
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header BillingCorrelationID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrFEIDHost(
                                   IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                                   IN RvChar*							strBuffer,
                                   IN RvUint							bufferLen,
                                   OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrFEIDHost
 * ------------------------------------------------------------------------
 * General:Sets the FEIDHost field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strBillingCorrelationID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrFEIDHost(
                                 IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                 IN    RvChar *							pFEIDHost);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header RKSGroupID field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrRKSGroupID(
						   IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
						   IN RvChar*							strBuffer,
						   IN RvUint							bufferLen,
						   OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General:Sets the BillingCorrelationID field in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    strRKSGroupID - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrRKSGroupID(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pRKSGroupID);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the PDCSBillingInfo header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the PDCSBillingInfo header object.
 *	  ePDCSBillingInfoAddrType - Enumeration for the field from which to get the address.	
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipPDCSBillingInfoHeaderGetAddrSpec(
                        IN RvSipPDCSBillingInfoHeaderHandle hHeader,
						IN RvSipPDCSBillingInfoAddressType	ePDCSBillingInfoAddrType);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the PDCSBillingInfo header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the PDCSBillingInfo header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *               Address Spec is removed from the PDCSBillingInfo header.
 *	  ePDCSBillingInfoAddrType - Enumeration for the field in which to set the address.	
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetAddrSpec(
                        IN	RvSipPDCSBillingInfoHeaderHandle	hHeader,
                        IN	RvSipAddressHandle					hAddrSpec,
						IN	RvSipPDCSBillingInfoAddressType		ePDCSBillingInfoAddrType);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the PDCSBillingInfo header other params field of the PDCSBillingInfo header object into a
 *          given buffer.
 *          Not all the PDCSBillingInfo header parameters have separated fields in the PDCSBillingInfo
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the PDCSBillingInfo header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetOtherParams(
                               IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                               IN RvChar*							strBuffer,
                               IN RvUint							bufferLen,
                               OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the PDCSBillingInfo header object.
 *         Not all the PDCSBillingInfo header parameters have separated fields in the PDCSBillingInfo
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the PDCSBillingInfo header object.
 *    OtherParams - The extended parameters field to be set in the PDCSBillingInfo header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetOtherParams(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							strOtherParams);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderGetStrBadSyntax
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
 *          implementation if the message contains a bad PDCSBillingInfo header,
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
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderGetStrBadSyntax(
                               IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                               IN RvChar*							strBuffer,
                               IN RvUint							bufferLen,
                               OUT RvUint*							actualLen);

/***************************************************************************
 * RvSipPDCSBillingInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal PDCSBillingInfo header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPDCSBillingInfoHeaderSetStrBadSyntax(
                          IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                          IN RvChar*							strBadSyntax);


#endif /*#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPPDCSBILLINGINFOHEADER_H */


