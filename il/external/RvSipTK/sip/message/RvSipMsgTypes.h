/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                       RvSipMsgTypes.h                                      *
 *                                                                            *
 * This file defines all the handles, and the enumeration types, for using    *
 * the message APIs.                                                          *                                                                                                  *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/


#ifndef RVSIPTYPES_H
#define RVSIPTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#define UNDEFINED -1

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

                            /* address definitions */
/*
 * RvSipAddressHandle
 * ----------------------
 * Handle of the sip address object
 */
RV_DECLARE_HANDLE(RvSipAddressHandle);


                            /* headers definitions */

/*
 * RvSipContactHeaderHandle
 * ----------------------
 * Handle of the contact header object
 */
RV_DECLARE_HANDLE(RvSipContactHeaderHandle);



/*
 * RvSipCSeqHeaderHandle
 * ----------------------
 * Handle of the CSeq header object
 */
RV_DECLARE_HANDLE(RvSipCSeqHeaderHandle);

/*
 * RvSipOtherHeaderHandle
 * ----------------------
 * Handle of the other header object
 */
RV_DECLARE_HANDLE(RvSipOtherHeaderHandle);

/*
 * RvSipPartyHeaderHandle
 * ----------------------
 * Handle of the party header object
 */
RV_DECLARE_HANDLE(RvSipPartyHeaderHandle);

/*
 * RvSipViaHeaderHandle
 * ----------------------
 * Handle of the allow header object
 */
RV_DECLARE_HANDLE(RvSipViaHeaderHandle);

/*
 * RvSipAuthenticationHeaderHandle
 * ----------------------
 * Handle of the authentication header object
 */
RV_DECLARE_HANDLE(RvSipAuthenticationHeaderHandle);

/*
* RvSipAuthorizationHeaderHandle
* ----------------------
* Handle of the authorization header object
*/
RV_DECLARE_HANDLE(RvSipAuthorizationHeaderHandle);

#ifndef RV_SIP_PRIMITIVES
/*
 * RvSipAllowHeaderHandle
 * ----------------------
 * Handle of the allow header object
 */
RV_DECLARE_HANDLE(RvSipAllowHeaderHandle);

/*
 * RvSipReferToHeaderHandle
 * ----------------------
 * Handle of the Refer-To header object
 */
RV_DECLARE_HANDLE(RvSipReferToHeaderHandle);

/*
 * RvSipReferredByHeaderHandle
 * ----------------------
 * Handle of the Referred-By header object
 */
RV_DECLARE_HANDLE(RvSipReferredByHeaderHandle);

/*
 * RvSipReplacesHeaderHandle
 * -------------------------
 * Handle of the Replaces header object
 */
RV_DECLARE_HANDLE(RvSipReplacesHeaderHandle);

/*
 * RvSipRSeqHeaderHandle
 * ----------------------
 * Handle of the RSeq header object
 */
RV_DECLARE_HANDLE(RvSipRSeqHeaderHandle);


/*
 * RvSipRAckHeaderHandle
 * ----------------------
 * Handle of the RAck header object
 */
RV_DECLARE_HANDLE(RvSipRAckHeaderHandle);

/*
 * RvSipBodyHandle
 * ----------------------
 * Handle of the Message Body object
 */
RV_DECLARE_HANDLE(RvSipBodyHandle);

/*
 * RvSipBodyPartHandle
 * ----------------------
 * Handle of the Message Body-Part object
 */
RV_DECLARE_HANDLE(RvSipBodyPartHandle);


/*
 * RvSipContentTypeHeaderHandle
 * ----------------------
 * Handle of the Content-Type header object
 */
RV_DECLARE_HANDLE(RvSipContentTypeHeaderHandle);

/*
 * RvSipContentDispositionHeaderHandle
 * ----------------------
 * Handle of the Content-Disposition header object
 */
RV_DECLARE_HANDLE(RvSipContentDispositionHeaderHandle);

/*
 * RvSipContentIDHeaderHandle
 * ----------------------
 * Handle of the Content-ID header object
 */
RV_DECLARE_HANDLE(RvSipContentIDHeaderHandle);

/*
 * RvSipRetryAfterHeaderHandle
 * ----------------------
 * Handle of the retry-after header object
 */
RV_DECLARE_HANDLE(RvSipRetryAfterHeaderHandle);

/*
 * RvSipSubscriptionStateHeaderHandle
 * ----------------------
 * Handle of the Subscription-State header object
 */
RV_DECLARE_HANDLE(RvSipSubscriptionStateHeaderHandle);

/*
 * RvSipEventHeaderHandle
 * ----------------------
 * Handle of the Event header object
 */
RV_DECLARE_HANDLE(RvSipEventHeaderHandle);

/*
 * RvSipAllowEventsHeaderHandle
 * ----------------------
 * Handle of the Allow-Events header object
 */
RV_DECLARE_HANDLE(RvSipAllowEventsHeaderHandle);

/*
 * RvSipSessionExpiresHeaderHandle
 * ----------------------
 * Handle of the Session-Expires header object
 */
RV_DECLARE_HANDLE(RvSipSessionExpiresHeaderHandle);
/*
 * RvSipMinSEHeaderHandle
 * ----------------------
 * Handle of the Min-SE header object
 */
RV_DECLARE_HANDLE(RvSipMinSEHeaderHandle);

#endif /* RV_SIP_PRIMITIVES */

/*
 * RvSipDateHeaderHandle
 * ----------------------
 * Handle of the expires header object
 */
RV_DECLARE_HANDLE(RvSipDateHeaderHandle);

/*
 * RvSipExpiresHeaderHandle
 * ----------------------
 * Handle of the expires header object
 */
RV_DECLARE_HANDLE(RvSipExpiresHeaderHandle);

/*
 * RvSipRouteHopHeaderHandle
 * -----------------------------
 * Handle of the Route header object
 */
RV_DECLARE_HANDLE(RvSipRouteHopHeaderHandle);

#ifdef RV_SIP_AUTH_ON
/*
* RvSipAuthenticationInfoHeaderHandle
* ----------------------
* Handle of the Authentication-Info header object
*/
RV_DECLARE_HANDLE(RvSipAuthenticationInfoHeaderHandle);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

/*
 * RvSipReasonHeaderHandle
 * -----------------------------
 * Handle of the Reason header object
 */
RV_DECLARE_HANDLE(RvSipReasonHeaderHandle);

/*
 * RvSipWarningHeaderHandle
 * -----------------------------
 * Handle of the Warning header object
 */
RV_DECLARE_HANDLE(RvSipWarningHeaderHandle);

/*
 * RvSipTimestampHeaderHandle
 * -----------------------------
 * Handle of the Timestamp header object
 */
RV_DECLARE_HANDLE(RvSipTimestampHeaderHandle);

/*
 * RvSipInfoHeaderHandle
 * -----------------------------
 * Handle of the Info header object
 */
RV_DECLARE_HANDLE(RvSipInfoHeaderHandle);

/*
 * RvSipAcceptHeaderHandle
 * -----------------------------
 * Handle of the Accept header object
 */
RV_DECLARE_HANDLE(RvSipAcceptHeaderHandle);

/*
 * RvSipAcceptEncodingHeaderHandle
 * -----------------------------
 * Handle of the Accept-Encoding header object
 */
RV_DECLARE_HANDLE(RvSipAcceptEncodingHeaderHandle);

/*
 * RvSipAcceptLanguageHeaderHandle
 * -----------------------------
 * Handle of the Accept-Language header object
 */
RV_DECLARE_HANDLE(RvSipAcceptLanguageHeaderHandle);

/*
 * RvSipReplyToHeaderHandle
 * -----------------------------
 * Handle of the Reply-To header object
 */
RV_DECLARE_HANDLE(RvSipReplyToHeaderHandle);

/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*
 * RvSipPUriHeaderHandle
 * -----------------------------
 * Handle of the P-Uri header objects
 */
RV_DECLARE_HANDLE(RvSipPUriHeaderHandle);

/*
 * RvSipPVisitedNetworkIDHeaderHandle
 * -----------------------------
 * Handle of the P-Visited-Network-ID header object
 */
RV_DECLARE_HANDLE(RvSipPVisitedNetworkIDHeaderHandle);

/*
 * RvSipPAccessNetworkInfoHeaderHandle
 * -----------------------------
 * Handle of the P-Access-Network-Info header object
 */
RV_DECLARE_HANDLE(RvSipPAccessNetworkInfoHeaderHandle);

/*
 * RvSipPChargingFunctionAddressesHeaderHandle
 * -----------------------------
 * Handle of the P-Charging-Function-Addresses header object
 */
RV_DECLARE_HANDLE(RvSipPChargingFunctionAddressesHeaderHandle);

/*
 * RvSipPChargingFunctionAddressesListElemHandle
 * -----------------------------
 * Handle of the P-Charging-Function-Addresses header's List Element object,
 * in the ccfList and ecfList params
 */
RV_DECLARE_HANDLE(RvSipPChargingFunctionAddressesListElemHandle);

/*
 * RvSipPChargingVectorHeaderHandle
 * -----------------------------
 * Handle of the P-Charging-Vector header object
 */
RV_DECLARE_HANDLE(RvSipPChargingVectorHeaderHandle);

/*
 * RvSipPChargingVectorInfoListElemHandle
 * -----------------------------
 * Handle of the P-Charging-Vector header's Info List Element object,
 * in the pdp-info and dsl-bearer-info params
 */
RV_DECLARE_HANDLE(RvSipPChargingVectorInfoListElemHandle);

/*
 * RvSipSecurityHeaderHandle
 * -----------------------------
 * Handle of the Security header object
 */
RV_DECLARE_HANDLE(RvSipSecurityHeaderHandle);

/*
 * RvSipPMediaAuthorizationHeaderHandle
 * -----------------------------
 * Handle of the PMediaAuthorization header object
 */
RV_DECLARE_HANDLE(RvSipPMediaAuthorizationHeaderHandle);

/*
 * RvSipPProfileKeyHeaderHandle
 * -----------------------------
 * Handle of the PProfileKey header object
 */
RV_DECLARE_HANDLE(RvSipPProfileKeyHeaderHandle);

/*
 * RvSipPUserDatabaseHeaderHandle
 * -----------------------------
 * Handle of the PUserDatabase header object
 */
RV_DECLARE_HANDLE(RvSipPUserDatabaseHeaderHandle);

/*
 * RvSipPAnswerStateHeaderHandle
 * -----------------------------
 * Handle of the PAnswerState header object
 */
RV_DECLARE_HANDLE(RvSipPAnswerStateHeaderHandle);

/*
 * RvSipPServedUserHeaderHandle
 * -----------------------------
 * Handle of the PServedUser header object
 */
RV_DECLARE_HANDLE(RvSipPServedUserHeaderHandle);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/*
 * RvSipPDCSTracePartyIDHeaderHandle
 * -----------------------------
 * Handle of the PDCSTracePartyID header object
 */
RV_DECLARE_HANDLE(RvSipPDCSTracePartyIDHeaderHandle);

/*
 * RvSipPDCSOSPSHeaderHandle
 * -----------------------------
 * Handle of the PDCSOSPS header object
 */
RV_DECLARE_HANDLE(RvSipPDCSOSPSHeaderHandle);	

