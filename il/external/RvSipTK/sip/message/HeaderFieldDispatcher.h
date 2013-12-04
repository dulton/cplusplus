/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        HeaderFieldDispatcher.h                             *
 *                                                                            *
 * This file contains set and get functions for each of the headers supported *
 * by the stack. These functions receive either RvInt32 or string parameters, *
 * and convert them according to the actual field expected by the header.     *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Mar 2005                                             *
 ******************************************************************************/

#ifndef HEADERFIELDDISPATCHER_H
#define HEADERFIELDDISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "rpool_API.h"

#include "RvSipAddress.h"
#include "RvSipAcceptHeader.h"
#include "RvSipAcceptEncodingHeader.h"
#include "RvSipAcceptLanguageHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthenticationInfoHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipContentTypeHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipDateHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipInfoHeader.h"
#include "RvSipRAckHeader.h"
#include "RvSipRSeqHeader.h"
#include "RvSipReasonHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReplyToHeader.h"
#include "RvSipRetryAfterHeader.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipTimestampHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipWarningHeader.h"
	
/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_JSR32_SUPPORT

/*-----------------------------------------------------------------------*/
/*                   ACCEPT HEADER                                       */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AcceptHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AcceptHeaderDestruct(IN RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * AcceptHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Accept header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AcceptHeaderSetStringField(
                                          IN RvSipAcceptHeaderHandle    hHeader,
										  IN RvSipAcceptHeaderFieldName eField,
                                          IN RvChar*                    buffer);

/***************************************************************************
 * AcceptHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Accept 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptHeaderGetStringFieldLength(
                                    IN  RvSipAcceptHeaderHandle       hHeader,
									IN  RvSipAcceptHeaderFieldName    eField,
                                    OUT RvInt32*                      pLength);

/***************************************************************************
 * AcceptHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Accept header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptHeaderGetStringField(
                                    IN  RvSipAcceptHeaderHandle       hHeader,
									IN  RvSipAcceptHeaderFieldName    eField,
                                    IN  RvChar*                       buffer,
                                    IN  RvInt32                       bufferLen,
                                    OUT RvInt32*                      actualLen);

/*-----------------------------------------------------------------------*/
/*                   ACCEPT ENCODING HEADER                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AcceptEncodingHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AcceptEncodingHeaderDestruct(IN RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * AcceptEncodingHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Accept-Encoding header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AcceptEncodingHeaderSetStringField(
                                          IN RvSipAcceptEncodingHeaderHandle    hHeader,
										  IN RvSipAcceptEncodingHeaderFieldName eField,
                                          IN RvChar*                            buffer);

/***************************************************************************
 * AcceptEncodingHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Accept-Encoding 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptEncodingHeaderGetStringFieldLength(
                                    IN  RvSipAcceptEncodingHeaderHandle       hHeader,
									IN  RvSipAcceptEncodingHeaderFieldName    eField,
                                    OUT RvInt32*                              pLength);

/***************************************************************************
 * AcceptEncodingHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Accept-Encoding header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptEncodingHeaderGetStringField(
                                    IN  RvSipAcceptEncodingHeaderHandle       hHeader,
									IN  RvSipAcceptEncodingHeaderFieldName    eField,
                                    IN  RvChar*                               buffer,
                                    IN  RvInt32                               bufferLen,
                                    OUT RvInt32*                              actualLen);

/*-----------------------------------------------------------------------*/
/*                   ACCEPT LANGUAGE HEADER                              */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AcceptLanguageHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AcceptLanguageHeaderDestruct(IN RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * AcceptLanguageHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Accept-Language header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AcceptLanguageHeaderSetStringField(
                                          IN RvSipAcceptLanguageHeaderHandle    hHeader,
										  IN RvSipAcceptLanguageHeaderFieldName eField,
                                          IN RvChar*                            buffer);

/***************************************************************************
 * AcceptLanguageHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Accept-Language 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptLanguageHeaderGetStringFieldLength(
                                    IN  RvSipAcceptLanguageHeaderHandle       hHeader,
									IN  RvSipAcceptLanguageHeaderFieldName    eField,
                                    OUT RvInt32*                              pLength);

/***************************************************************************
 * AcceptLanguageHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Accept-Language header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptLanguageHeaderGetStringField(
                                    IN  RvSipAcceptLanguageHeaderHandle       hHeader,
									IN  RvSipAcceptLanguageHeaderFieldName    eField,
                                    IN  RvChar*                               buffer,
                                    IN  RvInt32                               bufferLen,
                                    OUT RvInt32*                              actualLen);

/*-----------------------------------------------------------------------*/
/*                   ALLOW EVENTS HEADER                                 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AllowEventsHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AllowEventsHeaderDestruct(IN RvSipAllowEventsHeaderHandle hHeader);

/***************************************************************************
 * AllowEventsHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Allow-Events header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AllowEventsHeaderSetStringField(
                                        IN RvSipAllowEventsHeaderHandle    hHeader,
										IN RvSipAllowEventsHeaderFieldName eField,
                                        IN RvChar*                         buffer);

/***************************************************************************
 * AllowEventsHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Allow-Events header. 
 *          The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AllowEventsHeaderGetStringFieldLength(
                                    IN  RvSipAllowEventsHeaderHandle       hHeader,
									IN  RvSipAllowEventsHeaderFieldName    eField,
                                    OUT RvInt32*                           pLength);

/***************************************************************************
 * AllowEventsHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Allow-Events header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AllowEventsHeaderGetStringField(
                                    IN  RvSipAllowEventsHeaderHandle       hHeader,
									IN  RvSipAllowEventsHeaderFieldName    eField,
                                    IN  RvChar*                            buffer,
                                    IN  RvInt32                            bufferLen,
                                    OUT RvInt32*                           actualLen);

/*-----------------------------------------------------------------------*/
/*                   ALLOW HEADER                                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AllowHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AllowHeaderDestruct(IN RvSipAllowHeaderHandle hHeader);

/***************************************************************************
 * AllowHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Allow header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AllowHeaderSetStringField(
                                          IN RvSipAllowHeaderHandle    hHeader,
										  IN RvSipAllowHeaderFieldName eField,
                                          IN RvChar*                   buffer);

/***************************************************************************
 * AllowHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Allow header. 
 *          The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AllowHeaderGetStringFieldLength(
                                    IN  RvSipAllowHeaderHandle       hHeader,
									IN  RvSipAllowHeaderFieldName    eField,
                                    OUT RvInt32*                     pLength);

/***************************************************************************
 * AllowHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Allow header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AllowHeaderGetStringField(
                                    IN  RvSipAllowHeaderHandle       hHeader,
									IN  RvSipAllowHeaderFieldName    eField,
                                    IN  RvChar*                      buffer,
                                    IN  RvInt32                      bufferLen,
                                    OUT RvInt32*                     actualLen);

/*-----------------------------------------------------------------------*/
/*                   AUTHENTICATION HEADER                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AuthenticationHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AuthenticationHeaderDestruct(IN RvSipAuthenticationHeaderHandle hHeader);

/***************************************************************************
 * AuthenticationHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Authentication header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderSetStringField(
                                          IN RvSipAuthenticationHeaderHandle    hHeader,
										  IN RvSipAuthenticationHeaderFieldName eField,
                                          IN RvChar*                            buffer);

/***************************************************************************
 * AuthenticationHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Authentication 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderGetStringFieldLength(
                                    IN  RvSipAuthenticationHeaderHandle       hHeader,
									IN  RvSipAuthenticationHeaderFieldName    eField,
                                    OUT RvInt32*                              pLength);

/***************************************************************************
 * AuthenticationHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Authentication header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderGetStringField(
                                    IN  RvSipAuthenticationHeaderHandle       hHeader,
									IN  RvSipAuthenticationHeaderFieldName    eField,
                                    IN  RvChar*                               buffer,
                                    IN  RvInt32                               bufferLen,
                                    OUT RvInt32*                              actualLen);

/***************************************************************************
 * AuthenticationHeaderSetBoolField
 * ------------------------------------------------------------------------
 * General: Set boolean field to this Authentication header.
 *			The given boolean may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the boolean will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Bool value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderSetBoolField(
                                          IN RvSipAuthenticationHeaderHandle      hHeader,
										  IN RvSipAuthenticationHeaderFieldName   eField,
                                          IN RvBool                               fieldValue);

/***************************************************************************
 * AuthenticationHeaderGetBoolField
 * ------------------------------------------------------------------------
 * General: Get a boolean field of this Authentication header.
 *			The boolean returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned boolean).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderGetBoolField(
                                          IN  RvSipAuthenticationHeaderHandle      hHeader,
										  IN  RvSipAuthenticationHeaderFieldName   eField,
                                          OUT RvBool*                              pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   AUTHENTICATION-INFO HEADER                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AuthenticationInfoHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AuthenticationInfoHeaderDestruct(IN RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * AuthenticationInfoHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Authentication-Info header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderSetStringField(
                                          IN RvSipAuthenticationInfoHeaderHandle    hHeader,
										  IN RvSipAuthenticationInfoHeaderFieldName eField,
                                          IN RvChar*                                buffer);

/***************************************************************************
 * AuthenticationInfoHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Authentication-Info 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderGetStringFieldLength(
                                    IN  RvSipAuthenticationInfoHeaderHandle       hHeader,
									IN  RvSipAuthenticationInfoHeaderFieldName    eField,
                                    OUT RvInt32*                                  pLength);

/***************************************************************************
 * AuthenticationInfoHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Authentication-Info header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderGetStringField(
                                    IN  RvSipAuthenticationInfoHeaderHandle       hHeader,
									IN  RvSipAuthenticationInfoHeaderFieldName    eField,
                                    IN  RvChar*                                   buffer,
                                    IN  RvInt32                                   bufferLen,
                                    OUT RvInt32*                                  actualLen);

/***************************************************************************
 * AuthenticationInfoHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Authentication-Info header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderSetIntField(
                                          IN RvSipAuthenticationInfoHeaderHandle      hHeader,
										  IN RvSipAuthenticationInfoHeaderFieldName   eField,
                                          IN RvInt32                                  fieldValue);

/***************************************************************************
 * AuthenticationInfoHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Authentication-Info header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderGetIntField(
                                          IN  RvSipAuthenticationInfoHeaderHandle      hHeader,
										  IN  RvSipAuthenticationInfoHeaderFieldName   eField,
                                          OUT RvInt32*                                 pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   AUTHORIZATION  HEADER                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AuthorizationHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV AuthorizationHeaderDestruct(IN RvSipAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * AuthorizationHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Authorization header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderSetStringField(
                                          IN RvSipAuthorizationHeaderHandle    hHeader,
										  IN RvSipAuthorizationHeaderFieldName eField,
                                          IN RvChar*                           buffer);

/***************************************************************************
 * AuthorizationHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Authorization 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderGetStringFieldLength(
                                    IN  RvSipAuthorizationHeaderHandle       hHeader,
									IN  RvSipAuthorizationHeaderFieldName    eField,
                                    OUT RvInt32*                             pLength);

/***************************************************************************
 * AuthorizationHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Authorization header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderGetStringField(
                                    IN  RvSipAuthorizationHeaderHandle        hHeader,
									IN  RvSipAuthorizationHeaderFieldName     eField,
                                    IN  RvChar*                               buffer,
                                    IN  RvInt32                               bufferLen,
                                    OUT RvInt32*                              actualLen);

/***************************************************************************
 * AuthorizationHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Authorization header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderSetAddressField(
                                          IN    RvSipAuthorizationHeaderHandle     hHeader,
										  IN    RvSipAuthorizationHeaderFieldName  eField,
                                          INOUT RvSipAddressHandle                *phAddress);

/***************************************************************************
 * AuthorizationHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Authorization header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderGetAddressField(
                                    IN  RvSipAuthorizationHeaderHandle       hHeader,
									IN  RvSipAuthorizationHeaderFieldName    eField,
                                    OUT RvSipAddressHandle                  *phAddress);

/***************************************************************************
 * AuthorizationHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Authorization header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderSetIntField(
                                          IN RvSipAuthorizationHeaderHandle      hHeader,
										  IN RvSipAuthorizationHeaderFieldName   eField,
                                          IN RvInt32                             fieldValue);

/***************************************************************************
 * AuthorizationHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Authorization header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderGetIntField(
                                          IN  RvSipAuthorizationHeaderHandle      hHeader,
										  IN  RvSipAuthorizationHeaderFieldName   eField,
                                          OUT RvInt32*                            pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   CONTENT-DISPOSITION  HEADER                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ContentDispositionHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ContentDispositionHeaderDestruct(IN RvSipContentDispositionHeaderHandle hHeader);

/***************************************************************************
 * ContentDispositionHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Content-Disposition header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ContentDispositionHeaderSetStringField(
                                          IN RvSipContentDispositionHeaderHandle    hHeader,
										  IN RvSipContentDispositionHeaderFieldName eField,
                                          IN RvChar*                                buffer);

/***************************************************************************
 * ContentDispositionHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Content-Disposition 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContentDispositionHeaderGetStringFieldLength(
                                    IN  RvSipContentDispositionHeaderHandle       hHeader,
									IN  RvSipContentDispositionHeaderFieldName    eField,
                                    OUT RvInt32*                                  pLength);

/***************************************************************************
 * ContentDispositionHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Content-Disposition header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContentDispositionHeaderGetStringField(
                                    IN  RvSipContentDispositionHeaderHandle        hHeader,
									IN  RvSipContentDispositionHeaderFieldName     eField,
                                    IN  RvChar*                                    buffer,
                                    IN  RvInt32                                    bufferLen,
                                    OUT RvInt32*                                   actualLen);

/*-----------------------------------------------------------------------*/
/*                   CONTENT-TYPE  HEADER                                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ContentTypeHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ContentTypeHeaderDestruct(IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * ContenTypeHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Content-Type header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderSetStringField(
                                          IN RvSipContentTypeHeaderHandle    hHeader,
										  IN RvSipContentTypeHeaderFieldName eField,
                                          IN RvChar*                         buffer);

/***************************************************************************
 * ContentTypeHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Conten-Type 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderGetStringFieldLength(
                                    IN  RvSipContentTypeHeaderHandle       hHeader,
									IN  RvSipContentTypeHeaderFieldName    eField,
                                    OUT RvInt32*                           pLength);

/***************************************************************************
 * ContentTypeHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Content-Type header. The field to
 *          get is indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderGetStringField(
                                    IN  RvSipContentTypeHeaderHandle               hHeader,
									IN  RvSipContentTypeHeaderFieldName            eField,
                                    IN  RvChar*                                    buffer,
                                    IN  RvInt32                                    bufferLen,
                                    OUT RvInt32*                                   actualLen);

/*-----------------------------------------------------------------------*/
/*                   CONTACT HEADER                                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ContactHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ContactHeaderDestruct(IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * ContactHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Contact header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderSetAddressField(
                                          IN    RvSipContactHeaderHandle    hHeader,
										  IN    RvSipContactHeaderFieldName eField,
                                          INOUT RvSipAddressHandle          *phAddress);

/***************************************************************************
 * ContactHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Contact header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetAddressField(
                                    IN  RvSipContactHeaderHandle       hHeader,
									IN  RvSipContactHeaderFieldName    eField,
                                    OUT RvSipAddressHandle            *phAddress);

/***************************************************************************
 * ContactHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Contact header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderSetStringField(
                                          IN RvSipContactHeaderHandle    hHeader,
										  IN RvSipContactHeaderFieldName eField,
                                          IN RvChar*                     buffer);

/***************************************************************************
 * ContactHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Contact 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetStringFieldLength(
                                    IN  RvSipContactHeaderHandle       hHeader,
									IN  RvSipContactHeaderFieldName    eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * ContactHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Contact header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetStringField(
                                    IN  RvSipContactHeaderHandle       hHeader,
									IN  RvSipContactHeaderFieldName    eField,
                                    IN  RvChar*                        buffer,
                                    IN  RvInt32                        bufferLen,
                                    OUT RvInt32*                       actualLen);

/***************************************************************************
 * ContactHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Contact header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int32 value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderSetIntField(
                                          IN RvSipContactHeaderHandle      hHeader,
										  IN RvSipContactHeaderFieldName   eField,
                                          IN RvInt32                       fieldValue);

/***************************************************************************
 * ContactHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Contact header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int32 value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetIntField(
                                          IN  RvSipContactHeaderHandle      hHeader,
										  IN  RvSipContactHeaderFieldName   eField,
                                          OUT RvInt32*                      pFieldValue);

/***************************************************************************
 * ContactHeaderSetBoolField
 * ------------------------------------------------------------------------
 * General: Set boolean field to this Contact header.
 *			The given boolean may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the boolean will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Bool value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderSetBoolField(
                                          IN RvSipContactHeaderHandle      hHeader,
										  IN RvSipContactHeaderFieldName   eField,
                                          IN RvBool                        fieldValue);
/***************************************************************************
 * ContactHeaderGetBoolField
 * ------------------------------------------------------------------------
 * General: Get a boolean field of this Contact header.
 *			The boolean returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned boolean).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetBoolField(
                                          IN  RvSipContactHeaderHandle      hHeader,
										  IN  RvSipContactHeaderFieldName   eField,
                                          OUT RvBool*                       pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   CSEQ HEADER                                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CSeqHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV CSeqHeaderDestruct(IN RvSipCSeqHeaderHandle hHeader);

/***************************************************************************
 * CSeqHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this CSeq header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderSetStringField(
                                          IN RvSipCSeqHeaderHandle    hHeader,
										  IN RvSipCSeqHeaderFieldName eField,
                                          IN RvChar*                  buffer);

/***************************************************************************
 * CSeqHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this CSeq 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderGetStringFieldLength(
                                    IN  RvSipCSeqHeaderHandle          hHeader,
									IN  RvSipCSeqHeaderFieldName       eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * CSeqHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this CSeq header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderGetStringField(
                                    IN  RvSipCSeqHeaderHandle       hHeader,
									IN  RvSipCSeqHeaderFieldName    eField,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen);

/***************************************************************************
 * CSeqHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this CSeq header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderSetIntField(
                                          IN RvSipCSeqHeaderHandle      hHeader,
										  IN RvSipCSeqHeaderFieldName   eField,
                                          IN RvInt32                    fieldValue);

/***************************************************************************
 * CSeqHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this CSeq header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderGetIntField(
                                          IN  RvSipCSeqHeaderHandle      hHeader,
										  IN  RvSipCSeqHeaderFieldName   eField,
                                          OUT RvInt32*                   pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   DATE HEADER                                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * DateHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV DateHeaderDestruct(IN RvSipDateHeaderHandle hHeader);

/***************************************************************************
 * DateHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Date header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderSetStringField(
                                          IN RvSipDateHeaderHandle    hHeader,
										  IN RvSipDateHeaderFieldName eField,
                                          IN RvChar*                  buffer);

/***************************************************************************
 * DateHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Date 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderGetStringFieldLength(
                                    IN  RvSipDateHeaderHandle          hHeader,
									IN  RvSipDateHeaderFieldName       eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * DateHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Date header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderGetStringField(
                                    IN  RvSipDateHeaderHandle       hHeader,
									IN  RvSipDateHeaderFieldName    eField,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen);

/***************************************************************************
 * DateHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Date header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderSetIntField(
                                          IN RvSipDateHeaderHandle         hHeader,
										  IN RvSipDateHeaderFieldName      eField,
                                          IN RvInt32                       fieldValue);

/***************************************************************************
 * DateHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Date header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderGetIntField(
                                          IN  RvSipDateHeaderHandle      hHeader,
										  IN  RvSipDateHeaderFieldName   eField,
                                          OUT RvInt32*                   pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   EVENT HEADER                                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * EventHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV EventHeaderDestruct(IN RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * EventHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Event header. The field being
 *          set is indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV EventHeaderSetStringField(
                                        IN RvSipEventHeaderHandle    hHeader,
										IN RvSipEventHeaderFieldName eField,
                                        IN RvChar*                   buffer);

/***************************************************************************
 * EventHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Allow-Events header. 
 *          The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV EventHeaderGetStringFieldLength(
                                    IN  RvSipEventHeaderHandle       hHeader,
									IN  RvSipEventHeaderFieldName    eField,
                                    OUT RvInt32*                     pLength);

/***************************************************************************
 * EventHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Event header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV EventHeaderGetStringField(
                                    IN  RvSipEventHeaderHandle             hHeader,
									IN  RvSipEventHeaderFieldName          eField,
                                    IN  RvChar*                            buffer,
                                    IN  RvInt32                            bufferLen,
                                    OUT RvInt32*                           actualLen);

/*-----------------------------------------------------------------------*/
/*                   EXPIRES HEADER                                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ExpiresHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ExpiresHeaderDestruct(IN RvSipExpiresHeaderHandle hHeader);

/***************************************************************************
 * ExpiresHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Expires header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ExpiresHeaderSetStringField(
                                          IN RvSipExpiresHeaderHandle    hHeader,
										  IN RvSipExpiresHeaderFieldName eField,
                                          IN RvChar*                     buffer);

/***************************************************************************
 * ExpiresHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Expires 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ExpiresHeaderGetStringFieldLength(
                                    IN  RvSipExpiresHeaderHandle          hHeader,
									IN  RvSipExpiresHeaderFieldName       eField,
                                    OUT RvInt32*                          pLength);

/***************************************************************************
 * ExpiresHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Expires header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ExpiresHeaderGetStringField(
                                    IN  RvSipExpiresHeaderHandle       hHeader,
									IN  RvSipExpiresHeaderFieldName    eField,
                                    IN  RvChar*                        buffer,
                                    IN  RvInt32                        bufferLen,
                                    OUT RvInt32*                       actualLen);

/***************************************************************************
 * ExpiresHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Expires header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ExpiresHeaderSetIntField(
                                          IN RvSipExpiresHeaderHandle      hHeader,
										  IN RvSipExpiresHeaderFieldName   eField,
                                          IN RvInt32                       fieldValue);

/***************************************************************************
 * ExpiresHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this CSeq header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ExpiresHeaderGetIntField(
                                          IN  RvSipExpiresHeaderHandle      hHeader,
										  IN  RvSipExpiresHeaderFieldName   eField,
                                          OUT RvInt32*                      pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   INFO HEADER                                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * InfoHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV InfoHeaderDestruct(IN RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * InfoHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Info header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderSetStringField(
                                          IN RvSipInfoHeaderHandle    hHeader,
										  IN RvSipInfoHeaderFieldName eField,
                                          IN RvChar*                  buffer);

/***************************************************************************
 * InfoHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Info 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderGetStringFieldLength(
                                    IN  RvSipInfoHeaderHandle          hHeader,
									IN  RvSipInfoHeaderFieldName       eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * InfoHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Info header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderGetStringField(
                                    IN  RvSipInfoHeaderHandle        hHeader,
									IN  RvSipInfoHeaderFieldName     eField,
                                    IN  RvChar*                      buffer,
                                    IN  RvInt32                      bufferLen,
                                    OUT RvInt32*                     actualLen);

/***************************************************************************
 * InfoHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Info header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderSetAddressField(
                                          IN    RvSipInfoHeaderHandle     hHeader,
										  IN    RvSipInfoHeaderFieldName  eField,
                                          INOUT RvSipAddressHandle       *phAddress);

/***************************************************************************
 * InfoHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Info header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderGetAddressField(
                                    IN  RvSipInfoHeaderHandle        hHeader,
									IN  RvSipInfoHeaderFieldName     eField,
                                    OUT RvSipAddressHandle          *phAddress);

/*-----------------------------------------------------------------------*/
/*                   PARTY HEADER                                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * PartyHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV PartyHeaderDestruct(IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * PartyHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Party header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderSetStringField(
                                          IN RvSipPartyHeaderHandle    hHeader,
										  IN RvSipPartyHeaderFieldName eField,
                                          IN RvChar*                   buffer);

/***************************************************************************
 * PartyHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Party 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderGetStringFieldLength(
                                    IN  RvSipPartyHeaderHandle          hHeader,
									IN  RvSipPartyHeaderFieldName       eField,
                                    OUT RvInt32*                        pLength);

/***************************************************************************
 * PartyHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Party header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderGetStringField(
                                    IN  RvSipPartyHeaderHandle       hHeader,
									IN  RvSipPartyHeaderFieldName    eField,
                                    IN  RvChar*                      buffer,
                                    IN  RvInt32                      bufferLen,
                                    OUT RvInt32*                     actualLen);

/***************************************************************************
 * PartyHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Party header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderSetAddressField(
                                          IN    RvSipPartyHeaderHandle    hHeader,
										  IN    RvSipPartyHeaderFieldName eField,
                                          INOUT RvSipAddressHandle       *phAddress);

/***************************************************************************
 * PartyHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Party header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderGetAddressField(
                                    IN  RvSipPartyHeaderHandle       hHeader,
									IN  RvSipPartyHeaderFieldName    eField,
                                    OUT RvSipAddressHandle          *phAddress);

/*-----------------------------------------------------------------------*/
/*                   RACK HEADER                                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RAckHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV RAckHeaderDestruct(IN RvSipRAckHeaderHandle hHeader);

/***************************************************************************
 * RAckHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this RAck header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderSetStringField(
                                          IN RvSipRAckHeaderHandle    hHeader,
										  IN RvSipRAckHeaderFieldName eField,
                                          IN RvChar*                  buffer);

/***************************************************************************
 * RAckHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this RAck 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderGetStringFieldLength(
                                    IN  RvSipRAckHeaderHandle          hHeader,
									IN  RvSipRAckHeaderFieldName       eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * RAckHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this RAck header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderGetStringField(
                                    IN  RvSipRAckHeaderHandle       hHeader,
									IN  RvSipRAckHeaderFieldName    eField,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen);

/***************************************************************************
 * RAckHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this RAck header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderSetIntField(
                                          IN RvSipRAckHeaderHandle      hHeader,
										  IN RvSipRAckHeaderFieldName   eField,
                                          IN RvInt32                    fieldValue);

/***************************************************************************
 * RAckHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this RAck header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderGetIntField(
                                          IN  RvSipRAckHeaderHandle      hHeader,
										  IN  RvSipRAckHeaderFieldName   eField,
                                          OUT RvInt32*                   pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   RSEQ HEADER                                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RSeqHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV RSeqHeaderDestruct(IN RvSipRSeqHeaderHandle hHeader);

/***************************************************************************
 * RSeqHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this RSeq header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderSetStringField(
                                          IN RvSipRSeqHeaderHandle    hHeader,
										  IN RvSipRSeqHeaderFieldName eField,
                                          IN RvChar*                  buffer);

/***************************************************************************
 * RSeqHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this RSeq 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderGetStringFieldLength(
                                    IN  RvSipRSeqHeaderHandle          hHeader,
									IN  RvSipRSeqHeaderFieldName       eField,
                                    OUT RvInt32*                       pLength);

/***************************************************************************
 * RSeqHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this RSeq header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderGetStringField(
                                    IN  RvSipRSeqHeaderHandle       hHeader,
									IN  RvSipRSeqHeaderFieldName    eField,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen);

/***************************************************************************
 * RSeqHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this RSeq header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderSetIntField(
                                          IN RvSipRSeqHeaderHandle      hHeader,
										  IN RvSipRSeqHeaderFieldName   eField,
                                          IN RvInt32                    fieldValue);

/***************************************************************************
 * RSeqHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this RSeq header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderGetIntField(
                                          IN  RvSipRSeqHeaderHandle      hHeader,
										  IN  RvSipRSeqHeaderFieldName   eField,
                                          OUT RvInt32*                   pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   REASON HEADER                                       */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ReasonHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ReasonHeaderDestruct(IN RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * ReasonHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Reason header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderSetStringField(
                                          IN RvSipReasonHeaderHandle    hHeader,
										  IN RvSipReasonHeaderFieldName eField,
                                          IN RvChar*                    buffer);

/***************************************************************************
 * ReasonHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Reason 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderGetStringFieldLength(
                                    IN  RvSipReasonHeaderHandle          hHeader,
									IN  RvSipReasonHeaderFieldName       eField,
                                    OUT RvInt32*                         pLength);

/***************************************************************************
 * ReasonHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Reason header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderGetStringField(
                                    IN  RvSipReasonHeaderHandle       hHeader,
									IN  RvSipReasonHeaderFieldName    eField,
                                    IN  RvChar*                       buffer,
                                    IN  RvInt32                       bufferLen,
                                    OUT RvInt32*                      actualLen);

/***************************************************************************
 * ReasonHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Reason header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderSetIntField(
                                          IN RvSipReasonHeaderHandle      hHeader,
										  IN RvSipReasonHeaderFieldName   eField,
                                          IN RvInt32                      fieldValue);

/***************************************************************************
 * ReasonHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Reason header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderGetIntField(
                                          IN  RvSipReasonHeaderHandle      hHeader,
										  IN  RvSipReasonHeaderFieldName   eField,
                                          OUT RvInt32*                     pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   REFER-TO HEADER                                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ReferToHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ReferToHeaderDestruct(IN RvSipReferToHeaderHandle hHeader);

/***************************************************************************
 * ReferToHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this ReferTo header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderSetStringField(
                                          IN RvSipReferToHeaderHandle    hHeader,
										  IN RvSipReferToHeaderFieldName eField,
                                          IN RvChar*                     buffer);

/***************************************************************************
 * ReferToHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Refer-To 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderGetStringFieldLength(
                                    IN  RvSipReferToHeaderHandle          hHeader,
									IN  RvSipReferToHeaderFieldName       eField,
                                    OUT RvInt32*                          pLength);

/***************************************************************************
 * ReferToHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Refer-To header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderGetStringField(
                                    IN  RvSipReferToHeaderHandle       hHeader,
									IN  RvSipReferToHeaderFieldName    eField,
                                    IN  RvChar*                        buffer,
                                    IN  RvInt32                        bufferLen,
                                    OUT RvInt32*                       actualLen);

/***************************************************************************
 * ReferToHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Refer-To header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderSetAddressField(
                                          IN    RvSipReferToHeaderHandle     hHeader,
										  IN    RvSipReferToHeaderFieldName  eField,
                                          INOUT RvSipAddressHandle          *phAddress);

/***************************************************************************
 * ReferToHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Refer-To header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderGetAddressField(
                                    IN  RvSipReferToHeaderHandle       hHeader,
									IN  RvSipReferToHeaderFieldName    eField,
                                    OUT RvSipAddressHandle            *phAddress);

/*-----------------------------------------------------------------------*/
/*                   REPLY-TO HEADER                                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ReplyToHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ReplyToHeaderDestruct(IN RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * ReplyToHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Reply-To header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderSetStringField(
                                          IN RvSipReplyToHeaderHandle    hHeader,
										  IN RvSipReplyToHeaderFieldName eField,
                                          IN RvChar*                     buffer);

/***************************************************************************
 * ReplyToHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Reply-To 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderGetStringFieldLength(
                                    IN  RvSipReplyToHeaderHandle          hHeader,
									IN  RvSipReplyToHeaderFieldName       eField,
                                    OUT RvInt32*                          pLength);

/***************************************************************************
 * ReplyToHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Reply-To header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderGetStringField(
                                    IN  RvSipReplyToHeaderHandle       hHeader,
									IN  RvSipReplyToHeaderFieldName    eField,
                                    IN  RvChar*                        buffer,
                                    IN  RvInt32                        bufferLen,
                                    OUT RvInt32*                       actualLen);

/***************************************************************************
 * ReplyToHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Reply-To header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderSetAddressField(
                                          IN    RvSipReplyToHeaderHandle     hHeader,
										  IN    RvSipReplyToHeaderFieldName  eField,
                                          INOUT RvSipAddressHandle          *phAddress);

/***************************************************************************
 * ReplyToHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Reply-To header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderGetAddressField(
                                    IN  RvSipReplyToHeaderHandle       hHeader,
									IN  RvSipReplyToHeaderFieldName    eField,
                                    OUT RvSipAddressHandle            *phAddress);

/*-----------------------------------------------------------------------*/
/*                   RETRY-AFTER HEADER                                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RetryAfterHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV RetryAfterHeaderDestruct(IN RvSipRetryAfterHeaderHandle hHeader);

/***************************************************************************
 * RetryAfterHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Retry-After header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV RetryAfterHeaderSetStringField(
                                          IN RvSipRetryAfterHeaderHandle    hHeader,
										  IN RvSipRetryAfterHeaderFieldName eField,
                                          IN RvChar*						buffer);

/***************************************************************************
 * RetryAfterHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Retry-After 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RetryAfterHeaderGetStringFieldLength(
                                    IN  RvSipRetryAfterHeaderHandle          hHeader,
									IN  RvSipRetryAfterHeaderFieldName       eField,
                                    OUT RvInt32*							 pLength);

/***************************************************************************
 * RetryAfterHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Retry-After header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RetryAfterHeaderGetStringField(
                                    IN  RvSipRetryAfterHeaderHandle       hHeader,
									IN  RvSipRetryAfterHeaderFieldName    eField,
                                    IN  RvChar*							  buffer,
									IN  RvInt32							  bufferLen,
                                    OUT RvInt32*						  actualLen);

/***************************************************************************
 * RetryAfterHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Retry-After header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV RetryAfterHeaderSetIntField(
                                          IN RvSipRetryAfterHeaderHandle         hHeader,
										  IN RvSipRetryAfterHeaderFieldName      eField,
                                          IN RvInt32                             fieldValue);

/***************************************************************************
 * RetryAfterHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Retry-After header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV RetryAfterHeaderGetIntField(
                                          IN  RvSipRetryAfterHeaderHandle      hHeader,
										  IN  RvSipRetryAfterHeaderFieldName   eField,
                                          OUT RvInt32*                         pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   ROUTE-HOP HEADER                                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RouteHopHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV RouteHopHeaderDestruct(IN RvSipRouteHopHeaderHandle hHeader);

/***************************************************************************
 * RouteHopHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Route-Hop header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderSetStringField(
                                          IN RvSipRouteHopHeaderHandle    hHeader,
										  IN RvSipRouteHopHeaderFieldName eField,
                                          IN RvChar*                      buffer);

/***************************************************************************
 * RouteHopHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Route-Hop 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderGetStringFieldLength(
                                    IN  RvSipRouteHopHeaderHandle          hHeader,
									IN  RvSipRouteHopHeaderFieldName       eField,
                                    OUT RvInt32*                           pLength);

/***************************************************************************
 * RouteHopHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Route-Hop header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderGetStringField(
                                    IN  RvSipRouteHopHeaderHandle       hHeader,
									IN  RvSipRouteHopHeaderFieldName    eField,
                                    IN  RvChar*                         buffer,
                                    IN  RvInt32                         bufferLen,
                                    OUT RvInt32*                        actualLen);

/***************************************************************************
 * RouteHopHeaderSetAddressField
 * ------------------------------------------------------------------------
 * General: Set an address object to this Route-Hop header. The field being set is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object.
 *          eField   - The enumeration value that indicates the field being set.
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderSetAddressField(
                                          IN    RvSipRouteHopHeaderHandle     hHeader,
										  IN    RvSipRouteHopHeaderFieldName  eField,
                                          INOUT RvSipAddressHandle           *phAddress);

/***************************************************************************
 * RouteHopHeaderGetAddressField
 * ------------------------------------------------------------------------
 * General: Get an address object this Route-Hop header. The field to get is
 *          indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderGetAddressField(
                                    IN  RvSipRouteHopHeaderHandle       hHeader,
									IN  RvSipRouteHopHeaderFieldName    eField,
                                    OUT RvSipAddressHandle             *phAddress);

/*-----------------------------------------------------------------------*/
/*                   SUBSCRIPTION STATE HEADER                           */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SubscriptionStateHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV SubscriptionStateHeaderDestruct(IN RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SubscriptionStateHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Subscription-State header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV SubscriptionStateHeaderSetStringField(
                                          IN RvSipSubscriptionStateHeaderHandle    hHeader,
										  IN RvSipSubscriptionStateHeaderFieldName eField,
                                          IN RvChar*                               buffer);

/***************************************************************************
 * SubscriptionStateHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this RSeq 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV SubscriptionStateHeaderGetStringFieldLength(
                                    IN  RvSipSubscriptionStateHeaderHandle          hHeader,
									IN  RvSipSubscriptionStateHeaderFieldName       eField,
                                    OUT RvInt32*                                    pLength);

/***************************************************************************
 * SubscriptionStateHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Subscription-State header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV SubscriptionStateHeaderGetStringField(
                                    IN  RvSipSubscriptionStateHeaderHandle       hHeader,
									IN  RvSipSubscriptionStateHeaderFieldName    eField,
                                    IN  RvChar*                                  buffer,
                                    IN  RvInt32                                  bufferLen,
                                    OUT RvInt32*                                 actualLen);

/***************************************************************************
 * SubscriptionStateHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Subscription-State header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV SubscriptionStateHeaderSetIntField(
                                          IN RvSipSubscriptionStateHeaderHandle      hHeader,
										  IN RvSipSubscriptionStateHeaderFieldName   eField,
                                          IN RvInt32                                 fieldValue);

/***************************************************************************
 * SubscriptionStateHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Subscription-State header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV SubscriptionStateHeaderGetIntField(
                                          IN  RvSipSubscriptionStateHeaderHandle      hHeader,
										  IN  RvSipSubscriptionStateHeaderFieldName   eField,
                                          OUT RvInt32*                                pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   TIMESTAMP HEADER                                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TimestampHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV TimestampHeaderDestruct(IN RvSipTimestampHeaderHandle hHeader);

/***************************************************************************
 * TimestampHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Timestamp header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV TimestampHeaderSetStringField(
                                          IN RvSipTimestampHeaderHandle    hHeader,
										  IN RvSipTimestampHeaderFieldName eField,
                                          IN RvChar*                       buffer);

/***************************************************************************
 * TimestampHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Timestamp 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV TimestampHeaderGetStringFieldLength(
                                    IN  RvSipTimestampHeaderHandle          hHeader,
									IN  RvSipTimestampHeaderFieldName       eField,
                                    OUT RvInt32*                            pLength);

/***************************************************************************
 * TimestampHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Timestamp header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV TimestampHeaderGetStringField(
                                    IN  RvSipTimestampHeaderHandle               hHeader,
									IN  RvSipTimestampHeaderFieldName            eField,
                                    IN  RvChar*                                  buffer,
                                    IN  RvInt32                                  bufferLen,
                                    OUT RvInt32*                                 actualLen);

/*-----------------------------------------------------------------------*/
/*                   VIA HEADER                                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * ViaHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV ViaHeaderDestruct(IN RvSipViaHeaderHandle hHeader);

/***************************************************************************
 * ViaHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Via header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderSetStringField(
                                          IN RvSipViaHeaderHandle    hHeader,
										  IN RvSipViaHeaderFieldName eField,
                                          IN RvChar*                 buffer);

/***************************************************************************
 * ViaHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Via 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderGetStringFieldLength(
                                    IN  RvSipViaHeaderHandle            hHeader,
									IN  RvSipViaHeaderFieldName         eField,
                                    OUT RvInt32*                        pLength);

/***************************************************************************
 * ViaHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Via header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderGetStringField(
                                    IN  RvSipViaHeaderHandle        hHeader,
									IN  RvSipViaHeaderFieldName     eField,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen);

/***************************************************************************
 * ViaHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Via header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int32 value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderSetIntField(
                                          IN RvSipViaHeaderHandle       hHeader,
										  IN RvSipViaHeaderFieldName    eField,
                                          IN RvInt32                    fieldValue);

/***************************************************************************
 * ViaHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Via header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int32 value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderGetIntField(
                                          IN  RvSipViaHeaderHandle       hHeader,
										  IN  RvSipViaHeaderFieldName    eField,
                                          OUT RvInt32*                   pFieldValue);

/***************************************************************************
 * ViaHeaderSetBoolField
 * ------------------------------------------------------------------------
 * General: Set boolean field to this Via header.
 *			The given boolean may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the boolean will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Bool value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderSetBoolField(
                                          IN RvSipViaHeaderHandle      hHeader,
										  IN RvSipViaHeaderFieldName   eField,
                                          IN RvBool                    fieldValue);

/***************************************************************************
 * ViaHeaderGetBoolField
 * ------------------------------------------------------------------------
 * General: Get a boolean field of this Via header.
 *			The boolean returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned boolean).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderGetBoolField(
                                          IN  RvSipViaHeaderHandle      hHeader,
										  IN  RvSipViaHeaderFieldName   eField,
                                          OUT RvBool*                   pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   WARNING HEADER                                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * WarningHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV WarningHeaderDestruct(IN RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * WarningHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Warning header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderSetStringField(
                                          IN RvSipWarningHeaderHandle    hHeader,
										  IN RvSipWarningHeaderFieldName eField,
                                          IN RvChar*                     buffer);

/***************************************************************************
 * WarningHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Warning 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderGetStringFieldLength(
                                    IN  RvSipWarningHeaderHandle          hHeader,
									IN  RvSipWarningHeaderFieldName       eField,
                                    OUT RvInt32*                          pLength);

/***************************************************************************
 * WarningHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Warning header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderGetStringField(
                                    IN  RvSipWarningHeaderHandle       hHeader,
									IN  RvSipWarningHeaderFieldName    eField,
                                    IN  RvChar*                        buffer,
                                    IN  RvInt32                        bufferLen,
                                    OUT RvInt32*                       actualLen);

/***************************************************************************
 * WarningHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: Set an integer field to this Warning header. The field being set is
 *          indicated by eField.
 *          The given Int32 may be converted into other types of integers (Int8,
 *          Int16, Uint32...), based on the field being set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader    - A handle to the header object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderSetIntField(
                                          IN RvSipWarningHeaderHandle      hHeader,
										  IN RvSipWarningHeaderFieldName   eField,
                                          IN RvInt32                       fieldValue);

/***************************************************************************
 * WarningHeaderGetIntField
 * ------------------------------------------------------------------------
 * General: Get an integer field of this Warning header. The field to get is
 *          indicated by eField.
 *          The returned Int32 may have been converted from other types of
 *          integers (Int8, Int16, Uint32...), based on the field being get.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader     - A handle to the header object.
 *          eField      - The enumeration value that indicates the field to retrieve.
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderGetIntField(
                                          IN  RvSipWarningHeaderHandle      hHeader,
										  IN  RvSipWarningHeaderFieldName   eField,
                                          OUT RvInt32*                      pFieldValue);

/*-----------------------------------------------------------------------*/
/*                   OTHER HEADER                                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * OtherHeaderDestruct
 * ------------------------------------------------------------------------
 * General: Destruct this header by freeing the page it lies on.
 *          Notice: This function can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader  - A handle to the header object to destruct.
 ***************************************************************************/
void RVCALLCONV OtherHeaderDestruct(IN RvSipOtherHeaderHandle hHeader);

/***************************************************************************
 * OtherHeaderSetStringField
 * ------------------------------------------------------------------------
 * General: Set a string field to this Other header. The field being set is
 *          indicated by eField.
 *          The given string may be converted into another type,
 *          based on the field being set (for example, if eField is actually an
 *          enumerative field, the string will be converted into the corresponding
 *          enumeration).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - A handle to the header object.
 *          eField  - The enumeration value that indicates the field being set.
 *          buffer  - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV OtherHeaderSetStringField(
                                          IN RvSipOtherHeaderHandle    hHeader,
										  IN RvSipOtherHeaderFieldName eField,
                                          IN RvChar*                   buffer);

/***************************************************************************
 * OtherHeaderGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: Evaluate the length of a selected string from this Other 
 *          header. The string field to evaluate is indicated by eField.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve
 *                      its length.
 * Output:  pLength   - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV OtherHeaderGetStringFieldLength(
                                    IN  RvSipOtherHeaderHandle          hHeader,
									IN  RvSipOtherHeaderFieldName       eField,
                                    OUT RvInt32*                        pLength);

/***************************************************************************
 * OtherHeaderGetStringField
 * ------------------------------------------------------------------------
 * General: Get a string field of this Other header. The field to get is
 *          indicated by eField.
 *          The string returned may have been converted from another type,
 *          based on the field being get (for example, if eField is actually an
 *          enumerative field, the enumeration value will be converted into the
 *          returned string).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - A handle to the header object.
 *          eField    - The enumeration value that indicates the field to retrieve.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV OtherHeaderGetStringField(
                                    IN  RvSipOtherHeaderHandle       hHeader,
									IN  RvSipOtherHeaderFieldName    eField,
                                    IN  RvChar*                      buffer,
                                    IN  RvInt32                      bufferLen,
                                    OUT RvInt32*                     actualLen);


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* HEADERFIELDDISPATCHER_H */

