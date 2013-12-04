/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        HeaderFieldDispatcher.c                             *
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


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "HeaderFieldDispatcher.h"
#include "_SipCommonConversions.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsgTypes.h"

#include "_SipAddress.h"
#include "_SipAcceptHeader.h"
#include "_SipAcceptEncodingHeader.h"
#include "_SipAcceptLanguageHeader.h"
#include "_SipAllowHeader.h"
#include "_SipAllowEventsHeader.h"
#include "_SipAuthenticationHeader.h"
#include "_SipAuthenticationInfoHeader.h"
#include "_SipAuthorizationHeader.h"
#include "_SipContentDispositionHeader.h"
#include "_SipContentTypeHeader.h"
#include "_SipContactHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipDateHeader.h"
#include "_SipEventHeader.h"
#include "_SipExpiresHeader.h"
#include "_SipInfoHeader.h"
#include "_SipPartyHeader.h"
#include "_SipRAckHeader.h"
#include "_SipRSeqHeader.h"
#include "_SipReasonHeader.h"
#include "_SipReferToHeader.h"
#include "_SipReplyToHeader.h"
#include "_SipRetryAfterHeader.h"
#include "_SipRouteHopHeader.h"
#include "_SipSubscriptionStateHeader.h"
#include "_SipTimestampHeader.h"
#include "_SipViaHeader.h"
#include "_SipWarningHeader.h"
#include "_SipOtherHeader.h"
	