/*
 * RvSipPDCSBillingInfoHeaderHandle
 * -----------------------------
 * Handle of the PDCSBillingInfo header object
 */
RV_DECLARE_HANDLE(RvSipPDCSBillingInfoHeaderHandle);

/*
 * RvSipPDCSLAESHeaderHandle
 * -----------------------------
 * Handle of the PDCSLAES header object
 */
RV_DECLARE_HANDLE(RvSipPDCSLAESHeaderHandle);

/*
 * RvSipPDCSRedirectHeaderHandle
 * -----------------------------
 * Handle of the PDCSRedirect header object
 */
RV_DECLARE_HANDLE(RvSipPDCSRedirectHeaderHandle);

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

#ifdef RV_SIP_JSR32_SUPPORT
/* Content-Length is usually handled as RvInt32 by the SIP Stack.
   For JSR32 it is handled as a header */
/*
 * RvSipContentLengthHeaderHandle
 * -----------------------------
 * Handle of the Content-Length header object
 */
RV_DECLARE_HANDLE(RvSipContentLengthHeaderHandle);

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

        /* message definitions */
/*
 * RvSipMsgHandle
 * --------------
 * Handle of the message object
 */
RV_DECLARE_HANDLE(RvSipMsgHandle);

/*
 * RvSipHeadersListElemHandle
 * -----------------------------
 * Handle of the position of one element in the list of headers.
 * will be used only for the getting and setting of headers from the headers list.
 */

RV_DECLARE_HANDLE(RvSipHeaderListElemHandle);


        /* message manager definitions */
/*
 * RvSipMsgMgrHandle
 * --------------
 * Handle of the message manager object
 */
RV_DECLARE_HANDLE(RvSipMsgMgrHandle);

                /* enumerations definitions */

/*
 * RvSipMethodType
 * ---------------
 * defines the methods that are support in the SIP stack.
 */

typedef enum
{
    RVSIP_METHOD_UNDEFINED = -1, /* undefined method type. */
    RVSIP_METHOD_INVITE,         /* user or service is being invited to participate
                                    in a session. */
    RVSIP_METHOD_ACK,            /* confirmation that the client has received a final
                                    response to INVITE request. */
    RVSIP_METHOD_BYE,            /* The user agent uses BYE to indicate to the server that
                                    it wishes to release the call. */
    RVSIP_METHOD_REGISTER,       /* The user agent client requests to register
                                    to a registrar server by the addresses sent
                                    in the Contact header of the REGISTER request*/
    RVSIP_METHOD_REFER,          /* Triggers the server to initiate a call with a
                                    third party. */
    RVSIP_METHOD_NOTIFY,         /* Notifies of an event occurrence. */
    RVSIP_METHOD_OTHER,          /* not one of the above */
    RVSIP_METHOD_PRACK,
    RVSIP_METHOD_CANCEL,         /* cancel a request */
    RVSIP_METHOD_SUBSCRIBE       /* The user requests current state and state updates
                                    from a remote node */

} RvSipMethodType;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
// For detailed SIP statistics
// cctContext in fact cctUH
#if defined(UPDATED_BY_SPIRENT_ABACUS)
typedef  void (RVCALLCONV * RvSipMessageRetransmitEv) (RvSipMethodType method, int cctContext);
#else
typedef  void (RVCALLCONV * RvSipMessageRetransmitEv) (RvSipMethodType method, void *owner);
typedef  void (RVCALLCONV * RvSipMessageRetransmitRcvdEv) (RvSipMsgHandle hMsg, void *owner);
#endif
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


/*
 * RvSipTransport
 * -------------
 * determines the transport protocol to be use for sending
 * SIP requests and responses.
 */
typedef enum
{
    RVSIP_TRANSPORT_UNDEFINED = -1,  /* undefined transport. */
    RVSIP_TRANSPORT_UDP,             /* UDP  */
    RVSIP_TRANSPORT_TCP,             /* TCP  */
    RVSIP_TRANSPORT_SCTP,            /* SCTP */
    RVSIP_TRANSPORT_TLS,             /* TLS  */
    RVSIP_TRANSPORT_OTHER             /* not one of the above */
}RvSipTransport;

#ifdef RV_SIP_IMS_HEADER_SUPPORT  
/*
 * RvSipDiameterProtocol
 * -------------
 * determines the transport protocol to be use for sending
 * SIP requests and responses.
 */
typedef enum
{
    RVSIP_DIAMETER_PROTOCOL_UNDEFINED = -1,  /* Undefined protocol */
    RVSIP_DIAMETER_PROTOCOL_DIAMETER,        /* Diameter  */
    RVSIP_DIAMETER_PROTOCOL_RADIUS,          /* Radius */
    RVSIP_DIAMETER_PROTOCOL_TACACS_PLUS      /* Tacacs+ */
}RvSipDiameterProtocol;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/*
 * RvSipAddressType
 * ----------------
 * Enumeration of the sip address types. will be use in the address object.
 */

typedef enum
{
    RVSIP_ADDRTYPE_UNDEFINED = -1,
    RVSIP_ADDRTYPE_ABS_URI,
    RVSIP_ADDRTYPE_URL,
	RVSIP_ADDRTYPE_TEL,
	RVSIP_ADDRTYPE_PRES,
	RVSIP_ADDRTYPE_IM,
    RVSIP_ADDRTYPE_DIAMETER
}RvSipAddressType;

/*
 * RvSipUrlScheme
 * --------------
 * Enumeration of the sip-url scheme.
 */

typedef enum
{
    RVSIP_URL_SCHEME_UNDEFINED = -1,
    RVSIP_URL_SCHEME_SIP,  /* "sip:".  a SIP URI  */
    RVSIP_URL_SCHEME_SIPS  /* "sips:". a secure URI*/
} RvSipUrlScheme;

