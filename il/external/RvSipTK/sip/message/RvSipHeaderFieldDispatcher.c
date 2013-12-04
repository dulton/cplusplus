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


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipHeaderFieldDispatcher.h"
#include "HeaderFieldDispatcher.h"
#include "RvSipOtherHeader.h"


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
/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

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
                                          IN RvSipHeaderType    eType)
{
	if (hHeader == NULL)
	{
		return;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_ACCEPT:
		{
			AcceptHeaderDestruct((RvSipAcceptHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			AcceptEncodingHeaderDestruct((RvSipAcceptEncodingHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			AcceptLanguageHeaderDestruct((RvSipAcceptLanguageHeaderHandle)hHeader);
			break;
		}		
	case RVSIP_HEADERTYPE_ALLOW:
		{
			AllowHeaderDestruct((RvSipAllowHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			AllowEventsHeaderDestruct((RvSipAllowEventsHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			ViaHeaderDestruct((RvSipViaHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			AuthenticationHeaderDestruct((RvSipAuthenticationHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			AuthorizationHeaderDestruct((RvSipAuthorizationHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			AuthenticationInfoHeaderDestruct((RvSipAuthenticationInfoHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			ContentDispositionHeaderDestruct((RvSipContentDispositionHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			ContentTypeHeaderDestruct((RvSipContentTypeHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			CSeqHeaderDestruct((RvSipCSeqHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			ContactHeaderDestruct((RvSipContactHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			DateHeaderDestruct((RvSipDateHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_EVENT:
		{
			EventHeaderDestruct((RvSipEventHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		{
			ExpiresHeaderDestruct((RvSipExpiresHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			InfoHeaderDestruct((RvSipInfoHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			PartyHeaderDestruct((RvSipPartyHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_OTHER:
		{
			OtherHeaderDestruct((RvSipOtherHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			RSeqHeaderDestruct((RvSipRSeqHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			RAckHeaderDestruct((RvSipRAckHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			ReasonHeaderDestruct((RvSipReasonHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			ReferToHeaderDestruct((RvSipReferToHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			ReplyToHeaderDestruct((RvSipReplyToHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			RetryAfterHeaderDestruct((RvSipRetryAfterHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			RouteHopHeaderDestruct((RvSipRouteHopHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			SubscriptionStateHeaderDestruct((RvSipSubscriptionStateHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			TimestampHeaderDestruct((RvSipTimestampHeaderHandle)hHeader);
			break;
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			WarningHeaderDestruct((RvSipWarningHeaderHandle)hHeader);
			break;
		}

	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_UNDEFINED:
	default:
		break;
	}
}

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
										  IN    RvChar*             strCompName)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	switch(eType) {
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return RvSipAllowEventsHeaderSetCompactForm((RvSipAllowEventsHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			return RvSipContentTypeHeaderSetCompactForm((RvSipContentTypeHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return RvSipContactHeaderSetCompactForm((RvSipContactHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_EVENT:
		{
			return RvSipEventHeaderSetCompactForm((RvSipEventHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return RvSipPartyHeaderSetCompactForm((RvSipPartyHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return RvSipReferToHeaderSetCompactForm((RvSipReferToHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_OTHER:
		{
			return RvSipOtherHeaderSetName((RvSipOtherHeaderHandle)hHeader, strCompName);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return RvSipViaHeaderSetCompactForm((RvSipViaHeaderHandle)hHeader, bIsCompact);
		}
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_CSEQ:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	case RVSIP_HEADERTYPE_ACCEPT:
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_TIMESTAMP:
	case RVSIP_HEADERTYPE_INFO:
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			if (bIsCompact == RV_TRUE)
			{
				return RV_ERROR_BADPARAM;
			}
			return RV_OK;
		}
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}
}

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
                                          INOUT RvSipAddressHandle* phAddress)
{
	if (hHeader == NULL || phAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderSetAddressField((RvSipAuthorizationHeaderHandle)hHeader,
													  (RvSipAuthorizationHeaderFieldName)eField,
													  phAddress);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderSetAddressField((RvSipContactHeaderHandle)hHeader,
											    (RvSipContactHeaderFieldName)eField,
												phAddress);
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			return InfoHeaderSetAddressField((RvSipInfoHeaderHandle)hHeader,
											 (RvSipInfoHeaderFieldName)eField,
											 phAddress);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return PartyHeaderSetAddressField((RvSipPartyHeaderHandle)hHeader,
											  (RvSipPartyHeaderFieldName)eField,
											  phAddress);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return ReferToHeaderSetAddressField(
											  (RvSipReferToHeaderHandle)hHeader,
											  (RvSipReferToHeaderFieldName)eField,
											  phAddress);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return ReplyToHeaderSetAddressField(
											  (RvSipReplyToHeaderHandle)hHeader,
											  (RvSipReplyToHeaderFieldName)eField,
											  phAddress);
		}
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			return RouteHopHeaderSetAddressField(
											  (RvSipRouteHopHeaderHandle)hHeader,
											  (RvSipRouteHopHeaderFieldName)eField,
											  phAddress);
		}
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_VIA:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_CSEQ:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvSipAddressHandle  *phAddress)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (phAddress == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderGetAddressField((RvSipAuthorizationHeaderHandle)hHeader,
													 (RvSipAuthorizationHeaderFieldName)eField,
													 phAddress);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderGetAddressField((RvSipContactHeaderHandle)hHeader,
												(RvSipContactHeaderFieldName)eField,
												phAddress);
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			return InfoHeaderGetAddressField((RvSipInfoHeaderHandle)hHeader,
											(RvSipInfoHeaderFieldName)eField,
											phAddress);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return PartyHeaderGetAddressField((RvSipPartyHeaderHandle)hHeader,
											  (RvSipPartyHeaderFieldName)eField,
											  phAddress);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return ReferToHeaderGetAddressField((RvSipReferToHeaderHandle)hHeader,
											    (RvSipReferToHeaderFieldName)eField,
											    phAddress);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return ReplyToHeaderGetAddressField((RvSipReplyToHeaderHandle)hHeader,
											    (RvSipReplyToHeaderFieldName)eField,
											    phAddress);
		}
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			return RouteHopHeaderGetAddressField((RvSipRouteHopHeaderHandle)hHeader,
											    (RvSipRouteHopHeaderFieldName)eField,
											    phAddress);
		}
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_VIA:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_CSEQ:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          IN RvChar*            buffer)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_ACCEPT:
		{
			return AcceptHeaderSetStringField((RvSipAcceptHeaderHandle)hHeader,
											  (RvSipAcceptHeaderFieldName)eField,
											  buffer);
		}
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			return AcceptEncodingHeaderSetStringField((RvSipAcceptEncodingHeaderHandle)hHeader,
													  (RvSipAcceptEncodingHeaderFieldName)eField,
													  buffer);
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			return AcceptLanguageHeaderSetStringField((RvSipAcceptLanguageHeaderHandle)hHeader,
													  (RvSipAcceptLanguageHeaderFieldName)eField,
													  buffer);
		}
	case RVSIP_HEADERTYPE_ALLOW:
		{
			return AllowHeaderSetStringField((RvSipAllowHeaderHandle)hHeader,
											 (RvSipAllowHeaderFieldName)eField,
											 buffer);
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return AllowEventsHeaderSetStringField((RvSipAllowEventsHeaderHandle)hHeader,
													(RvSipAllowEventsHeaderFieldName)eField,
													buffer);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderSetStringField((RvSipViaHeaderHandle)hHeader,
									       (RvSipViaHeaderFieldName)eField,
										   buffer);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return AuthenticationHeaderSetStringField((RvSipAuthenticationHeaderHandle)hHeader,
													  (RvSipAuthenticationHeaderFieldName)eField,
													  buffer);
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderSetStringField((RvSipAuthorizationHeaderHandle)hHeader,
													 (RvSipAuthorizationHeaderFieldName)eField,
													 buffer);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return AuthenticationInfoHeaderSetStringField((RvSipAuthenticationInfoHeaderHandle)hHeader,
														  (RvSipAuthenticationInfoHeaderFieldName)eField,
														  buffer);
		}
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			return ContentDispositionHeaderSetStringField((RvSipContentDispositionHeaderHandle)hHeader,
														  (RvSipContentDispositionHeaderFieldName)eField,
														  buffer);
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			return ContentTypeHeaderSetStringField((RvSipContentTypeHeaderHandle)hHeader,
												   (RvSipContentTypeHeaderFieldName)eField,
												   buffer);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return CSeqHeaderSetStringField((RvSipCSeqHeaderHandle)hHeader,
											(RvSipCSeqHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderSetStringField((RvSipContactHeaderHandle)hHeader,
												(RvSipContactHeaderFieldName)eField,
												buffer);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return DateHeaderSetStringField((RvSipDateHeaderHandle)hHeader,
											(RvSipDateHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_EVENT:
		{
			return EventHeaderSetStringField((RvSipEventHeaderHandle)hHeader,
											 (RvSipEventHeaderFieldName)eField,
											 buffer);
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		{
			return ExpiresHeaderSetStringField((RvSipExpiresHeaderHandle)hHeader,
											 (RvSipExpiresHeaderFieldName)eField,
											 buffer);
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			return InfoHeaderSetStringField((RvSipInfoHeaderHandle)hHeader,
											(RvSipInfoHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return PartyHeaderSetStringField((RvSipPartyHeaderHandle)hHeader,
											(RvSipPartyHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RSeqHeaderSetStringField((RvSipRSeqHeaderHandle)hHeader,
											(RvSipRSeqHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RAckHeaderSetStringField((RvSipRAckHeaderHandle)hHeader,
											(RvSipRAckHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			return ReasonHeaderSetStringField((RvSipReasonHeaderHandle)hHeader,
											  (RvSipReasonHeaderFieldName)eField,
											  buffer);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return ReferToHeaderSetStringField((RvSipReferToHeaderHandle)hHeader,
											   (RvSipReferToHeaderFieldName)eField,
											   buffer);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return ReplyToHeaderSetStringField((RvSipReplyToHeaderHandle)hHeader,
											   (RvSipReplyToHeaderFieldName)eField,
											   buffer);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return SubscriptionStateHeaderSetStringField(
											(RvSipSubscriptionStateHeaderHandle)hHeader,
											(RvSipSubscriptionStateHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RetryAfterHeaderSetStringField(
											(RvSipRetryAfterHeaderHandle)hHeader,
											(RvSipRetryAfterHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			return RouteHopHeaderSetStringField(
											(RvSipRouteHopHeaderHandle)hHeader,
											(RvSipRouteHopHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_OTHER:
		{
			return OtherHeaderSetStringField(
											(RvSipOtherHeaderHandle)hHeader,
											(RvSipOtherHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			return TimestampHeaderSetStringField(
											(RvSipTimestampHeaderHandle)hHeader,
											(RvSipTimestampHeaderFieldName)eField,
											buffer);
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			return WarningHeaderSetStringField(
											(RvSipWarningHeaderHandle)hHeader,
											(RvSipWarningHeaderFieldName)eField,
											buffer);
		}
		
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_UNDEFINED:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
											OUT RvInt32*          pLength)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pLength == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	switch(eType) {
	case RVSIP_HEADERTYPE_ACCEPT:
		{
			return AcceptHeaderGetStringFieldLength(
										(RvSipAcceptHeaderHandle)hHeader,
										(RvSipAcceptHeaderFieldName)eField,
										pLength);
		}	
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			return AcceptEncodingHeaderGetStringFieldLength(
										(RvSipAcceptEncodingHeaderHandle)hHeader,
										(RvSipAcceptEncodingHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			return AcceptLanguageHeaderGetStringFieldLength(
										(RvSipAcceptLanguageHeaderHandle)hHeader,
										(RvSipAcceptLanguageHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_ALLOW:
		{
			return AllowHeaderGetStringFieldLength(
										(RvSipAllowHeaderHandle)hHeader,
										(RvSipAllowHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return AllowEventsHeaderGetStringFieldLength(
										(RvSipAllowEventsHeaderHandle)hHeader,
										(RvSipAllowEventsHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return AuthenticationHeaderGetStringFieldLength(
										(RvSipAuthenticationHeaderHandle)hHeader,
										(RvSipAuthenticationHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderGetStringFieldLength(
										(RvSipAuthorizationHeaderHandle)hHeader,
										(RvSipAuthorizationHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return AuthenticationInfoHeaderGetStringFieldLength(
										(RvSipAuthenticationInfoHeaderHandle)hHeader,
										(RvSipAuthenticationInfoHeaderFieldName)eField,
										pLength);	
		}
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			return ContentDispositionHeaderGetStringFieldLength(
								(RvSipContentDispositionHeaderHandle)hHeader,
								(RvSipContentDispositionHeaderFieldName)eField,
								pLength);	
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			return ContentTypeHeaderGetStringFieldLength(
								(RvSipContentTypeHeaderHandle)hHeader,
								(RvSipContentTypeHeaderFieldName)eField,
								pLength);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return CSeqHeaderGetStringFieldLength(
										(RvSipCSeqHeaderHandle)hHeader,
										(RvSipCSeqHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderGetStringFieldLength(
										(RvSipContactHeaderHandle)hHeader,
										(RvSipContactHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return DateHeaderGetStringFieldLength(
										(RvSipDateHeaderHandle)hHeader,
										(RvSipDateHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_EVENT:
		{
			return EventHeaderGetStringFieldLength(
										(RvSipEventHeaderHandle)hHeader,
										(RvSipEventHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		return ExpiresHeaderGetStringFieldLength(
										(RvSipExpiresHeaderHandle)hHeader,
										(RvSipExpiresHeaderFieldName)eField,
										pLength);
	case RVSIP_HEADERTYPE_INFO:
		{
			return InfoHeaderGetStringFieldLength(
										(RvSipInfoHeaderHandle)hHeader,
										(RvSipInfoHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return PartyHeaderGetStringFieldLength(
										(RvSipPartyHeaderHandle)hHeader,
										(RvSipPartyHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderGetStringFieldLength(
										(RvSipViaHeaderHandle)hHeader,
									    (RvSipViaHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RSeqHeaderGetStringFieldLength(
										(RvSipRSeqHeaderHandle)hHeader,
									    (RvSipRSeqHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RAckHeaderGetStringFieldLength(
										(RvSipRAckHeaderHandle)hHeader,
									    (RvSipRAckHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			return ReasonHeaderGetStringFieldLength(
										(RvSipReasonHeaderHandle)hHeader,
									    (RvSipReasonHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return ReferToHeaderGetStringFieldLength(
										(RvSipReferToHeaderHandle)hHeader,
									    (RvSipReferToHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return ReplyToHeaderGetStringFieldLength(
										(RvSipReplyToHeaderHandle)hHeader,
									    (RvSipReplyToHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RetryAfterHeaderGetStringFieldLength(
										(RvSipRetryAfterHeaderHandle)hHeader,
									    (RvSipRetryAfterHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			return RouteHopHeaderGetStringFieldLength(
										(RvSipRouteHopHeaderHandle)hHeader,
									    (RvSipRouteHopHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return SubscriptionStateHeaderGetStringFieldLength(
										(RvSipSubscriptionStateHeaderHandle)hHeader,
									    (RvSipSubscriptionStateHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_OTHER:
		{
			return OtherHeaderGetStringFieldLength(
										(RvSipOtherHeaderHandle)hHeader,
									    (RvSipOtherHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			return TimestampHeaderGetStringFieldLength(
										(RvSipTimestampHeaderHandle)hHeader,
									    (RvSipTimestampHeaderFieldName)eField,
										pLength);
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			return WarningHeaderGetStringFieldLength(
										(RvSipWarningHeaderHandle)hHeader,
									    (RvSipWarningHeaderFieldName)eField,
										pLength);
		}
		
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_UNDEFINED:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*           actualLen)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (buffer == NULL || actualLen == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	switch(eType) {
	case RVSIP_HEADERTYPE_ACCEPT:
		{
			return AcceptHeaderGetStringField(
											 (RvSipAcceptHeaderHandle)hHeader,
											 (RvSipAcceptHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}	
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			return AcceptEncodingHeaderGetStringField(
											 (RvSipAcceptEncodingHeaderHandle)hHeader,
											 (RvSipAcceptEncodingHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			return AcceptLanguageHeaderGetStringField(
											 (RvSipAcceptLanguageHeaderHandle)hHeader,
											 (RvSipAcceptLanguageHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_ALLOW:
		{
			return AllowHeaderGetStringField((RvSipAllowHeaderHandle)hHeader,
											 (RvSipAllowHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return AllowEventsHeaderGetStringField((RvSipAllowEventsHeaderHandle)hHeader,
												   (RvSipAllowEventsHeaderFieldName)eField,
												   buffer,
												   bufferLen,
												   actualLen);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return AuthenticationHeaderGetStringField((RvSipAuthenticationHeaderHandle)hHeader,
													  (RvSipAuthenticationHeaderFieldName)eField,
													  buffer,
													  bufferLen,
													  actualLen);
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderGetStringField((RvSipAuthorizationHeaderHandle)hHeader,
													 (RvSipAuthorizationHeaderFieldName)eField,
													 buffer,
													 bufferLen,
												     actualLen);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return AuthenticationInfoHeaderGetStringField((RvSipAuthenticationInfoHeaderHandle)hHeader,
														(RvSipAuthenticationInfoHeaderFieldName)eField,
														buffer,
														bufferLen,
														actualLen);
		}
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			return ContentDispositionHeaderGetStringField((RvSipContentDispositionHeaderHandle)hHeader,
														 (RvSipContentDispositionHeaderFieldName)eField,
														 buffer,
														 bufferLen,
														 actualLen);
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			return ContentTypeHeaderGetStringField((RvSipContentTypeHeaderHandle)hHeader,
												   (RvSipContentTypeHeaderFieldName)eField,
												   buffer,
												   bufferLen,
												   actualLen);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return CSeqHeaderGetStringField((RvSipCSeqHeaderHandle)hHeader,
											(RvSipCSeqHeaderFieldName)eField,
											buffer,
											bufferLen,
											actualLen);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderGetStringField((RvSipContactHeaderHandle)hHeader,
											   (RvSipContactHeaderFieldName)eField,
											   buffer,
											   bufferLen,
											   actualLen);
		}
	case RVSIP_HEADERTYPE_EVENT:
		{
			return EventHeaderGetStringField((RvSipEventHeaderHandle)hHeader,
											 (RvSipEventHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		{
			return ExpiresHeaderGetStringField((RvSipExpiresHeaderHandle)hHeader,
											 (RvSipExpiresHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return DateHeaderGetStringField((RvSipDateHeaderHandle)hHeader,
											 (RvSipDateHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			return InfoHeaderGetStringField((RvSipInfoHeaderHandle)hHeader,
											 (RvSipInfoHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
		{
			return PartyHeaderGetStringField((RvSipPartyHeaderHandle)hHeader,
											 (RvSipPartyHeaderFieldName)eField,
											 buffer,
											 bufferLen,
											 actualLen);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderGetStringField((RvSipViaHeaderHandle)hHeader,
									       (RvSipViaHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RSeqHeaderGetStringField((RvSipRSeqHeaderHandle)hHeader,
									       (RvSipRSeqHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RAckHeaderGetStringField((RvSipRAckHeaderHandle)hHeader,
									       (RvSipRAckHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			return ReasonHeaderGetStringField((RvSipReasonHeaderHandle)hHeader,
									         (RvSipReasonHeaderFieldName)eField,
										     buffer,
										     bufferLen,
										     actualLen);
		}
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return ReferToHeaderGetStringField(
										   (RvSipReferToHeaderHandle)hHeader,
									       (RvSipReferToHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}	
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return ReplyToHeaderGetStringField(
										   (RvSipReplyToHeaderHandle)hHeader,
									       (RvSipReplyToHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}	
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RetryAfterHeaderGetStringField(
										   (RvSipRetryAfterHeaderHandle)hHeader,
									       (RvSipRetryAfterHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}	
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			return RouteHopHeaderGetStringField(
										   (RvSipRouteHopHeaderHandle)hHeader,
									       (RvSipRouteHopHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}	
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return SubscriptionStateHeaderGetStringField(
										   (RvSipSubscriptionStateHeaderHandle)hHeader,
									       (RvSipSubscriptionStateHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_OTHER:
		{
			return OtherHeaderGetStringField(
										   (RvSipOtherHeaderHandle)hHeader,
									       (RvSipOtherHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			return TimestampHeaderGetStringField(
										   (RvSipTimestampHeaderHandle)hHeader,
									       (RvSipTimestampHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			return WarningHeaderGetStringField(
										   (RvSipWarningHeaderHandle)hHeader,
									       (RvSipWarningHeaderFieldName)eField,
										   buffer,
										   bufferLen,
										   actualLen);
		}

	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_UNDEFINED:
	default:
		return RV_ERROR_BADPARAM;
	}
}

/***************************************************************************
 * RvSipHeaderSetIntField
 * ------------------------------------------------------------------------
 * General: This is a generic function that can be used for all types of headers,
 *          to set any desirable integer field value (including emunerations).
 *          For example: to set the CSeq step of a CSeq header, call
 *          this function with eType RVSIP_HEADERTYPE_CSEQ, eField 0 and the
 *          integer indicating the CSeq step.
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
                                          IN RvInt32            fieldValue)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderSetIntField((RvSipAuthorizationHeaderHandle)hHeader,
												  (RvSipAuthorizationHeaderFieldName)eField,
												  fieldValue);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return AuthenticationInfoHeaderSetIntField(
												  (RvSipAuthenticationInfoHeaderHandle)hHeader,
												  (RvSipAuthenticationInfoHeaderFieldName)eField,
												  fieldValue);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderSetIntField((RvSipContactHeaderHandle)hHeader,
											(RvSipContactHeaderFieldName)eField,
											fieldValue);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return CSeqHeaderSetIntField((RvSipCSeqHeaderHandle)hHeader,
										 (RvSipCSeqHeaderFieldName)eField,
										 fieldValue);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return DateHeaderSetIntField((RvSipDateHeaderHandle)hHeader,
										 (RvSipDateHeaderFieldName)eField,
										 fieldValue);
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		{
			return ExpiresHeaderSetIntField((RvSipExpiresHeaderHandle)hHeader,
										    (RvSipExpiresHeaderFieldName)eField,
											fieldValue);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderSetIntField((RvSipViaHeaderHandle)hHeader,
				   				        (RvSipViaHeaderFieldName)eField,
										fieldValue);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RSeqHeaderSetIntField((RvSipRSeqHeaderHandle)hHeader,
										 (RvSipRSeqHeaderFieldName)eField,
										 fieldValue);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RAckHeaderSetIntField((RvSipRAckHeaderHandle)hHeader,
										 (RvSipRAckHeaderFieldName)eField,
										 fieldValue);
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			return ReasonHeaderSetIntField((RvSipReasonHeaderHandle)hHeader,
										   (RvSipReasonHeaderFieldName)eField,
										   fieldValue);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RetryAfterHeaderSetIntField((RvSipRetryAfterHeaderHandle)hHeader,
										   (RvSipRetryAfterHeaderFieldName)eField,
										   fieldValue);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return SubscriptionStateHeaderSetIntField(
										 (RvSipSubscriptionStateHeaderHandle)hHeader,
										 (RvSipSubscriptionStateHeaderFieldName)eField,
										 fieldValue);
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			return WarningHeaderSetIntField(
										 (RvSipWarningHeaderHandle)hHeader,
										 (RvSipWarningHeaderFieldName)eField,
										 fieldValue);
		}

	case RVSIP_HEADERTYPE_TIMESTAMP:
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvInt32*           pFieldValue)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pFieldValue == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return AuthorizationHeaderGetIntField((RvSipAuthorizationHeaderHandle)hHeader,
												  (RvSipAuthorizationHeaderFieldName)eField,
												  pFieldValue);
		}
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return AuthenticationInfoHeaderGetIntField((RvSipAuthenticationInfoHeaderHandle)hHeader,
													(RvSipAuthenticationInfoHeaderFieldName)eField,
													pFieldValue);
		}
	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderGetIntField((RvSipContactHeaderHandle)hHeader,
											(RvSipContactHeaderFieldName)eField,
											pFieldValue);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return CSeqHeaderGetIntField((RvSipCSeqHeaderHandle)hHeader,
										(RvSipCSeqHeaderFieldName)eField,
										pFieldValue);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return DateHeaderGetIntField((RvSipDateHeaderHandle)hHeader,
										(RvSipDateHeaderFieldName)eField,
										pFieldValue);
		}
	case RVSIP_HEADERTYPE_EXPIRES:
		{
			return ExpiresHeaderGetIntField((RvSipExpiresHeaderHandle)hHeader,
											(RvSipExpiresHeaderFieldName)eField,
											pFieldValue);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderGetIntField((RvSipViaHeaderHandle)hHeader,
										(RvSipViaHeaderFieldName)eField,
										pFieldValue);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RSeqHeaderGetIntField((RvSipRSeqHeaderHandle)hHeader,
										 (RvSipRSeqHeaderFieldName)eField,
										  pFieldValue);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RAckHeaderGetIntField((RvSipRAckHeaderHandle)hHeader,
										 (RvSipRAckHeaderFieldName)eField,
										  pFieldValue);
		}
	case RVSIP_HEADERTYPE_REASON:
		{
			return ReasonHeaderGetIntField((RvSipReasonHeaderHandle)hHeader,
										   (RvSipReasonHeaderFieldName)eField,
										   pFieldValue);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RetryAfterHeaderGetIntField((RvSipRetryAfterHeaderHandle)hHeader,
										      (RvSipRetryAfterHeaderFieldName)eField,
										      pFieldValue);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return SubscriptionStateHeaderGetIntField(
										 (RvSipSubscriptionStateHeaderHandle)hHeader,
										 (RvSipSubscriptionStateHeaderFieldName)eField,
										  pFieldValue);
		}
	case RVSIP_HEADERTYPE_WARNING:
		{
			return WarningHeaderGetIntField(
										 (RvSipWarningHeaderHandle)hHeader,
										 (RvSipWarningHeaderFieldName)eField,
										  pFieldValue);
		}

	case RVSIP_HEADERTYPE_TIMESTAMP:
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          IN RvBool             fieldValue)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return AuthenticationHeaderSetBoolField((RvSipAuthenticationHeaderHandle)hHeader,
													(RvSipAuthenticationHeaderFieldName)eField,
													fieldValue);
		}

	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderSetBoolField((RvSipContactHeaderHandle)hHeader,
													(RvSipContactHeaderFieldName)eField,
													fieldValue);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderSetBoolField((RvSipViaHeaderHandle)hHeader,
										(RvSipViaHeaderFieldName)eField,
										fieldValue);
		}
	case RVSIP_HEADERTYPE_CSEQ:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

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
                                          OUT RvBool*            pFieldValue)
{
	if (hHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pFieldValue == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	switch(eType) {
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return AuthenticationHeaderGetBoolField((RvSipAuthenticationHeaderHandle)hHeader,
												    (RvSipAuthenticationHeaderFieldName)eField,
												    pFieldValue);
		}

	case RVSIP_HEADERTYPE_CONTACT:
		{
			return ContactHeaderGetBoolField((RvSipContactHeaderHandle)hHeader,
												    (RvSipContactHeaderFieldName)eField,
												    pFieldValue);
		}
	case RVSIP_HEADERTYPE_VIA:
		{
			return ViaHeaderGetBoolField((RvSipViaHeaderHandle)hHeader,
										(RvSipViaHeaderFieldName)eField,
										pFieldValue);
		}
	case RVSIP_HEADERTYPE_CSEQ:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_TO:
	case RVSIP_HEADERTYPE_FROM:
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_UNDEFINED:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	default:
		return RV_ERROR_BADPARAM;
	}
}

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif

