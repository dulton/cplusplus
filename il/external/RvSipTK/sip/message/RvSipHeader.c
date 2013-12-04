/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipHeader.c                                       *
 *                                                                            *
 * The file defines general methods to handle headers by its types.           *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             April 2003                                           *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipHeader.h"
#include "_SipHeader.h"
#include "RvSipDateHeader.h"
#include "_SipMsg.h"
#include "_SipMsgMgr.h"
#include "_SipOtherHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include "RvSipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipBody.h"
#include "RvSipContentTypeHeader.h"
#include "RvSipContentIDHeader.h"
#include "RvSipRAckHeader.h"
#include "RvSipRSeqHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipRetryAfterHeader.h"
#include "RvSipReplacesHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "_SipAllowHeader.h"
#include "_SipContactHeader.h"
#include "_SipCSeqHeader.h"
#include "_SipPartyHeader.h"
#include "_SipOtherHeader.h"
#include "_SipViaHeader.h"
#include "_SipExpiresHeader.h"
#include "_SipRouteHopHeader.h"
#include "_SipReferToHeader.h"
#include "_SipReferredByHeader.h"
#include "_SipContentTypeHeader.h"
#include "_SipContentIDHeader.h"
#include "_SipContentDispositionHeader.h"
#include "_SipRAckHeader.h"
#include "_SipRetryAfterHeader.h"
#include "_SipAuthenticationHeader.h"
#include "_SipAuthorizationHeader.h"
#include "_SipEventHeader.h"
#include "_SipAllowEventsHeader.h"
#include "_SipSubscriptionStateHeader.h"
#include "_SipSessionExpiresHeader.h"
#include "_SipMinSEHeader.h"
#include "_SipReplacesHeader.h"
#include "_SipDateHeader.h"
#include "_SipRSeqHeader.h"
#include "RvSipAuthenticationInfoHeader.h"
#include "_SipAuthenticationInfoHeader.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
#include "RvSipReasonHeader.h"
#include "_SipReasonHeader.h"
#include "RvSipWarningHeader.h"
#include "_SipWarningHeader.h"
#include "RvSipTimestampHeader.h"
#include "_SipTimestampHeader.h"
#include "RvSipInfoHeader.h"
#include "_SipInfoHeader.h"
#include "RvSipAcceptHeader.h"
#include "_SipAcceptHeader.h"
#include "RvSipAcceptEncodingHeader.h"
#include "_SipAcceptEncodingHeader.h"
#include "RvSipAcceptLanguageHeader.h"
#include "_SipAcceptLanguageHeader.h"
#include "RvSipReplyToHeader.h"
#include "_SipReplyToHeader.h"
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
	
	/* XXX */
#ifdef RV_SIP_IMS_HEADER_SUPPORT	
#include "RvSipPUriHeader.h"
#include  "_SipPUriHeader.h"
#include "RvSipPVisitedNetworkIDHeader.h"
#include  "_SipPVisitedNetworkIDHeader.h"
#include "RvSipPAccessNetworkInfoHeader.h"
#include  "_SipPAccessNetworkInfoHeader.h"
#include "RvSipPChargingFunctionAddressesHeader.h"
#include  "_SipPChargingFunctionAddressesHeader.h"
#include "RvSipPChargingVectorHeader.h"
#include  "_SipPChargingVectorHeader.h"
#include "RvSipPMediaAuthorizationHeader.h"
#include  "_SipPMediaAuthorizationHeader.h"	
#include "RvSipSecurityHeader.h"
#include "_SipSecurityHeader.h"
#include "RvSipPProfileKeyHeader.h"
#include  "_SipPProfileKeyHeader.h"
#include "RvSipPUserDatabaseHeader.h"
#include  "_SipPUserDatabaseHeader.h"
#include "RvSipPAnswerStateHeader.h"
#include  "_SipPAnswerStateHeader.h"
#include "RvSipPServedUserHeader.h"
#include  "_SipPServedUserHeader.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */	

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
#include "RvSipPDCSTracePartyIDHeader.h"
#include "_SipPDCSTracePartyIDHeader.h"
#include "RvSipPDCSOSPSHeader.h"
#include "_SipPDCSOSPSHeader.h"
#include "RvSipPDCSBillingInfoHeader.h"
#include "_SipPDCSBillingInfoHeader.h"
#include "RvSipPDCSLAESHeader.h"
#include "_SipPDCSLAESHeader.h"
#include "RvSipPDCSRedirectHeader.h"
#include "_SipPDCSRedirectHeader.h"
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

#include "_SipCommonUtils.h"