/*
 * RvSipUrlLrParamType
 * --------------
 * enumeration defines how is the lr parameter is kept in the sip address:
 * lr; lr=1; lr=true;
 */

typedef enum
{
    RVSIP_URL_LR_TYPE_UNDEFINED = -1,
    RVSIP_URL_LR_TYPE_EMPTY,   /* "lr;" */
    RVSIP_URL_LR_TYPE_1,       /* "lr=1;" */
    RVSIP_URL_LR_TYPE_TRUE,    /* "lr=true;" */
	RVSIP_URL_LR_TYPE_ON       /* "lr=on;" */
} RvSipUrlLrType;


/*
 * RvSipUserParam determines the user info part in the SIP-URL.
 */
typedef enum
{
    RVSIP_USERPARAM_UNDEFINED = -1, /* no user parameter. */
    RVSIP_USERPARAM_PHONE,          /* indicates that the SIP URL contains a phone
                                       number */
    RVSIP_USERPARAM_IP,                /* indicates that the SIP URL contains either
                                       a host name or an IP address */
    RVSIP_USERPARAM_OTHER            /* not one of the above */
} RvSipUserParam;

/*
 * RvSipHeaderType
 * ---------------
 * defines the header type that stored in the
 * headers list in a SIP message.
 * The headers: To/From/CallId/Cseq/ContentLength/ContentType are stored in the
 * sip message by value and they are not part of the headers list.
 */
typedef enum
{
    RVSIP_HEADERTYPE_UNDEFINED = -1,
    RVSIP_HEADERTYPE_ALLOW,
    RVSIP_HEADERTYPE_CONTACT,
    RVSIP_HEADERTYPE_VIA,
    RVSIP_HEADERTYPE_EXPIRES,
    RVSIP_HEADERTYPE_DATE,
    RVSIP_HEADERTYPE_ROUTE_HOP,             /* 5*/
    RVSIP_HEADERTYPE_AUTHENTICATION,
    RVSIP_HEADERTYPE_AUTHORIZATION,
	RVSIP_HEADERTYPE_REFER_TO,
    RVSIP_HEADERTYPE_REFERRED_BY,
    RVSIP_HEADERTYPE_CONTENT_DISPOSITION,   /*10 */
    RVSIP_HEADERTYPE_RETRY_AFTER,
    RVSIP_HEADERTYPE_OTHER,
    RVSIP_HEADERTYPE_RSEQ,
    RVSIP_HEADERTYPE_RACK,
    RVSIP_HEADERTYPE_TO,                    /* 15 */
    RVSIP_HEADERTYPE_FROM,
    RVSIP_HEADERTYPE_CSEQ,
    RVSIP_HEADERTYPE_SUBSCRIPTION_STATE,
    RVSIP_HEADERTYPE_EVENT,
    RVSIP_HEADERTYPE_ALLOW_EVENTS,          /* 20 */
    RVSIP_HEADERTYPE_SESSION_EXPIRES,
    RVSIP_HEADERTYPE_MINSE,
    RVSIP_HEADERTYPE_REPLACES,
    RVSIP_HEADERTYPE_CONTENT_TYPE,
	RVSIP_HEADERTYPE_AUTHENTICATION_INFO,
    RVSIP_HEADERTYPE_REASON,
	RVSIP_HEADERTYPE_WARNING,
	RVSIP_HEADERTYPE_TIMESTAMP, 
	RVSIP_HEADERTYPE_INFO, 
	RVSIP_HEADERTYPE_ACCEPT,                /* 30 */  
	RVSIP_HEADERTYPE_ACCEPT_ENCODING,
	RVSIP_HEADERTYPE_ACCEPT_LANGUAGE,
	RVSIP_HEADERTYPE_REPLY_TO,
	RVSIP_HEADERTYPE_P_URI,                 /* RV_SIP_IMS_HEADER_SUPPORT */
	RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID,
	RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO,
	RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES,
	RVSIP_HEADERTYPE_P_CHARGING_VECTOR,
	RVSIP_HEADERTYPE_SECURITY,              /* 40 */
	RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION,	
	RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID,  /* RV_SIP_IMS_DCS_HEADER_SUPPORT */
	RVSIP_HEADERTYPE_P_DCS_OSPS,
	RVSIP_HEADERTYPE_P_DCS_BILLING_INFO,
	RVSIP_HEADERTYPE_P_DCS_LAES,
	RVSIP_HEADERTYPE_P_DCS_REDIRECT,
    RVSIP_HEADERTYPE_CONTENT_ID,
    RVSIP_HEADERTYPE_P_PROFILE_KEY,
    RVSIP_HEADERTYPE_P_USER_DATABASE,
    RVSIP_HEADERTYPE_P_ANSWER_STATE,
    RVSIP_HEADERTYPE_P_SERVED_USER
} RvSipHeaderType;
/* XXX */

/* RvSipMsgType
 * ------------
 * determines the type of the message it could be request or response.
 */
typedef enum
{
    RVSIP_MSG_UNDEFINED = -1,   /* Undefined Message Type  */
    RVSIP_MSG_REQUEST,          /* REQUSET Message Type  */
    RVSIP_MSG_RESPONSE          /* RESPONSE Message Type */

} RvSipMsgType;

/* RvSipHeadersLocation
 * --------------------
 * The enumeration used for retrieving parameters from the header list. The
 * parameter indicate the location the header you wish to retrieve.
 * It indicates which header the user wishes to get.
 */

typedef enum
{
    RVSIP_FIRST_HEADER = 0,
    RVSIP_LAST_HEADER,
    RVSIP_NEXT_HEADER,
    RVSIP_PREV_HEADER

} RvSipHeadersLocation;


