/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        AddressFieldDispatcher.h                            *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Apr 2005                                             *
 ******************************************************************************/

#ifndef ADDRESSFIELDDISPATCHER_H
#define ADDRESSFIELDDISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_JSR32_SUPPORT

/***************************************************************************
 * AddressDestruct
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
void RVCALLCONV AddressDestruct(IN RvSipAddressHandle           hAddress);

/***************************************************************************
 * AddressSetStringField
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
RvStatus RVCALLCONV AddressSetStringField(
                                          IN RvSipAddressHandle           hAddress,
										  IN RvSipAddressFieldName        eField,
                                          IN RvChar*                      buffer);

/***************************************************************************
 * AddressGetStringFieldLength
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
RvStatus RVCALLCONV AddressGetStringFieldLength(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          OUT RvInt32*                     pLength);

/***************************************************************************
 * AddressGetStringField
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
RvStatus RVCALLCONV AddressGetStringField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvChar*                      buffer,
                                          IN  RvInt32                      bufferLen,
                                          OUT RvInt32*                     actualLen);

/***************************************************************************
 * AddressSetIntField
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
RvStatus RVCALLCONV AddressSetIntField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvInt32                      fieldValue);

/***************************************************************************
 * AddressGetIntField
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
RvStatus RVCALLCONV AddressGetIntField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvInt32*                     pFieldValue);

/***************************************************************************
 * AddressSetBoolField
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
RvStatus RVCALLCONV AddressSetBoolField(
									IN  RvSipAddressHandle           hAddress,
									IN  RvSipAddressFieldName        eField,
									IN  RvBool                       fieldValue);

/***************************************************************************
 * AddressGetBoolField
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
RvStatus RVCALLCONV AddressGetBoolField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvBool*                      pFieldValue);


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* ADDRESSFIELDDISPATCHER_H */








