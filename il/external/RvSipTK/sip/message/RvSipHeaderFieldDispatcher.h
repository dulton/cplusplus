/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipHeaderFieldDispatcher.h                        *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Mar 2005                                             *
 ******************************************************************************/

#ifndef RVSIPHEADERFIELDDISPATCHER_H
#define RVSIPHEADERFIELDDISPATCHER_H

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
 * RvSipHeaderDestruct
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to free all their memory resources.
 *          For example: to destruct a Contact header, call
 *          this function with eType RVSIP_HEADERTYPE_CONTACT.
 *          Notice: This function will free the page that the header lies on.
 *          Therefore it can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 *          eType    - The Type of the given header.
 ***************************************************************************/
RVAPI void RVCALLCONV RvSipHeaderDestruct(IN void*              hHeader,
                                          IN RvSipHeaderType    eType);

/***************************************************************************
 * RvSipHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to instruct the header to use the compact header name when the
 *          header is encoded.
 *          For example: to set the compact form of a Contact header, call
 *          this function with eType RVSIP_HEADERTYPE_CONTACT.
 *          Notice: Not all headers have a compact form. You can only use
 *          RV_FALSE for headers that do not have a compact form
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eType       - The Type of the given header.
 *          bIsCompact  - Specify whether or not to use a compact form
 *          strCompName - The compact header name
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderSetCompactForm(
                                          IN    void*               hHeader,
                                          IN    RvSipHeaderType     eType,
										  IN    RvBool              bIsCompact,
										  IN    RvChar*             strCompName);

/***************************************************************************
 * RvSipHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to set an address field value.
 *          For example: to set the address-spec of a Contact header, call
 *          this function with eType RVSIP_HEADERTYPE_CONTACT, eField 0 and
 *          the address object to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eType     - The Type of the given header.
 *          eField    - The enumeration value that indicates the field being set.
 *                      This enumeration is header specific and
 *                      can be found in the specific RvSipXXXHeader.h file. It
 *                      must correlate to the header type indicated by eType.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderSetAddressField(
                                          IN    void*               hHeader,
                                          IN    RvSipHeaderType     eType,
										  IN    RvInt32             eField,
                                          INOUT RvSipAddressHandle* phAddress);

/***************************************************************************
 * RvSipHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to get an address field value.
 *          For example: to get the address-spec of a Contact header, call
 *          this function with eType RVSIP_HEADERTYPE_CONTACT and eField 0, and
 *          the address object will be returned.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eType     - The Type of the given header.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *                      This enumeration is header specific and
 *                      can be found in the specific RvSipXXXHeader.h file. It
 *                      must correlate to the header type indicated by eType.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderGetAddressField(
                                          IN  void*                hHeader,
                                          IN  RvSipHeaderType      eType,
										  IN  RvInt32              eField,
                                          OUT RvSipAddressHandle  *phAddress);

/***************************************************************************
 * RvSipHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to set any desirable string field value.
 *          For example: to set the Tag parameter of a To header, call
 *          this function with eType RVSIP_HEADERTYPE_TO, eField 1 and buffer
 *          containing the Tag value.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field being set.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderSetStringField(
                                          IN void*              hHeader,
                                          IN RvSipHeaderType    eType,
										  IN RvInt32            eField,
                                          IN RvChar*            buffer);

/***************************************************************************
 * RvSipHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to get the length of any string field. Use this function to evaluate 
 *          the appropriate buffer size to allocate before calling the GetStringField 
 *          function.
 *          For example: to get the length of the Tag parameter of a To header, call
 *          this function with eType RVSIP_HEADERTYPE_TO and eField 1.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field being set.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 *          pLength - The length of the requested string including the null
 *                    terminator character.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderGetStringFieldLength(
											IN void*              hHeader,
											IN RvSipHeaderType    eType,
											IN RvInt32            eField,
											OUT RvInt32*          pLength);

/***************************************************************************
 * RvSipHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to get any desirable string field value.
 *          For example: to get the Tag parameter of a To header, call
 *          this function with eType RVSIP_HEADERTYPE_TO, eField 1 and buffer
 *          adequate for the Tag value.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eType     - The Type of the given header.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *                      This enumeration is header specific and
 *                      can be found in the specific RvSipXXXHeader.h file. It
 *                      must correlate to the header type indicated by eType.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderGetStringField(
                                          IN  void*              hHeader,
                                          IN  RvSipHeaderType    eType,
										  IN  RvInt32            eField,
                                          IN  RvChar*            buffer,
                                          IN  RvInt32            bufferLen,
                                          OUT RvInt32*           actualLen);

/***************************************************************************
 * RvSipHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to set any desirable integer field value (including emunerations).
 *          For example: to set the CSeq step of a CSeq header, call
 *          this function with eType RVSIP_HEADERTYPE_CSEQ, eField 0 and the
 *          CSeq value.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field being set.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderSetIntField(
                                          IN void*              hHeader,
                                          IN RvSipHeaderType    eType,
										  IN RvInt32            eField,
                                          IN RvInt32            fieldValue);

/***************************************************************************
 * RvSipHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to get any desirable integer field value (including emunerations).
 *          For example: to get the CSeq step of a CSeq header, call
 *          this function with eType RVSIP_HEADERTYPE_CSEQ and eField 0.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field to retrieve.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderGetIntField(
                                          IN  void*              hHeader,
                                          IN  RvSipHeaderType    eType,
										  IN  RvInt32            eField,
                                          OUT RvInt32*           pFieldValue);

/***************************************************************************
 * RvSipHeaderSetBoolField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to set a boolean field value.
 *          For example: to set the Stale parameter of an Authentication header,
 *          call this function with eType RVSIP_HEADERTYPE_AUTHENTICATION, eField 7
 *          and the boolean indicating the stale parameter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field being set.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 *          fieldValue - The Bool value to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderSetBoolField(
                                          IN void*              hHeader,
                                          IN RvSipHeaderType    eType,
										  IN RvInt32            eField,
                                          IN RvBool             fieldValue);

/***************************************************************************
 * RvSipHeaderGetBoolField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to get a boolean field value.
 *          For example: to get the Stale parameter of an Authentication header,
 *          call this function with eType RVSIP_HEADERTYPE_AUTHENTICATION and
 *          eField 7.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eType   - The Type of the given header.
 *          eField  - The enumeration value that indicates the field to retrieve.
 *                    This enumeration is header specific and
 *                    can be found in the specific RvSipXXXHeader.h file. It
 *                    must correlate to the header type indicated by eType.
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderGetBoolField(
                                          IN  void*              hHeader,
                                          IN  RvSipHeaderType    eType,
										  IN  RvInt32            eField,
                                          OUT RvBool*            pFieldValue);


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* RVSIPHEADERFIELDDISPATCHER_H */