/* RvSipMsgHeadersOption
 * --------------------
 * This enumeration is for RvSipMsgGetHeaderExt,RvSipMsgGetHeaderByTypeExt,
 * RvSipMsgGetHeaderByNameExt API functions.
 * It defines which type of headers the user wishes to get.
 * RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX - Only legal headers, with no syntax errors.
 * RVSIP_MSG_HEADERS_OPTION_ALL          - All headers, bad-syntax or not.
 */
typedef enum
{
    RVSIP_MSG_HEADERS_OPTION_UNDEFINED = -1,
    RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX,
    RVSIP_MSG_HEADERS_OPTION_ALL
} RvSipMsgHeadersOption;

/* RvSipAuthAlgorithm
 * --------------------
 * This enumeration is for the authentication algorithm type
 */
typedef enum {
    RVSIP_AUTH_ALGORITHM_UNDEFINED = -1,
    RVSIP_AUTH_ALGORITHM_MD5,
    RVSIP_AUTH_ALGORITHM_OTHER
}RvSipAuthAlgorithm;

/* RvSipAuthScheme
 * --------------------
 * This enumeration is for the authentication scheme
 */
typedef enum {
    RVSIP_AUTH_SCHEME_UNDEFINED = -1,
    RVSIP_AUTH_SCHEME_DIGEST,
    RVSIP_AUTH_SCHEME_OTHER
}RvSipAuthScheme;


/* RvSipAuthQopOption
 * --------------------
 * The enumeration used to indicate the authentication Qop options.
 */
typedef enum {
    RVSIP_AUTH_QOP_UNDEFINED = -1,
    RVSIP_AUTH_QOP_OTHER,
    RVSIP_AUTH_QOP_AUTH_ONLY,
    RVSIP_AUTH_QOP_AUTHINT_ONLY,
    RVSIP_AUTH_QOP_AUTH_AND_AUTHINT
}RvSipAuthQopOption;

/* RvSipAuthStale
 * --------------------
 * This enumeration is for the authentication stale parameter
 */
typedef enum {
    RVSIP_AUTH_STALE_UNDEFINED = -1,
    RVSIP_AUTH_STALE_FALSE,
    RVSIP_AUTH_STALE_TRUE
}RvSipAuthStale;

/* RvSipAuthIntegrityProtected
 * --------------------
 * This enumeration is for the authentication integrity-protected parameter
 */
typedef enum {
    RVSIP_AUTH_INTEGRITY_PROTECTED_UNDEFINED = -1,
    RVSIP_AUTH_INTEGRITY_PROTECTED_NO,
    RVSIP_AUTH_INTEGRITY_PROTECTED_YES
}RvSipAuthIntegrityProtected;

#ifndef RV_SIP_PRIMITIVES
/* RvSipMediaType
 * --------------------
 * This enumeration is used to indicate the media type of the message body.
 * The media type enumeration is found in the Content-Type header (The
 * Content-Type has the format of Content-Type: media type/media sub type).
 * When the media type is not set it has the value of RVSIP_MEDIATYPE_UNDEFINED
 */
typedef enum {
    RVSIP_MEDIATYPE_UNDEFINED = -1,
    RVSIP_MEDIATYPE_TEXT,
    RVSIP_MEDIATYPE_IMAGE,
    RVSIP_MEDIATYPE_AUDIO,
    RVSIP_MEDIATYPE_VIDEO,
    RVSIP_MEDIATYPE_APPLICATION,
    RVSIP_MEDIATYPE_MULTIPART,
    RVSIP_MEDIATYPE_MESSAGE,
    RVSIP_MEDIATYPE_OTHER

} RvSipMediaType;

/* RvSipMediaSubType
 * --------------------
 * This enumeration is used to indicate the media sub type of the message body.
 * The media sub type enumeration is found in the Content-Type header (The
 * Content-Type has the format of Content-Type: media type/media sub type).
 * When the media sub type is not set it has the value of
 * RVSIP_MEDIASUBTYPE_UNDEFINED
 */
typedef enum {
    RVSIP_MEDIASUBTYPE_UNDEFINED = -1,
    RVSIP_MEDIASUBTYPE_PLAIN,
    RVSIP_MEDIASUBTYPE_SDP,
    RVSIP_MEDIASUBTYPE_ISUP,
    RVSIP_MEDIASUBTYPE_QSIG,
    RVSIP_MEDIASUBTYPE_MIXED,
    RVSIP_MEDIASUBTYPE_ALTERNATIVE,
    RVSIP_MEDIASUBTYPE_DIGEST,
    RVSIP_MEDIASUBTYPE_RFC822,
	RVSIP_MEDIASUBTYPE_3GPP_IMS_XML,
    RVSIP_MEDIASUBTYPE_PIDF_XML,
	RVSIP_MEDIASUBTYPE_PARTIAL_PIDF_XML,
	RVSIP_MEDIASUBTYPE_WATCHERINFO_XML,
	RVSIP_MEDIASUBTYPE_RELATED,
    RVSIP_MEDIASUBTYPE_CSTA_XML,
	RVSIP_MEDIASUBTYPE_OTHER

} RvSipMediaSubType;

/* RvSipDispositionType
 * --------------------
 * This enumeration is used to indicate the disposition type of a Content-Disposition
 * header.
 */
typedef enum {
    RVSIP_DISPOSITIONTYPE_UNDEFINED = -1,
    RVSIP_DISPOSITIONTYPE_RENDER,
    RVSIP_DISPOSITIONTYPE_SESSION,
    RVSIP_DISPOSITIONTYPE_ICON,
    RVSIP_DISPOSITIONTYPE_ALERT,
    RVSIP_DISPOSITIONTYPE_SIGNAL,    /* see: draft-ietf-sip-isup-mime */
    RVSIP_DISPOSITIONTYPE_OTHER

} RvSipDispositionType;

