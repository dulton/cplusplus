/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipAddressFieldDispatcher.h                        *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Mar 2005                                             *
 ******************************************************************************/

#ifndef RVSIPADDRESSFIELDDISPATCHER_H
#define RVSIPADDRESSFIELDDISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "RvSipAddress.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_JSR32_SUPPORT

/***************************************************************************
 * RvSipAddressDestruct
 * ------------------------------------------------------------------------
 * General: Destruct an address object and the memory it lies on.
 *          Notice: This function will free the page that the header lies on.
 *          Therefore it can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object to destruct.
 ***************************************************************************/
RVAPI void RVCALLCONV RvSipAddressDestruct(
                                          IN RvSipAddressHandle           hAddress);

/***************************************************************************
 * RvSipAddrEncodeWithoutDisplayName
 * ------------------------------------------------------------------------
 * General: Encodes an Address object to a textual address object. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          Notice: This function does not encode the display name as part of
 *          the textual address. In order to include the display name, use 
 *          RvSipAddrEncode().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle to the Address object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded object.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrEncodeWithoutDisplayName(
										   IN  RvSipAddressHandle hSipAddr,
                                           IN  HRPOOL             hPool,
                                           OUT HPAGE*             hPage,
                                           OUT RvUint32*          length);

/***************************************************************************
 * RvSipAddressSetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to set any string field of an address object.
 *          For example: to set the host of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST and buffer containing the 
 *          host value.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress - A handle to the address object.
 *          eField   - The enumeration value that indicates the field being set.
 *          buffer   - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressSetStringField(
                                          IN RvSipAddressHandle           hAddress,
										  IN RvSipAddressFieldName        eField,
                                          IN RvChar*                      buffer);

/***************************************************************************
 * RvSipAddressGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: This function is used to get the length of any string field of 
 *          an address object. Use this function to evaluate the appropriate 
 *          buffer size to allocate before calling the GetStringField function.
 *          For example: to get the length of the host field of a SIP URL, 
 *          call this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST. 
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object.
 *          eField    - The enumeration value that indicates the field being get.
 * Output:  pLength   - The length of the requested string including the null
 *                      terminator character.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressGetStringFieldLength(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          OUT RvInt32*                     pLength);

/***************************************************************************
 * RvSipAddressGetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to get any string field of an address object.
 *          For example: to get the host of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST and an adequate buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object.
 *          eField    - The enumeration value that indicates the field being get.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressGetStringField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvChar*                      buffer,
                                          IN  RvInt32                      bufferLen,
                                          OUT RvInt32*                     actualLen);

/***************************************************************************
 * RvSipAddressSetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to set an integer field of an address object.
 *          For example: to set the port of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_PORT and the port value
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress   - A handle to the address object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressSetIntField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvInt32                      fieldValue);

/***************************************************************************
 * RvSipAddressGetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to get an integer field of an address object.
 *          For example: to get the port of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_PORT.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being get.          
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressGetIntField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvInt32*                     pFieldValue);

/***************************************************************************
 * RvSipAddressSetBoolField
 * ------------------------------------------------------------------------
 * General: This function is used to set a boolean field of an address object.
 *          For example: to set the Is-Secure indication of a SIP URL, call 
 *          this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being set.          
 *          fieldValue  - The Bool value to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressSetBoolField(
									IN  RvSipAddressHandle           hAddress,
									IN  RvSipAddressFieldName        eField,
									IN  RvBool                       fieldValue);

/***************************************************************************
 * RvSipAddressGetBoolField
 * ------------------------------------------------------------------------
 * General: This function is used to get a boolean field of an address object.
 *          For example: to get the Is-Secure indication of a SIP URL, call 
 *          this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being set.          
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddressGetBoolField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvBool*                      pFieldValue);


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* RVSIPADDRESSFIELDDISPATCHER_H */