/*#include <memory.h>*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvLogSource* RVCALLCONV GetLogIdFromHeader(void*           pHeader,
                                                  RvSipHeaderType eHeaderType);
#else
#define GetLogIdFromHeader(x,y) 0
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef RV_SIP_JSR32_SUPPORT
static RvSipHeaderType RVCALLCONV ResolveHeaderTypeFromBuffer(
											  IN  RvChar*             buffer);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs a header of any type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * input:  hMsgMgr  - Handle to the Message manager.
 *         eType    - Type of the header to construct.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: ppHeader - Pointer to the newly constructed header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderConstruct(
                                             IN  RvSipMsgMgrHandle         hMsgMgr,
											 IN  RvSipHeaderType           eType,
											 IN  HRPOOL                    hPool,
											 IN  HPAGE                     hPage,
											 OUT void**                    ppHeader)
{
	return SipHeaderConstruct(hMsgMgr, eType, hPool, hPage, ppHeader);
}
	
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs a header of any type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * input:  hMsgMgr  - Handle to the Message manager.
 *         eType    - Type of the header to construct.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: ppHeader - Pointer to the newly constructed header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderConstruct(IN  RvSipMsgMgrHandle         hMsgMgr,
                                       IN  RvSipHeaderType           eType,
                                       IN  HRPOOL                    hPool,
                                       IN  HPAGE                     hPage,
                                       OUT void**                    ppHeader)
{
    RvStatus rv = RV_OK;

	switch(eType) 
    {
    case RVSIP_HEADERTYPE_VIA:
        {
            return RvSipViaHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipViaHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            return RvSipRouteHopHeaderConstruct(hMsgMgr, hPool, hPage,
                (RvSipRouteHopHeaderHandle*)ppHeader);
        }	
#ifdef RV_SIP_AUTH_ON
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return RvSipAuthenticationHeaderConstruct(hMsgMgr, hPool, hPage, 
													  (RvSipAuthenticationHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return RvSipAuthorizationHeaderConstruct(hMsgMgr, hPool, hPage, 
													(RvSipAuthorizationHeaderHandle*)ppHeader);
		}
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_SUBS_ON
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return RvSipReferToHeaderConstruct(hMsgMgr, hPool, hPage, 
											  (RvSipReferToHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_REFERRED_BY:
		{
			return RvSipReferredByHeaderConstruct(hMsgMgr, hPool, hPage, 
												 (RvSipReferredByHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return RvSipSubscriptionStateHeaderConstruct(hMsgMgr, hPool, hPage, 
				(RvSipSubscriptionStateHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return RvSipAllowEventsHeaderConstruct(hMsgMgr, hPool, hPage, 
				(RvSipAllowEventsHeaderHandle*)ppHeader);
		}
#endif /*#ifdef RV_SIP_SUBS_ON*/    
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_HEADERTYPE_EVENT:
		{
			return RvSipEventHeaderConstruct(hMsgMgr, hPool, hPage, 
				(RvSipEventHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ALLOW:
        {
            return RvSipAllowHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipAllowHeaderHandle*)ppHeader);
        }
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			return RvSipContentDispositionHeaderConstruct(hMsgMgr, hPool, hPage, 
														 (RvSipContentDispositionHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RvSipRetryAfterHeaderConstruct(hMsgMgr, hPool, hPage, 
												 (RvSipRetryAfterHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RvSipRSeqHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipRSeqHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RvSipRAckHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipRAckHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            return RvSipSessionExpiresHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipSessionExpiresHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_MINSE:
        {
            return RvSipMinSEHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipMinSEHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_REPLACES:
        {
            return RvSipReplacesHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipReplacesHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_CONTENT_TYPE:
        {
            return RvSipContentTypeHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipContentTypeHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_CONTENT_ID:
        {
            return RvSipContentIDHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipContentIDHeaderHandle*)ppHeader);
		}
#endif /*#ifndef RV_SIP_PRIMITIVES*/
#ifndef RV_SIP_LIGHT
    case RVSIP_HEADERTYPE_CONTACT:
        {
            return RvSipContactHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipContactHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_EXPIRES:
        {
            return RvSipExpiresHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipExpiresHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_DATE:
        {
            return RvSipDateHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipDateHeaderHandle*)ppHeader);
        }
	case RVSIP_HEADERTYPE_TO:
		{
			RvSipPartyHeaderHandle *phToHeader = (RvSipPartyHeaderHandle*)ppHeader;
			rv = RvSipPartyHeaderConstruct(hMsgMgr, hPool, hPage, phToHeader);
			if (RV_OK != rv)
			{
				return rv;
			}
			return RvSipPartyHeaderSetType(*phToHeader, eType);
		}
	case RVSIP_HEADERTYPE_FROM:
		{
			RvSipPartyHeaderHandle *phFromHeader = (RvSipPartyHeaderHandle*)ppHeader;
			rv = RvSipPartyHeaderConstruct(hMsgMgr, hPool, hPage, phFromHeader);
			if (RV_OK != rv)
			{
				return rv;
			}
			return RvSipPartyHeaderSetType(*phFromHeader, eType);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return RvSipCSeqHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipCSeqHeaderHandle*)ppHeader);
		}
#endif /*#ifndef RV_SIP_LIGHT*/
    case RVSIP_HEADERTYPE_OTHER:
        {
            return RvSipOtherHeaderConstruct(hMsgMgr, hPool, hPage, 
                (RvSipOtherHeaderHandle*)ppHeader);
        }
#ifdef RV_SIP_AUTH_ON	
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return RvSipAuthenticationInfoHeaderConstruct(hMsgMgr, hPool, hPage, 
														  (RvSipAuthenticationInfoHeaderHandle*)ppHeader);
		}	
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_REASON:
		{
			return RvSipReasonHeaderConstruct(hMsgMgr, hPool, hPage, 
											 (RvSipReasonHeaderHandle*)ppHeader);
		}	
	case RVSIP_HEADERTYPE_WARNING:
		{
			return RvSipWarningHeaderConstruct(hMsgMgr, hPool, hPage, 
			   								  (RvSipWarningHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			return RvSipTimestampHeaderConstruct(hMsgMgr, hPool, hPage, 
			   								    (RvSipTimestampHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_INFO:
		{
			return RvSipInfoHeaderConstruct(hMsgMgr, hPool, hPage, 
										   (RvSipInfoHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT:
		{
			return RvSipAcceptHeaderConstruct(hMsgMgr, hPool, hPage, 
											  (RvSipAcceptHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			return RvSipAcceptEncodingHeaderConstruct(hMsgMgr, hPool, hPage, 
											          (RvSipAcceptEncodingHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			return RvSipAcceptLanguageHeaderConstruct(hMsgMgr, hPool, hPage, 
													  (RvSipAcceptLanguageHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			return RvSipReplyToHeaderConstruct(hMsgMgr, hPool, hPage, 
												(RvSipReplyToHeaderHandle*)ppHeader);
		}
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */	
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_URI:
	{
		return RvSipPUriHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPUriHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
	{
		return RvSipPVisitedNetworkIDHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPVisitedNetworkIDHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
	{
		return RvSipPAccessNetworkInfoHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPAccessNetworkInfoHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
	{
		return RvSipPChargingFunctionAddressesHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPChargingFunctionAddressesHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
	{
		return RvSipPChargingVectorHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPChargingVectorHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
	{
		return RvSipPMediaAuthorizationHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPMediaAuthorizationHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_SECURITY:
	{
		return RvSipSecurityHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipSecurityHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_PROFILE_KEY:
	{
		return RvSipPProfileKeyHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPProfileKeyHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_USER_DATABASE:
	{
		return RvSipPUserDatabaseHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPUserDatabaseHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_ANSWER_STATE:
	{
		return RvSipPAnswerStateHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPAnswerStateHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_SERVED_USER:
	{
		return RvSipPServedUserHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPServedUserHeaderHandle*)ppHeader);
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
	{
		return RvSipPDCSTracePartyIDHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPDCSTracePartyIDHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_OSPS:
	{
		return RvSipPDCSOSPSHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPDCSOSPSHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
	{
		return RvSipPDCSBillingInfoHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPDCSBillingInfoHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_LAES:
	{
		return RvSipPDCSLAESHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPDCSLAESHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
	{
		return RvSipPDCSRedirectHeaderConstruct(hMsgMgr, hPool, hPage, 
											(RvSipPDCSRedirectHeaderHandle*)ppHeader);
	}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}

#ifdef RV_SIP_LIGHT
    RV_UNUSED_ARG(rv)
#endif
}

/***************************************************************************
 * RvSipHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: ConstructInMsgs a header of any type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * input:  hMsgMgr  - Handle to the Message manager.
 *         eType    - Type of the header to ConstructInMsg.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: ppHeader - Pointer to the newly ConstructInMsged header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderConstructInMsg(
                                             IN  RvSipMsgHandle         hMsg,
											 IN  RvSipHeaderType        eType,
											 OUT void**                 ppHeader)
{
    RvStatus rv = RV_OK;
	switch(eType) 
    {
	case RVSIP_HEADERTYPE_VIA:
		{
			return RvSipViaHeaderConstructInMsg(hMsg, RV_FALSE, 
										  (RvSipViaHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            return RvSipRouteHopHeaderConstructInMsg(hMsg, RV_FALSE,
                (RvSipRouteHopHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_OTHER:
        {
            return RvSipOtherHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipOtherHeaderHandle*)ppHeader);
        }
#ifdef RV_SIP_AUTH_ON
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		{
			return RvSipAuthenticationHeaderConstructInMsg(hMsg, RV_FALSE, 
													  (RvSipAuthenticationHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		{
			return RvSipAuthorizationHeaderConstructInMsg(hMsg, RV_FALSE, 
													(RvSipAuthorizationHeaderHandle*)ppHeader);
		}
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_SUBS_ON
	case RVSIP_HEADERTYPE_REFER_TO:
		{
			return RvSipReferToHeaderConstructInMsg(hMsg, RV_FALSE, 
											  (RvSipReferToHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_REFERRED_BY:
		{
			return RvSipReferredByHeaderConstructInMsg(hMsg, RV_FALSE, 
												 (RvSipReferredByHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		{
			return RvSipSubscriptionStateHeaderConstructInMsg(hMsg, RV_FALSE, 
				(RvSipSubscriptionStateHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		{
			return RvSipAllowEventsHeaderConstructInMsg(hMsg, RV_FALSE, 
				(RvSipAllowEventsHeaderHandle*)ppHeader);
		}

#endif /*#ifdef RV_SIP_SUBS_ON*/
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_HEADERTYPE_EVENT:
		{
			return RvSipEventHeaderConstructInMsg(hMsg, RV_FALSE, 
				(RvSipEventHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ALLOW:
        {
            return RvSipAllowHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipAllowHeaderHandle*)ppHeader);
        }
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		{
			return RvSipContentDispositionHeaderConstructInMsg(hMsg, RV_FALSE, 
														 (RvSipContentDispositionHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		{
			return RvSipRetryAfterHeaderConstructInMsg(hMsg, RV_FALSE, 
												 (RvSipRetryAfterHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RSEQ:
		{
			return RvSipRSeqHeaderConstructInMsg(hMsg, RV_FALSE, 
											(RvSipRSeqHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_RACK:
		{
			return RvSipRAckHeaderConstructInMsg(hMsg, RV_FALSE, 
											(RvSipRAckHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            return RvSipSessionExpiresHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipSessionExpiresHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_MINSE:
        {
            return RvSipMinSEHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipMinSEHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_REPLACES:
        {
            return RvSipReplacesHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipReplacesHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_CONTENT_TYPE:
        {
            RvSipBodyHandle hBodyHandle;
            
            hBodyHandle = RvSipMsgGetBodyObject(hMsg);
            if (NULL == hBodyHandle)
            {
				rv = RvSipBodyConstructInMsg(hMsg,&hBodyHandle);
                if(RV_OK != rv)
                {
                    return rv;
                }
            }
            /* Construct ContentType header in the body */
            return  RvSipContentTypeHeaderConstructInBody(hBodyHandle,(RvSipContentTypeHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_CONTENT_ID:
        {
            RvSipBodyHandle hBodyHandle;
            
            hBodyHandle = RvSipMsgGetBodyObject(hMsg);
            if (NULL == hBodyHandle)
            {
				rv = RvSipBodyConstructInMsg(hMsg,&hBodyHandle);
                if(RV_OK != rv)
                {
                    return rv;
                }
            }
            /* Construct ContentID header in the body */
            return  RvSipContentIDHeaderConstructInBody(hBodyHandle,(RvSipContentIDHeaderHandle*)ppHeader);
		}
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    
#ifndef RV_SIP_LIGHT
    case RVSIP_HEADERTYPE_CONTACT:
        {
            return RvSipContactHeaderConstructInMsg(hMsg, RV_FALSE, 
                (RvSipContactHeaderHandle*)ppHeader);
        }
    case RVSIP_HEADERTYPE_EXPIRES:
		{
			return RvSipExpiresHeaderConstructInMsg(hMsg, RV_FALSE ,
											  (RvSipExpiresHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_DATE:
		{
			return RvSipDateHeaderConstructInMsg(hMsg, RV_FALSE, 
											(RvSipDateHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_TO:
        {
			RvSipPartyHeaderHandle *phToHeader = (RvSipPartyHeaderHandle*)ppHeader;
			rv = RvSipToHeaderConstructInMsg(hMsg, phToHeader);
			if (RV_OK != rv)
			{
				return rv;
			}
			return RvSipPartyHeaderSetType(*phToHeader, eType);
		}
	case RVSIP_HEADERTYPE_FROM:
		{
			RvSipPartyHeaderHandle *phFromHeader = (RvSipPartyHeaderHandle*)ppHeader;
			rv = RvSipFromHeaderConstructInMsg(hMsg, phFromHeader);
			if (RV_OK != rv)
			{
				return rv;
			}
			return RvSipPartyHeaderSetType(*phFromHeader, eType);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return RvSipCSeqHeaderConstructInMsg(hMsg, (RvSipCSeqHeaderHandle*)ppHeader);
		}
#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef RV_SIP_AUTH_ON
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			return RvSipAuthenticationInfoHeaderConstructInMsg(hMsg, RV_FALSE, 
														 (RvSipAuthenticationInfoHeaderHandle*)ppHeader);
		}
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_REASON:
		{
			return RvSipReasonHeaderConstructInMsg(hMsg, RV_FALSE, 
											        (RvSipReasonHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_WARNING:
        {
            return RvSipWarningHeaderConstructInMsg(hMsg, RV_FALSE,
                                                    (RvSipWarningHeaderHandle*)ppHeader);
		}
    case RVSIP_HEADERTYPE_TIMESTAMP:
        {
            return RvSipTimestampHeaderConstructInMsg(hMsg, RV_FALSE, 
                                                      (RvSipTimestampHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_INFO:
        {
            return RvSipInfoHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipInfoHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT:
        {
            return RvSipAcceptHeaderConstructInMsg(hMsg, RV_FALSE, 
													(RvSipAcceptHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
        {
            return RvSipAcceptEncodingHeaderConstructInMsg(hMsg, RV_FALSE, 
															(RvSipAcceptEncodingHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
        {
            return RvSipAcceptLanguageHeaderConstructInMsg(hMsg, RV_FALSE, 
															(RvSipAcceptLanguageHeaderHandle*)ppHeader);
		}
	case RVSIP_HEADERTYPE_REPLY_TO:
        {
            return RvSipReplyToHeaderConstructInMsg(hMsg, RV_FALSE, 
													(RvSipReplyToHeaderHandle*)ppHeader);
		}
		/* XXX */
#endif
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_URI:
	{
		return RvSipPUriHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPUriHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
	{
		return RvSipPVisitedNetworkIDHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPVisitedNetworkIDHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
	{
		return RvSipPAccessNetworkInfoHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPAccessNetworkInfoHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
	{
		return RvSipPChargingFunctionAddressesHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPChargingFunctionAddressesHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
	{
		return RvSipPChargingVectorHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPChargingVectorHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
	{
		return RvSipPMediaAuthorizationHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPMediaAuthorizationHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_SECURITY:
	{
		return RvSipSecurityHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipSecurityHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_PROFILE_KEY:
	{
		return RvSipPProfileKeyHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPProfileKeyHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_USER_DATABASE:
	{
		return RvSipPUserDatabaseHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPUserDatabaseHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_ANSWER_STATE:
	{
		return RvSipPAnswerStateHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPAnswerStateHeaderHandle*)ppHeader);
	}
    case RVSIP_HEADERTYPE_P_SERVED_USER:
	{
		return RvSipPServedUserHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPServedUserHeaderHandle*)ppHeader);
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
	{
		return RvSipPDCSTracePartyIDHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPDCSTracePartyIDHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_OSPS:
	{
		return RvSipPDCSOSPSHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPDCSOSPSHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
	{
		return RvSipPDCSBillingInfoHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPDCSBillingInfoHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_LAES:
	{
		return RvSipPDCSLAESHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPDCSLAESHeaderHandle*)ppHeader);
	}
	case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
	{
		return RvSipPDCSRedirectHeaderConstructInMsg(hMsg, RV_FALSE, 
												(RvSipPDCSRedirectHeaderHandle*)ppHeader);
	}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
	default:
		{
			return RV_ERROR_BADPARAM;
		}
	}

#ifdef RV_SIP_LIGHT
    RV_UNUSED_ARG(rv)
#endif
}
/***************************************************************************
 * RvSipHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a header of any type to a textual header.
 *          The textual header is placed on a page taken from a specified pool.
 *          In order to copy the textual header from the page to a
 *          consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pHeader - Handle to the header structure.
 *          eType   - Type of the header to encode.
 *          hPool   - Handle to the specified memory pool.
 * output:  hPage   - The memory page allocated to contain the encoded header.
 *          length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderEncode(IN    void*           pHeader,
                                             IN    RvSipHeaderType eType,
                                             IN    HRPOOL          hPool,
                                             OUT   HPAGE*          hPage,
                                             OUT   RvUint32*      length)
{
    RvStatus stat;
    RvLogSource* logId;

    if((pHeader == NULL) || (hPool == NULL) || (hPage == NULL_PAGE) || (length == NULL))
        return RV_ERROR_BADPARAM;

    *hPage = NULL_PAGE;
    *length = 0;

    logId = GetLogIdFromHeader(pHeader, eType);
    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(logId,(logId,
                "RvSipHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, pHeader));
        return stat;
    }
    else
    {
        RvLogInfo(logId,(logId,
                "RvSipHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, pHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = MsgHeaderEncode(pHeader, eType, hPool, *hPage, RV_FALSE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(logId,(logId,
                "RvSipHeaderEncode - Failed. Free page 0x%p on pool 0x%p. MsgHeaderByTypeEncodeHeade failed",
                *hPage, hPool));
    }
    return stat;
}
/***************************************************************************
 * MsgHeaderEncode
 * ------------------------------------------------------------------------
 * General: The function gets a header and it's type and encode it.
 *
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pHeader -   Handle to the header structure.
 *            eType -     Type of the header to encode.
 *          hPool -     Pool of the given page.
 *            hPage -     A page to fill with encoded string.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * Output:     length -    previous length + the header encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgHeaderEncode(
                                    IN    void*                   pHeader,
                                    IN    RvSipHeaderType         eType,
                                    IN    HRPOOL                  hPool,
                                    IN    HPAGE                   hPage,
                                    IN    RvBool                 bInUrlHeaders,
                                    IN    RvBool                 bForceCompactForm,
                                    INOUT RvUint32*              length)
{
    switch(eType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
            return  ContactHeaderEncode((RvSipContactHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        bInUrlHeaders,
                                        bForceCompactForm,
                                        length);
        case RVSIP_HEADERTYPE_EXPIRES:
            return  ExpiresHeaderEncode((RvSipExpiresHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        RV_FALSE,
                                        bInUrlHeaders,
                                        length);

        case RVSIP_HEADERTYPE_DATE:
            return  DateHeaderEncode((RvSipDateHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        RV_FALSE,
                                        bInUrlHeaders,
                                        length);
        case RVSIP_HEADERTYPE_TO:
            return PartyHeaderEncode((RvSipPartyHeaderHandle)pHeader,
                                      hPool,
                                      hPage,
                                      RV_TRUE,
                                      bInUrlHeaders,
                                      bForceCompactForm,
                                      length);
        case RVSIP_HEADERTYPE_FROM:
            return PartyHeaderEncode((RvSipPartyHeaderHandle)pHeader,
                                      hPool,
                                      hPage,
                                      RV_FALSE,
                                      bInUrlHeaders,
                                      bForceCompactForm,
                                      length);
        case RVSIP_HEADERTYPE_CSEQ:
            return CSeqHeaderEncode((RvSipCSeqHeaderHandle)pHeader,
                                      hPool,
                                      hPage,
                                      RV_TRUE,
                                      bInUrlHeaders,
                                      length);


#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_VIA:
            return  ViaHeaderEncode((RvSipViaHeaderHandle)pHeader,
                                    hPool,
                                    hPage,
                                    bInUrlHeaders,
                                    bForceCompactForm,
                                    length);

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            return  AuthenticationHeaderEncode((RvSipAuthenticationHeaderHandle)pHeader,
                                    hPool,
                                    hPage,
                                    bInUrlHeaders,
                                    length);

        case RVSIP_HEADERTYPE_AUTHORIZATION:
            return  AuthorizationHeaderEncode((RvSipAuthorizationHeaderHandle)pHeader,
                                    hPool,
                                    hPage,
                                    bInUrlHeaders,
                                    length);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
            return  EventHeaderEncode((RvSipEventHeaderHandle)pHeader,
                hPool,
                hPage,
                bInUrlHeaders,
                bForceCompactForm,
                length);

        case RVSIP_HEADERTYPE_ALLOW:
            return  AllowHeaderEncode((RvSipAllowHeaderHandle)pHeader,
                                       hPool,
                                       hPage,
                                       RV_TRUE,
                                       bInUrlHeaders,
                                       length);
#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                     SUB-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
            return  ReferToHeaderEncode((RvSipReferToHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        bInUrlHeaders,
                                        bForceCompactForm,
                                        length);

        case RVSIP_HEADERTYPE_REFERRED_BY:
            return  ReferredByHeaderEncode((RvSipReferredByHeaderHandle)pHeader,
                                            hPool,
                                            hPage,
                                            bInUrlHeaders,
                                            bForceCompactForm,
                                            length);
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
            return  SubscriptionStateHeaderEncode((RvSipSubscriptionStateHeaderHandle)pHeader,
                hPool,
                hPage,
                bInUrlHeaders,
                length);

        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
            return  AllowEventsHeaderEncode((RvSipAllowEventsHeaderHandle)pHeader,
                hPool,
                hPage,
                RV_TRUE,
                bInUrlHeaders,
                bForceCompactForm,
                length);

#endif /* #ifdef RV_SIP_SUBS_ON */

        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
            return  ContentDispositionHeaderEncode(
                                    (RvSipContentDispositionHeaderHandle)pHeader,
                                    hPool,
                                    hPage,
                                    bInUrlHeaders,
                                    length);

        case RVSIP_HEADERTYPE_RETRY_AFTER:
            return  RetryAfterHeaderEncode((RvSipRetryAfterHeaderHandle)pHeader,
                                            hPool,
                                            hPage,
                                            bInUrlHeaders,
                                            length);

        case RVSIP_HEADERTYPE_RSEQ:
            return  RSeqHeaderEncode((RvSipRSeqHeaderHandle)pHeader,
                                       hPool,
                                       hPage,
                                       bInUrlHeaders,
                                       length);

        case RVSIP_HEADERTYPE_RACK:
            return  RAckHeaderEncode((RvSipRAckHeaderHandle)pHeader,
                                       hPool,
                                       hPage,
                                       bInUrlHeaders,
                                       length);

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
            return  SessionExpiresHeaderEncode((RvSipSessionExpiresHeaderHandle)pHeader,
                                                hPool,
                                                hPage,
                                                bInUrlHeaders,
                                                bForceCompactForm,
                                                length);

        case RVSIP_HEADERTYPE_MINSE:
            return  MinSEHeaderEncode((RvSipMinSEHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        bInUrlHeaders,
                                        length);

        case RVSIP_HEADERTYPE_REPLACES:
            return  ReplacesHeaderEncode((RvSipReplacesHeaderHandle)pHeader,
                                        hPool,
                                        hPage,
                                        bInUrlHeaders,
                                        length);
#endif /* RV_SIP_PRIMITIVES */

        case RVSIP_HEADERTYPE_ROUTE_HOP:
           return  RouteHopHeaderEncode((RvSipRouteHopHeaderHandle)pHeader,
                                     hPool,
                                     hPage,
                                     bInUrlHeaders,
                                     length);

        case RVSIP_HEADERTYPE_OTHER:
            return  OtherHeaderEncode((RvSipOtherHeaderHandle)pHeader,
                                     hPool,
                                     hPage,
                                     RV_TRUE,
                                     bInUrlHeaders,
                                     bForceCompactForm,
                                     length);
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
            return ContentTypeHeaderEncode((RvSipContentTypeHeaderHandle)pHeader,
                                            hPool,
                                            hPage,
                                            bInUrlHeaders,
                                            bForceCompactForm,
                                            length);
        case RVSIP_HEADERTYPE_CONTENT_ID:
            return ContentIDHeaderEncode((RvSipContentIDHeaderHandle)pHeader,
                                            hPool,
                                            hPage,
                                            bInUrlHeaders,
                                            length);
#endif /* #ifndef RV_SIP_PRIMITIVES*/
#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
			return  AuthenticationInfoHeaderEncode((RvSipAuthenticationInfoHeaderHandle)pHeader,
				hPool,
				hPage,
				bInUrlHeaders,
				length);
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

		case RVSIP_HEADERTYPE_REASON:
            return ReasonHeaderEncode((RvSipReasonHeaderHandle)pHeader,
									  hPool,
									  hPage,
								      bInUrlHeaders,
									  length);
		case RVSIP_HEADERTYPE_WARNING:
            return WarningHeaderEncode((RvSipWarningHeaderHandle)pHeader,
									  hPool,
									  hPage,
									  bInUrlHeaders,
									  length);

		case RVSIP_HEADERTYPE_TIMESTAMP:
            return TimestampHeaderEncode((RvSipTimestampHeaderHandle)pHeader,
				                          hPool,
				                          hPage,
		                                  bInUrlHeaders,
			                              length);
		case RVSIP_HEADERTYPE_INFO:
            return InfoHeaderEncode((RvSipInfoHeaderHandle)pHeader,
				                          hPool,
				                          hPage,
		                                  bInUrlHeaders,
			                              length);
		case RVSIP_HEADERTYPE_ACCEPT:
            return AcceptHeaderEncode((RvSipAcceptHeaderHandle)pHeader,
				                          hPool,
				                          hPage,
		                                  bInUrlHeaders,
			                              length);
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
            return AcceptEncodingHeaderEncode((RvSipAcceptEncodingHeaderHandle)pHeader,
				                          hPool,
				                          hPage,
		                                  bInUrlHeaders,
			                              length);
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
            return AcceptLanguageHeaderEncode((RvSipAcceptLanguageHeaderHandle)pHeader,
				                          hPool,
				                          hPage,
		                                  bInUrlHeaders,
			                              length);
		case RVSIP_HEADERTYPE_REPLY_TO:
            return ReplyToHeaderEncode((RvSipReplyToHeaderHandle)pHeader,
				                       hPool,
				                       hPage,
		                               bInUrlHeaders,
			                           length);
			
		/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
			return PUriHeaderEncode((RvSipPUriHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);	
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
			return PVisitedNetworkIDHeaderEncode((RvSipPVisitedNetworkIDHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
			return PAccessNetworkInfoHeaderEncode((RvSipPAccessNetworkInfoHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
			return PChargingFunctionAddressesHeaderEncode((RvSipPChargingFunctionAddressesHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
			return PChargingVectorHeaderEncode((RvSipPChargingVectorHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
			return PMediaAuthorizationHeaderEncode((RvSipPMediaAuthorizationHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_SECURITY:
			return SecurityHeaderEncode((RvSipSecurityHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
			return PProfileKeyHeaderEncode((RvSipPProfileKeyHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
			return PUserDatabaseHeaderEncode((RvSipPUserDatabaseHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
			return PAnswerStateHeaderEncode((RvSipPAnswerStateHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
        case RVSIP_HEADERTYPE_P_SERVED_USER:
			return PServedUserHeaderEncode((RvSipPServedUserHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
			return PDCSTracePartyIDHeaderEncode((RvSipPDCSTracePartyIDHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
			return PDCSOSPSHeaderEncode((RvSipPDCSOSPSHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
			return PDCSBillingInfoHeaderEncode((RvSipPDCSBillingInfoHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_DCS_LAES:
			return PDCSLAESHeaderEncode((RvSipPDCSLAESHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
			return PDCSRedirectHeaderEncode((RvSipPDCSRedirectHeaderHandle)pHeader,
	                                    hPool,
	                                    hPage,
	                                    bInUrlHeaders,
	                                    length);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
            return RV_ERROR_BADPARAM;
      }
}

/***************************************************************************
 * RvSipHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual header, of any type into a header object.
 *          All the textual fields are placed inside the object.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - A handle to the header object.
 *          eType   - Type of the header to parse.
 *          buffer  - Buffer containing a textual header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderParse(
                                          IN    void*           hHeader,
                                          IN    RvSipHeaderType eType,
                                          IN    RvChar*        buffer)
{
    RvStatus stat;
    RvLogSource* logId;

    if((buffer == NULL) || (hHeader == NULL) || (eType == RVSIP_HEADERTYPE_UNDEFINED))
        return RV_ERROR_BADPARAM;

    logId = GetLogIdFromHeader(hHeader, eType);
    switch(eType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
        {
            stat = RvSipContactHeaderParse((RvSipContactHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_EXPIRES:
            {
                stat = RvSipExpiresHeaderParse( (RvSipExpiresHeaderHandle)hHeader, buffer);
                break;
            }
        case RVSIP_HEADERTYPE_DATE:
            {
                stat = RvSipDateHeaderParse((RvSipDateHeaderHandle)hHeader, buffer);
                break;
            }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            stat = RvSipRouteHopHeaderParse((RvSipRouteHopHeaderHandle)hHeader, buffer);
            break;
        }

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            {
                stat = RvSipAuthenticationHeaderParse((RvSipAuthenticationHeaderHandle)hHeader, buffer);
                break;
            }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            {
                stat = RvSipAuthorizationHeaderParse((RvSipAuthorizationHeaderHandle)hHeader, buffer);;
                break;
            }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            stat = RvSipEventHeaderParse((RvSipEventHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            stat = RvSipAllowHeaderParse((RvSipAllowHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			stat = RvSipContentTypeHeaderParse((RvSipContentTypeHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_CONTENT_ID:
		{
			stat = RvSipContentIDHeaderParse((RvSipContentIDHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_RSEQ:
        {
            stat = RvSipRSeqHeaderParse((RvSipRSeqHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            stat = RvSipRAckHeaderParse((RvSipRAckHeaderHandle)hHeader, buffer);
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                     SUBS-REFER HEADERS                       */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            stat = RvSipReferToHeaderParse((RvSipReferToHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            stat = RvSipReferredByHeaderParse((RvSipReferredByHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            stat = RvSipSubscriptionStateHeaderParse((RvSipSubscriptionStateHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            stat = RvSipAllowEventsHeaderParse((RvSipAllowEventsHeaderHandle)hHeader, buffer);
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
        {
            stat = RvSipReplacesHeaderParse((RvSipReplacesHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            stat = RvSipContentDispositionHeaderParse((RvSipContentDispositionHeaderHandle)hHeader, buffer);
            break;
        }

        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            stat = RvSipRetryAfterHeaderParse((RvSipRetryAfterHeaderHandle)hHeader, buffer);
            break;
        }

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            stat = RvSipSessionExpiresHeaderParse((RvSipSessionExpiresHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            stat = RvSipMinSEHeaderParse((RvSipMinSEHeaderHandle)hHeader, buffer);
            break;
        }
#endif /* RV_SIP_PRIMITIVES */

        case RVSIP_HEADERTYPE_VIA:
        {
            stat = RvSipViaHeaderParse((RvSipViaHeaderHandle)hHeader, buffer);
            break;
        }
#ifndef RV_SIP_LIGHT
         case RVSIP_HEADERTYPE_CSEQ:
        {
            stat = RvSipCSeqHeaderParse((RvSipCSeqHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_TO:
        {
            stat = RvSipPartyHeaderParse((RvSipPartyHeaderHandle)hHeader, RV_TRUE, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_FROM:
        {
            stat = RvSipPartyHeaderParse((RvSipPartyHeaderHandle)hHeader, RV_FALSE, buffer);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_OTHER:
        {
            stat = RvSipOtherHeaderParse((RvSipOtherHeaderHandle)hHeader, buffer);
            break;
        }

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
            {
                stat = RvSipAuthenticationInfoHeaderParse((RvSipAuthenticationInfoHeaderHandle)hHeader, buffer);;
                break;
            }
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_REASON:
        {
            stat = RvSipReasonHeaderParse((RvSipReasonHeaderHandle)hHeader, buffer);
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            stat = RvSipWarningHeaderParse((RvSipWarningHeaderHandle)hHeader, buffer);
            break;
        }
		case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			stat = RvSipTimestampHeaderParse((RvSipTimestampHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_INFO:
		{
			stat = RvSipInfoHeaderParse((RvSipInfoHeaderHandle)hHeader, buffer);
			break;
		}	
		case RVSIP_HEADERTYPE_ACCEPT:
		{
			stat = RvSipAcceptHeaderParse((RvSipAcceptHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		{
			stat = RvSipAcceptEncodingHeaderParse((RvSipAcceptEncodingHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		{
			stat = RvSipAcceptLanguageHeaderParse((RvSipAcceptLanguageHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_REPLY_TO:
		{
			stat = RvSipReplyToHeaderParse((RvSipReplyToHeaderHandle)hHeader, buffer);
			break;
		}
			
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		{
			stat = RvSipPUriHeaderParse((RvSipPUriHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			stat = RvSipPVisitedNetworkIDHeaderParse((RvSipPVisitedNetworkIDHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			stat = RvSipPAccessNetworkInfoHeaderParse((RvSipPAccessNetworkInfoHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			stat = RvSipPChargingFunctionAddressesHeaderParse((RvSipPChargingFunctionAddressesHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			stat = RvSipPChargingVectorHeaderParse((RvSipPChargingVectorHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			stat = RvSipPMediaAuthorizationHeaderParse((RvSipPMediaAuthorizationHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			stat = RvSipSecurityHeaderParse((RvSipSecurityHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			stat = RvSipPProfileKeyHeaderParse((RvSipPProfileKeyHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			stat = RvSipPUserDatabaseHeaderParse((RvSipPUserDatabaseHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			stat = RvSipPAnswerStateHeaderParse((RvSipPAnswerStateHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			stat = RvSipPServedUserHeaderParse((RvSipPServedUserHeaderHandle)hHeader, buffer);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			stat = RvSipPDCSTracePartyIDHeaderParse((RvSipPDCSTracePartyIDHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			stat = RvSipPDCSOSPSHeaderParse((RvSipPDCSOSPSHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			stat = RvSipPDCSBillingInfoHeaderParse((RvSipPDCSBillingInfoHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			stat = RvSipPDCSLAESHeaderParse((RvSipPDCSLAESHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			stat = RvSipPDCSRedirectHeaderParse((RvSipPDCSRedirectHeaderHandle)hHeader, buffer);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            RvLogExcep(logId,(logId,
                "RvSipHeaderParse - unknown headerType. cannot parse it to the list!"));
            return RV_ERROR_BADPARAM;
        }
    }
    return stat;
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipHeaderConstructAndParse
 * ------------------------------------------------------------------------
 * General: Constructs and parses a stand-alone header object. The object is
 *          constructed on a given page taken from a specified pool. The type
 *          of the constructed header and the value of its fields is determined 
 *          by parsing the given buffer. The handle to the new header object is 
 *          returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool     - Handle to the memory pool. that the object uses.
 *         hPage     - Handle to the memory page that the object uses.
 *         buffer    - Buffer containing a textual Address.
 * output: peType    - The type of the new header.
 *         hSipAddr  - Handle to the newly constructed header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderConstructAndParse(
											  IN  RvSipMsgMgrHandle   hMsgMgr,
                                              IN  HRPOOL              hPool,
                                              IN  HPAGE               hPage,
                                              IN  RvChar*             buffer,
											  OUT RvSipHeaderType*    peType,
                                              OUT void**              ppHeader)
{
	MsgMgr*			 pMsgMgr     = (MsgMgr*)hMsgMgr;
	RvStatus         rv;

	*peType = ResolveHeaderTypeFromBuffer(buffer);
	if (*peType == RVSIP_HEADERTYPE_UNDEFINED)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
				  "RvSipHeaderConstructAndParse - Failed to identify header type from buffer"));
		return RV_ERROR_UNKNOWN;
	}
	
	rv = RvSipHeaderConstruct(hMsgMgr, *peType, hPool, hPage, ppHeader);
	if (RV_OK != rv)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
					"RvSipHeaderConstructAndParse - Failed in RvSipHeaderConstruct, rv=%d", rv));
		return RV_ERROR_UNKNOWN;
	}

	rv = RvSipHeaderParse(*ppHeader, *peType, buffer);
	if (RV_OK != rv)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
				  "RvSipHeaderConstructAndParse - Failed in RvSipHeaderParse, rv=%d", rv));
		return RV_ERROR_UNKNOWN;
	}

	return RV_OK;
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual header value, of any header type, into a
 *          header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipHeaderParse() function to parse strings that also
 *          include the header-name.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - The handle to the header object.
 *          eType   - Type of the header to parse.
 *          buffer  - The buffer containing the textual header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderParseValue(
                                          IN    void*           hHeader,
                                          IN    RvSipHeaderType eType,
                                          IN    RvChar*        buffer)
{
    RvStatus stat;
    RvLogSource* logId;

    if((buffer == NULL) || (hHeader == NULL) || (eType == RVSIP_HEADERTYPE_UNDEFINED))
    {
        return RV_ERROR_BADPARAM;
    }

    logId = GetLogIdFromHeader(hHeader, eType);
    switch(eType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
        {
            stat = RvSipContactHeaderParseValue((RvSipContactHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_EXPIRES:
            {
                stat = RvSipExpiresHeaderParseValue( (RvSipExpiresHeaderHandle)hHeader, buffer);
                break;
            }
        case RVSIP_HEADERTYPE_DATE:
            {
                stat = RvSipDateHeaderParseValue((RvSipDateHeaderHandle)hHeader, buffer);
                break;
            }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            stat = RvSipRouteHopHeaderParseValue((RvSipRouteHopHeaderHandle)hHeader, buffer);
            break;
        }

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            {
                stat = RvSipAuthenticationHeaderParseValue((RvSipAuthenticationHeaderHandle)hHeader, buffer);
                break;
            }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            {
                stat = RvSipAuthorizationHeaderParseValue((RvSipAuthorizationHeaderHandle)hHeader, buffer);;
                break;
            }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            stat = RvSipEventHeaderParseValue((RvSipEventHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            stat = RvSipAllowHeaderParseValue((RvSipAllowHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			stat = RvSipContentTypeHeaderParseValue((RvSipContentTypeHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_CONTENT_ID:
		{
			stat = RvSipContentIDHeaderParseValue((RvSipContentIDHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_RSEQ:
        {
            stat = RvSipRSeqHeaderParseValue((RvSipRSeqHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            stat = RvSipRAckHeaderParseValue((RvSipRAckHeaderHandle)hHeader, buffer);
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                    SUBS-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            stat = RvSipReferToHeaderParseValue((RvSipReferToHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            stat = RvSipReferredByHeaderParseValue((RvSipReferredByHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            stat = RvSipSubscriptionStateHeaderParseValue((RvSipSubscriptionStateHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            stat = RvSipAllowEventsHeaderParseValue((RvSipAllowEventsHeaderHandle)hHeader, buffer);
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */

        case RVSIP_HEADERTYPE_REPLACES:
        {
            stat = RvSipReplacesHeaderParseValue((RvSipReplacesHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            stat = RvSipContentDispositionHeaderParseValue((RvSipContentDispositionHeaderHandle)hHeader, buffer);
            break;
        }

        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            stat = RvSipRetryAfterHeaderParseValue((RvSipRetryAfterHeaderHandle)hHeader, buffer);
            break;
        }

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            stat = RvSipSessionExpiresHeaderParseValue((RvSipSessionExpiresHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            stat = RvSipMinSEHeaderParseValue((RvSipMinSEHeaderHandle)hHeader, buffer);
            break;
        }
#endif /* RV_SIP_PRIMITIVES */

        case RVSIP_HEADERTYPE_VIA:
        {
            stat = RvSipViaHeaderParseValue((RvSipViaHeaderHandle)hHeader, buffer);
            break;
        }
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CSEQ:
        {
            stat = RvSipCSeqHeaderParseValue((RvSipCSeqHeaderHandle)hHeader, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_TO:
        {
            stat = RvSipPartyHeaderParseValue((RvSipPartyHeaderHandle)hHeader, RV_TRUE, buffer);
            break;
        }
        case RVSIP_HEADERTYPE_FROM:
        {
            stat = RvSipPartyHeaderParseValue((RvSipPartyHeaderHandle)hHeader, RV_FALSE, buffer);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
            {
                stat = RvSipAuthenticationInfoHeaderParseValue((RvSipAuthenticationInfoHeaderHandle)hHeader, buffer);
                break;
            }
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_REASON:
        {
            stat = RvSipReasonHeaderParseValue((RvSipReasonHeaderHandle)hHeader, buffer);
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            stat = RvSipWarningHeaderParseValue((RvSipWarningHeaderHandle)hHeader, buffer);
            break;
        }
		case RVSIP_HEADERTYPE_TIMESTAMP:
			{
				stat = RvSipTimestampHeaderParseValue((RvSipTimestampHeaderHandle)hHeader, buffer);
				break;
			}
		case RVSIP_HEADERTYPE_INFO:
			{
				stat = RvSipInfoHeaderParseValue((RvSipInfoHeaderHandle)hHeader, buffer);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT:
			{
				stat = RvSipAcceptHeaderParseValue((RvSipAcceptHeaderHandle)hHeader, buffer);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
			{
				stat = RvSipAcceptEncodingHeaderParseValue((RvSipAcceptEncodingHeaderHandle)hHeader, buffer);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
			{
				stat = RvSipAcceptLanguageHeaderParseValue((RvSipAcceptLanguageHeaderHandle)hHeader, buffer);
				break;
			}
		case RVSIP_HEADERTYPE_REPLY_TO:
			{
				stat = RvSipReplyToHeaderParseValue((RvSipReplyToHeaderHandle)hHeader, buffer);
				break;
			}
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		{
			stat = RvSipPUriHeaderParseValue((RvSipPUriHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			stat = RvSipPVisitedNetworkIDHeaderParseValue((RvSipPVisitedNetworkIDHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			stat = RvSipPAccessNetworkInfoHeaderParseValue((RvSipPAccessNetworkInfoHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			stat = RvSipPChargingFunctionAddressesHeaderParseValue((RvSipPChargingFunctionAddressesHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			stat = RvSipPChargingVectorHeaderParseValue((RvSipPChargingVectorHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			stat = RvSipPMediaAuthorizationHeaderParseValue((RvSipPMediaAuthorizationHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			stat = RvSipSecurityHeaderParseValue((RvSipSecurityHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			stat = RvSipPProfileKeyHeaderParseValue((RvSipPProfileKeyHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			stat = RvSipPUserDatabaseHeaderParseValue((RvSipPUserDatabaseHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			stat = RvSipPAnswerStateHeaderParseValue((RvSipPAnswerStateHeaderHandle)hHeader, buffer);
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			stat = RvSipPServedUserHeaderParseValue((RvSipPServedUserHeaderHandle)hHeader, buffer);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			stat = RvSipPDCSTracePartyIDHeaderParseValue((RvSipPDCSTracePartyIDHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			stat = RvSipPDCSOSPSHeaderParseValue((RvSipPDCSOSPSHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			stat = RvSipPDCSBillingInfoHeaderParseValue((RvSipPDCSBillingInfoHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			stat = RvSipPDCSLAESHeaderParseValue((RvSipPDCSLAESHeaderHandle)hHeader, buffer);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			stat = RvSipPDCSRedirectHeaderParseValue((RvSipPDCSRedirectHeaderHandle)hHeader, buffer);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_OTHER:
		{
			stat = RvSipOtherHeaderSetValue((RvSipOtherHeaderHandle)hHeader, buffer);
			break;
		}

        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            RvLogError(logId,(logId,"RvSipHeaderParseValue - unknown headerType. cannot parse it to the list!"));
            return RV_ERROR_BADPARAM;
        }
    }
    return stat;
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source header object to a destination header
 *          object. The headers must be of the same type, or of twin types 
 *          (such as To and From).
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination     - Handle to the destination header object.
 *    eDestinationType - The type of the destination header.
 *    hSource          - Handle to the source header object.
 *    eSourceType      - The type of the source header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipHeaderCopy(INOUT void*                  hDestination,
										  IN    RvSipHeaderType        eDestinationType,
										  IN    void*                  hSource,
										  IN    RvSipHeaderType        eSourceType)
{
	RvStatus     stat;
    RvLogSource* logId;

    if((hSource == NULL) || (hDestination == NULL) || (eSourceType == RVSIP_HEADERTYPE_UNDEFINED))
    {
        return RV_ERROR_BADPARAM;
    }

	if (eSourceType      != eDestinationType      &&
		eSourceType      != RVSIP_HEADERTYPE_TO   &&
		eSourceType      != RVSIP_HEADERTYPE_FROM &&
		eDestinationType != RVSIP_HEADERTYPE_TO   &&
		eDestinationType != RVSIP_HEADERTYPE_FROM)
	{
		return RV_ERROR_BADPARAM;
	}

    logId = GetLogIdFromHeader(hSource, eSourceType);
    switch(eSourceType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
        {
            stat = RvSipContactHeaderCopy((RvSipContactHeaderHandle)hDestination, 
										  (RvSipContactHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_EXPIRES:
        {
            stat = RvSipExpiresHeaderCopy((RvSipExpiresHeaderHandle)hDestination, 
										  (RvSipExpiresHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_DATE:
        {
            stat = RvSipDateHeaderCopy((RvSipDateHeaderHandle)hDestination, 
									   (RvSipDateHeaderHandle)hSource);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            stat = RvSipRouteHopHeaderCopy((RvSipRouteHopHeaderHandle)hDestination, 
										   (RvSipRouteHopHeaderHandle)hSource);
            break;
        }

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            {
                stat = RvSipAuthenticationHeaderCopy((RvSipAuthenticationHeaderHandle)hDestination, 
													 (RvSipAuthenticationHeaderHandle)hSource);
                break;
            }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            {
                stat = RvSipAuthorizationHeaderCopy((RvSipAuthorizationHeaderHandle)hDestination,
													(RvSipAuthorizationHeaderHandle)hSource);
                break;
            }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            stat = RvSipEventHeaderCopy((RvSipEventHeaderHandle)hDestination, 
										(RvSipEventHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            stat = RvSipAllowHeaderCopy((RvSipAllowHeaderHandle)hDestination, 
										(RvSipAllowHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_TYPE:/*NANA*/
        {
            stat = RvSipContentTypeHeaderCopy((RvSipContentTypeHeaderHandle)hDestination, 
											  (RvSipContentTypeHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_ID:
        {
            stat = RvSipContentIDHeaderCopy((RvSipContentIDHeaderHandle)hDestination, 
											  (RvSipContentIDHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_RSEQ:
        {
            stat = RvSipRSeqHeaderCopy((RvSipRSeqHeaderHandle)hDestination, 
									   (RvSipRSeqHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            stat = RvSipRAckHeaderCopy((RvSipRAckHeaderHandle)hDestination, 
									   (RvSipRAckHeaderHandle)hSource);
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                    SUBS-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            stat = RvSipReferToHeaderCopy((RvSipReferToHeaderHandle)hDestination, 
										  (RvSipReferToHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            stat = RvSipReferredByHeaderCopy((RvSipReferredByHeaderHandle)hDestination,
											 (RvSipReferredByHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            stat = RvSipSubscriptionStateHeaderCopy((RvSipSubscriptionStateHeaderHandle)hDestination, 
													(RvSipSubscriptionStateHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            stat = RvSipAllowEventsHeaderCopy((RvSipAllowEventsHeaderHandle)hDestination, 
											  (RvSipAllowEventsHeaderHandle)hSource);
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */

        case RVSIP_HEADERTYPE_REPLACES:
        {
            stat = RvSipReplacesHeaderCopy((RvSipReplacesHeaderHandle)hDestination, 
										   (RvSipReplacesHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            stat = RvSipContentDispositionHeaderCopy((RvSipContentDispositionHeaderHandle)hDestination, 
													 (RvSipContentDispositionHeaderHandle)hSource);
            break;
        }

        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            stat = RvSipRetryAfterHeaderCopy((RvSipRetryAfterHeaderHandle)hDestination, 
												   (RvSipRetryAfterHeaderHandle)hSource);
            break;
        }

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            stat = RvSipSessionExpiresHeaderCopy((RvSipSessionExpiresHeaderHandle)hDestination, 
												 (RvSipSessionExpiresHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            stat = RvSipMinSEHeaderCopy((RvSipMinSEHeaderHandle)hDestination, 
										(RvSipMinSEHeaderHandle)hSource);
            break;
        }
#endif /* RV_SIP_PRIMITIVES */

        case RVSIP_HEADERTYPE_VIA:
        {
            stat = RvSipViaHeaderCopy((RvSipViaHeaderHandle)hDestination, 
									  (RvSipViaHeaderHandle)hSource);
            break;
        }
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CSEQ:
        {
            stat = RvSipCSeqHeaderCopy((RvSipCSeqHeaderHandle)hDestination, 
									   (RvSipCSeqHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_TO:
        {
            stat = RvSipPartyHeaderCopy((RvSipPartyHeaderHandle)hDestination, 
										(RvSipPartyHeaderHandle)hSource);
            break;
        }
        case RVSIP_HEADERTYPE_FROM:
        {
            stat = RvSipPartyHeaderCopy((RvSipPartyHeaderHandle)hDestination, 
										(RvSipPartyHeaderHandle)hSource);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
            {
                stat = RvSipAuthenticationInfoHeaderCopy((RvSipAuthenticationInfoHeaderHandle)hDestination, 
														 (RvSipAuthenticationInfoHeaderHandle)hSource);
                break;
            }
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_REASON:
        {
            stat = RvSipReasonHeaderCopy((RvSipReasonHeaderHandle)hDestination, 
										 (RvSipReasonHeaderHandle)hSource);
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            stat = RvSipWarningHeaderCopy((RvSipWarningHeaderHandle)hDestination, 
										  (RvSipWarningHeaderHandle)hSource);
            break;
        }
		case RVSIP_HEADERTYPE_TIMESTAMP:
        {
            stat = RvSipTimestampHeaderCopy((RvSipTimestampHeaderHandle)hDestination, 
										    (RvSipTimestampHeaderHandle)hSource);
            break;
        }
		case RVSIP_HEADERTYPE_INFO:
			{
				stat = RvSipInfoHeaderCopy((RvSipInfoHeaderHandle)hDestination, 
											(RvSipInfoHeaderHandle)hSource);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT:
			{
				stat = RvSipAcceptHeaderCopy((RvSipAcceptHeaderHandle)hDestination, 
											 (RvSipAcceptHeaderHandle)hSource);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
			{
				stat = RvSipAcceptEncodingHeaderCopy((RvSipAcceptEncodingHeaderHandle)hDestination, 
													 (RvSipAcceptEncodingHeaderHandle)hSource);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
			{
				stat = RvSipAcceptLanguageHeaderCopy((RvSipAcceptLanguageHeaderHandle)hDestination, 
													 (RvSipAcceptLanguageHeaderHandle)hSource);
				break;
			}
		case RVSIP_HEADERTYPE_REPLY_TO:
			{
				stat = RvSipReplyToHeaderCopy((RvSipReplyToHeaderHandle)hDestination, 
											  (RvSipReplyToHeaderHandle)hSource);
				break;
			}
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		{
			stat = RvSipPUriHeaderCopy((RvSipPUriHeaderHandle)hDestination, 
										  (RvSipPUriHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			stat = RvSipPVisitedNetworkIDHeaderCopy((RvSipPVisitedNetworkIDHeaderHandle)hDestination, 
										  (RvSipPVisitedNetworkIDHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			stat = RvSipPAccessNetworkInfoHeaderCopy((RvSipPAccessNetworkInfoHeaderHandle)hDestination, 
										  (RvSipPAccessNetworkInfoHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			stat = RvSipPChargingFunctionAddressesHeaderCopy((RvSipPChargingFunctionAddressesHeaderHandle)hDestination, 
										  (RvSipPChargingFunctionAddressesHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			stat = RvSipPChargingVectorHeaderCopy((RvSipPChargingVectorHeaderHandle)hDestination, 
										  (RvSipPChargingVectorHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			stat = RvSipPMediaAuthorizationHeaderCopy((RvSipPMediaAuthorizationHeaderHandle)hDestination, 
										  (RvSipPMediaAuthorizationHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			stat = RvSipSecurityHeaderCopy((RvSipSecurityHeaderHandle)hDestination, 
										  (RvSipSecurityHeaderHandle)hSource);
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			stat = RvSipPProfileKeyHeaderCopy((RvSipPProfileKeyHeaderHandle)hDestination, 
										  (RvSipPProfileKeyHeaderHandle)hSource);
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			stat = RvSipPUserDatabaseHeaderCopy((RvSipPUserDatabaseHeaderHandle)hDestination, 
										  (RvSipPUserDatabaseHeaderHandle)hSource);
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			stat = RvSipPAnswerStateHeaderCopy((RvSipPAnswerStateHeaderHandle)hDestination, 
										  (RvSipPAnswerStateHeaderHandle)hSource);
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			stat = RvSipPServedUserHeaderCopy((RvSipPServedUserHeaderHandle)hDestination, 
										  (RvSipPServedUserHeaderHandle)hSource);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			stat = RvSipPDCSTracePartyIDHeaderCopy((RvSipPDCSTracePartyIDHeaderHandle)hDestination, 
										  (RvSipPDCSTracePartyIDHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			stat = RvSipPDCSOSPSHeaderCopy((RvSipPDCSOSPSHeaderHandle)hDestination, 
										  (RvSipPDCSOSPSHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			stat = RvSipPDCSBillingInfoHeaderCopy((RvSipPDCSBillingInfoHeaderHandle)hDestination, 
										  (RvSipPDCSBillingInfoHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			stat = RvSipPDCSLAESHeaderCopy((RvSipPDCSLAESHeaderHandle)hDestination, 
										  (RvSipPDCSLAESHeaderHandle)hSource);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			stat = RvSipPDCSRedirectHeaderCopy((RvSipPDCSRedirectHeaderHandle)hDestination, 
										  (RvSipPDCSRedirectHeaderHandle)hSource);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_OTHER:
		{
			stat = RvSipOtherHeaderCopy((RvSipOtherHeaderHandle)hDestination, 
										(RvSipOtherHeaderHandle)hSource);
			break;
		}

        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            RvLogError(logId,(logId,"RvSipHeaderCopy - unknown headerType. cannot parse it to the list!"));
            return RV_ERROR_BADPARAM;
        }
    }
    return stat;
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


/***************************************************************************
 * RvSipHeaderIsBadSyntax
 * ------------------------------------------------------------------------
 * General: Checks if the given header is a "bad-syntax" header or not.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to check whether this header contains a "bad-
 *          syntax" string or not..
 *
 * Return Value: RV_TRUE  - if header contains a bad-syntax string.
 *               RV_FALSE - if header does not contain a bad-syntax string.
 *                 If pHeader is NULL the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pHeader     - A handle to the header object.
 *          eHeaderType - Type of given header.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipHeaderIsBadSyntax(IN void*           pHeader,
                                               IN RvSipHeaderType eHeaderType)
{
    RvInt32        BSoffset;

    if(pHeader == NULL)
    {
        return RV_FALSE;
    }

    switch(eHeaderType)
    {
        case RVSIP_HEADERTYPE_CONTACT:
        {
            BSoffset = ((MsgContactHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_EXPIRES:
            {
                BSoffset = ((MsgExpiresHeader*)pHeader)->strBadSyntax;
                break;
            }
        case RVSIP_HEADERTYPE_DATE:
            {
                BSoffset = ((MsgDateHeader*)pHeader)->strBadSyntax;
                break;
            }
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            BSoffset = ((MsgRouteHopHeader*)pHeader)->strBadSyntax;
            break;
        }

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            {
                BSoffset = ((MsgAuthenticationHeader*)pHeader)->strBadSyntax;
                break;
            }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            {
                BSoffset = ((MsgAuthorizationHeader*)pHeader)->strBadSyntax;
                break;
            }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef  RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            BSoffset = ((MsgEventHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            BSoffset = ((MsgAllowHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_RSEQ:
        {
            BSoffset = ((MsgRSeqHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            BSoffset = ((MsgRAckHeader*)pHeader)->strBadSyntax;
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                    SUBS-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            BSoffset = ((MsgReferToHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            BSoffset = ((MsgReferredByHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            BSoffset = ((MsgAllowEventsHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            BSoffset = ((MsgSubscriptionStateHeader*)pHeader)->strBadSyntax;
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
        {
            BSoffset = ((MsgReplacesHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            BSoffset = ((MsgContentDispositionHeader*)pHeader)->strBadSyntax;
            break;
        }

        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            BSoffset = ((MsgRetryAfterHeader*)pHeader)->strBadSyntax;
            break;
        }

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            BSoffset = ((MsgSessionExpiresHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            BSoffset = ((MsgMinSEHeader*)pHeader)->strBadSyntax;
            break;
        }
#endif /* RV_SIP_PRIMITIVES */

        case RVSIP_HEADERTYPE_VIA:
        {
            BSoffset = ((MsgViaHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_TO:
        case RVSIP_HEADERTYPE_FROM:
        {
            BSoffset = ((MsgPartyHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_CSEQ:
        {
            BSoffset = ((MsgCSeqHeader*)pHeader)->strBadSyntax;
            break;
        }

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
        {
            BSoffset = ((MsgContentTypeHeader*)pHeader)->strBadSyntax;
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_ID:
        {
            BSoffset = ((MsgContentIDHeader*)pHeader)->strBadSyntax;
            break;
        }
#endif /*#ifndef RV_SIP_PRIMITIVES*/

        case RVSIP_HEADERTYPE_OTHER:
        {
            BSoffset = ((MsgOtherHeader*)pHeader)->strBadSyntax;
            break;
        }

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
        {
            BSoffset = ((MsgAuthenticationInfoHeader*)pHeader)->strBadSyntax;
            break;
        }
		case RVSIP_HEADERTYPE_REASON:
        {
            BSoffset = ((MsgReasonHeader*)pHeader)->strBadSyntax;
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            BSoffset = ((MsgWarningHeader*)pHeader)->strBadSyntax;
            break;
        }
		case RVSIP_HEADERTYPE_TIMESTAMP:
		{
			BSoffset = ((MsgTimestampHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_INFO:
			{
				BSoffset = ((MsgInfoHeader*)pHeader)->strBadSyntax;
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT:
			{
				BSoffset = ((MsgAcceptHeader*)pHeader)->strBadSyntax;
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
			{
				BSoffset = ((MsgAcceptEncodingHeader*)pHeader)->strBadSyntax;
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
			{
				BSoffset = ((MsgAcceptLanguageHeader*)pHeader)->strBadSyntax;
				break;
			}
		case RVSIP_HEADERTYPE_REPLY_TO:
			{
				BSoffset = ((MsgReplyToHeader*)pHeader)->strBadSyntax;
				break;
			}
		/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		{
			BSoffset = ((MsgPUriHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			BSoffset = ((MsgPVisitedNetworkIDHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			BSoffset = ((MsgPAccessNetworkInfoHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			BSoffset = ((MsgPChargingFunctionAddressesHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			BSoffset = ((MsgPChargingVectorHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			BSoffset = ((MsgPMediaAuthorizationHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			BSoffset = ((MsgSecurityHeader*)pHeader)->strBadSyntax;
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			BSoffset = ((MsgPProfileKeyHeader*)pHeader)->strBadSyntax;
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			BSoffset = ((MsgPUserDatabaseHeader*)pHeader)->strBadSyntax;
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			BSoffset = ((MsgPAnswerStateHeader*)pHeader)->strBadSyntax;
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			BSoffset = ((MsgPServedUserHeader*)pHeader)->strBadSyntax;
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			BSoffset = ((MsgPDCSTracePartyIDHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			BSoffset = ((MsgPDCSOSPSHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			BSoffset = ((MsgPDCSBillingInfoHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			BSoffset = ((MsgPDCSLAESHeader*)pHeader)->strBadSyntax;
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			BSoffset = ((MsgPDCSRedirectHeader*)pHeader)->strBadSyntax;
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            return RV_FALSE;
        }
    }

    if(BSoffset > UNDEFINED)
    {
        return RV_TRUE;
    }
    else
    {
        return RV_FALSE;
    }
}

/***************************************************************************
 * SipHeaderSetBadSyntaxStr
 * ------------------------------------------------------------------------
 * General: The function sets bad-syntax string in a header,
 *          according to it's type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          pstrBadSyntax - pointer to the bad-syntax string.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderSetBadSyntaxStr(
                                          IN    void*           pHeader,
                                          IN    RvSipHeaderType eType,
                                          IN    RvChar*         pstrBadSyntax)
{
    RvStatus rv;
    if((pHeader == NULL) || (eType == RVSIP_HEADERTYPE_UNDEFINED))
    {
        return RV_ERROR_BADPARAM;
    }

    switch(eType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
        {
            rv = RvSipContactHeaderSetStrBadSyntax((RvSipContactHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_EXPIRES:
        {
           rv = RvSipExpiresHeaderSetStrBadSyntax((RvSipExpiresHeaderHandle)pHeader, pstrBadSyntax);
           break;
        }
        case RVSIP_HEADERTYPE_DATE:
        {
            rv = RvSipDateHeaderSetStrBadSyntax((RvSipDateHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            rv = RvSipRouteHopHeaderSetStrBadSyntax((RvSipRouteHopHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            rv = RvSipEventHeaderSetStrBadSyntax((RvSipEventHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            rv = RvSipAllowHeaderSetStrBadSyntax((RvSipAllowHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_RSEQ:
        {
            rv = RvSipRSeqHeaderSetStrBadSyntax((RvSipRSeqHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            rv = RvSipRAckHeaderSetStrBadSyntax((RvSipRAckHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                    SUBS-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            rv = RvSipReferToHeaderSetStrBadSyntax((RvSipReferToHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            rv = RvSipReferredByHeaderSetStrBadSyntax((RvSipReferredByHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            rv = RvSipAllowEventsHeaderSetStrBadSyntax((RvSipAllowEventsHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            rv = RvSipSubscriptionStateHeaderSetStrBadSyntax((RvSipSubscriptionStateHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
        {
            rv = RvSipReplacesHeaderSetStrBadSyntax((RvSipReplacesHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            rv = RvSipContentDispositionHeaderSetStrBadSyntax((RvSipContentDispositionHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
        {
            rv = RvSipAuthenticationHeaderSetStrBadSyntax((RvSipAuthenticationHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
        {
            rv = RvSipAuthorizationHeaderSetStrBadSyntax((RvSipAuthorizationHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            rv = RvSipRetryAfterHeaderSetStrBadSyntax((RvSipRetryAfterHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            rv = RvSipSessionExpiresHeaderSetStrBadSyntax((RvSipSessionExpiresHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            rv = RvSipMinSEHeaderSetStrBadSyntax((RvSipMinSEHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /* #ifndef RV_SIP_PRIMITIVES*/

        case RVSIP_HEADERTYPE_VIA:
        {
            rv = RvSipViaHeaderSetStrBadSyntax((RvSipViaHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_TO:
        case RVSIP_HEADERTYPE_FROM:
        {
            rv = RvSipPartyHeaderSetStrBadSyntax((RvSipPartyHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_CSEQ:
        {
            rv = RvSipCSeqHeaderSetStrBadSyntax((RvSipCSeqHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
        {
            rv = RvSipContentTypeHeaderSetStrBadSyntax((RvSipContentTypeHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_ID:
        {
            rv = RvSipContentIDHeaderSetStrBadSyntax((RvSipContentIDHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
#endif /* #ifndef RV_SIP_PRIMITIVES */
        case RVSIP_HEADERTYPE_OTHER:
        {
            rv = RvSipOtherHeaderSetStrBadSyntax((RvSipOtherHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		{
			rv = RvSipAuthenticationInfoHeaderSetStrBadSyntax((RvSipAuthenticationInfoHeaderHandle)pHeader, pstrBadSyntax);
			break;
		}
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_REASON:
        {
            rv = RvSipReasonHeaderSetStrBadSyntax((RvSipReasonHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            rv = RvSipWarningHeaderSetStrBadSyntax((RvSipWarningHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_TIMESTAMP:
        {
            rv = RvSipTimestampHeaderSetStrBadSyntax((RvSipTimestampHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_INFO:
        {
            rv = RvSipInfoHeaderSetStrBadSyntax((RvSipInfoHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_ACCEPT:
        {
            rv = RvSipAcceptHeaderSetStrBadSyntax((RvSipAcceptHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
        {
            rv = RvSipAcceptEncodingHeaderSetStrBadSyntax((RvSipAcceptEncodingHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
        {
            rv = RvSipAcceptLanguageHeaderSetStrBadSyntax((RvSipAcceptLanguageHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		case RVSIP_HEADERTYPE_REPLY_TO:
        {
            rv = RvSipReplyToHeaderSetStrBadSyntax((RvSipReplyToHeaderHandle)pHeader, pstrBadSyntax);
            break;
        }
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		{
			rv = RvSipPUriHeaderSetStrBadSyntax((RvSipPUriHeaderHandle)pHeader, 
															pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			rv = RvSipPVisitedNetworkIDHeaderSetStrBadSyntax((RvSipPVisitedNetworkIDHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			rv = RvSipPAccessNetworkInfoHeaderSetStrBadSyntax((RvSipPAccessNetworkInfoHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			rv = RvSipPChargingFunctionAddressesHeaderSetStrBadSyntax((RvSipPChargingFunctionAddressesHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			rv = RvSipPChargingVectorHeaderSetStrBadSyntax((RvSipPChargingVectorHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			rv = RvSipPMediaAuthorizationHeaderSetStrBadSyntax((RvSipPMediaAuthorizationHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			rv = RvSipSecurityHeaderSetStrBadSyntax((RvSipSecurityHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			rv = RvSipPProfileKeyHeaderSetStrBadSyntax((RvSipPProfileKeyHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			rv = RvSipPUserDatabaseHeaderSetStrBadSyntax((RvSipPUserDatabaseHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			rv = RvSipPAnswerStateHeaderSetStrBadSyntax((RvSipPAnswerStateHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			rv = RvSipPServedUserHeaderSetStrBadSyntax((RvSipPServedUserHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			rv = RvSipPDCSTracePartyIDHeaderSetStrBadSyntax((RvSipPDCSTracePartyIDHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			rv = RvSipPDCSOSPSHeaderSetStrBadSyntax((RvSipPDCSOSPSHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			rv = RvSipPDCSBillingInfoHeaderSetStrBadSyntax((RvSipPDCSBillingInfoHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			rv = RvSipPDCSLAESHeaderSetStrBadSyntax((RvSipPDCSLAESHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			rv = RvSipPDCSRedirectHeaderSetStrBadSyntax((RvSipPDCSRedirectHeaderHandle)pHeader, 
																pstrBadSyntax);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            return RV_ERROR_BADPARAM;
        }
    }
    return rv;
}

/* SPIRENT_BEGIN */
//#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
#if(RV_TRUE)   // make the following functions always available for usages 
/* SPIRENT_END */

/***************************************************************************
 * SipHeaderName2Str
 * ------------------------------------------------------------------------
 * General: The function converts header type to string.
 *          for printing to log usage.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          badSyntaxOffset - offset to bad-syntax information.
 ***************************************************************************/
RvChar* RVCALLCONV SipHeaderName2Str(RvSipHeaderType eHeaderType)
{
    switch(eHeaderType)
    {
	case RVSIP_HEADERTYPE_ALLOW:
		return "Allow";
	case RVSIP_HEADERTYPE_CONTACT:
		return "Contact";
	case RVSIP_HEADERTYPE_VIA:
		return "Via";
	case RVSIP_HEADERTYPE_EXPIRES:
		return "Expires";
	case RVSIP_HEADERTYPE_DATE:
		return "Date";
	case RVSIP_HEADERTYPE_ROUTE_HOP:
		return "Route-Hop";
	case RVSIP_HEADERTYPE_AUTHENTICATION:
		return "Authentication";
	case RVSIP_HEADERTYPE_AUTHORIZATION:
		return "Authorization";
	case RVSIP_HEADERTYPE_REFER_TO:
		return "Refer-To";
	case RVSIP_HEADERTYPE_REFERRED_BY:
		return "Referred-By";
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
		return "Content-Disposition";
	case RVSIP_HEADERTYPE_RETRY_AFTER:
		return "Retry-After";
	case RVSIP_HEADERTYPE_OTHER:
		return "Other";
	case RVSIP_HEADERTYPE_RSEQ:
		return "RSeq";
	case RVSIP_HEADERTYPE_RACK:
		return "RAck";
	case RVSIP_HEADERTYPE_TO:
		return "To";
	case RVSIP_HEADERTYPE_FROM:
		return "From";
	case RVSIP_HEADERTYPE_CSEQ:
		return "CSeq";
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
		return "Subscription-State";
	case RVSIP_HEADERTYPE_EVENT:
		return "Event";
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
		return "Allow-Events";
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
		return "Session-Expires";
	case RVSIP_HEADERTYPE_MINSE:
		return "MinSE";
	case RVSIP_HEADERTYPE_REPLACES:
		return "Replaces";
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		return "Content-Type";
    case RVSIP_HEADERTYPE_CONTENT_ID:
		return "Content-ID";
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
		return "Authentication-Info";
	case RVSIP_HEADERTYPE_REASON:
		return "Reason";
	case RVSIP_HEADERTYPE_WARNING:
		return "Warning";
	case RVSIP_HEADERTYPE_TIMESTAMP:
		return "Timestamp";
	case RVSIP_HEADERTYPE_INFO:
		return "Info";
	case RVSIP_HEADERTYPE_ACCEPT:
		return "Accept";
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		return "Accept-Encoding";
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		return "Accecpt-Language";
	case RVSIP_HEADERTYPE_REPLY_TO:
		return "Reply-To";

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   case RVSIP_HEADERTYPE_P_URI:
      return "P-Associated-URI";
   case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
      return "P-Visited-Network-ID";
   case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
      return "P-Access-Network-Info";
   case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
      return "P-Charging-Function-Addresses";
   case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
      return "P-Charging-Vector";
   case RVSIP_HEADERTYPE_SECURITY:
      return "Security-Client";
   case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
      return "P-Media-Authorization";
   case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
      return "P-DCS-Trace-Party-ID";
   case RVSIP_HEADERTYPE_P_DCS_OSPS:
      return "P-DCS-OSPS";
   case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
      return "P-DCS-Billing-Info";
   case RVSIP_HEADERTYPE_P_DCS_LAES:
      return "P-DCS-LAES";
   case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
      return "P-DCS-Redirect";
   case RVSIP_HEADERTYPE_P_PROFILE_KEY:
      return "P-Profile-Key";
   case RVSIP_HEADERTYPE_P_USER_DATABASE:
      return "P-User-Database";
   case RVSIP_HEADERTYPE_P_ANSWER_STATE:
      return "P-Answer-State";
   case RVSIP_HEADERTYPE_P_SERVED_USER:
      return "P-Served-User";
#endif
/* SPIRENT_END */

    default:
		return "Undefined";
    }
}

/***************************************************************************
 * SipAddressType2Str
 * ------------------------------------------------------------------------
 * General: The function converts header type to string.
 *          for printing to log usage.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          badSyntaxOffset - offset to bad-syntax information.
 ***************************************************************************/
RvChar* RVCALLCONV SipAddressType2Str(RvSipAddressType eAddrType)
{
    switch(eAddrType)
    {
	case RVSIP_ADDRTYPE_ABS_URI:
		return "Abs-Uri";
    case RVSIP_ADDRTYPE_URL:
		return "Url";
    case RVSIP_ADDRTYPE_TEL:
		return "Tel";
    case RVSIP_ADDRTYPE_PRES:
		return "Pres";
    case RVSIP_ADDRTYPE_IM:
		return "Im";
    case RVSIP_ADDRTYPE_UNDEFINED:
    default:
		return "Undefined";
    }
}

/* SPIRENT_BEGIN */
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/* SPIRENT_END */
/***************************************************************************
 * GetLogIdFromHeader
 * ------------------------------------------------------------------------
 * General: The function converts header type to string.
 *          for printing to log usage.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          badSyntaxOffset - offset to bad-syntax information.
 ***************************************************************************/
static RvLogSource* RVCALLCONV GetLogIdFromHeader(void*           pHeader,
                                                  RvSipHeaderType eHeaderType)
{
    switch(eHeaderType)
    {

        case RVSIP_HEADERTYPE_CONTACT:
           return ((MsgContactHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_EXPIRES:
            return ((MsgExpiresHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_DATE:
            return ((MsgDateHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_ROUTE_HOP:
           return ((MsgRouteHopHeader*)pHeader)->pMsgMgr->pLogSrc;
#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            return ((MsgAuthenticationHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            return ((MsgAuthorizationHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
            return ((MsgEventHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_ALLOW:
           return ((MsgAllowHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_RSEQ:
           return ((MsgRSeqHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_RACK:
           return ((MsgRAckHeader*)pHeader)->pMsgMgr->pLogSrc;
#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                     SUBS-REFER HEADERS                       */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
           return ((MsgReferToHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_REFERRED_BY:
           return ((MsgReferredByHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
            return ((MsgAllowEventsHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
            return ((MsgSubscriptionStateHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
           return ((MsgReplacesHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
           return ((MsgContentDispositionHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_RETRY_AFTER:
           return ((MsgRetryAfterHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
           return ((MsgSessionExpiresHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_MINSE:
           return ((MsgMinSEHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /* RV_SIP_PRIMITIVES */
        case RVSIP_HEADERTYPE_VIA:
           return ((MsgViaHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_OTHER:
           return ((MsgOtherHeader*)pHeader)->pMsgMgr->pLogSrc;

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
           return ((MsgContentTypeHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_CONTENT_ID:
           return ((MsgContentIDHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
        case RVSIP_HEADERTYPE_CSEQ:
           return ((MsgCSeqHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_TO:
        case RVSIP_HEADERTYPE_FROM:
           return ((MsgPartyHeader*)pHeader)->pMsgMgr->pLogSrc;
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
            return ((MsgAuthenticationInfoHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_REASON:
			return ((MsgReasonHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_WARNING:
			return ((MsgWarningHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_TIMESTAMP:
			return ((MsgTimestampHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_INFO:
			return ((MsgInfoHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_ACCEPT:
			return ((MsgAcceptHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
			return ((MsgAcceptEncodingHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
			return ((MsgAcceptLanguageHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_REPLY_TO:
			return ((MsgReplyToHeader*)pHeader)->pMsgMgr->pLogSrc;
			

		/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
			return ((MsgPUriHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
			return ((MsgPVisitedNetworkIDHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
			return ((MsgPAccessNetworkInfoHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
			return ((MsgPChargingFunctionAddressesHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
			return ((MsgPChargingVectorHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
			return ((MsgPMediaAuthorizationHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_SECURITY:
			return ((MsgSecurityHeader*)pHeader)->pMsgMgr->pLogSrc;	
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
			return ((MsgPProfileKeyHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
			return ((MsgPUserDatabaseHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
			return ((MsgPAnswerStateHeader*)pHeader)->pMsgMgr->pLogSrc;
        case RVSIP_HEADERTYPE_P_SERVED_USER:
			return ((MsgPServedUserHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
			return ((MsgPDCSTracePartyIDHeader*)pHeader)->pMsgMgr->pLogSrc;	
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
			return ((MsgPDCSOSPSHeader*)pHeader)->pMsgMgr->pLogSrc;	
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
			return ((MsgPDCSBillingInfoHeader*)pHeader)->pMsgMgr->pLogSrc;	
		case RVSIP_HEADERTYPE_P_DCS_LAES:
			return ((MsgPDCSLAESHeader*)pHeader)->pMsgMgr->pLogSrc;
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
			return ((MsgPDCSRedirectHeader*)pHeader)->pMsgMgr->pLogSrc;
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
            return 0;
    }
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * ResolveHeaderTypeFromBuffer
 * ------------------------------------------------------------------------
 * General: Resolves the header type of the given buffer
 * Return Value: The type of the given header
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  buffer    - Buffer containing a textual Address.
 ***************************************************************************/
static RvSipHeaderType RVCALLCONV ResolveHeaderTypeFromBuffer(
											  IN  RvChar*             buffer)
{
	RvSipHeaderType eType;
	RvUint32        typeLen   = 0;
	RvUint32        headerLen = strlen(buffer);
	RvChar          tab       = 0x09;
	RvChar          space     = ' ';
	RvChar          colon     = ':';
	RvChar          tempChar;
	
	while (typeLen < headerLen &&
		   buffer[typeLen] != colon &&
		   buffer[typeLen] != space &&
		   buffer[typeLen] != tab)
	{
		typeLen += 1;
	}
	if (typeLen == headerLen)
	{
		return RVSIP_HEADERTYPE_UNDEFINED;
	}

	tempChar = buffer[typeLen];
	buffer[typeLen] = '\0';

	if (SipCommonStricmp(buffer, "Allow") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ALLOW;
	}
	else if (SipCommonStricmp(buffer, "Contact") == RV_TRUE)
	{
		eType =  RVSIP_HEADERTYPE_CONTACT;
	}
	else if (SipCommonStricmp(buffer, "Via") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_VIA;
	}
	else if (SipCommonStricmp(buffer, "Expires") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_EXPIRES;
	}
	else if (SipCommonStricmp(buffer, "Date") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_DATE;
	}
	else if (SipCommonStricmp(buffer, "Route") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ROUTE_HOP;
	}
	else if (SipCommonStricmp(buffer, "Record-Route") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ROUTE_HOP;
	}
	else if (SipCommonStricmp(buffer, "WWW-Authenticate") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_AUTHENTICATION;
	}
	else if (SipCommonStricmp(buffer, "Proxy-Authenticate") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_AUTHENTICATION;
	}
	else if (SipCommonStricmp(buffer, "Authorization") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_AUTHORIZATION;
	}
	else if (SipCommonStricmp(buffer, "Proxy-Authorization") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_AUTHORIZATION;
	}
	else if (SipCommonStricmp(buffer, "Refer-To") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_REFER_TO;
	}
	else if (SipCommonStricmp(buffer, "Content-Disposition") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_CONTENT_DISPOSITION;
	}
	else if (SipCommonStricmp(buffer, "Retry-After") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_RETRY_AFTER;
	}
	else if (SipCommonStricmp(buffer, "RSeq") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_RSEQ;
	}
	else if (SipCommonStricmp(buffer, "RAck") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_RACK;
	}
	else if (SipCommonStricmp(buffer, "To") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_TO;
	}
	else if (SipCommonStricmp(buffer, "From") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_FROM;
	}
	else if (SipCommonStricmp(buffer, "CSeq") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_CSEQ;
	}
	else if (SipCommonStricmp(buffer, "Subscription-State") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_SUBSCRIPTION_STATE;
	}
	else if (SipCommonStricmp(buffer, "Event") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_EVENT;
	}
	else if (SipCommonStricmp(buffer, "Allow-Events") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ALLOW_EVENTS;
	}
	else if (SipCommonStricmp(buffer, "Content-Type") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_CONTENT_TYPE;
	}
    else if (SipCommonStricmp(buffer, "Content-ID") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_CONTENT_ID;
	}
	else if (SipCommonStricmp(buffer, "Authentication-Info") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_AUTHENTICATION_INFO;
	}
	else if (SipCommonStricmp(buffer, "Reason") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_REASON;
	}
	else if (SipCommonStricmp(buffer, "Warning") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_WARNING;
	}
	else if (SipCommonStricmp(buffer, "Timestamp") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_TIMESTAMP;
	}
	else if (SipCommonStricmp(buffer, "Alert-Info") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_INFO;
	}
	else if (SipCommonStricmp(buffer, "Call-Info") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_INFO;
	}
	else if (SipCommonStricmp(buffer, "Error-Info") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_INFO;
	}
	else if (SipCommonStricmp(buffer, "Accept") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ACCEPT;
	}
	else if (SipCommonStricmp(buffer, "Accept-Encoding") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ACCEPT_ENCODING;
	}
	else if (SipCommonStricmp(buffer, "Accept-Language") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_ACCEPT_LANGUAGE;
	}
	else if (SipCommonStricmp(buffer, "Reply-To") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_REPLY_TO;
	}
	else if (SipCommonStricmp(buffer, "P-Called-Party-ID") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_URI;
	}
	else if (SipCommonStricmp(buffer, "P-Associated-URI") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_URI;
	}
	else if (SipCommonStricmp(buffer, "P-Asserted-Identity") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_URI;
	}
	else if (SipCommonStricmp(buffer, "P-Preferred-Identity") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_URI;
	}
	else if (SipCommonStricmp(buffer, "P-Visited-Network-ID") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID;
	}
	else if (SipCommonStricmp(buffer, "P-Access-Network-Info") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO;
	}
	else if (SipCommonStricmp(buffer, "P-Charging-Function-Addresses") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES;
	}
	else if (SipCommonStricmp(buffer, "P-Charging-Vector") == RV_TRUE)
	{
		eType = RVSIP_HEADERTYPE_P_CHARGING_VECTOR;
	}
	else
	{
		eType = RVSIP_HEADERTYPE_OTHER;
	}
	
	/* XXX */

	buffer[typeLen] = tempChar;

	return eType;
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/* SPIRENT_BEGIN */
//#if defined(UPDATED_BY_SPIRENT)
RVAPI RvChar* RVCALLCONV RvSipHeaderName2Str(IN RvSipHeaderType eHeaderType)
{
   return SipHeaderName2Str(eHeaderType);
}

RVAPI RvChar* RVCALLCONV RvSipAddressType2Str(IN RvSipAddressType eAddrType)
{
   return SipAddressType2Str(eAddrType);
}
//#endif
/* SPIRENT_END */


#ifdef __cplusplus
}
#endif