/* RvSipDispositionHandling
 * --------------------
 * This enumeration is used to indicate the handling type of a Content-Disposition
 * header.
 */
typedef enum {
    RVSIP_DISPOSITIONHANDLING_UNDEFINED = -1,
    RVSIP_DISPOSITIONHANDLING_OPTIONAL,
    RVSIP_DISPOSITIONHANDLING_REQUIRED,
    RVSIP_DISPOSITIONHANDLING_OTHER

} RvSipDispositionHandling;

#endif /* RV_SIP_PRIMITIVES */


/* RvSipDateWeekDay
 * --------------------
 * This enumeration is used to indicate the week day within a given date.
 */
typedef enum {
    RVSIP_WEEKDAY_UNDEFINED = -1,
    RVSIP_WEEKDAY_SUN,
    RVSIP_WEEKDAY_MON,
    RVSIP_WEEKDAY_TUE,
    RVSIP_WEEKDAY_WED,
    RVSIP_WEEKDAY_THU,
    RVSIP_WEEKDAY_FRI,
    RVSIP_WEEKDAY_SAT

}RvSipDateWeekDay;

/* RvSipDateMonth
 * --------------------
 * This enumeration is used to indicate the month within a given date.
 */
typedef enum {
    RVSIP_MONTH_UNDEFINED = -1,
    RVSIP_MONTH_JAN,
    RVSIP_MONTH_FEB,
    RVSIP_MONTH_MAR,
    RVSIP_MONTH_APR,
    RVSIP_MONTH_MAY,
    RVSIP_MONTH_JUN,
    RVSIP_MONTH_JUL,
    RVSIP_MONTH_AUG,
    RVSIP_MONTH_SEP,
    RVSIP_MONTH_OCT,
    RVSIP_MONTH_NOV,
    RVSIP_MONTH_DEC

}RvSipDateMonth;

/* RvSipExpiresFormat
 * --------------------
 * This enumeration is used to indicate the format of a given Expires header.
 * If the header is not initialized the format is undefined. An initialized
 * Expires header can either contain a date structure or an integer indicating
 * delta-seconds time.
 */
typedef enum {
    RVSIP_EXPIRES_FORMAT_UNDEFINED = -1,
    RVSIP_EXPIRES_FORMAT_DELTA_SECONDS,
    RVSIP_EXPIRES_FORMAT_DATE

}RvSipExpiresFormat;


/* RvSipContactAction
 * --------------------
 * This enumeration is used to indicate the contact action.
 */
typedef enum {
    RVSIP_CONTACT_ACTION_UNDEFINED = -1,
    RVSIP_CONTACT_ACTION_PROXY,
    RVSIP_CONTACT_ACTION_REDIRECT
} RvSipContactAction;

#ifdef RV_SIP_IMS_HEADER_SUPPORT 
/* RvSipContactFeatureTagType
 * --------------------
 * This enumeration is used to indicate the contact feature tag type.
 */
typedef enum {
    RVSIP_CONTACT_FEATURE_TAG_TYPE_UNDEFINED = -1,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS,
    RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE
} RvSipContactFeatureTagType;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

#ifndef RV_SIP_PRIMITIVES
/* RvSipSubscriptionSubstate
 * --------------------
 * This enumeration is used to indicate the substate in Subscription-State header.
 */
typedef enum {
    RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED = -1,
    RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE,
    RVSIP_SUBSCRIPTION_SUBSTATE_PENDING,
    RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED,
    RVSIP_SUBSCRIPTION_SUBSTATE_OTHER
}RvSipSubscriptionSubstate;

/* RvSipSubscriptionReason
 * --------------------
 * This enumeration is used to indicate the reason in Subscription-State header.
 */
typedef enum {
    RVSIP_SUBSCRIPTION_REASON_UNDEFINED = -1,
    RVSIP_SUBSCRIPTION_REASON_DEACTIVATED,
    RVSIP_SUBSCRIPTION_REASON_PROBATION,
    RVSIP_SUBSCRIPTION_REASON_REJECTED,
    RVSIP_SUBSCRIPTION_REASON_TIMEOUT,
    RVSIP_SUBSCRIPTION_REASON_GIVEUP,
    RVSIP_SUBSCRIPTION_REASON_NORESOURCE,
    RVSIP_SUBSCRIPTION_REASON_OTHER
}RvSipSubscriptionReason;

/* RvSipSessionExpiresRefresherType
 * --------------------
 * This enumeration is used when session timer feature is enable.
 * RVSIP_SESSION_EXPIRES_REFRESHER_NONE - the refresher side of the call has not been set.
 * RVSIP_SESSION_EXPIRES_REFRESHER_UAC  - the refresher of the call is the uac
 * RVSIP_SESSION_EXPIRES_REFRESHER_UAS  - the refresher of the call is the uas
 */
typedef enum
{
    RVSIP_SESSION_EXPIRES_REFRESHER_NONE = -1,
    RVSIP_SESSION_EXPIRES_REFRESHER_UAC,
    RVSIP_SESSION_EXPIRES_REFRESHER_UAS
}RvSipSessionExpiresRefresherType;


/*
 * RvSipReplacesEarlyFlagType
 * --------------
 * enumeration defines how the early-only parameter was received:
 * (early-only; early-only=true; early-only=1)
 */

typedef enum
{
    RVSIP_REPLACES_EARLY_FLAG_TYPE_UNDEFINED = -1,
    RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_EMPTY,   /* "early-only;" */
    RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_1,       /* "early-only=1;" */
    RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_TRUE     /* "early-only=true;" */
} RvSipReplacesEarlyFlagType;
#endif /* RV_SIP_PRIMITIVES */