/*#include <memory.h>*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_JSR32_SUPPORT

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV StringFieldCopy(
								IN  RvChar*                     strToCopy,
								IN  RvChar*                     buffer,
								IN  RvInt32                     bufferLen,
								OUT RvInt32*                    actualLen);

/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

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
void RVCALLCONV AcceptHeaderDestruct(IN RvSipAcceptHeaderHandle hHeader)
{
	HRPOOL                   hPool;
	HPAGE                    hPage;
	MsgAcceptHeader         *pHeader = (MsgAcceptHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"AcceptHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipAcceptHeaderGetPool(hHeader);
	hPage = SipAcceptHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                    buffer)
{
	switch (eField) {
	case RVSIP_ACCEPT_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType;
			
			if (NULL == buffer)
			{
				return RvSipAcceptHeaderSetMediaType(hHeader, RVSIP_MEDIATYPE_UNDEFINED, NULL);
			}
			
			/* Convert the retrieved enumeration into string */
			eMediaType = SipCommonConvertStrToEnumMediaType(buffer);
			return RvSipAcceptHeaderSetMediaType(hHeader, eMediaType, buffer);
		}
	case RVSIP_ACCEPT_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType;
			
			if (NULL == buffer)
			{
				return RvSipAcceptHeaderSetMediaSubType(hHeader, RVSIP_MEDIASUBTYPE_UNDEFINED, NULL);
			}
			
			/* Convert the retrieved enumeration into string */
			eMediaSubType = SipCommonConvertStrToEnumMediaSubType(buffer);
			return RvSipAcceptHeaderSetMediaSubType(hHeader, eMediaSubType, buffer);
		}
	case RVSIP_ACCEPT_FIELD_QVAL:
		{
			return RvSipAcceptHeaderSetQVal(hHeader, buffer);
		}
	case RVSIP_ACCEPT_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_ACCEPT_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                      pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_ACCEPT_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType = RvSipAcceptHeaderGetMediaType(hHeader);
			if (RVSIP_MEDIATYPE_OTHER != eMediaType &&
				RVSIP_MEDIATYPE_UNDEFINED != eMediaType)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAcceptHeaderGetStringLength(hHeader, RVSIP_ACCEPT_MEDIATYPE);
			}
			break;
		}
	case RVSIP_ACCEPT_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType = RvSipAcceptHeaderGetMediaSubType(hHeader);
			if (RVSIP_MEDIASUBTYPE_OTHER != eMediaSubType &&
				RVSIP_MEDIASUBTYPE_UNDEFINED != eMediaSubType)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAcceptHeaderGetStringLength(hHeader, RVSIP_ACCEPT_MEDIASUBTYPE);
			}
			break;
		}
	case RVSIP_ACCEPT_FIELD_QVAL:
		{
			*pLength = RvSipAcceptHeaderGetStringLength(hHeader, RVSIP_ACCEPT_QVAL);
			break;
		}
	case RVSIP_ACCEPT_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipAcceptHeaderGetStringLength(hHeader, RVSIP_ACCEPT_OTHER_PARAMS);
			break;
		}
	case RVSIP_ACCEPT_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAcceptHeaderGetStringLength(hHeader, RVSIP_ACCEPT_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                      actualLen)
{
	switch (eField) {
	case RVSIP_ACCEPT_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType = RvSipAcceptHeaderGetMediaType(hHeader);
			if (RVSIP_MEDIATYPE_OTHER == eMediaType)
			{
			/* The OTHER enumeration indicates that there is a string 
				media-type set to this allowed header */
				return RvSipAcceptHeaderGetStrMediaType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_MEDIATYPE_UNDEFINED != eMediaType)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strMediaType = SipCommonConvertEnumToStrMediaType(eMediaType);
				return StringFieldCopy(strMediaType, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_ACCEPT_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType = RvSipAcceptHeaderGetMediaSubType(hHeader);
			if (RVSIP_MEDIASUBTYPE_OTHER == eMediaSubType)
			{
			/* The OTHER enumeration indicates that there is a string 
				media-subtype set to this allowed header */
				return RvSipAcceptHeaderGetStrMediaSubType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_MEDIASUBTYPE_UNDEFINED != eMediaSubType)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strMediaSubType = SipCommonConvertEnumToStrMediaSubType(eMediaSubType);
				return StringFieldCopy(strMediaSubType, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_ACCEPT_FIELD_QVAL:
		{
			return RvSipAcceptHeaderGetQVal(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
void RVCALLCONV AcceptEncodingHeaderDestruct(IN RvSipAcceptEncodingHeaderHandle hHeader)
{
	HRPOOL                   hPool;
	HPAGE                    hPage;
	MsgAcceptEncodingHeader *pHeader = (MsgAcceptEncodingHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"AcceptEncodingHeaderDestruct - Destructing header 0x%p",
		hHeader));	
	
	hPool = SipAcceptEncodingHeaderGetPool(hHeader);
	hPage = SipAcceptEncodingHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                            buffer)
{
	switch (eField) {
	case RVSIP_ACCEPT_ENCODING_FIELD_CODING:
		{
			return RvSipAcceptEncodingHeaderSetCoding(hHeader, buffer);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_QVAL:
		{
			return RvSipAcceptEncodingHeaderSetQVal(hHeader, buffer);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptEncodingHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptEncodingHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                              pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_ACCEPT_ENCODING_FIELD_CODING:
		{
			*pLength = RvSipAcceptEncodingHeaderGetStringLength(hHeader, RVSIP_ACCEPT_ENCODING_CODING);
			break;
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_QVAL:
		{
			*pLength = RvSipAcceptEncodingHeaderGetStringLength(hHeader, RVSIP_ACCEPT_ENCODING_QVAL);
			break;
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipAcceptEncodingHeaderGetStringLength(hHeader, RVSIP_ACCEPT_ENCODING_OTHER_PARAMS);
			break;
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAcceptEncodingHeaderGetStringLength(hHeader, RVSIP_ACCEPT_ENCODING_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                              actualLen)
{
	switch (eField) {
	case RVSIP_ACCEPT_ENCODING_FIELD_CODING:
		{
			return RvSipAcceptEncodingHeaderGetCoding(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_QVAL:
		{
			return RvSipAcceptEncodingHeaderGetQVal(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptEncodingHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_ENCODING_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptEncodingHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
void RVCALLCONV AcceptLanguageHeaderDestruct(IN RvSipAcceptLanguageHeaderHandle hHeader)
{
	HRPOOL                   hPool;
	HPAGE                    hPage;
	MsgAcceptLanguageHeader *pHeader = (MsgAcceptLanguageHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"AcceptLanguageHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipAcceptLanguageHeaderGetPool(hHeader);
	hPage = SipAcceptLanguageHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                            buffer)
{
	switch (eField) {
	case RVSIP_ACCEPT_LANGUAGE_FIELD_LANGUAGE:
		{
			return RvSipAcceptLanguageHeaderSetLanguage(hHeader, buffer);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_QVAL:
		{
			return RvSipAcceptLanguageHeaderSetQVal(hHeader, buffer);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptLanguageHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptLanguageHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                              pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_ACCEPT_LANGUAGE_FIELD_LANGUAGE:
		{
			*pLength = RvSipAcceptLanguageHeaderGetStringLength(hHeader, RVSIP_ACCEPT_LANGUAGE_LANGUAGE);
			break;
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_QVAL:
		{
			*pLength = RvSipAcceptLanguageHeaderGetStringLength(hHeader, RVSIP_ACCEPT_LANGUAGE_QVAL);
			break;
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipAcceptLanguageHeaderGetStringLength(hHeader, RVSIP_ACCEPT_LANGUAGE_OTHER_PARAMS);
			break;
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAcceptLanguageHeaderGetStringLength(hHeader, RVSIP_ACCEPT_LANGUAGE_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                              actualLen)
{
	switch (eField) {
	case RVSIP_ACCEPT_LANGUAGE_FIELD_LANGUAGE:
		{
			return RvSipAcceptLanguageHeaderGetLanguage(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_QVAL:
		{
			return RvSipAcceptLanguageHeaderGetQVal(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_OTHER_PARAMS:
		{
			return RvSipAcceptLanguageHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ACCEPT_LANGUAGE_FIELD_BAD_SYNTAX:
		{
			return RvSipAcceptLanguageHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
void RVCALLCONV AllowEventsHeaderDestruct(IN RvSipAllowEventsHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgAllowEventsHeader *pHeader = (MsgAllowEventsHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"AllowEventsHeaderDestruct - Destructing header 0x%p",
		hHeader));	
	
	hPool = SipAllowEventsHeaderGetPool(hHeader);
	hPage = SipAllowEventsHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                        IN RvChar*                         buffer)
{
	switch(eField) {
	case RVSIP_ALLOW_EVENTS_FIELD_EVENT_TYPE:
	{		
		RvStatus rv;
		
		if (NULL == buffer)
		{
			rv = RvSipAllowEventsHeaderSetEventPackage(hHeader, NULL);
			if (RV_OK != rv)
			{
				return rv;
			}
			return RvSipAllowEventsHeaderSetEventTemplate(hHeader, NULL);
		}
		
		/* JSR32 supplies package and template together, i.e. event-type */
		return RvSipAllowEventsHeaderParseValue(hHeader, buffer);
	}
	case RVSIP_ALLOW_EVENTS_FIELD_BAD_SYNTAX:
		return RvSipAllowEventsHeaderSetStrBadSyntax(hHeader, buffer);
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                           pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_ALLOW_EVENTS_FIELD_EVENT_TYPE:
		{
			RvInt32   packageLen;
			RvInt32   templateLen;
			
			packageLen  = RvSipAllowEventsHeaderGetStringLength(hHeader ,RVSIP_ALLOW_EVENTS_EVENT_PACKAGE);
			templateLen = RvSipAllowEventsHeaderGetStringLength(hHeader ,RVSIP_ALLOW_EVENTS_EVENT_TEMPLATE);
			
			/* Notice that both packageLen and templateLen have extra 1 for '\0'
			If they both exist, one of the '\0' will be used for the extra '.' 
			that separates them. If not, the template length is 0 and therefore 
			the package length is exactly what we are looking for */
			
			*pLength = packageLen + templateLen;
		}
		break;
	case RVSIP_ALLOW_EVENTS_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAllowEventsHeaderGetStringLength(hHeader ,RVSIP_ALLOW_EVENTS_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                           actualLen)
{
	switch(eField) {
	case RVSIP_ALLOW_EVENTS_FIELD_EVENT_TYPE:
		{
			RvInt32   packageLen;
			RvInt32   templateLen;
			RvInt32   tempActualLen;
			RvStatus  rv;
			
			*actualLen = 0;
			
			packageLen  = RvSipAllowEventsHeaderGetStringLength(hHeader ,RVSIP_ALLOW_EVENTS_EVENT_PACKAGE);
			templateLen = RvSipAllowEventsHeaderGetStringLength(hHeader ,RVSIP_ALLOW_EVENTS_EVENT_TEMPLATE);
			/* Notice that both packageLen and templateLen have extra 1 for '\0'
			One of them will be used for the extra '.' that separates them. */
			
			if (packageLen + templateLen > bufferLen)
			{
				return RV_ERROR_BADPARAM;
			}
			
			/* JSR32 supplies package and template together, i.e. event-type */
			rv = RvSipAllowEventsHeaderGetEventPackage(hHeader, buffer, (RvUint)bufferLen, (RvUint*)&tempActualLen);
			if (RV_OK != rv)
			{
				return rv;
			}
			*actualLen = tempActualLen;
			
			if (templateLen > 0)
			{
				buffer[tempActualLen-1] = '.';
				rv = RvSipAllowEventsHeaderGetEventTemplate(hHeader, buffer+tempActualLen, (RvUint)(bufferLen-tempActualLen), (RvUint*)&tempActualLen);
				if (RV_OK != rv)
				{
					return rv;
				}
				*actualLen += tempActualLen;
			}
		}
		break;
	case RVSIP_ALLOW_EVENTS_FIELD_BAD_SYNTAX:
		{
			return RvSipAllowEventsHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV AllowHeaderDestruct(IN RvSipAllowHeaderHandle hHeader)
{
	HRPOOL				  hPool;
	HPAGE				  hPage;
	MsgAllowHeader       *pHeader = (MsgAllowHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"AllowHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipAllowHeaderGetPool(hHeader);
	hPage = SipAllowHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                   buffer)
{
	switch(eField) {
	case RVSIP_ALLOW_FIELD_METHOD:
		{
			RvSipMethodType eMethodType;
			
			if (NULL == buffer)
			{
				return RvSipAllowHeaderSetMethodType(hHeader, RVSIP_METHOD_UNDEFINED, NULL);
			}
			
			/* Convert the string method into enumeration */
			eMethodType = SipCommonConvertStrToEnumMethodType(buffer);
			
			return RvSipAllowHeaderSetMethodType(hHeader, eMethodType, buffer);
		}
		break;
	case RVSIP_ALLOW_FIELD_BAD_SYNTAX:
		{
			return RvSipAllowHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                     pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_ALLOW_FIELD_METHOD:
		{
			RvSipMethodType   eMethodType;
			
			eMethodType = RvSipAllowHeaderGetMethodType(hHeader);
			
			if (eMethodType != RVSIP_METHOD_OTHER &&
				eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipAllowHeaderGetStringLength(hHeader, RVSIP_ALLOW_METHOD);
			}
		}
		break;
	case RVSIP_ALLOW_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipAllowHeaderGetStringLength(hHeader, RVSIP_ALLOW_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                     actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_ALLOW_FIELD_METHOD:
		{
			RvSipMethodType   eMethodType;
			RvChar           *strMethodType;
			
			eMethodType = RvSipAllowHeaderGetMethodType(hHeader);
			
			if (eMethodType == RVSIP_METHOD_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				method set to this allowed header */
				return RvSipAllowHeaderGetStrMethodType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* First convert enueration method into string and then set it */
				strMethodType = SipCommonConvertEnumToStrMethodType(eMethodType);
				return StringFieldCopy(strMethodType, buffer, bufferLen, actualLen);
			}
		}
		break;
	case RVSIP_ALLOW_FIELD_BAD_SYNTAX:
		{
			return RvSipAllowHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV AuthenticationHeaderDestruct(IN RvSipAuthenticationHeaderHandle hHeader)
{
	HRPOOL						   hPool;
	HPAGE						   hPage;
	MsgAuthenticationHeader       *pHeader = (MsgAuthenticationHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"AuthenticationHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipAuthenticationHeaderGetPool(hHeader);
	hPage = SipAuthenticationHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                            buffer)
{
	switch (eField) {
	case RVSIP_AUTHENTICATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme;

			if (NULL == buffer)
			{
				return RvSipAuthenticationHeaderSetAuthScheme(hHeader, RVSIP_AUTH_SCHEME_UNDEFINED, NULL);
			}

			/* First convert string into enumeration and then set */
			eAuthScheme = SipCommonConvertStrToEnumAuthScheme(buffer);
			return RvSipAuthenticationHeaderSetAuthScheme(hHeader, eAuthScheme, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_REALM:
		{
			return RvSipAuthenticationHeaderSetRealm(hHeader, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_DOMAIN:
		{
			return RvSipAuthenticationHeaderSetDomain(hHeader, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_NONCE:
		{
			return RvSipAuthenticationHeaderSetNonce(hHeader, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_OPAQUE:
		{
			return RvSipAuthenticationHeaderSetOpaque(hHeader, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm;

			if (NULL == buffer)
			{
				return RvSipAuthenticationHeaderSetAuthAlgorithm(hHeader, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL);
			}

			/* First convert string into enumeration and then set */
			eAuthAlgorithm = SipCommonConvertStrToEnumAuthAlgorithm(buffer);
			return RvSipAuthenticationHeaderSetAuthAlgorithm(hHeader, eAuthAlgorithm, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption;
			RvStatus           rv;

			if (NULL == buffer)
			{
				RvSipAuthenticationHeaderSetQopOption(hHeader, RVSIP_AUTH_QOP_UNDEFINED);
				RvSipAuthenticationHeaderSetStrQop(hHeader, NULL);
				return RV_OK;
			}

			/* First convert string into enumeration and then set */
			eAuthQopOption = SipCommonConvertStrToEnumAuthQopOption(buffer);
			rv = RvSipAuthenticationHeaderSetQopOption(hHeader, eAuthQopOption);
			if (RV_OK == rv && RVSIP_AUTH_QOP_OTHER == eAuthQopOption)
			{
				rv = RvSipAuthenticationHeaderSetStrQop(hHeader, buffer);
			}
			return rv;
		}
	case RVSIP_AUTHENTICATION_FIELD_OTHER_PARAMS:
		{
			return RvSipAuthenticationHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_AUTHENTICATION_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthenticationHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                              pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_AUTHENTICATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme = RvSipAuthenticationHeaderGetAuthScheme(hHeader);
			if (RVSIP_AUTH_SCHEME_OTHER != eAuthScheme &&
				RVSIP_AUTH_SCHEME_UNDEFINED != eAuthScheme)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_AUTHSCHEME);
			}
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_REALM:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_REALM);
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_DOMAIN:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_DOMAIN);
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_NONCE:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_NONCE);
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_OPAQUE:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_OPAQUE);
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm = RvSipAuthenticationHeaderGetAuthAlgorithm(hHeader);
			if (RVSIP_AUTH_ALGORITHM_OTHER != eAuthAlgorithm &&
				RVSIP_AUTH_ALGORITHM_UNDEFINED != eAuthAlgorithm)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_ALGORITHM);
			}
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthenticationHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_OTHER != eAuthQopOption &&
				RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_QOP);
			}
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_OTHER_PARAMS);
			break;
		}
	case RVSIP_AUTHENTICATION_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAuthenticationHeaderGetStringLength(hHeader, RVSIP_AUTHENTICATION_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                              actualLen)
{
	switch (eField) {
	case RVSIP_AUTHENTICATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme = RvSipAuthenticationHeaderGetAuthScheme(hHeader);
			if (RVSIP_AUTH_SCHEME_OTHER == eAuthScheme)
			{
				/* The OTHER enumeration indicates that there is a string 
				scheme set to this allowed header */
				return RvSipAuthenticationHeaderGetStrAuthScheme(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_SCHEME_UNDEFINED != eAuthScheme)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthScheme = SipCommonConvertEnumToStrAuthScheme(eAuthScheme);
				return StringFieldCopy(strAuthScheme, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHENTICATION_FIELD_REALM:
		{
			return RvSipAuthenticationHeaderGetRealm(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHENTICATION_FIELD_DOMAIN:
		{
			return RvSipAuthenticationHeaderGetDomain(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHENTICATION_FIELD_NONCE:
		{
			return RvSipAuthenticationHeaderGetNonce(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHENTICATION_FIELD_OPAQUE:
		{
			return RvSipAuthenticationHeaderGetOpaque(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHENTICATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm = RvSipAuthenticationHeaderGetAuthAlgorithm(hHeader);
			if (RVSIP_AUTH_ALGORITHM_OTHER == eAuthAlgorithm)
			{
				/* The OTHER enumeration indicates that there is a string 
				algorithm set to this allowed header */
				return RvSipAuthenticationHeaderGetStrAuthAlgorithm(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_ALGORITHM_UNDEFINED != eAuthAlgorithm)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthAlgorithm = SipCommonConvertEnumToStrAuthAlgorithm(eAuthAlgorithm);
				return StringFieldCopy(strAuthAlgorithm, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHENTICATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthenticationHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_OTHER == eAuthQopOption)
			{
			    /* The OTHER enumeration indicates that there is a string 
				qop set to this allowed header */
				return RvSipAuthenticationHeaderGetStrQop(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthQopOption = SipCommonConvertEnumToStrAuthQopOption(eAuthQopOption);
				return StringFieldCopy(strAuthQopOption, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHENTICATION_FIELD_OTHER_PARAMS:
		{
			return RvSipAuthenticationHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHENTICATION_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthenticationHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          IN RvBool                               fieldValue)
{
	RvSipAuthStale eStale;

	if (eField != RVSIP_AUTHENTICATION_FIELD_STALE)
	{
		return RV_ERROR_BADPARAM;
	}

	eStale = SipCommonConvertBoolToEnumAuthStale(fieldValue);
	return RvSipAuthenticationHeaderSetStale(hHeader, eStale);
}

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
                                          OUT RvBool*                              pFieldValue)
{
	RvSipAuthStale eStale;

	if (eField != RVSIP_AUTHENTICATION_FIELD_STALE)
	{
		return RV_ERROR_BADPARAM;
	}

	eStale = RvSipAuthenticationHeaderGetStale(hHeader);
	*pFieldValue = SipCommonConvertEnumToBoolAuthStale(eStale);

	return RV_OK;
}

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
void RVCALLCONV AuthenticationInfoHeaderDestruct(IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
	HRPOOL						   hPool;
	HPAGE						   hPage;
	MsgAuthenticationInfoHeader       *pHeader = (MsgAuthenticationInfoHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"AuthenticationInfoHeaderDestruct - Destructing header 0x%p",
			hHeader));	

	hPool = SipAuthenticationInfoHeaderGetPool(hHeader);
	hPage = SipAuthenticationInfoHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                                buffer)
{
	switch (eField) {
	case RVSIP_AUTH_INFO_FIELD_NEXT_NONCE:
		{
			return RvSipAuthenticationInfoHeaderSetNextNonce(hHeader, buffer);
		}
	case RVSIP_AUTH_INFO_FIELD_CNONCE:
		{
			return RvSipAuthenticationInfoHeaderSetCNonce(hHeader, buffer);
		}
	case RVSIP_AUTH_INFO_FIELD_RSP_AUTH:
		{
			return RvSipAuthenticationInfoHeaderSetResponseAuth(hHeader, buffer);
		}
	case RVSIP_AUTH_INFO_FIELD_MSG_QOP:
		{
			RvSipAuthQopOption eAuthQopOption;
			
			if (NULL == buffer)
			{
				return RvSipAuthenticationInfoHeaderSetQopOption(hHeader, RVSIP_AUTH_QOP_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eAuthQopOption = SipCommonConvertStrToEnumAuthQopOption(buffer);
			return RvSipAuthenticationInfoHeaderSetQopOption(hHeader, eAuthQopOption, buffer);
		}
	case RVSIP_AUTH_INFO_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthenticationInfoHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                                  pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_AUTH_INFO_FIELD_NEXT_NONCE:
		{
			*pLength = RvSipAuthenticationInfoHeaderGetStringLength(hHeader, RVSIP_AUTH_INFO_NEXT_NONCE);
			break;
		}
	case RVSIP_AUTH_INFO_FIELD_CNONCE:
		{
			*pLength = RvSipAuthenticationInfoHeaderGetStringLength(hHeader, RVSIP_AUTH_INFO_CNONCE);
			break;
		}
	case RVSIP_AUTH_INFO_FIELD_RSP_AUTH:
		{
			*pLength = RvSipAuthenticationInfoHeaderGetStringLength(hHeader, RVSIP_AUTH_INFO_RSP_AUTH);
			break;
		}
	case RVSIP_AUTH_INFO_FIELD_MSG_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthenticationInfoHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_OTHER != eAuthQopOption &&
				RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthenticationInfoHeaderGetStringLength(hHeader, RVSIP_AUTH_INFO_MSG_QOP);
			}
			break;
		}
	case RVSIP_AUTH_INFO_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAuthenticationInfoHeaderGetStringLength(hHeader, RVSIP_AUTH_INFO_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                                  actualLen)
{
	switch (eField) {
	case RVSIP_AUTH_INFO_FIELD_NEXT_NONCE:
		{
			return RvSipAuthenticationInfoHeaderGetNextNonce(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTH_INFO_FIELD_CNONCE:
		{
			return RvSipAuthenticationInfoHeaderGetCNonce(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTH_INFO_FIELD_RSP_AUTH:
		{
			return RvSipAuthenticationInfoHeaderGetResponseAuth(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTH_INFO_FIELD_MSG_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthenticationInfoHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_OTHER == eAuthQopOption)
			{
				/* The OTHER enumeration indicates that there is a string 
				qop-option set to this allowed header */
				return RvSipAuthenticationInfoHeaderGetStrQop(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthQopOption = SipCommonConvertEnumToStrAuthQopOption(eAuthQopOption);
				return StringFieldCopy(strAuthQopOption, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTH_INFO_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthenticationInfoHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          IN RvInt32                                  fieldValue)
{
	if (eField != RVSIP_AUTH_INFO_FIELD_NONCE_COUNT)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipAuthenticationInfoHeaderSetNonceCount(hHeader, fieldValue);
}

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
                                          OUT RvInt32*                                 pFieldValue)
{
	if (eField != RVSIP_AUTH_INFO_FIELD_NONCE_COUNT)
	{
		return RV_ERROR_BADPARAM;
	}

	*pFieldValue = RvSipAuthenticationInfoHeaderGetNonceCount(hHeader);

	return RV_OK;
}

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
void RVCALLCONV AuthorizationHeaderDestruct(IN RvSipAuthorizationHeaderHandle hHeader)
{
	HRPOOL						   hPool;
	HPAGE						   hPage;
	MsgAuthorizationHeader        *pHeader = (MsgAuthorizationHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"AuthorizationHeaderDestruct - Destructing header 0x%p",
			hHeader));	

	hPool = SipAuthorizationHeaderGetPool(hHeader);
	hPage = SipAuthorizationHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                           buffer)
{
	switch (eField) {
	case RVSIP_AUTHORIZATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme;

			if (NULL == buffer)
			{
				return RvSipAuthorizationHeaderSetAuthScheme(hHeader, RVSIP_AUTH_SCHEME_UNDEFINED, NULL);
			}

			/* First convert string into enumeration and then set */
			eAuthScheme = SipCommonConvertStrToEnumAuthScheme(buffer);
			return RvSipAuthorizationHeaderSetAuthScheme(hHeader, eAuthScheme, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_REALM:
		{
			return RvSipAuthorizationHeaderSetRealm(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_NONCE:
		{
			return RvSipAuthorizationHeaderSetNonce(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_CNONCE:
		{
			return RvSipAuthorizationHeaderSetCNonce(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_OPAQUE:
		{
			return RvSipAuthorizationHeaderSetOpaque(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_USERNAME:
		{
			return RvSipAuthorizationHeaderSetUserName(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_RESPONSE:
		{
			return RvSipAuthorizationHeaderSetResponse(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm;
			
			if (buffer == NULL)
			{
				return RvSipAuthorizationHeaderSetAuthAlgorithm(hHeader, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eAuthAlgorithm = SipCommonConvertStrToEnumAuthAlgorithm(buffer);
			return RvSipAuthorizationHeaderSetAuthAlgorithm(hHeader, eAuthAlgorithm, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption;
			
			if (NULL == buffer)
			{
				return RvSipAuthorizationHeaderSetQopOption(hHeader, RVSIP_AUTH_QOP_UNDEFINED);
			}
			
			eAuthQopOption = SipCommonConvertStrToEnumAuthQopOption(buffer);
			if (eAuthQopOption == RVSIP_AUTH_QOP_OTHER)
			{
				/* Qop must be enumerated */
				return RV_ERROR_BADPARAM;
			}
			return RvSipAuthorizationHeaderSetQopOption(hHeader, eAuthQopOption);
		}
	case RVSIP_AUTHORIZATION_FIELD_OTHER_PARAMS:
		{
			return RvSipAuthorizationHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_AUTHORIZATION_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthorizationHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                             pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_AUTHORIZATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme = RvSipAuthorizationHeaderGetAuthScheme(hHeader);
			if (RVSIP_AUTH_SCHEME_OTHER != eAuthScheme &&
				RVSIP_AUTH_SCHEME_UNDEFINED != eAuthScheme)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_FIELD_AUTHSCHEME);
			}
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_REALM:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_REALM);
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_NONCE:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_NONCE);
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_CNONCE:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_CNONCE);
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_OPAQUE:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_OPAQUE);
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_USERNAME:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_USERNAME);
			break;
		}		
	case RVSIP_AUTHORIZATION_FIELD_RESPONSE:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_RESPONSE);
			break;
		}		
	case RVSIP_AUTHORIZATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm = RvSipAuthorizationHeaderGetAuthAlgorithm(hHeader);
			if (RVSIP_AUTH_ALGORITHM_OTHER != eAuthAlgorithm &&
				RVSIP_AUTH_ALGORITHM_UNDEFINED != eAuthAlgorithm)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_ALGORITHM);
			}
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthorizationHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_OTHER != eAuthQopOption &&
				RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_QOP);
			}
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_OTHER_PARAMS);
			break;
		}
	case RVSIP_AUTHORIZATION_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipAuthorizationHeaderGetStringLength(hHeader, RVSIP_AUTHORIZATION_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                              actualLen)
{
	switch (eField) {
	case RVSIP_AUTHORIZATION_FIELD_AUTHSCHEME:
		{
			RvSipAuthScheme eAuthScheme = RvSipAuthorizationHeaderGetAuthScheme(hHeader);
			if (RVSIP_AUTH_SCHEME_OTHER == eAuthScheme)
			{
			/* The OTHER enumeration indicates that there is a string 
				sceme set to this allowed header */
				return RvSipAuthorizationHeaderGetStrAuthScheme(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_SCHEME_UNDEFINED != eAuthScheme)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthScheme = SipCommonConvertEnumToStrAuthScheme(eAuthScheme);
				return StringFieldCopy(strAuthScheme, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHORIZATION_FIELD_REALM:
		{
			return RvSipAuthorizationHeaderGetRealm(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_NONCE:
		{
			return RvSipAuthorizationHeaderGetNonce(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_CNONCE:
		{
			return RvSipAuthorizationHeaderGetCNonce(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_OPAQUE:
		{
			return RvSipAuthorizationHeaderGetOpaque(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_USERNAME:
		{
			return RvSipAuthorizationHeaderGetUserName(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_RESPONSE:
		{
			return RvSipAuthorizationHeaderGetResponse(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_ALGORITHM:
		{
			RvSipAuthAlgorithm eAuthAlgorithm = RvSipAuthorizationHeaderGetAuthAlgorithm(hHeader);
			if (RVSIP_AUTH_ALGORITHM_OTHER == eAuthAlgorithm)
			{
			/* The OTHER enumeration indicates that there is a string 
				algorithm set to this allowed header */
				return RvSipAuthorizationHeaderGetStrAuthAlgorithm(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_AUTH_ALGORITHM_UNDEFINED != eAuthAlgorithm)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthAlgorithm = SipCommonConvertEnumToStrAuthAlgorithm(eAuthAlgorithm);
				return StringFieldCopy(strAuthAlgorithm, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHORIZATION_FIELD_QOP:
		{
			RvSipAuthQopOption eAuthQopOption = RvSipAuthorizationHeaderGetQopOption(hHeader);
			if (RVSIP_AUTH_QOP_UNDEFINED != eAuthQopOption)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strAuthQopOption = SipCommonConvertEnumToStrAuthQopOption(eAuthQopOption);
				return StringFieldCopy(strAuthQopOption, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_AUTHORIZATION_FIELD_OTHER_PARAMS:
		{
			return RvSipAuthorizationHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_AUTHORIZATION_FIELD_BAD_SYNTAX:
		{
			return RvSipAuthorizationHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}


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
                                          INOUT RvSipAddressHandle                *phAddress)
{
	RvStatus rv;

	if (RVSIP_AUTHORIZATION_FIELD_URI != eField)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipAuthorizationHeaderSetDigestUri(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipAuthorizationHeaderGetDigestUri(hHeader);

	return RV_OK;
}

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
                                    OUT RvSipAddressHandle                  *phAddress)
{
	if (RVSIP_AUTHORIZATION_FIELD_URI != eField)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipAuthorizationHeaderGetDigestUri(hHeader);

	return RV_OK;
}

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
                                          IN RvInt32                             fieldValue)
{
	if (eField != RVSIP_AUTHORIZATION_FIELD_NONCE_COUNT)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipAuthorizationHeaderSetNonceCount(hHeader, fieldValue);
}

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
                                          OUT RvInt32*                            pFieldValue)
{
	if (eField != RVSIP_AUTHORIZATION_FIELD_NONCE_COUNT)
	{
		return RV_ERROR_BADPARAM;
	}

	*pFieldValue = RvSipAuthorizationHeaderGetNonceCount(hHeader);

	return RV_OK;
}

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
void RVCALLCONV ContentDispositionHeaderDestruct(IN RvSipContentDispositionHeaderHandle hHeader)
{
	HRPOOL						        hPool;
	HPAGE						        hPage;
	MsgContentDispositionHeader        *pHeader = (MsgContentDispositionHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"ContentDispositionHeaderDestruct - Destructing header 0x%p",
			hHeader));	

	hPool = SipContentDispositionHeaderGetPool(hHeader);
	hPage = SipContentDispositionHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                                buffer)
{
	switch (eField) {
	case RVSIP_CONTENT_DISPOSITION_FIELD_TYPE:
		{
			RvSipDispositionType eDisposition;
			
			if (NULL == buffer)
			{
				return RvSipContentDispositionHeaderSetType(hHeader, RVSIP_DISPOSITIONTYPE_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eDisposition = SipCommonConvertStrToEnumDispositionType(buffer);
			return RvSipContentDispositionHeaderSetType(hHeader, eDisposition, buffer);
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_HANDLING:
		{
			RvSipDispositionHandling eHandling;
			
			if (NULL == buffer)
			{
				return RvSipContentDispositionHeaderSetHandling(hHeader, RVSIP_DISPOSITIONHANDLING_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eHandling = SipCommonConvertStrToEnumDispositionHandlingType(buffer);
			return RvSipContentDispositionHeaderSetHandling(hHeader, eHandling, buffer);
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_OTHER_PARAMS:
		{
			return RvSipContentDispositionHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_BAD_SYNTAX:
		{
			return RvSipContentDispositionHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                                  pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_CONTENT_DISPOSITION_FIELD_TYPE:
		{
			RvSipDispositionType eDisposition = RvSipContentDispositionHeaderGetType(hHeader);
			if (RVSIP_DISPOSITIONTYPE_OTHER != eDisposition &&
				RVSIP_DISPOSITIONTYPE_UNDEFINED != eDisposition)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipContentDispositionHeaderGetStringLength(hHeader, RVSIP_CONTENT_DISPOSITION_TYPE);
			}
			break;
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_HANDLING:
		{
			RvSipDispositionHandling eHandling = RvSipContentDispositionHeaderGetHandling(hHeader);
			if (RVSIP_DISPOSITIONHANDLING_OTHER != eHandling &&
				RVSIP_DISPOSITIONHANDLING_UNDEFINED != eHandling)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipContentDispositionHeaderGetStringLength(hHeader, RVSIP_CONTENT_DISPOSITION_HANDLING);
			}
			break;
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipContentDispositionHeaderGetStringLength(hHeader, RVSIP_CONTENT_DISPOSITION_OTHER_PARAMS);
			break;
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipContentDispositionHeaderGetStringLength(hHeader, RVSIP_CONTENT_DISPOSITION_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                                   actualLen)
{
	switch (eField) {
	case RVSIP_CONTENT_DISPOSITION_FIELD_TYPE:
		{
			RvSipDispositionType eDisposition = RvSipContentDispositionHeaderGetType(hHeader);
			if (RVSIP_DISPOSITIONTYPE_OTHER == eDisposition)
			{
			/* The OTHER enumeration indicates that there is a string 
				disposition-type set to this allowed header */
				return RvSipContentDispositionHeaderGetStrType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_DISPOSITIONTYPE_UNDEFINED != eDisposition)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strDisposition = SipCommonConvertEnumToStrDispositionType(eDisposition);
				return StringFieldCopy(strDisposition, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_HANDLING:
		{
			RvSipDispositionHandling eHandling = RvSipContentDispositionHeaderGetHandling(hHeader);
			if (RVSIP_DISPOSITIONHANDLING_OTHER == eHandling)
			{
			/* The OTHER enumeration indicates that there is a string 
				disposition-handling set to this allowed header */
				return RvSipContentDispositionHeaderGetStrHandling(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_DISPOSITIONHANDLING_UNDEFINED != eHandling)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strHandling = SipCommonConvertEnumToStrDispositionHandlingType(eHandling);
				return StringFieldCopy(strHandling, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_OTHER_PARAMS:
		{
			return RvSipContentDispositionHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTENT_DISPOSITION_FIELD_BAD_SYNTAX:
		{
			return RvSipContentDispositionHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}	
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
void RVCALLCONV ContentTypeHeaderDestruct(IN RvSipContentTypeHeaderHandle hHeader)
{
	HRPOOL						        hPool;
	HPAGE						        hPage;
	MsgContentTypeHeader               *pHeader = (MsgContentTypeHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"ContentTypeHeaderDestruct - Destructing header 0x%p",
			hHeader));	

	hPool = SipContentTypeHeaderGetPool(hHeader);
	hPage = SipContentTypeHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                         buffer)
{
	switch (eField) {
	case RVSIP_CONTENT_TYPE_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType;
			
			if (NULL == buffer)
			{
				return RvSipContentTypeHeaderSetMediaType(hHeader, RVSIP_MEDIATYPE_UNDEFINED, NULL);
			}
			
			/* Convert the retrieved enumeration into string */
			eMediaType = SipCommonConvertStrToEnumMediaType(buffer);
			return RvSipContentTypeHeaderSetMediaType(hHeader, eMediaType, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType;
			
			if (NULL == buffer)
			{
				return RvSipContentTypeHeaderSetMediaSubType(hHeader, RVSIP_MEDIASUBTYPE_UNDEFINED, NULL);
			}
			
			/* Convert the retrieved enumeration into string */
			eMediaSubType = SipCommonConvertStrToEnumMediaSubType(buffer);
			return RvSipContentTypeHeaderSetMediaSubType(hHeader, eMediaSubType, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_BOUNDARY:
		{
			return RvSipContentTypeHeaderSetBoundary(hHeader, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_VERSION:
		{
			return RvSipContentTypeHeaderSetVersion(hHeader, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_BASE:
		{
			return RvSipContentTypeHeaderSetBase(hHeader, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_OTHER_PARAMS:
		{
			return RvSipContentTypeHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_CONTENT_TYPE_FIELD_BAD_SYNTAX:
		{
			return RvSipContentTypeHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                           pLength)
{
	*pLength = 0;
	
	switch (eField) {
	case RVSIP_CONTENT_TYPE_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType = RvSipContentTypeHeaderGetMediaType(hHeader);
			if (RVSIP_MEDIATYPE_OTHER != eMediaType &&
				RVSIP_MEDIATYPE_UNDEFINED != eMediaType)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_MEDIATYPE);
			}
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType = RvSipContentTypeHeaderGetMediaSubType(hHeader);
			if (RVSIP_MEDIASUBTYPE_OTHER != eMediaSubType &&
				RVSIP_MEDIASUBTYPE_UNDEFINED != eMediaSubType)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_MEDIASUBTYPE);
			}
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_BOUNDARY:
		{
			*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_BOUNDARY);
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_VERSION:
		{
			*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_VERSION);
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_BASE:
		{
			*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_BASE);
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_OTHER_PARAMS:
		{
			*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_OTHER_PARAMS);
			break;
		}
	case RVSIP_CONTENT_TYPE_FIELD_BAD_SYNTAX:
		{
			*pLength = RvSipContentTypeHeaderGetStringLength(hHeader, RVSIP_CONTENT_TYPE_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                                   actualLen)
{
	switch (eField) {
	case RVSIP_CONTENT_TYPE_FIELD_MEDIATYPE:
		{
			RvSipMediaType eMediaType = RvSipContentTypeHeaderGetMediaType(hHeader);
			if (RVSIP_MEDIATYPE_OTHER == eMediaType)
			{
			/* The OTHER enumeration indicates that there is a string 
				media-type set to this allowed header */
				return RvSipContentTypeHeaderGetStrMediaType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_MEDIATYPE_UNDEFINED != eMediaType)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strMediaType = SipCommonConvertEnumToStrMediaType(eMediaType);
				return StringFieldCopy(strMediaType, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_CONTENT_TYPE_FIELD_MEDIASUBTYPE:
		{
			RvSipMediaSubType eMediaSubType = RvSipContentTypeHeaderGetMediaSubType(hHeader);
			if (RVSIP_MEDIASUBTYPE_OTHER == eMediaSubType)
			{
			/* The OTHER enumeration indicates that there is a string 
				media-subtype set to this allowed header */
				return RvSipContentTypeHeaderGetStrMediaSubType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_MEDIASUBTYPE_UNDEFINED != eMediaSubType)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strMediaSubType = SipCommonConvertEnumToStrMediaSubType(eMediaSubType);
				return StringFieldCopy(strMediaSubType, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_CONTENT_TYPE_FIELD_BOUNDARY:
		{
			return RvSipContentTypeHeaderGetBoundary(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTENT_TYPE_FIELD_VERSION:
		{
			return RvSipContentTypeHeaderGetVersion(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTENT_TYPE_FIELD_BASE:
		{
			return RvSipContentTypeHeaderGetBase(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTENT_TYPE_FIELD_OTHER_PARAMS:
		{
			return RvSipContentTypeHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTENT_TYPE_FIELD_BAD_SYNTAX:
		{
			return RvSipContentTypeHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
void RVCALLCONV ContactHeaderDestruct(IN RvSipContactHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgContactHeader   *pHeader = (MsgContactHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"ContactHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipContactHeaderGetPool(hHeader);
	hPage = SipContactHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          INOUT RvSipAddressHandle          *phAddress)
{
	RvStatus rv;

	if (RVSIP_CONTACT_FIELD_ADDR_SPEC != eField)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipContactHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipContactHeaderGetAddrSpec(hHeader);

	return RV_OK;
}

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
                                    OUT RvSipAddressHandle            *phAddress)
{
	if (RVSIP_CONTACT_FIELD_ADDR_SPEC != eField)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipContactHeaderGetAddrSpec(hHeader);

	return RV_OK;
}


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
                                          IN RvChar*                     buffer)
{
	switch(eField) {
	case RVSIP_CONTACT_FIELD_QVAL:
		{
			return RvSipContactHeaderSetQVal(hHeader, buffer);
		}
	case RVSIP_CONTACT_FIELD_ACTION:
		{
			RvSipContactAction eContactAction;
			
			if (NULL == buffer)
			{
				return RvSipContactHeaderSetAction(hHeader, RVSIP_CONTACT_ACTION_UNDEFINED);
			}
			
			/* First convert string into enumeration and then set */
			eContactAction = SipCommonConvertStrToEnumContactAction(buffer);

			if (eContactAction == RVSIP_CONTACT_ACTION_UNDEFINED)
			{
				/* the string in buffer is not allowed for contact-action */
				return RV_ERROR_BADPARAM;
			}

			return RvSipContactHeaderSetAction(hHeader, eContactAction);
		}
	case RVSIP_CONTACT_FIELD_OTHER_PARAMS:
		{
			return RvSipContactHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_CONTACT_FIELD_BAD_SYNTAX:
		{
			return RvSipContactHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_CONTACT_FIELD_QVAL:
		{
			*pLength = (RvInt32)RvSipContactHeaderGetStringLength(hHeader, RVSIP_CONTACT_QVAL);
			break;
		}
	case RVSIP_CONTACT_FIELD_ACTION:
		{
			RvSipContactAction   eContactAction;
			
			eContactAction = RvSipContactHeaderGetAction(hHeader);
			
			if (eContactAction != RVSIP_CONTACT_ACTION_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
		}
	case RVSIP_CONTACT_FIELD_OTHER_PARAMS:
		{
			*pLength = (RvInt32)RvSipContactHeaderGetStringLength(hHeader, RVSIP_CONTACT_OTHER_PARAMS);
			break;
		}
	case RVSIP_CONTACT_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipContactHeaderGetStringLength(hHeader, RVSIP_CONTACT_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                       actualLen)
{
	switch(eField) {
	case RVSIP_CONTACT_FIELD_QVAL:
		{
			return RvSipContactHeaderGetQVal(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTACT_FIELD_ACTION:
		{
			RvSipContactAction   eContactAction;
			RvChar				*strContactAction;
			
			eContactAction = RvSipContactHeaderGetAction(hHeader);
			
			if (eContactAction != RVSIP_CONTACT_ACTION_UNDEFINED)
			{
				/* Convert the retrieved enumeration into string */
				strContactAction = SipCommonConvertEnumToStrContactAction(eContactAction);
				return StringFieldCopy(strContactAction, buffer, bufferLen, actualLen);
			}
		}
	case RVSIP_CONTACT_FIELD_OTHER_PARAMS:
		{
			return RvSipContactHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_CONTACT_FIELD_BAD_SYNTAX:
		{
			return RvSipContactHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderSetIntField(
                                          IN RvSipContactHeaderHandle      hHeader,
										  IN RvSipContactHeaderFieldName   eField,
                                          IN RvInt32                       fieldValue)
{
	RvSipExpiresHeaderHandle  hExpires;
	RvStatus                  rv;

	if (eField != RVSIP_CONTACT_FIELD_EXPIRES)
	{
		return RV_ERROR_BADPARAM;
	}

	/* The Contact header contains an Expires header, where the Expires value is held */

	hExpires = RvSipContactHeaderGetExpires(hHeader);
	if (hExpires == NULL)
	{
		/* construct a new Expires header */
		rv = RvSipExpiresConstructInContactHeader(hHeader, &hExpires);
		if (RV_OK != rv)
		{
			return rv;
		}
	}

	return RvSipExpiresHeaderSetDeltaSeconds(hExpires, fieldValue);
}

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
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ContactHeaderGetIntField(
                                          IN  RvSipContactHeaderHandle      hHeader,
										  IN  RvSipContactHeaderFieldName   eField,
                                          OUT RvInt32*                      pFieldValue)
{
	RvSipExpiresHeaderHandle  hExpires;

	if (eField != RVSIP_CONTACT_FIELD_EXPIRES)
	{
		return RV_ERROR_BADPARAM;
	}

	/* The Contact header contains an Expires header, where the Expires value is held */
	
	hExpires = RvSipContactHeaderGetExpires(hHeader);
	if (hExpires == NULL)
	{
		return RV_ERROR_NOT_FOUND;
	}

	return RvSipExpiresHeaderGetDeltaSeconds(hExpires, (RvUint32*)pFieldValue);
}

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
                                          IN RvBool                        fieldValue)
{
	if (eField != RVSIP_CONTACT_FIELD_STAR ||
		RV_FALSE == fieldValue)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipContactHeaderSetStar(hHeader);
}

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
                                          OUT RvBool*                       pFieldValue)
{
	if (eField != RVSIP_CONTACT_FIELD_STAR)
	{
		return RV_ERROR_BADPARAM;
	}

	*pFieldValue = RvSipContactHeaderIsStar(hHeader);

	return RV_OK;
}

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
void RVCALLCONV CSeqHeaderDestruct(IN RvSipCSeqHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgCSeqHeader      *pHeader = (MsgCSeqHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"CSeqHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipCSeqHeaderGetPool(hHeader);
	hPage = SipCSeqHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                  buffer)
{
	switch(eField) {
	case RVSIP_CSEQ_FIELD_METHOD:
		{
			RvSipMethodType eMethodType;

			if (NULL == buffer)
			{
				return RvSipCSeqHeaderSetMethodType(hHeader, RVSIP_METHOD_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eMethodType = SipCommonConvertStrToEnumMethodType(buffer);
			return RvSipCSeqHeaderSetMethodType(hHeader, eMethodType, buffer);
		}
		break;
	case RVSIP_CSEQ_FIELD_BAD_SYNTAX:
		{
			return RvSipCSeqHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_CSEQ_FIELD_METHOD:
		{
			RvSipMethodType   eMethodType;
			
			eMethodType = RvSipCSeqHeaderGetMethodType(hHeader);
			
			if (eMethodType != RVSIP_METHOD_OTHER &&
				eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipCSeqHeaderGetStringLength(hHeader, RVSIP_CSEQ_METHOD);
			}
		}
		break;
	case RVSIP_CSEQ_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipCSeqHeaderGetStringLength(hHeader, RVSIP_CSEQ_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}
									
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
                                    OUT RvInt32*                    actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_CSEQ_FIELD_METHOD:
		{
			RvSipMethodType   eMethodType;
			RvChar           *strMethodType;
			
			eMethodType = RvSipCSeqHeaderGetMethodType(hHeader);
			
			if (eMethodType == RVSIP_METHOD_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				method set to this allowed header */
				return RvSipCSeqHeaderGetStrMethodType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* Convert the retrieved enumeration into string */
				strMethodType = SipCommonConvertEnumToStrMethodType(eMethodType);
				return StringFieldCopy(strMethodType, buffer, bufferLen, actualLen);
			}
		}
		break;
	case RVSIP_CSEQ_FIELD_BAD_SYNTAX:
		{
			return RvSipCSeqHeaderGetStrMethodType(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                    fieldValue)
{
	if (eField != RVSIP_CSEQ_FIELD_STEP)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipCSeqHeaderSetStep(hHeader, fieldValue);
}

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
                                          OUT RvInt32*                   pFieldValue)
{
	if (eField != RVSIP_CSEQ_FIELD_STEP)
	{
		return RV_ERROR_BADPARAM;
	}

#ifdef RV_SIP_UNSIGNED_CSEQ
	return RvSipCSeqHeaderGetStep(hHeader, (RvUint32*)pFieldValue);
#else
	*pFieldValue = RvSipCSeqHeaderGetStep(hHeader);
	return RV_OK;
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */
}

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
void RVCALLCONV DateHeaderDestruct(IN RvSipDateHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgDateHeader      *pHeader = (MsgDateHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"DateHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipDateHeaderGetPool(hHeader);
	hPage = SipDateHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                  buffer)
{
	switch(eField) {
	case RVSIP_DATE_FIELD_BAD_SYNTAX:
		{
			return RvSipDateHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_DATE_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipDateHeaderGetStringLength(hHeader, RVSIP_DATE_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}
									
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
                                    OUT RvInt32*                    actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_DATE_FIELD_BAD_SYNTAX:
		{
			return RvSipDateHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                       fieldValue)
{
	switch(eField) {
	case RVSIP_DATE_FIELD_WEEK_DAY:
		{
			/* First convert string into enumeration and then set */
			RvSipDateWeekDay eWeekDay = SipCommonConvertIntToEnumWeekDay(fieldValue);
			return RvSipDateHeaderSetWeekDay(hHeader, eWeekDay);
		}
	case RVSIP_DATE_FIELD_DAY:
		{
			return RvSipDateHeaderSetDay(hHeader, (RvInt8)fieldValue);
		}
	case RVSIP_DATE_FIELD_MONTH:
		{
			/* First convert string into enumeration and then set */
			RvSipDateMonth eMonth = SipCommonConvertIntToEnumMonth(fieldValue);
			return RvSipDateHeaderSetMonth(hHeader, eMonth);
		}
	case RVSIP_DATE_FIELD_YEAR:
		{
			return RvSipDateHeaderSetYear(hHeader, (RvInt16)fieldValue);
		}
	case RVSIP_DATE_FIELD_HOUR:
		{
			return RvSipDateHeaderSetHour(hHeader, (RvInt8)fieldValue);
		}
	case RVSIP_DATE_FIELD_MINUTE:
		{
			return RvSipDateHeaderSetMinute(hHeader, (RvInt8)fieldValue);
		}
	case RVSIP_DATE_FIELD_SECOND:
		{
			return RvSipDateHeaderSetSecond(hHeader, (RvInt8)fieldValue);
		}
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}
}

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
                                          OUT RvInt32*                   pFieldValue)
{
	switch(eField) {
	case RVSIP_DATE_FIELD_WEEK_DAY:
		{
			/* Convert the retrieved enumeration into integer */
			RvSipDateWeekDay eWeekDay = RvSipDateHeaderGetWeekDay(hHeader);
			*pFieldValue = SipCommonConvertEnumToIntWeekDay(eWeekDay);
			break;
		}
	case RVSIP_DATE_FIELD_DAY:
		{
			*pFieldValue = (RvInt32)RvSipDateHeaderGetDay(hHeader);
			break;
		}
	case RVSIP_DATE_FIELD_MONTH:
		{
			RvSipDateMonth eMonth = RvSipDateHeaderGetMonth(hHeader);
			*pFieldValue = SipCommonConvertEnumToIntMonth(eMonth);
			break;
		}
	case RVSIP_DATE_FIELD_YEAR:
		{
			*pFieldValue = (RvInt32)RvSipDateHeaderGetYear(hHeader);
			break;
		}
	case RVSIP_DATE_FIELD_HOUR:
		{
			*pFieldValue = (RvInt32)RvSipDateHeaderGetHour(hHeader);
			break;
		}
	case RVSIP_DATE_FIELD_MINUTE:
		{
			*pFieldValue = (RvInt32)RvSipDateHeaderGetMinute(hHeader);
			break;
		}
	case RVSIP_DATE_FIELD_SECOND:
		{
			*pFieldValue = (RvInt32)RvSipDateHeaderGetSecond(hHeader);
			break;
		}
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

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
void RVCALLCONV EventHeaderDestruct(IN RvSipEventHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgEventHeader       *pHeader = (MsgEventHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"EventHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipEventHeaderGetPool(hHeader);
	hPage = SipEventHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                        IN RvChar*                   buffer)
{
	RvStatus   rv;
	
	switch (eField) 
	{
	case RVSIP_EVENT_FIELD_EVENT_TYPE:
		{
			/* JSR32 supplies package and template together, i.e. event-type.
			Therefore we parse this part of the header, and restore the rest of the
			parameters afterwards */
			RvInt32  idOffset;
			RvInt32  otherParamsOffset;
			
			if (buffer == NULL)
			{
				rv = RvSipEventHeaderSetEventPackage(hHeader, NULL);
				if (RV_OK != rv)
				{
					return rv;
				}
				return RvSipEventHeaderSetEventTemplate(hHeader, NULL);
			}

			idOffset = SipEventHeaderGetEventId(hHeader);
			otherParamsOffset = SipEventHeaderGetEventParam(hHeader);

			rv = RvSipEventHeaderParseValue(hHeader, buffer);
			
			/*Notice: the following calls to Sip*Set methods do not copy the string
			but rather update the parameter offset, since we supply the header's 
			pool and page */
			SipEventHeaderSetEventId(hHeader, NULL, 
									 SipEventHeaderGetPool(hHeader),
									 SipEventHeaderGetPage(hHeader),
									 idOffset);

			SipEventHeaderSetEventParam(hHeader, NULL, 
									 SipEventHeaderGetPool(hHeader),
									 SipEventHeaderGetPage(hHeader),
									 otherParamsOffset);
	
			return rv;
		}
	case RVSIP_EVENT_FIELD_EVENT_ID:
		{
			return RvSipEventHeaderSetEventId(hHeader, buffer);
		}
	case RVSIP_EVENT_FIELD_EVENT_OTHER_PARAMS:
		{
			return RvSipEventHeaderSetEventParam(hHeader, buffer);
		}
	case RVSIP_EVENT_FIELD_EVENT_BAD_SYNTAX:
		{
			return RvSipEventHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	} 
}

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
                                    OUT RvInt32*                     pLength)
{
	*pLength = 0;
	
	switch (eField) 
	{
	case RVSIP_EVENT_FIELD_EVENT_TYPE:
		{
			RvInt32   packageLen;
			RvInt32   templateLen;
			
			packageLen  = RvSipEventHeaderGetStringLength(hHeader ,RVSIP_EVENT_PACKAGE);
			templateLen = RvSipEventHeaderGetStringLength(hHeader ,RVSIP_EVENT_TEMPLATE);
			
			/* Notice that both packageLen and templateLen have extra 1 for '\0'
			If they both exist, one of the '\0' will be used for the extra '.' 
			that separates them. If not, the template length is 0 and therefore 
			the package length is exactly what we are looking for */
			
			*pLength = packageLen + templateLen;

			break;
		}
	case RVSIP_EVENT_FIELD_EVENT_ID:
		{
			*pLength = RvSipEventHeaderGetStringLength(hHeader, RVSIP_EVENT_ID);
			break;
		}
	case RVSIP_EVENT_FIELD_EVENT_OTHER_PARAMS:
		{
			*pLength = RvSipEventHeaderGetStringLength(hHeader, RVSIP_EVENT_PARAM);
			break;
		}
	case RVSIP_EVENT_FIELD_EVENT_BAD_SYNTAX:
		{
			*pLength = RvSipEventHeaderGetStringLength(hHeader, RVSIP_EVENT_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                           actualLen)
{
	switch (eField) 
	{
	case RVSIP_EVENT_FIELD_EVENT_TYPE:
		{
			RvInt32   packageLen;
			RvInt32   templateLen;
			RvInt32   tempActualLen;
			RvStatus  rv;
			
			*actualLen = 0;
			
			packageLen  = RvSipEventHeaderGetStringLength(hHeader ,RVSIP_EVENT_PACKAGE);
			templateLen = RvSipEventHeaderGetStringLength(hHeader ,RVSIP_EVENT_TEMPLATE);
			/* Notice that both packageLen and templateLen have extra 1 for '\0'
			One of them will be used for the extra '.' that separates them. */
			
			if (packageLen + templateLen > bufferLen)
			{
				return RV_ERROR_BADPARAM;
			}
			
			/* JSR32 supplies package and template together, i.e. event-type */
			rv = RvSipEventHeaderGetEventPackage(hHeader, buffer, (RvUint)bufferLen, (RvUint*)&tempActualLen);
			if (RV_OK != rv)
			{
				return rv;
			}
			*actualLen = tempActualLen;
			
			if (templateLen > 0)
			{
				buffer[tempActualLen-1] = '.';
				rv = RvSipEventHeaderGetEventTemplate(hHeader, buffer+tempActualLen, (RvUint)(bufferLen-tempActualLen), (RvUint*)&tempActualLen);
				if (RV_OK != rv)
				{
					return rv;
				}
				*actualLen += tempActualLen;
			}
			
			break;
		}
	case RVSIP_EVENT_FIELD_EVENT_ID:
		{
			return RvSipEventHeaderGetEventId(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_EVENT_FIELD_EVENT_OTHER_PARAMS:
		{
			return RvSipEventHeaderGetEventParam(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_EVENT_FIELD_EVENT_BAD_SYNTAX:
		{
			return RvSipEventHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV ExpiresHeaderDestruct(IN RvSipExpiresHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgExpiresHeader   *pHeader = (MsgExpiresHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"ExpiresHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipExpiresHeaderGetPool(hHeader);
	hPage = SipExpiresHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                     buffer)
{
	switch(eField) {
	case RVSIP_EXPIRES_FIELD_BAD_SYNTAX:
		{
			return RvSipExpiresHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                          pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_EXPIRES_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipExpiresHeaderGetStringLength(hHeader, RVSIP_EXPIRES_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}
									
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
                                    OUT RvInt32*                       actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_EXPIRES_FIELD_BAD_SYNTAX:
		{
			return RvSipExpiresHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}


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
                                          IN RvInt32                       fieldValue)
{
	if (eField != RVSIP_EXPIRES_FIELD_DELTA_SECONDS)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipExpiresHeaderSetDeltaSeconds(hHeader, fieldValue);
}

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
                                          OUT RvInt32*                      pFieldValue)
{
	if (eField != RVSIP_EXPIRES_FIELD_DELTA_SECONDS)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipExpiresHeaderGetDeltaSeconds(hHeader, (RvUint32*)pFieldValue);
}

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
void RVCALLCONV InfoHeaderDestruct(IN RvSipInfoHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgInfoHeader      *pHeader = (MsgInfoHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"InfoHeaderDestruct - Destructing header 0x%p",
				hHeader));	

	hPool = SipInfoHeaderGetPool(hHeader);
	hPage = SipInfoHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                  buffer)
{
	switch(eField) {
	case RVSIP_INFO_FIELD_OTHER_PARMAS:
		{
			return RvSipInfoHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_INFO_FIELD_BAD_SYNTAX:
		{
			return RvSipInfoHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_INFO_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipInfoHeaderGetStringLength(hHeader, RVSIP_INFO_OTHER_PARAMS);
			break;
		}
	case RVSIP_INFO_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipInfoHeaderGetStringLength(hHeader, RVSIP_INFO_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                     actualLen)
{
	switch(eField) {
	case RVSIP_INFO_FIELD_OTHER_PARMAS:
		{
			return RvSipInfoHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_INFO_FIELD_BAD_SYNTAX:
		{
			return RvSipInfoHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          INOUT RvSipAddressHandle       *phAddress)
{
	RvStatus rv;

	if (eField != RVSIP_INFO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipInfoHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipInfoHeaderGetAddrSpec(hHeader);
	
	return RV_OK;
}

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
                                    OUT RvSipAddressHandle          *phAddress)
{
	if (eField != RVSIP_INFO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipInfoHeaderGetAddrSpec(hHeader);

	return RV_OK;
}


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
void RVCALLCONV PartyHeaderDestruct(IN RvSipPartyHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgPartyHeader     *pHeader = (MsgPartyHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"PartyHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipPartyHeaderGetPool(hHeader);
	hPage = SipPartyHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                   buffer)
{
	switch(eField) {
	case RVSIP_PARTY_FIELD_TAG:
		{
			return RvSipPartyHeaderSetTag(hHeader, buffer);
		}
	case RVSIP_PARTY_FIELD_OTHER_PARMAS:
		{
			return RvSipPartyHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_PARTY_FIELD_BAD_SYNTAX:
		{
			return RvSipPartyHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                        pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_PARTY_FIELD_TAG:
		{
			*pLength = (RvInt32)RvSipPartyHeaderGetStringLength(hHeader, RVSIP_PARTY_TAG);
			break;
		}
	case RVSIP_PARTY_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipPartyHeaderGetStringLength(hHeader, RVSIP_PARTY_OTHER_PARAMS);
			break;
		}
	case RVSIP_PARTY_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipPartyHeaderGetStringLength(hHeader, RVSIP_PARTY_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                     actualLen)
{
	switch(eField) {
	case RVSIP_PARTY_FIELD_TAG:
		{
			return RvSipPartyHeaderGetTag(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_PARTY_FIELD_OTHER_PARMAS:
		{
			return RvSipPartyHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_PARTY_FIELD_BAD_SYNTAX:
		{
			return RvSipPartyHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          INOUT RvSipAddressHandle       *phAddress)
{
	RvStatus rv;

	if (eField != RVSIP_PARTY_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipPartyHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipPartyHeaderGetAddrSpec(hHeader);
	
	return RV_OK;
}

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
                                    OUT RvSipAddressHandle          *phAddress)
{
	if (eField != RVSIP_PARTY_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipPartyHeaderGetAddrSpec(hHeader);

	return RV_OK;
}

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
void RVCALLCONV RAckHeaderDestruct(IN RvSipRAckHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgRAckHeader      *pHeader = (MsgRAckHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"RAckHeaderDestruct - Destructing header 0x%p",
			hHeader));	
	
	hPool = SipRAckHeaderGetPool(hHeader);
	hPage = SipRAckHeaderGetPage(hHeader);
	
	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                  buffer)
{
	switch(eField) {
	case RVSIP_RACK_FIELD_METHOD:
		{
			RvSipCSeqHeaderHandle hCSeqHeader = RvSipRAckHeaderGetCSeqHandle(hHeader);
			RvStatus              rv;

			if (buffer == NULL && hCSeqHeader != NULL)
			{
				return CSeqHeaderSetStringField(hCSeqHeader, RVSIP_CSEQ_FIELD_METHOD, NULL);
			}
	
			if (hCSeqHeader == NULL)
			{
				rv = RvSipCSeqHeaderConstructInRAckHeader(hHeader, &hCSeqHeader);
				if (RV_OK != rv)
				{
					return rv;
				}
			}

			return CSeqHeaderSetStringField(hCSeqHeader, RVSIP_CSEQ_FIELD_METHOD, buffer);
		}
	case RVSIP_RACK_FIELD_BAD_SYNTAX:
		{
			return RvSipRAckHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_RACK_FIELD_METHOD:
		{
			RvSipCSeqHeaderHandle hCSeqHeader = RvSipRAckHeaderGetCSeqHandle(hHeader);
			if (NULL == hCSeqHeader)
			{
				*pLength = 0;
			}
			else
			{
				return CSeqHeaderGetStringFieldLength(hCSeqHeader, RVSIP_CSEQ_FIELD_METHOD, pLength);
			}
			break;
		}
	case RVSIP_RACK_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipRAckHeaderGetStringLength(hHeader, RVSIP_RACK_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                    actualLen)
{
	switch(eField) {
	case RVSIP_RACK_FIELD_METHOD:
		{
			RvSipCSeqHeaderHandle hCSeqHeader = RvSipRAckHeaderGetCSeqHandle(hHeader);
			if (NULL == hCSeqHeader)
			{
				*actualLen = 0;
			}
			else
			{
				return CSeqHeaderGetStringField(hCSeqHeader, RVSIP_CSEQ_FIELD_METHOD, buffer, bufferLen, actualLen);
			}
			break;
		}
	case RVSIP_RACK_FIELD_BAD_SYNTAX:
		{
			return RvSipRAckHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                    fieldValue)
{
	switch (eField) {
	case RVSIP_RACK_FIELD_STEP:
		{
			RvSipCSeqHeaderHandle hCSeqHeader = RvSipRAckHeaderGetCSeqHandle(hHeader);
			RvStatus              rv;
			
			if (hCSeqHeader == NULL)
			{
				rv = RvSipCSeqHeaderConstructInRAckHeader(hHeader, &hCSeqHeader);
				if (RV_OK != rv)
				{
					return rv;
				}
			}
			
			return RvSipCSeqHeaderSetStep(hCSeqHeader, fieldValue);
		}
	case RVSIP_RACK_FIELD_RESPONSE_NUM:
		{
			return RvSipRAckHeaderSetResponseNum(hHeader, (RvUint32)fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*                   pFieldValue)
{
	*pFieldValue = UNDEFINED;

	switch (eField) {
	case RVSIP_RACK_FIELD_STEP:
		{
			RvSipCSeqHeaderHandle hCSeqHeader = RvSipRAckHeaderGetCSeqHandle(hHeader);
			
			if (hCSeqHeader != NULL)
			{
				*pFieldValue = RvSipCSeqHeaderGetStep(hCSeqHeader);
			}
			break;
		}
	case RVSIP_RACK_FIELD_RESPONSE_NUM:
		{
			RvBool   bIsRespNum;
			RvStatus rv;

			rv = RvSipRAckHeaderIsResponseNumInitialized(hHeader, &bIsRespNum);
			if (RV_OK != rv)
			{
				return rv;
			}

			if (bIsRespNum == RV_TRUE)
			{
				*pFieldValue = (RvInt32)RvSipRAckHeaderGetResponseNum(hHeader);
			}
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV RSeqHeaderDestruct(IN RvSipRSeqHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgRSeqHeader      *pHeader = (MsgRSeqHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"RSeqHeaderDestruct - Destructing header 0x%p",
			hHeader));	
	
	hPool = SipRSeqHeaderGetPool(hHeader);
	hPage = SipRSeqHeaderGetPage(hHeader);
	
	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                  buffer)
{
	switch(eField) {
	case RVSIP_RSEQ_FIELD_BAD_SYNTAX:
		{
			return RvSipRSeqHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                       pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_RSEQ_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipRSeqHeaderGetStringLength(hHeader, RVSIP_RSEQ_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                    actualLen)
{
	switch(eField) {
	case RVSIP_RSEQ_FIELD_BAD_SYNTAX:
		{
			return RvSipRSeqHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                    fieldValue)
{
	switch (eField) {
	case RVSIP_RSEQ_FIELD_RESPONSE_NUM:
		{
			return RvSipRSeqHeaderSetResponseNum(hHeader, (RvUint32)fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*                   pFieldValue)
{
	*pFieldValue = UNDEFINED;

	switch (eField) {
	case RVSIP_RSEQ_FIELD_RESPONSE_NUM:
		{
			RvBool   bIsRespNum;
			RvStatus rv;

			rv = RvSipRSeqHeaderIsResponseNumInitialized(hHeader, &bIsRespNum);
			if (RV_OK != rv)
			{
				return rv;
			}

			if (bIsRespNum == RV_TRUE)
			{
				*pFieldValue = (RvInt32)RvSipRSeqHeaderGetResponseNum(hHeader);
			}
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV ReasonHeaderDestruct(IN RvSipReasonHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgReasonHeader      *pHeader = (MsgReasonHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
			"ReasonHeaderDestruct - Destructing header 0x%p",
			hHeader));	

	hPool = SipReasonHeaderGetPool(hHeader);
	hPage = SipReasonHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                    buffer)
{
	switch(eField) {
	case RVSIP_REASON_FIELD_TEXT:
		{
			return RvSipReasonHeaderSetText(hHeader, buffer);
		}
	case RVSIP_REASON_FIELD_PROTOCOL:
		{
			RvSipReasonProtocolType eProtocolType;

			if (NULL == buffer)
			{
				return RvSipReasonHeaderSetProtocol(hHeader, RVSIP_REASON_PROTOCOL_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eProtocolType = SipCommonConvertStrToEnumReasonProtocol(buffer);
			return RvSipReasonHeaderSetProtocol(hHeader, eProtocolType, buffer);
		}
		break;
	case RVSIP_REASON_FIELD_OTHER_PARAMS:
		{
			return RvSipReasonHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_REASON_FIELD_BAD_SYNTAX:
		{
			return RvSipReasonHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                         pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_REASON_FIELD_TEXT:
		{
			*pLength = (RvInt32)RvSipReasonHeaderGetStringLength(hHeader, RVSIP_REASON_TEXT);
		}
		break;
	case RVSIP_REASON_FIELD_PROTOCOL:
		{
			RvSipReasonProtocolType   eProtocolType;
			
			eProtocolType = RvSipReasonHeaderGetProtocol(hHeader);
			
			if (eProtocolType != RVSIP_REASON_PROTOCOL_OTHER &&
				eProtocolType != RVSIP_REASON_PROTOCOL_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipReasonHeaderGetStringLength(hHeader, RVSIP_REASON_PROTOCOL);
			}
		}
		break;
	case RVSIP_REASON_FIELD_OTHER_PARAMS:
		{
			*pLength = (RvInt32)RvSipReasonHeaderGetStringLength(hHeader, RVSIP_REASON_OTHER_PARAMS);
		}
	case RVSIP_REASON_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipReasonHeaderGetStringLength(hHeader, RVSIP_REASON_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}
									
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
                                    OUT RvInt32*                      actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_REASON_FIELD_TEXT:
		{
			return RvSipReasonHeaderGetText(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_REASON_FIELD_PROTOCOL:
		{
			RvSipReasonProtocolType   eProtocolType;
			RvChar                   *strProtocolType;
			
			eProtocolType = RvSipReasonHeaderGetProtocol(hHeader);
			
			if (eProtocolType == RVSIP_REASON_PROTOCOL_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				method set to this allowed header */
				return RvSipReasonHeaderGetStrProtocol(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eProtocolType != RVSIP_REASON_PROTOCOL_UNDEFINED)
			{
				/* Convert the retrieved enumeration into string */
				strProtocolType = SipCommonConvertEnumToStrReasonProtocol(eProtocolType);
				return StringFieldCopy(strProtocolType, buffer, bufferLen, actualLen);
			}
		}
		break;
	case RVSIP_REASON_FIELD_OTHER_PARAMS:
		{
			return RvSipReasonHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_REASON_FIELD_BAD_SYNTAX:
		{
			return RvSipReasonHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                      fieldValue)
{
	if (eField != RVSIP_REASON_FIELD_CAUSE)
	{
		return RV_ERROR_BADPARAM;
	}

	return RvSipReasonHeaderSetCause(hHeader, fieldValue);
}

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
                                          OUT RvInt32*                     pFieldValue)
{
	if (eField != RVSIP_REASON_FIELD_CAUSE)
	{
		return RV_ERROR_BADPARAM;
	}

	*pFieldValue = RvSipReasonHeaderGetCause(hHeader);;
	return RV_OK;
}

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
void RVCALLCONV ReferToHeaderDestruct(IN RvSipReferToHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgReferToHeader     *pHeader = (MsgReferToHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"ReferToHeaderDestruct - Destructing header 0x%p",
				hHeader));	

	hPool = SipReferToHeaderGetPool(hHeader);
	hPage = SipReferToHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                     buffer)
{
	switch(eField) {
	case RVSIP_REFER_TO_FIELD_REPLACES:
		{
			RvSipReplacesHeaderHandle hReplacesHeader;
			RvStatus                  rv;

			if (buffer == NULL)
			{
				return RvSipReferToHeaderSetReplacesHeader(hHeader, NULL);
			}

			rv = RvSipReferToHeaderGetReplacesHeader(hHeader, &hReplacesHeader);
			if (RV_OK != rv)
			{
				return rv;
			}

			if (hReplacesHeader == NULL)
			{
				rv = RvSipReplacesHeaderConstructInReferToHeader(hHeader, &hReplacesHeader);
				if (RV_OK != rv)
				{
					return rv;
				}
			}

			return RvSipReplacesHeaderParseValue(hReplacesHeader, buffer);
		}
	case RVSIP_REFER_TO_FIELD_OTHER_PARMAS:
		{
			return RvSipReferToHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_REFER_TO_FIELD_BAD_SYNTAX:
		{
			return RvSipReferToHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                          pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_REFER_TO_FIELD_REPLACES:
		{
			RvSipReplacesHeaderHandle hReplacesHeader;
			HPAGE                     hPage;
			RvUint32                  length;
			RvStatus                  rv;
			
			rv = RvSipReferToHeaderGetReplacesHeader(hHeader, &hReplacesHeader);
			if (RV_OK != rv || hReplacesHeader == NULL)
			{
				return rv;
			}
			
			rv = RvSipReplacesHeaderEncode(hReplacesHeader, 
										   SipReferToHeaderGetPool(hHeader), 
										   &hPage, &length);
			if (RV_OK != rv)
			{
				return rv;
			}
			
			*pLength = (RvInt32)length - strlen("Replaces:");

			break;				
		}
	case RVSIP_REFER_TO_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipReferToHeaderGetStringLength(hHeader, RVSIP_REFER_TO_OTHER_PARAMS);
			break;
		}
	case RVSIP_REFER_TO_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipReferToHeaderGetStringLength(hHeader, RVSIP_REFER_TO_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                       actualLen)
{
	switch(eField) {
	case RVSIP_REFER_TO_FIELD_REPLACES:
		{
			RvSipReplacesHeaderHandle hReplacesHeader;
			HRPOOL                    hPool = SipReferToHeaderGetPool(hHeader);
			HPAGE                     hPage;
			RvInt32                   length;
			RvStatus                  rv;
			
			rv = RvSipReferToHeaderGetReplacesHeader(hHeader, &hReplacesHeader);
			if (RV_OK != rv)
			{
				return rv;
			}

			if (hReplacesHeader == NULL)
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
			
			rv = RvSipReplacesHeaderEncode(hReplacesHeader, 
											hPool, &hPage, (RvUint32*)&length);
			if (RV_OK != rv)
			{
				return rv;
			}

			*actualLen = length;
			
			if (bufferLen < length)
			{
				return RV_ERROR_INSUFFICIENT_BUFFER;
			}
				
			return RPOOL_CopyToExternal(hPool, hPage, 0, buffer, length);
		}
	case RVSIP_REFER_TO_FIELD_OTHER_PARMAS:
		{
			return RvSipReferToHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_REFER_TO_FIELD_BAD_SYNTAX:
		{
			return RvSipReferToHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          INOUT RvSipAddressHandle          *phAddress)
{
	RvStatus rv;

	if (eField != RVSIP_REFER_TO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipReferToHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipReferToHeaderGetAddrSpec(hHeader);
	
	return RV_OK;
}

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
                                    OUT RvSipAddressHandle            *phAddress)
{
	if (eField != RVSIP_REFER_TO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipReferToHeaderGetAddrSpec(hHeader);

	return RV_OK;
}

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
void RVCALLCONV ReplyToHeaderDestruct(IN RvSipReplyToHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgReplyToHeader     *pHeader = (MsgReplyToHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"ReplyToHeaderDestruct - Destructing header 0x%p",
				hHeader));	

	hPool = SipReplyToHeaderGetPool(hHeader);
	hPage = SipReplyToHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                     buffer)
{
	switch(eField) {
	case RVSIP_REPLY_TO_FIELD_OTHER_PARMAS:
		{
			return RvSipReplyToHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_REPLY_TO_FIELD_BAD_SYNTAX:
		{
			return RvSipReplyToHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                          pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_REPLY_TO_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipReplyToHeaderGetStringLength(hHeader, RVSIP_REPLY_TO_OTHER_PARAMS);
			break;
		}
	case RVSIP_REPLY_TO_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipReplyToHeaderGetStringLength(hHeader, RVSIP_REPLY_TO_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                       actualLen)
{
	switch(eField) {
	case RVSIP_REPLY_TO_FIELD_OTHER_PARMAS:
		{
			return RvSipReplyToHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_REPLY_TO_FIELD_BAD_SYNTAX:
		{
			return RvSipReplyToHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          INOUT RvSipAddressHandle          *phAddress)
{
	RvStatus rv;

	if (eField != RVSIP_REPLY_TO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipReplyToHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipReplyToHeaderGetAddrSpec(hHeader);
	
	return RV_OK;
}

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
                                    OUT RvSipAddressHandle            *phAddress)
{
	if (eField != RVSIP_REPLY_TO_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipReplyToHeaderGetAddrSpec(hHeader);

	return RV_OK;
}

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
void RVCALLCONV RetryAfterHeaderDestruct(IN RvSipRetryAfterHeaderHandle hHeader)
{
	HRPOOL					  hPool;
	HPAGE					  hPage;
	MsgRetryAfterHeader      *pHeader = (MsgRetryAfterHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"RetryAfterHeaderDestruct - Destructing header 0x%p",
				hHeader));	

	hPool = SipRetryAfterHeaderGetPool(hHeader);
	hPage = SipRetryAfterHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*						buffer)
{
	switch(eField) {
	case RVSIP_RETRY_AFTER_FIELD_COMMENT:
		{
			return RvSipRetryAfterHeaderSetStrComment(hHeader, buffer);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_OTHER_PARMAS:
		{
			return RvSipRetryAfterHeaderSetRetryAfterParams(hHeader, buffer);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_BAD_SYNTAX:
		{
			return RvSipRetryAfterHeaderSetStrBadSyntax(hHeader, buffer);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*							 pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_RETRY_AFTER_FIELD_COMMENT:
		{
			*pLength = (RvInt32)RvSipRetryAfterHeaderGetStringLength(hHeader, RVSIP_RETRY_AFTER_COMMENT);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipRetryAfterHeaderGetStringLength(hHeader, RVSIP_RETRY_AFTER_PARAMS);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipRetryAfterHeaderGetStringLength(hHeader, RVSIP_RETRY_AFTER_BAD_SYNTAX);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}
									
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
                                    OUT RvInt32*						  actualLen)
{
	*actualLen = 0;
	
	switch(eField) {
	case RVSIP_RETRY_AFTER_FIELD_COMMENT:
		{
			return RvSipRetryAfterHeaderGetStrComment(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_OTHER_PARMAS:
		{
			return RvSipRetryAfterHeaderGetRetryAfterParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	case RVSIP_RETRY_AFTER_FIELD_BAD_SYNTAX:
		{
			return RvSipRetryAfterHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
		break;
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                             fieldValue)
{
	switch(eField) {
	case RVSIP_RETRY_AFTER_FIELD_RETRY_AFTER:
		{
			return RvSipRetryAfterHeaderSetDeltaSeconds(hHeader, fieldValue);
		}
	case RVSIP_RETRY_AFTER_FIELD_DURATION:
		{
			return RvSipRetryAfterHeaderSetDuration(hHeader, fieldValue);
		}
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}
}

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
                                          OUT RvInt32*                         pFieldValue)
{
	switch(eField) {
	case RVSIP_RETRY_AFTER_FIELD_RETRY_AFTER:
		{
			return RvSipRetryAfterHeaderGetDeltaSeconds(hHeader, (RvUint32*)pFieldValue);
		}
	case RVSIP_RETRY_AFTER_FIELD_DURATION:
		{
			return RvSipRetryAfterHeaderGetDuration(hHeader, pFieldValue);
		}
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}
}

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
void RVCALLCONV RouteHopHeaderDestruct(IN RvSipRouteHopHeaderHandle hHeader)
{
	HRPOOL                 hPool;
	HPAGE                  hPage;
	MsgRouteHopHeader     *pHeader = (MsgRouteHopHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"RouteHopHeaderDestruct - Destructing header 0x%p",
				hHeader));	

	hPool = SipRouteHopHeaderGetPool(hHeader);
	hPage = SipRouteHopHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                      buffer)
{
	switch(eField) {
	case RVSIP_ROUTE_HOP_FIELD_OTHER_PARMAS:
		{
			return RvSipRouteHopHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_ROUTE_HOP_FIELD_BAD_SYNTAX:
		{
			return RvSipRouteHopHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                           pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_ROUTE_HOP_FIELD_OTHER_PARMAS:
		{
			*pLength = (RvInt32)RvSipRouteHopHeaderGetStringLength(hHeader, RVSIP_ROUTE_HOP_OTHER_PARAMS);
			break;
		}
	case RVSIP_ROUTE_HOP_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipRouteHopHeaderGetStringLength(hHeader, RVSIP_ROUTE_HOP_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                        actualLen)
{
	switch(eField) {
	case RVSIP_ROUTE_HOP_FIELD_OTHER_PARMAS:
		{
			return RvSipRouteHopHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ROUTE_HOP_FIELD_BAD_SYNTAX:
		{
			return RvSipRouteHopHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          INOUT RvSipAddressHandle           *phAddress)
{
	RvStatus rv;

	if (eField != RVSIP_ROUTE_HOP_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = RvSipRouteHopHeaderSetAddrSpec(hHeader, *phAddress);
	if (RV_OK != rv)
	{
		return rv;
	}

	*phAddress = RvSipRouteHopHeaderGetAddrSpec(hHeader);
	
	return RV_OK;
}

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
                                    OUT RvSipAddressHandle             *phAddress)
{
	if (eField != RVSIP_ROUTE_HOP_FIELD_ADDR_SPEC)
	{
		return RV_ERROR_BADPARAM;
	}

	*phAddress = RvSipRouteHopHeaderGetAddrSpec(hHeader);

	return RV_OK;
}

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
void RVCALLCONV TimestampHeaderDestruct(IN RvSipTimestampHeaderHandle hHeader)
{
	HRPOOL                           hPool;
	HPAGE                            hPage;
	MsgTimestampHeader				*pHeader = (MsgTimestampHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"TimestampHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipTimestampHeaderGetPool(hHeader);
	hPage = SipTimestampHeaderGetPage(hHeader);
	
	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                       buffer)
{
	RvChar                      temp;
	RvInt32						i, len;
			
	switch(eField) {
	case RVSIP_TIMESTAMP_FIELD_TIMESTAMP_VAL:
		{
			RvInt32						timestampInt = UNDEFINED;
			RvInt32                     timestampDec = UNDEFINED;
			
			if (buffer == NULL)
			{
				return RvSipTimestampHeaderSetTimestampVal(hHeader, UNDEFINED, UNDEFINED);
			}
			
			len  = strlen(buffer);
			
			for (i = 0; i < len; i++)
			{
				if (buffer[i] == '.')
				{
					break;
				}
			}
			
			temp = buffer[i];
			buffer[i] = '\0';
			
			timestampInt = atoi(buffer);
			if (i+1 < len)
			{
				timestampDec = atoi(buffer+i+1);
			}
			
			buffer[i] = temp;
			
			return RvSipTimestampHeaderSetTimestampVal(hHeader, timestampInt, timestampDec);
		}
	case RVSIP_TIMESTAMP_FIELD_DELAY_VAL:
		{
			RvInt32						delayInt = UNDEFINED;
			RvInt32                     delayDec = UNDEFINED;
			
			if (buffer == NULL)
			{
				return RvSipTimestampHeaderSetDelayVal(hHeader, UNDEFINED, UNDEFINED);
			}
			
			len  = strlen(buffer);
			
			for (i = 0; i < len; i++)
			{
				if (buffer[i] == '.')
				{
					break;
				}
			}
			
			temp = buffer[i];
			buffer[i] = '\0';
			
			delayInt = atoi(buffer);
			if (i+1 < len)
			{
				delayDec = atoi(buffer+i+1);
			}
			
			buffer[i] = temp;
			
			return RvSipTimestampHeaderSetDelayVal(hHeader, delayInt, delayDec);
		}
	case RVSIP_TIMESTAMP_FIELD_BAD_SYNTAX:
		{
			return RvSipTimestampHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                            pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_TIMESTAMP_FIELD_TIMESTAMP_VAL:
	case RVSIP_TIMESTAMP_FIELD_DELAY_VAL:
		{
			*pLength = MAX_LENGTH_OF_FLOAT;
			break;
		}
	case RVSIP_TIMESTAMP_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipTimestampHeaderGetStringLength(hHeader, RVSIP_TIMESTAMP_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                                 actualLen)
{
	RvStatus                    rv;
	RvInt32                     len;
	
	switch(eField) {
	case RVSIP_TIMESTAMP_FIELD_TIMESTAMP_VAL:
		{
			RvInt32						timestampInt = UNDEFINED;
			RvInt32                     timestampDec = UNDEFINED;

			if (bufferLen < MAX_LENGTH_OF_FLOAT)
			{
				return RV_ERROR_INSUFFICIENT_BUFFER;
			}
			
			rv = RvSipTimestampHeaderGetTimestampVal(hHeader, &timestampInt, &timestampDec);
			if (RV_OK != rv)
			{
				return rv;
			}
			
			if (UNDEFINED == timestampInt)
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
			
			MsgUtils_itoa(timestampInt, buffer);
			len = strlen(buffer);
			
			if (UNDEFINED != timestampDec)
			{
				buffer[len] = '.';
				MsgUtils_itoa(timestampDec, buffer+len+1);
			}
			
			*actualLen = strlen(buffer);
			break;
		}
		case RVSIP_TIMESTAMP_FIELD_DELAY_VAL:
		{
			RvInt32						delayInt = UNDEFINED;
			RvInt32                     delayDec = UNDEFINED;
			
			if (bufferLen < MAX_LENGTH_OF_FLOAT)
			{
				return RV_ERROR_INSUFFICIENT_BUFFER;
			}
			
			rv = RvSipTimestampHeaderGetDelayVal(hHeader, &delayInt, &delayDec);
			if (RV_OK != rv)
			{
				return rv;
			}
			
			if (UNDEFINED == delayInt)
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
			
			MsgUtils_itoa(delayInt, buffer);
			len = strlen(buffer);
			
			if (UNDEFINED != delayDec)
			{
				buffer[len] = '.';
				MsgUtils_itoa(delayDec, buffer+len+1);
			}
			
			*actualLen = strlen(buffer);
			break;
		}
	case RVSIP_TIMESTAMP_FIELD_BAD_SYNTAX:
		{
			return RvSipTimestampHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
void RVCALLCONV SubscriptionStateHeaderDestruct(IN RvSipSubscriptionStateHeaderHandle hHeader)
{
	HRPOOL                           hPool;
	HPAGE                            hPage;
	MsgSubscriptionStateHeader      *pHeader = (MsgSubscriptionStateHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"SubscriptionStateHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipSubscriptionStateHeaderGetPool(hHeader);
	hPage = SipSubscriptionStateHeaderGetPage(hHeader);
	
	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                               buffer)
{
	switch(eField) {
	case RVSIP_SUBSCRIPTION_STATE_FIELD_STATE:
		{
			RvSipSubscriptionSubstate eSubsState;

			if (NULL == buffer)
			{
				return RvSipSubscriptionStateHeaderSetSubstate(hHeader, RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED, NULL);
			}

			eSubsState = SipCommonConvertStrToEnumSubscriptionSubstate(buffer);
			return RvSipSubscriptionStateHeaderSetSubstate(hHeader, eSubsState, buffer);
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_REASON:
		{
			RvSipSubscriptionReason eSubsReason;
			
			if (NULL == buffer)
			{
				return RvSipSubscriptionStateHeaderSetReason(hHeader, RVSIP_SUBSCRIPTION_REASON_UNDEFINED, NULL);
			}
			
			eSubsReason = SipCommonConvertStrToEnumSubscriptionReason(buffer);
			return RvSipSubscriptionStateHeaderSetReason(hHeader, eSubsReason, buffer);
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_OTHER_PARAMS:
		{
			return RvSipSubscriptionStateHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_BAD_SYNTAX:
		{
			return RvSipSubscriptionStateHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                                    pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_SUBSCRIPTION_STATE_FIELD_STATE:
		{
			RvSipSubscriptionSubstate eSubsState = RvSipSubscriptionStateHeaderGetSubstate(hHeader);
			if (RVSIP_SUBSCRIPTION_SUBSTATE_OTHER != eSubsState &&
				RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED != eSubsState)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipSubscriptionStateHeaderGetStringLength(hHeader, RVSIP_SUBSCRIPTION_STATE_SUBSTATE_VAL);
			}
			break;
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_REASON:
		{
			RvSipSubscriptionReason eSubsReason = RvSipSubscriptionStateHeaderGetReason(hHeader);
			if (RVSIP_SUBSCRIPTION_REASON_OTHER != eSubsReason &&
				RVSIP_SUBSCRIPTION_REASON_UNDEFINED != eSubsReason)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipSubscriptionStateHeaderGetStringLength(hHeader, RVSIP_SUBSCRIPTION_STATE_REASON);
			}
			break;
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_OTHER_PARAMS:
		{
			*pLength = (RvInt32)RvSipSubscriptionStateHeaderGetStringLength(hHeader, RVSIP_SUBSCRIPTION_STATE_OTHER_PARAMS);
			break;
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipSubscriptionStateHeaderGetStringLength(hHeader, RVSIP_SUBSCRIPTION_STATE_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                                 actualLen)
{
	switch(eField) {
	case RVSIP_SUBSCRIPTION_STATE_FIELD_STATE:
		{
			RvSipSubscriptionSubstate eSubsState = RvSipSubscriptionStateHeaderGetSubstate(hHeader);
			if (RVSIP_SUBSCRIPTION_SUBSTATE_OTHER == eSubsState)
			{
				/* The OTHER enumeration indicates that there is a string 
				scheme set to this allowed header */
				return RvSipSubscriptionStateHeaderGetStrSubstate(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED != eSubsState)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strSubsState = SipCommonConvertEnumToStrSubscriptionSubstate(eSubsState);
				return StringFieldCopy(strSubsState, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_REASON:
		{
			RvSipSubscriptionReason eSubsReason = RvSipSubscriptionStateHeaderGetReason(hHeader);
			if (RVSIP_SUBSCRIPTION_REASON_OTHER == eSubsReason)
			{
				/* The OTHER enumeration indicates that there is a string 
				scheme set to this allowed header */
				return RvSipSubscriptionStateHeaderGetStrReason(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (RVSIP_SUBSCRIPTION_REASON_UNDEFINED != eSubsReason)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strSubsReason = SipCommonConvertEnumToStrSubscriptionReason(eSubsReason);
				return StringFieldCopy(strSubsReason, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_OTHER_PARAMS:
		{
			return RvSipSubscriptionStateHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_BAD_SYNTAX:
		{
			return RvSipSubscriptionStateHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                                 fieldValue)
{
	switch (eField) {
	case RVSIP_SUBSCRIPTION_STATE_FIELD_EXPIRES:
		{
			return RvSipSubscriptionStateHeaderSetExpiresParam(hHeader, fieldValue);
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_RETRY_AFTER:
		{
			return RvSipSubscriptionStateHeaderSetRetryAfter(hHeader, fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*                                pFieldValue)
{
	*pFieldValue = UNDEFINED;

	switch (eField) {
	case RVSIP_SUBSCRIPTION_STATE_FIELD_EXPIRES:
		{
			*pFieldValue = RvSipSubscriptionStateHeaderGetExpiresParam(hHeader);
			break;
		}
	case RVSIP_SUBSCRIPTION_STATE_FIELD_RETRY_AFTER:
		{
			*pFieldValue = RvSipSubscriptionStateHeaderGetRetryAfter(hHeader);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV ViaHeaderDestruct(IN RvSipViaHeaderHandle hHeader)
{
	HRPOOL              hPool;
	HPAGE               hPage;
	MsgViaHeader       *pHeader = (MsgViaHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"ViaHeaderDestruct - Destructing header 0x%p",
		hHeader));	

	hPool = SipViaHeaderGetPool(hHeader);
	hPage = SipViaHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                 buffer)
{
	switch(eField) {
	case RVSIP_VIA_FIELD_HOST:
		{
			return RvSipViaHeaderSetHost(hHeader, buffer);
		}
	case RVSIP_VIA_FIELD_TRANSPORT:
		{
			RvSipTransport eTransport;
			
			if (NULL == buffer)
			{
				return RvSipViaHeaderSetTransport(hHeader, RVSIP_TRANSPORT_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eTransport = SipCommonConvertStrToEnumTransportType(buffer);
			return RvSipViaHeaderSetTransport(hHeader, eTransport, buffer);
		}
	case RVSIP_VIA_FIELD_RPORT_PARAM:
		{
			RvInt32  rport;
			
			if (NULL == buffer)
			{
				return RvSipViaHeaderSetRportParam(hHeader, UNDEFINED, RV_FALSE);
			}

			if ('\0' == buffer[0])
			{
				return RvSipViaHeaderSetRportParam(hHeader, UNDEFINED, RV_TRUE);
			}

			rport = atoi(buffer);

			return RvSipViaHeaderSetRportParam(hHeader, rport, RV_TRUE);
		}
	case RVSIP_VIA_FIELD_MADDR_PARAM:
		{
			return RvSipViaHeaderSetMaddrParam(hHeader, buffer);
		}
	case RVSIP_VIA_FIELD_RECEIVED_PARAM:
		{
			return RvSipViaHeaderSetReceivedParam(hHeader, buffer);
		}
	case RVSIP_VIA_FIELD_BRANCH_PARAM:
		{
			return RvSipViaHeaderSetBranchParam(hHeader, buffer);
		}
	case RVSIP_VIA_FIELD_COMP_PARAM:
		{
			RvSipCompType eCompType;
			
			if (NULL == buffer)
			{
				return RvSipViaHeaderSetCompParam(hHeader, RVSIP_COMP_UNDEFINED, NULL);
			}
			
			/* First convert string into enumeration and then set */
			eCompType = SipCommonConvertStrToEnumCompType(buffer);
			return RvSipViaHeaderSetCompParam(hHeader, eCompType, buffer);
		}
	case RVSIP_VIA_FIELD_OTHER_PARAMS:
		{
			return RvSipViaHeaderSetOtherParams(hHeader, buffer);
		}
	case RVSIP_VIA_FIELD_BAD_SYNTAX:
		{
			return RvSipViaHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                        pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_VIA_FIELD_HOST:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_HOST);
			break;
		}
	case RVSIP_VIA_FIELD_TRANSPORT:
		{
			RvSipTransport eTransport = RvSipViaHeaderGetTransport(hHeader);
			if (eTransport != RVSIP_TRANSPORT_OTHER &&
				eTransport != RVSIP_TRANSPORT_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_TRANSPORT);
			}
			break;
		}
	case RVSIP_VIA_FIELD_RPORT_PARAM:
		{
			RvInt32  rport;
			RvBool   bUseRport;
			RvStatus rv;

			rv = RvSipViaHeaderGetRportParam(hHeader, &rport, &bUseRport);
			if (RV_OK != rv || bUseRport == RV_FALSE)
			{
				return rv;
			}

			if (rport == UNDEFINED)
			{
				*pLength = 1;
			}
			else
			{
				*pLength = MAX_LENGTH_OF_INT;
			}
			break;
		}
	case RVSIP_VIA_FIELD_MADDR_PARAM:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_MADDR_PARAM);
			break;
		}
	case RVSIP_VIA_FIELD_RECEIVED_PARAM:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_RECEIVED_PARAM);
			break;
		}
	case RVSIP_VIA_FIELD_BRANCH_PARAM:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_BRANCH_PARAM);
			break;
		}
	case RVSIP_VIA_FIELD_COMP_PARAM:
		{
			RvSipCompType eCompType = RvSipViaHeaderGetCompParam(hHeader);
			if (eCompType != RVSIP_COMP_OTHER &&
				eCompType != RVSIP_COMP_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_COMP_PARAM);
			}
			break;
		}
	case RVSIP_VIA_FIELD_OTHER_PARAMS:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_OTHER_PARAMS);
			break;
		}
	case RVSIP_VIA_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipViaHeaderGetStringLength(hHeader, RVSIP_VIA_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                    OUT RvInt32*                    actualLen)
{
	switch(eField) {
	case RVSIP_VIA_FIELD_HOST:
		{
			return RvSipViaHeaderGetHost(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_VIA_FIELD_TRANSPORT:
		{
			RvSipTransport eTransport = RvSipViaHeaderGetTransport(hHeader);
			if (eTransport == RVSIP_TRANSPORT_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				transport set to this allowed header */
				return RvSipViaHeaderGetStrTransport(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eTransport != RVSIP_TRANSPORT_UNDEFINED)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strTransport = SipCommonConvertEnumToStrTransportType(eTransport);
				return StringFieldCopy(strTransport, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_VIA_FIELD_RPORT_PARAM:
		{
			RvInt32  rport;
			RvBool   bUseRport;
			RvStatus rv;
			
			rv = RvSipViaHeaderGetRportParam(hHeader, &rport, &bUseRport);
			if (RV_OK != rv)
			{
				return rv;
			}
			
			if (bUseRport == RV_FALSE)
			{
				return RV_ERROR_NOT_FOUND;
			}
			
			if (UNDEFINED == rport)
			{
				if (bufferLen < 1)
				{
					return RV_ERROR_INSUFFICIENT_BUFFER;
				}
				buffer[0] = '\0';
				*actualLen = 1;
			}
			else
			{
				MsgUtils_itoa(rport, buffer);
				*actualLen = strlen(buffer);
			}

			break;
		}
	case RVSIP_VIA_FIELD_MADDR_PARAM:
		{
			return RvSipViaHeaderGetMaddrParam(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_VIA_FIELD_RECEIVED_PARAM:
		{
			return RvSipViaHeaderGetReceivedParam(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_VIA_FIELD_BRANCH_PARAM:
		{
			return RvSipViaHeaderGetBranchParam(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_VIA_FIELD_COMP_PARAM:
		{
			RvSipCompType eCompType = RvSipViaHeaderGetCompParam(hHeader);
			if (eCompType == RVSIP_COMP_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				transport set to this allowed header */
				return RvSipViaHeaderGetStrCompParam(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eCompType != RVSIP_COMP_UNDEFINED)
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strCompType = SipCommonConvertEnumToStrCompType(eCompType);
				return StringFieldCopy(strCompType, buffer, bufferLen, actualLen);
			}
			else
			{
				*actualLen = 0;
				return RV_ERROR_NOT_FOUND;
			}
		}
	case RVSIP_VIA_FIELD_OTHER_PARAMS:
		{
			return RvSipViaHeaderGetOtherParams(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_VIA_FIELD_BAD_SYNTAX:
		{
			return RvSipViaHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderSetIntField(
                                          IN RvSipViaHeaderHandle       hHeader,
										  IN RvSipViaHeaderFieldName    eField,
                                          IN RvInt32                    fieldValue)
{
	switch (eField) {
	case RVSIP_VIA_FIELD_PORT:
		{
			return RvSipViaHeaderSetPortNum(hHeader, fieldValue);
		}
	case RVSIP_VIA_FIELD_TTL_PARAM:
		{
			return RvSipViaHeaderSetTtlNum(hHeader, (RvInt16)fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderGetIntField(
                                          IN  RvSipViaHeaderHandle       hHeader,
										  IN  RvSipViaHeaderFieldName    eField,
                                          OUT RvInt32*                   pFieldValue)
{
	switch (eField) {
	case RVSIP_VIA_FIELD_PORT:
		{
			*pFieldValue = RvSipViaHeaderGetPortNum(hHeader);
			break;
		}
	case RVSIP_VIA_FIELD_TTL_PARAM:
		{
			*pFieldValue = (RvInt32)RvSipViaHeaderGetTtlNum(hHeader);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvBool                    fieldValue)
{
	switch(eField) {
	case RVSIP_VIA_FIELD_HIDDEN_PARAM:
		{
			return RvSipViaHeaderSetHiddenParam(hHeader, fieldValue);
		}
	case RVSIP_VIA_FIELD_ALIAS_PARAM:
		{
			return RvSipViaHeaderSetAliasParam(hHeader, fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvBool*                   pFieldValue)
{
	switch(eField) {
	case RVSIP_VIA_FIELD_HIDDEN_PARAM:
		{
			*pFieldValue = RvSipViaHeaderGetHiddenParam(hHeader);
			break;
		}
	case RVSIP_VIA_FIELD_ALIAS_PARAM:
		{
			*pFieldValue = RvSipViaHeaderGetAliasParam(hHeader);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}


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
void RVCALLCONV WarningHeaderDestruct(IN RvSipWarningHeaderHandle hHeader)
{
	HRPOOL                           hPool;
	HPAGE                            hPage;
	MsgWarningHeader			    *pHeader = (MsgWarningHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
				"WarningHeaderDestruct - Destructing header 0x%p",
				hHeader));	
	
	hPool = SipWarningHeaderGetPool(hHeader);
	hPage = SipWarningHeaderGetPage(hHeader);
	
	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                     buffer)
{
	switch(eField) {
	case RVSIP_WARNING_FIELD_WARN_AGENT:
		{
			return RvSipWarningHeaderSetWarnAgent(hHeader, buffer);
		}
	case RVSIP_WARNING_FIELD_WARN_TEXT:
		{
			return RvSipWarningHeaderSetWarnText(hHeader, buffer);
		}
	case RVSIP_WARNING_FIELD_BAD_SYNTAX:
		{
			return RvSipWarningHeaderSetStrBadSyntax(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                          pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_WARNING_FIELD_WARN_AGENT:
		{
			*pLength = (RvInt32)RvSipWarningHeaderGetStringLength(hHeader, RVSIP_WARNING_WARN_AGENT);
			break;
		}
	case RVSIP_WARNING_FIELD_WARN_TEXT:
		{
			*pLength = (RvInt32)RvSipWarningHeaderGetStringLength(hHeader, RVSIP_WARNING_WARN_TEXT);
			break;
		}
	case RVSIP_WARNING_FIELD_BAD_SYNTAX:
		{
			*pLength = (RvInt32)RvSipWarningHeaderGetStringLength(hHeader, RVSIP_WARNING_BAD_SYNTAX);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                       actualLen)
{
	switch(eField) {
	case RVSIP_WARNING_FIELD_WARN_AGENT:
		{
			return RvSipWarningHeaderGetWarnAgent(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_WARNING_FIELD_WARN_TEXT:
		{
			return RvSipWarningHeaderGetWarnText(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_WARNING_FIELD_BAD_SYNTAX:
		{
			return RvSipWarningHeaderGetStrBadSyntax(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

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
                                          IN RvInt32                       fieldValue)
{
	switch (eField) {
	case RVSIP_WARNING_FIELD_WARN_CODE:
		{
			return RvSipWarningHeaderSetWarnCode(hHeader, (RvInt16)fieldValue);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*                      pFieldValue)
{
	*pFieldValue = UNDEFINED;

	switch (eField) {
	case RVSIP_WARNING_FIELD_WARN_CODE:
		{
			*pFieldValue = (RvInt32)RvSipWarningHeaderGetWarnCode(hHeader);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
void RVCALLCONV OtherHeaderDestruct(IN RvSipOtherHeaderHandle hHeader)
{
	HRPOOL                hPool;
	HPAGE                 hPage;
	MsgOtherHeader       *pHeader = (MsgOtherHeader*)hHeader;
	
	RvLogInfo(pHeader->pMsgMgr->pLogSrc, (pHeader->pMsgMgr->pLogSrc,
		"OtherHeaderDestruct - Destructing header 0x%p",
		hHeader));	
	
	hPool = SipOtherHeaderGetPool(hHeader);
	hPage = SipOtherHeaderGetPage(hHeader);

	RPOOL_FreePage(hPool, hPage);
}

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
                                          IN RvChar*                   buffer)
{
	switch(eField) {
	case RVSIP_OTHER_HEADER_FIELD_NAME:
		{
			return RvSipOtherHeaderSetName(hHeader, buffer);
		}
	case RVSIP_OTHER_HEADER_FIELD_VALUE:
		{
			return RvSipOtherHeaderSetValue(hHeader, buffer);
		}
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                    OUT RvInt32*                        pLength)
{
	*pLength = 0;
	
	switch(eField) {
	case RVSIP_OTHER_HEADER_FIELD_NAME:
		{
			*pLength = (RvInt32)RvSipOtherHeaderGetStringLength(hHeader, RVSIP_OTHER_HEADER_NAME);
			break;
		}
	case RVSIP_OTHER_HEADER_FIELD_VALUE:
		{
			*pLength = (RvInt32)RvSipOtherHeaderGetStringLength(hHeader, RVSIP_OTHER_HEADER_VALUE);
			break;
		}
	default:
		return RV_ERROR_BADPARAM;
	}
	
	return RV_OK;
}

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
                                    OUT RvInt32*                     actualLen)
{
	switch(eField) {
	case RVSIP_OTHER_HEADER_FIELD_NAME:
		{
			return RvSipOtherHeaderGetName(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_OTHER_HEADER_FIELD_VALUE:
		{
			return RvSipOtherHeaderGetValue(hHeader, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	default:
		return RV_ERROR_BADPARAM;
	}

	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * StringFieldCopy
 * ------------------------------------------------------------------------
 * General: This function copies a static string onto a given buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strToCopy - The string to copy
 *          buffer    - The buffer to fill
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the copied string
 ***************************************************************************/
static RvStatus RVCALLCONV StringFieldCopy(
                                    IN  RvChar*                     strToCopy,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen)
{
	RvInt32     strLen;

	strLen  = strlen(strToCopy);

	*actualLen = strLen + 1;

	if (strLen > bufferLen - 1)
	{
		return RV_ERROR_INSUFFICIENT_BUFFER;
	}

	strcpy(buffer, strToCopy);
	
	return RV_OK;
}


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif






