/*
 * RvSipCompType
 * --------------
 * This enumeration is used to indicate the compression type of SIP messages
 * RVSIP_COMP_UNDEFINED - Undefined compression type.
 * RVSIP_COMP_SIGCOMP   - SigComp compression.
 * RVSIP_COMP_OTHER     - not one of the above.
 */
typedef enum
{
    RVSIP_COMP_UNDEFINED = -1,
    RVSIP_COMP_SIGCOMP,
    RVSIP_COMP_OTHER
}RvSipCompType;

#ifdef RV_SIP_TEL_URI_SUPPORT
/*
 * RvSipTelUriEnumdiType
 * --------------
 * This enumeration is used to indicate the enumdi type of a TEL URI address
 * RVSIP_ENUMDI_TYPE_UNDEFINED    - The enumdi parameter was not found in the TEL URI address
 * RVSIP_ENUMDI_TYPE_EXISTS_EMPTY - ";enumdi" - The enumdi parameter was found in the TEL URI address
 */
typedef enum
{
    RVSIP_ENUMDI_TYPE_UNDEFINED = -1,
    RVSIP_ENUMDI_TYPE_EXISTS_EMPTY
}RvSipTelUriEnumdiType;
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

/*
 * RvSipCompType
 * --------------
 * This enumeration is used to indicate the protocol in a Reason header
 * RVSIP_REASON_PROTOCOL_UNDEFINED - Undefined compression type.
 * RVSIP_REASON_PROTOCOL_SIP       - Sip protocol.
 * RVSIP_REASON_PROTOCOL_Q_850     - Q.850 protocol.
 * RVSIP_REASON_PROTOCOL_OTHER     - Not one of the above.
 */
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
typedef enum
{
	RVSIP_REASON_PROTOCOL_UNDEFINED,
	RVSIP_REASON_PROTOCOL_SIP,
	RVSIP_REASON_PROTOCOL_Q_850,
	RVSIP_REASON_PROTOCOL_OTHER
}RvSipReasonProtocolType;
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*
 * RvSipUriCPCType
 * --------------
 * This enumeration is used to indicate the cpc type of a TEL URI address
 * RVSIP_CPC_TYPE_UNDEFINED     - The cpc parameter was not found in the TEL URI address
 * RVSIP_CPC_TYPE_CELLULAR      - cpc of type cellular.
 * RVSIP_CPC_TYPE_OTHER         - not one of the above.
 */
typedef enum
{
    RVSIP_CPC_TYPE_UNDEFINED = -1,
    RVSIP_CPC_TYPE_CELLULAR,
    RVSIP_CPC_TYPE_CELLULAR_ROAMING,
    RVSIP_CPC_TYPE_HOSPITAL,
    RVSIP_CPC_TYPE_OPERATOR,
    RVSIP_CPC_TYPE_ORDINARY,
    RVSIP_CPC_TYPE_PAYPHONE,
    RVSIP_CPC_TYPE_POLICE,
    RVSIP_CPC_TYPE_PRISON,
    RVSIP_CPC_TYPE_TEST,
    RVSIP_CPC_TYPE_UNKNOWN,
    RVSIP_CPC_TYPE_OTHER
}RvSipUriCPCType;

/*
 * RvSipPAccessNetworkInfoAccessType
 * --------------
 * This enumeration is used to indicate the access type of a PAccessNetworkInfo header.
 */
typedef enum
{
    RVSIP_ACCESS_TYPE_UNDEFINED = -1,
    RVSIP_ACCESS_TYPE_IEEE_802_11A,
	RVSIP_ACCESS_TYPE_IEEE_802_11B,
	RVSIP_ACCESS_TYPE_3GPP_GERAN,
	RVSIP_ACCESS_TYPE_3GPP_UTRAN_FDD,
	RVSIP_ACCESS_TYPE_3GPP_UTRAN_TDD,
	RVSIP_ACCESS_TYPE_3GPP_CDMA2000,
	RVSIP_ACCESS_TYPE_ADSL,
	RVSIP_ACCESS_TYPE_ADSL2,
	RVSIP_ACCESS_TYPE_ADSL2_PLUS,
	RVSIP_ACCESS_TYPE_RADSL,
	RVSIP_ACCESS_TYPE_SDSL,
	RVSIP_ACCESS_TYPE_HDSL,
	RVSIP_ACCESS_TYPE_HDSL2,
	RVSIP_ACCESS_TYPE_G_SHDSL,
	RVSIP_ACCESS_TYPE_VDSL,
	RVSIP_ACCESS_TYPE_IDSL,
	RVSIP_ACCESS_TYPE_3GPP2_1X,
	RVSIP_ACCESS_TYPE_3GPP2_1X_HRPD,
	RVSIP_ACCESS_TYPE_DOCSIS,
    RVSIP_ACCESS_TYPE_IEEE_802_11,
    RVSIP_ACCESS_TYPE_IEEE_802_11G,
	RVSIP_ACCESS_TYPE_OTHER
}RvSipPAccessNetworkInfoAccessType;	

/*
 * RvSipPChargingFunctionAddressesListType
 * --------------
 * This enumeration is used to indicate the info list type of a list in P-Charging-Function-Addresses header.
 */
typedef enum
{
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_UNDEFINED = -1,
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF,
	RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF
}RvSipPChargingFunctionAddressesListType;	

/*
 * RvSipPChargingFunctionAddressesListElemType
 * --------------
 * This enumeration is used to indicate the info list element type of a list in P-Charging-Function-Addresses header.
 */
typedef enum
{
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_ELEMENT_TYPE_UNDEFINED = -1,
    RVSIP_P_CHARGING_FUNCTION_ADDRESSES_LIST_ELEMENT_TYPE_CCF_ECF
}RvSipPChargingFunctionAddressesListElemType;	

/*
 * RvSipPChargingVectorInfoListType
 * --------------
 * This enumeration is used to indicate the info list type of a list in P-Charging-Vector header.
 */
typedef enum
{
    RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_UNDEFINED = -1,
    RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_PDP,
	RVSIP_P_CHARGING_VECTOR_INFO_LIST_TYPE_DSL_BEARER
}RvSipPChargingVectorInfoListType;	

/*
 * RvSipPChargingVectorInfoListElemType
 * --------------
 * This enumeration is used to indicate the info list element type of a list in P-Charging-Vector header.
 */
typedef enum
{
    RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_TYPE_UNDEFINED = -1,
    RVSIP_P_CHARGING_VECTOR_INFO_LIST_ELEMENT_TYPE_INFO
}RvSipPChargingVectorInfoListElemType;	

/*
 * RvSipSecurityMechanismType
 * --------------
 * This enumeration is used to indicate the security mechanism type of the Security header.
 */
typedef enum
{
    RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED  = -1,
    RVSIP_SECURITY_MECHANISM_TYPE_DIGEST     = 0x01,
	RVSIP_SECURITY_MECHANISM_TYPE_TLS        = 0x02,
	RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE  = 0x04,
	RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN  = 0x08,
	RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP = 0x10,
	RVSIP_SECURITY_MECHANISM_TYPE_OTHER      = 0x20
}RvSipSecurityMechanismType;	

/*
 * RvSipSecurityAlgorithmType
 * --------------
 * This enumeration is used to indicate the security Algorithm type of the Security header.
 */
typedef enum
{
    RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED = -1,
    RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96,
	RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96
}RvSipSecurityAlgorithmType;

/*
 * RvSipSecurityProtocolType
 * --------------
 * This enumeration is used to indicate the security Protocol type of the Security header.
 */
typedef enum
{
    RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED = -1,
    RVSIP_SECURITY_PROTOCOL_TYPE_ESP,
	RVSIP_SECURITY_PROTOCOL_TYPE_AH
}RvSipSecurityProtocolType;

/*
 * RvSipSecurityModeType
 * --------------
 * This enumeration is used to indicate the security Mode type of the Security header.
 */
typedef enum
{
    RVSIP_SECURITY_MODE_TYPE_UNDEFINED = -1,
    RVSIP_SECURITY_MODE_TYPE_TRANS,
	RVSIP_SECURITY_MODE_TYPE_TUN,
	RVSIP_SECURITY_MODE_TYPE_UDP_ENC_TUN
}RvSipSecurityModeType;

/*
 * RvSipSecurityEncryptAlgorithmType
 * --------------
 * This enumeration is used to indicate the security Encrypt-Algorithm type of the Security header.
 */
typedef enum
{
    RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED = -1,
	RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC,
	RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC,
    RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL
}RvSipSecurityEncryptAlgorithmType;

/*
 * RvSipPAnswerStateAnswerType
 * --------------
 * This enumeration is used to indicate the answer type of a PAnswerState header.
 */
typedef enum
{
    RVSIP_ANSWER_TYPE_UNDEFINED = -1,
    RVSIP_ANSWER_TYPE_CONFIRMED,
	RVSIP_ANSWER_TYPE_UNCONFIRMED,
	RVSIP_ANSWER_TYPE_OTHER
}RvSipPAnswerStateAnswerType;	

/*
 * RvSipPServedUserSessionCaseType
 * --------------
 * This enumeration is used to indicate the Session Case type of a PServedUser header.
 */
typedef enum
{
    RVSIP_SESSION_CASE_TYPE_UNDEFINED = -1,
    RVSIP_SESSION_CASE_TYPE_ORIG,
	RVSIP_SESSION_CASE_TYPE_TERM
}RvSipPServedUserSessionCaseType;

/*
 * RvSipPServedUserRegistrationStateType
 * --------------
 * This enumeration is used to indicate the Registration State type of a PServedUser header.
 */
typedef enum
{
    RVSIP_REGISTRATION_STATE_TYPE_UNDEFINED = -1,
    RVSIP_REGISTRATION_STATE_TYPE_UNREG,
	RVSIP_REGISTRATION_STATE_TYPE_REG
}RvSipPServedUserRegistrationStateType;

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/*
 * RvSipOSPSTagType
 * --------------
 * This enumeration is used to indicate the OSPS type of the PDCSOSPS header.
 */
typedef enum
{
    RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED = -1,
    RVSIP_P_DCS_OSPS_TAG_TYPE_BLV,
	RVSIP_P_DCS_OSPS_TAG_TYPE_EI,
	RVSIP_P_DCS_OSPS_TAG_TYPE_RING,
	RVSIP_P_DCS_OSPS_TAG_TYPE_OTHER
}RvSipOSPSTagType;

/*
 * RvSipPDCSBillingInfoAddressType
 * --------------
 * This enumeration is used to indicate the field for the address in the PDCSBillingInfo header.
 */
typedef enum
{
    RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_UNDEFINED = -1,
    RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE,
	RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING,
	RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED,
	RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING,
	RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE
}RvSipPDCSBillingInfoAddressType;

/*
 * RvSipPDCSRedirectAddressType
 * --------------
 * This enumeration is used to indicate the field for the address in the PDCSRedirect header.
 */
typedef enum
{
    RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_UNDEFINED = -1,
    RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID,
	RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR
}RvSipPDCSRedirectAddressType;
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
#ifdef __cplusplus
}
#endif
#endif /* RVSIPTYPES_H */
