/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              <ParserTypes.h>
 *    This file defines structures which is used in the parser to save tokens of the
 *  message.
 *
 *    Author                         Date
 *    ------                        ------
 *  Micha Mashiach                  Nov 2000
 *********************************************************************************/

#ifndef PARSERTYPES_H
#define PARSERTYPES_H


#include "RV_SIP_DEF.h"

#include "_SipParserManager.h"
#include "RvSipMsgTypes.h"

#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "AdsRpool.h"
#include "RvSipCommonList.h"

#ifdef __cplusplus
extern "C" {
#endif


/* HEADER_PREFIX_LEN defines the length of the external prefix that we add to each 
   header before parsing. 
   According to the prefix, the parser know which rule to implement.
   all headers must have prefixes with the same length!!!
   for the list of prefixes - see function initHeaderPrefix() */
#define HEADER_PREFIX_LEN 4

/*
 * ParserMsgType defines the message type, in case of message parsing.
 */
typedef enum
{
    PARSER_MSG_TYPE_UNDEFINED = -1,
    PARSER_MSG_TYPE_REQUEST,
    PARSER_MSG_TYPE_RESPONSE
}ParserMsgType;


/*
 * SipParsePCBStructType defines the structs that are held in the PCB.
 */
typedef enum {
    SIP_PARSE_PCBSTRUCT_AUTHORIZATION,
    SIP_PARSE_PCBSTRUCT_DIGEST_CHALLENGE,
    SIP_PARSE_PCBSTRUCT_CONTACT,
    SIP_PARSE_PCBSTRUCT_PARTY,
    SIP_PARSE_PCBSTRUCT_REFERRED_BY,
    SIP_PARSE_PCBSTRUCT_REQUEST_LINE,
    SIP_PARSE_PCBSTRUCT_ROUTE,
    SIP_PARSE_PCBSTRUCT_NAME_ADDR,
    SIP_PARSE_PCBSTRUCT_REFER_TO,
    SIP_PARSE_PCBSTRUCT_EXURI,
    SIP_PARSE_PCBSTRUCT_SIP_URL,
    SIP_PARSE_PCBSTRUCT_TEL_URI,
    SIP_PARSE_PCBSTRUCT_SINGLE_VIA,
    SIP_PARSE_PCBSTRUCT_AUTHENTICATION,
    SIP_PARSE_PCBSTRUCT_AUTHENTICATION_INFO,
    SIP_PARSE_PCBSTRUCT_CONTENT_TYPE,
    SIP_PARSE_PCBSTRUCT_CONTENT_ID,
    SIP_PARSE_PCBSTRUCT_URL_PARAMETER,
    SIP_PARSE_PCBSTRUCT_RETRY_AFTER,
    SIP_PARSE_PCBSTRUCT_EXPIRES,
    SIP_PARSE_PCBSTRUCT_CONTENT_DISPOSITION,
    SIP_PARSE_PCBSTRUCT_SENT_PROTOCOL,
    SIP_PARSE_PCBSTRUCT_RACK,
    SIP_PARSE_PCBSTRUCT_VIA_SENT_BY,
    SIP_PARSE_PCBSTRUCT_EVENT_HEADER,
    SIP_PARSE_PCBSTRUCT_SUBS_STATE_HEADER,
    SIP_PARSE_PCBSTRUCT_REPLACES_PARAMS,
    SIP_PARSE_PCBSTRUCT_REPLACES_PARAM,
	SIP_PARSE_PCBSTRUCT_REASON,
	SIP_PARSE_PCBSTRUCT_WARNING,
	SIP_PARSE_PCBSTRUCT_TIMESTAMP,
	SIP_PARSE_PCBSTRUCT_INFO,
	SIP_PARSE_PCBSTRUCT_ACCEPT,
	SIP_PARSE_PCBSTRUCT_ACCEPT_ENCODING,
	SIP_PARSE_PCBSTRUCT_ACCEPT_LANGUAGE,
	SIP_PARSE_PCBSTRUCT_REPLY_TO,
	SIP_PARSE_PCBSTRUCT_P_URI,
	SIP_PARSE_PCBSTRUCT_P_VISITED_NETWORK_ID,
	SIP_PARSE_PCBSTRUCT_P_ACCESS_NETWORK_INFO,
	SIP_PARSE_PCBSTRUCT_P_CHARGING_FUNCTION_ADDRESSES,
	SIP_PARSE_PCBSTRUCT_P_CHARGING_VECTOR,
	SIP_PARSE_PCBSTRUCT_P_MEDIA_AUTHORIZATION,
	SIP_PARSE_PCBSTRUCT_SECURITY,
	SIP_PARSE_PCBSTRUCT_P_DCS_TRACE_PARTY_ID,
	SIP_PARSE_PCBSTRUCT_P_DCS_OSPS,
	SIP_PARSE_PCBSTRUCT_P_DCS_BILLING_INFO,
	SIP_PARSE_PCBSTRUCT_P_DCS_LAES,
	SIP_PARSE_PCBSTRUCT_P_DCS_REDIRECT,
    SIP_PARSE_PCBSTRUCT_P_PROFILE_KEY,
    SIP_PARSE_PCBSTRUCT_P_USER_DATABASE,
    SIP_PARSE_PCBSTRUCT_P_ANSWER_STATE,
    SIP_PARSE_PCBSTRUCT_DIAMETER_URL,
    SIP_PARSE_PCBSTRUCT_P_SERVED_USER
}SipParsePCBStructType;
/* XXX */


/*
 * ParseStatus defines the error that could be return from the parser
 */
typedef enum {
    RV_SIPPARSER_STOP_ERROR = 0,       /* Syntax error accoured        */
    RV_SIPPARSER_STOP_REDUCTION_ERROR, /* Error in reduction function  */
    RV_SIPPARSER_PARSING               /* Parser is working (no error) */
} ParseStatus;

/*
 * there are several cases, that while parsing we set a prefix that will
 * mark the parser how to continue.
 * this enumeration is for all those cases:
 * "USERINFO:"   - is set after 'sip:', before url with userinfo.
 * "HOST:"       - is set after 'sip:', before url without userinfo, only host.
 * "COLON:"      - is set in bad-syntax rule if header contains colon.
 * "NOCOLON:"    - is set in bad-syntax rule if header does not contain colon.
 */
typedef enum {
    RV_SIPPARSER_PREFIX_UNDEFINED,
    RV_SIPPARSER_PREFIX_USERINFO,
    RV_SIPPARSER_PREFIX_HOST,
    RV_SIPPARSER_PREFIX_RV_ABS,
	RV_SIPPARSER_PREFIX_AKAV,
	RV_SIPPARSER_PREFIX_ALGORITHM,
	RV_SIPPARSER_PREFIX_OLD_ADDR_SPEC,
	RV_SIPPARSER_PREFIX_NEW_ADDR_SPEC
} ParseMarkPrefix;

/*
 * ParserBasicToken structure is used to store a pointer to a string and the length
 * of the token.
 */
typedef struct ParserBasicToken_
{
    RvChar * buf;
    RvUint32 len;
} ParserBasicToken;

typedef ParserBasicToken ParserQuotedString;

/* ParserExtensionString structure is used to save tokens from the parser which
   are saved in the extension params in a header.
 */
typedef struct
{
    HRPOOL     hPool;
    HPAGE      hPage;
    RvUint32  size;
}ParserExtensionString;

/*-----------------------------------------------------------------------
 *  S I P  D A T E
 *-----------------------------------------------------------------------*/
/* TIME */
typedef RvInt32 ParserInt32;

typedef enum {
    SIPPARSER_TIME_ZONE_GMT,
    SIPPARSER_TIME_ZONE_UTC,
    SIPPARSER_TIME_ZONE_GMST
} ParserTimeZone;

typedef struct
{
    ParserInt32      day;      /* The day number within a month (for
                                       example 20 in "20 Jan 2000") */
    RvSipDateMonth   eMonth;   /* The enumeration of the month */
    ParserInt32      year;     /* Year */

}ParserDDMMYY;

typedef struct
{
    ParserInt32     minute;   /* Minute */
    ParserInt32     second;   /* Second */
    ParserInt32     hour;     /* Hour */
    ParserTimeZone  zone;
}ParserTime;

/* SIP DATE */
typedef struct
{
    RvSipDateWeekDay      eWeekDay; /* The enumeration of the week day */
    ParserDDMMYY          ddmmyy;
    ParserTime            time;
}ParserSipDate;





/* DELTA SECONDS */
typedef ParserBasicToken ParserDeltaSeconds;

/*-----------------------------------------------------------------------*/
/*  C O N T E N T  L E N G T H                                           */
/*-----------------------------------------------------------------------*/
typedef struct {
    RV_BOOL          isCompact;
    ParserBasicToken contentLenVal;
}ParserContentLength;


/*-----------------------------------------------------------------------*/
/* C A L L  I D                                                          */
/*-----------------------------------------------------------------------*/
typedef struct {
    RvBool          isCompact;
    ParserBasicToken callIdVal;
}ParserCallId;

/*-----------------------------------------------------------------------*/
/* C S E Q                                                               */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserOtherMethod;

typedef enum
{

    PARSER_METHOD_TYPE_INVITE,    /* user or service is being invited to participate
                               in a session. */
    PARSER_METHOD_TYPE_ACK,       /* confirmation that the client has received a final
                               response to INVITE request. */
    PARSER_METHOD_TYPE_BYE,       /* The user agent uses BYE to indicate to the server that
                               it wishes to release the call. */
    PARSER_METHOD_TYPE_CANCEL,    /* The CANCEL request cancels a pending request with the
                              same Call-ID, To, From and CSeq (sequence number only)
                              header field values, but does not affect a completed
                              request or existing calls. (A request is considered
                              completed if the server has returned a final status
                              response.)*/
    PARSER_METHOD_TYPE_REGISTER,  /* A client uses the REGISTER method to bind the address
                              listed in the To header field with a SIP server to
                              one or more URIs where the client can be reached,
                              contained in the Contact header fields. These URIs may
                               use any URI scheme, not limited to SIP.*/
	PARSER_METHOD_TYPE_PUBLISH,  /* A client uses the PUBLISH method to bind the address
                              listed in the To header field with a SIP server to
                              one or more URIs where the client can be reached,
                              contained in the Contact header fields. These URIs may
                               use any URI scheme, not limited to SIP.*/							   
    PARSER_METHOD_TYPE_REFER,
    PARSER_METHOD_TYPE_NOTIFY,
    PARSER_METHOD_TYPE_PRACK,
    PARSER_METHOD_TYPE_SUBSCRIBE,
    PARSER_METHOD_TYPE_OTHER      /* not one of the above */

} ParserMethodType;


typedef struct
{
    ParserMethodType     type;
    ParserOtherMethod    other;
}ParserMethod;

typedef ParserBasicToken ParserSequenceNumber;

typedef struct
{
    ParserSequenceNumber sequenceNumber;
    ParserMethod           method;
}ParserCSeq;

/*-----------------------------------------------------------------------*/
/* S T A T U S   L I N E                                                 */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken  ParserSipVersion;
typedef RvUint16         ParserStatusCode;
typedef ParserBasicToken  ParserReasonPhrase;

typedef struct ParserStatusLine
{
    ParserSipVersion   sipVersion;
    ParserStatusCode   statusCode;
    ParserReasonPhrase reasonPhrase;

}ParserStatusLine;

/*-----------------------------------------------------------------------*/
/* HOST PORT                                                             */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserHostName;
typedef ParserBasicToken ParserPort;

/*-----------------------------------------------------------------------*/
/* U S E R N A M E                                                       */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserUserName;

/*-----------------------------------------------------------------------*/
/* P A S S W O R D                                                       */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserPassword;

/*-----------------------------------------------------------------------*/
/* U R L    U S E R    A N D    H O S T                                  */
/*-----------------------------------------------------------------------*/

typedef struct
{
    ParserBasicToken     userName;
    RvBool              isPassword;
    ParserPassword       password;
}ParserUserInfo;

typedef struct
{
    ParserBasicToken     hostName;
    RvBool              isPort;
    ParserPort           port;
}ParserHostPort;
/*-----------------------------------------------------------------------*/
/* T R A N S P O R T                                                     */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserStrTransport;

typedef struct
{
    RvSipTransport        transport;
    ParserStrTransport    strTransport;
}ParserTransport;

/*-----------------------------------------------------------------------*/
/* T T L  P A R A M                                                      */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserTtlParam;

/*-----------------------------------------------------------------------*/
/* U S E R  P A R A M                                                    */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserOtherUser;
typedef struct
{
    RvSipUserParam  paramType;
    ParserOtherUser otherUser;
}ParserUserParam;

/*-----------------------------------------------------------------------*/
/* M A D D R  P A R A M                                                  */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserMaddrParam;

/*-----------------------------------------------------------------------*/
/* L R  P A R A M                                                        */
/*-----------------------------------------------------------------------*/
typedef enum
{
    ParserLrParamUndefined,
    ParserLrParamEmpty,
    ParserLrParam1,
    ParserLrParamTrue,
	ParserLrParamOn
}ParserLrParamType;

/*-----------------------------------------------------------------------*/
/* C O M P  P A R A M                                                    */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserStrCompParam;
typedef struct
{
    RvSipCompType         compType;
    ParserStrCompParam    strCompParam;
}ParserCompParam;

/*-----------------------------------------------------------------------*/
/* C O M P - I D   P A R A M                                             */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserSigCompIdParam;

/*-----------------------------------------------------------------------*/
/* T O K E N I Z E D   B Y   P A R A M                                   */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserTokenizedByParam;

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*-----------------------------------------------------------------------*/
/* C P C    P A R A M                                                    */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvSipUriCPCType     cpcType;
    ParserBasicToken    strCpcParam;
} ParserCPCParam;

/*-----------------------------------------------------------------------*/
/* G R    P A R A M                                                    */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvBool              bGrParam;
    ParserBasicToken    strGrParam;
} ParserGrParam;

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

/*-----------------------------------------------------------------------*/
/* O T H E R  P A R A M                                                  */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserOtherParam;

/*-----------------------------------------------------------------------*/
/* U R L  P A R A M                                                      */
/*-----------------------------------------------------------------------*/
typedef enum
{
    PARSER_TRANSPORT_PARAM,
    PARSER_USER_PARAM,
    PARSER_TTL_PARAM,
    PARSER_MADDR_PARAM,
    PARSER_METHOD_PARAM,
    PARSER_LR_PARAM,
    PARSER_COMP_PARAM,
	PARSER_SIGCOMPID_PARAM,
	PARSER_TOKENIZED_BY_PARAM,
	PARSER_ORIG_PARAM,
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    PARSER_CPC_PARAM,
    PARSER_GR_PARAM,
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
    PARSER_OTHER_PARAM
}ParserParamTypeUrl;

typedef union
{
    ParserTransport			transport;
    ParserUserParam			userParam;
    ParserTtlParam			ttlParam;
    ParserMaddrParam		maddrParam;
    ParserMethod			methodParam;
    ParserLrParamType		lrParamType;
    ParserCompParam			compParam;
	ParserSigCompIdParam    sigCompIdParam;
	ParserTokenizedByParam	tokenizedByParam;
	RvBool					bOrigParam;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    ParserCPCParam          cpcParam;
    ParserGrParam           grParam;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
    ParserOtherParam		otherParam;
}ParserParamUrl;

typedef struct
{
    ParserParamTypeUrl paramType;
    ParserParamUrl     param;
}ParserUrlParameter;

/*-----------------------------------------------------------------------*/
/* U R L  P A R A M E T E R S                                            */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvBool					isValid;
							
    RvBool					isTransport;
    ParserTransport			transport;
							
    RvBool					isUserParam;
    ParserUserParam			userParam;
							
    RvBool					isTtlParam;
    ParserTtlParam			ttlParam;
							
    RvBool					isMaddrParam;
    ParserMaddrParam		maddrParam;
							
    RvBool					isMethodParam;
    ParserMethod			methodParam;
							
    ParserLrParamType		lrParamType;
							
    RvBool					isCompParam;
    ParserCompParam			compParam;

	RvBool                  isSigCompIdParam;
	ParserSigCompIdParam    sigCompIdParam;
							
	RvBool					isTokenizedByParam;
    ParserTokenizedByParam	tokenizedByParam;
							
	RvBool					bOrigParam;

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvBool                  isCpcParam;
    ParserCPCParam          cpcParam;

    RvBool                  isGrParam;
    ParserGrParam           grParam;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 							

    RvBool					isOtherParams;
    void					*genericParamList;
}ParserUrlParameters;

/*-----------------------------------------------------------------------*/
/* O P T I O N A L  H E A D E R S                                        */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvBool				isSpecified;
    ParserBasicToken	body;
}ParserOptionalHeaders;

/*-----------------------------------------------------------------------*/
/* S I P  U R L                                                          */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvBool                bIsUserInfo;
    ParserUserInfo        urlUserInfo;
    ParserHostPort        urlHost;
    ParserUrlParameters   urlParameters;
    ParserOptionalHeaders optionalHeaders;
    RvBool                bIsSecure;
    RvBool                bOldAddrSpec;
}ParserSipUrl;

/*-----------------------------------------------------------------------*/
/* T E L  U R L                                                          */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_TEL_URI_SUPPORT
/* enumdi parameter */
typedef enum
{
	ParserEnumdiParamUndefined,
	ParserEnumdiParamEmpty
}ParserEnumdiParamType;

/*A structure to represent a single tel-uri parameter, out of all possible ones*/
typedef enum
{
    PARSER_TEL_URI_EXTENSION_PARAM,
	PARSER_TEL_URI_POST_DIAL_PARAM,
	PARSER_TEL_URI_ISDN_SUB_ADDR_PARAM,
	PARSER_TEL_URI_CONTEXT_PARAM,
	PARSER_TEL_URI_ENUMDI_PARAM,
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    PARSER_TEL_URI_CPC_PARAM,
    PARSER_TEL_URI_RN_PARAM,
    PARSER_TEL_URI_RN_CONTEXT_PARAM,
    PARSER_TEL_URI_CIC_PARAM,
    PARSER_TEL_URI_CIC_CONTEXT_PARAM,
    PARSER_TEL_URI_NPDI_PARAM,
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
	PARSER_TEL_URI_OTHER_PARAM
}ParserParamTypeTelUri;

typedef union
{
    ParserBasicToken      strExtension;
	ParserBasicToken      strPostDial;
	ParserBasicToken      strIsdnSubAddr;
	ParserBasicToken      strContext;
	ParserBasicToken      strOtherParams;
	ParserEnumdiParamType eEnumdiType;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    ParserCPCParam        cpcParam;
    ParserBasicToken      strRn;
    ParserBasicToken      strRnContext;
    ParserBasicToken      strCic;
    ParserBasicToken      strCicContext;
    RvBool                bNpdi;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
}ParserParamUnionTelUri;

typedef struct
{
    ParserParamTypeTelUri      paramType;
    ParserParamUnionTelUri     param;
}ParserTelUriParameter;

/* A structure for any string of tel-uri: indicates if the strings was updated,
   and holds the string token */
typedef struct
{
    RvBool                  bIsSpecified;
    ParserBasicToken        strToken;
}ParserTelUriToken;

typedef struct
{
    RvBool                  bIsSpecified;
    void                   *Params;
}ParserTelUriOtherParams;

typedef struct
{
    RvBool                  bIsGlobalPhoneNumber;
    ParserTelUriToken       strPhoneNumber;
	ParserTelUriToken       strExtension;
	ParserTelUriToken       strPostDial;
	ParserTelUriToken       strIsdnSubAddr;
	ParserTelUriToken       strContext;
	ParserTelUriOtherParams strOtherParams;
	ParserEnumdiParamType   eEnumdiType;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvBool                  isCpcParam;
    ParserCPCParam          cpcParam;
    
    ParserTelUriToken       strRn;
    ParserTelUriToken       strRnContext;
    ParserTelUriToken       strCic;
    ParserTelUriToken       strCicContext;
    RvBool                  bNpdi;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
}ParserTelUri;

#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

/*-----------------------------------------------------------------------*/
/* A B S O L U T E    U R I                                              */
/*-----------------------------------------------------------------------*/
typedef struct
{
    ParserBasicToken scheme;
    ParserBasicToken idefntifier;
}ParserAbsoluteUri;

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*-----------------------------------------------------------------------*/
/* D I A M E T E R    U R I                                               */
/*-----------------------------------------------------------------------*/

/* A structure for any string of diameter-uri: indicates if the strings was updated,
   and holds the string token */
typedef struct
{
    RvBool                  bIsSecure;
    
    ParserHostPort          uriHostPort;
    
    RvBool					isTransport;
    ParserTransport			transport;

    RvBool					isProtocol;
    RvSipDiameterProtocol   eProtocolType;
	
    RvBool					isOtherParams;
    void					*otherParams;
}ParserDiameterUri;

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/*-----------------------------------------------------------------------*/
/* E X T E N S I O N    U R I                                            */
/*-----------------------------------------------------------------------*/

typedef enum
{
    PARSER_ADDR_PARAM_TYPE_SIP_URL,
	PARSER_ADDR_PARAM_TYPE_ABS_URI,
	PARSER_ADDR_PARAM_TYPE_TEL_URI,
    PARSER_ADDR_PARAM_TYPE_PRES_URI,
    PARSER_ADDR_PARAM_TYPE_IM_URI,
    PARSER_ADDR_PARAM_TYPE_DIAMETER_URI
} ParserTypeAddr;

typedef union
{
    ParserSipUrl      SipUrl;
    ParserAbsoluteUri absUri;
#ifdef RV_SIP_TEL_URI_SUPPORT
	ParserTelUri      telUri;
#endif
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	ParserDiameterUri diameterUri;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
}exUriUnion;

typedef struct
{
    exUriUnion           ExUriInfo;
    ParserTypeAddr       uriType;
}ParserExUri;
/*-----------------------------------------------------------------------*/
/* R E Q U E S T  L I N E                                                */
/*-----------------------------------------------------------------------*/

typedef struct
{
    ParserMethod      method;
    ParserSipVersion  sipVer;
    ParserExUri       exUri;
    void              *pUrlAddress;

}ParserRequestLine;

/*-----------------------------------------------------------------------*/
/* S E N T  P R O T O C O L                                              */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserProtocolName;
typedef ParserBasicToken ParserProtocolVersion;
typedef struct
{
    ParserProtocolName       protocolName;
    ParserProtocolVersion    protocolVersion;
    ParserTransport          transport;
}ParserSentProtocol;

/*-----------------------------------------------------------------------*/
/* S E N T  B Y                                                          */
/*-----------------------------------------------------------------------*/
typedef struct
{
    ParserHostName hostName;
    RvBool        isPort;
    ParserPort     port;
}ParserSentByInVia;
/*-----------------------------------------------------------------------*/
/* T T L  P A R A M                                                      */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserTTLParam;

/*-----------------------------------------------------------------------*/
/* V I A  P A R A M                                                      */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken ParserComment;
typedef RvBool          ParserHidden;
typedef RvBool          ParserAlias;
typedef ParserBasicToken ParserReceived;
typedef ParserBasicToken ParserBranch;
typedef ParserBasicToken ParserExtend;

typedef enum
{
    PARSER_PARAM_TYPE_HIDDEN,
    PARSER_PARAM_TYPE_ALIAS,
    PARSER_PARAM_TYPE_TTL,
    PARSER_PARAM_TYPE_MADDR,
    PARSER_PARAM_TYPE_RECEIVED,
    PARSER_PARAM_TYPE_BRANCH,
    PARSER_PARAM_TYPE_RPORT,
    PARSER_PARAM_TYPE_COMP,
	PARSER_PARAM_TYPE_SIGCOMPID,
    PARSER_PARAM_TYPE_OTHER
} ParserParamTypeVia;

typedef union
{
    ParserHidden			hidden;
    ParserTTLParam			ttl;
    ParserMaddrParam		maddr;
    ParserReceived			received;
    ParserBranch			branch;
    ParserPort				rport;
    ParserCompParam			comp;
	ParserSigCompIdParam	sigCompId;
    ParserAlias				alias;
} ParserParamVia;

typedef struct
{
    ParserParamTypeVia paramType;
    ParserParamVia     param;
    RvBool            isOther;
    void               *otherParam;
}ParserViaParam;

/*-----------------------------------------------------------------------*/
/* V I A  P A R A M S                                                    */
/*-----------------------------------------------------------------------*/
typedef struct
{
    ParserHidden     hidden;

    RvBool          isTtl;
    ParserTTLParam   ttl;

    RvBool          isMaddr;
    ParserMaddrParam maddr;

    RvBool          isReceived;
    ParserReceived   received;

    RvBool          isBranch;
    ParserBranch     branch;

    RvBool          isRport;
    ParserPort       rPort;

    RvBool          isComp;
    ParserCompParam  comp;

	RvBool               isSigCompId;
	ParserSigCompIdParam sigCompId;

    RvBool          isOtherParams;
    void             *otherParam;

    RvBool          alias;
}ParserViaParams;

/*-----------------------------------------------------------------------*/
/* S I N G L E  V I A                                                    */
/*-----------------------------------------------------------------------*/
typedef struct {
    ParserSentProtocol sentProtocol;
    ParserSentByInVia  sentBy;
    ParserViaParams    params;

    RvBool            isCompact;

}ParserSingleVia;

/*-----------------------------------------------------------------------*/
/* P A R T Y  H E A D E R (base class for TO and FROM headers)           */
/*-----------------------------------------------------------------------*/
typedef ParserBasicToken  ParserDisplayName;
typedef ParserBasicToken  ParserTagParam;
typedef ParserBasicToken  ParserAddrParams;

typedef struct
{
   ParserExUri          exUri;
   RvBool              isDisplayName;
   ParserDisplayName    name;
}ParserNameAddr;

typedef struct
{
    RvBool          isTag;
    ParserTagParam   tag;
    RvBool          isAddrParams;
    void             *addrParams;
}ParserPartyParams;

typedef struct
{
    RvBool           isCompact;
    ParserNameAddr    nameAddr;
    ParserPartyParams partyParams;
}ParserPartyHeader;

/*-----------------------------------------------------------------------*/
/* C O N T A C T                                                         */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvSipExpiresFormat          eFormat;
    ParserDeltaSeconds          deltaSeconds;
    ParserSipDate               sipDate;
}ParserExpiresHeader;



typedef ParserBasicToken ParserQVal;
typedef ParserBasicToken ParserContactExtention;



typedef enum
{
    PARSER_CONTACT_TYPE_Q,
    PARSER_CONTACT_TYPE_ACTION,
    PARSER_CONTACT_TYPE_EXPIRES,
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    PARSER_CONTACT_TYPE_FEATURE_TAG,
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    PARSER_CONTACT_TYPE_OTHER
} ParserContactParamType;

typedef enum
{
    PARSER_CONTACT_ACTION_REDIRECT,
    PARSER_CONTACT_ACTION_PROXY
} ParserContactActionType;



typedef union
{
    ParserExpiresHeader     expires;
    ParserQVal              q;
    ParserContactActionType action;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    ParserBasicToken        featureTag;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
} ParserContactParamUnion;

typedef struct 
{
    ParserBasicToken  value;
    RvBool            isValue;
} ParserContactFeatureTag;

typedef struct
{
    ParserContactParamType  eType;
    ParserContactParamUnion param;
    RvBool                 isExten;
    void                    *exten;
}ParserContactParam;

typedef struct
{
    ParserQVal              q;
    RvBool                 isQ;

    ParserContactActionType action;
    RvBool                 isAction;

    ParserExpiresHeader     expires;
    RvBool                 isExpires;

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    ParserBasicToken        tempGruu;
    RvBool                  isTempGruu;
    ParserBasicToken        pubGruu;
    RvBool                  isPubGruu;
    ParserBasicToken        regIDNum;
    RvBool                  isRegIDNum;

    ParserContactFeatureTag featureAudio;
    RvBool                  isFeatureAudio;
    ParserContactFeatureTag featureAutomata;
    RvBool                  isFeatureAutomata;
    ParserContactFeatureTag featureClass;
    RvBool                  isFeatureClass;
    ParserContactFeatureTag featureDuplex;
    RvBool                  isFeatureDuplex;
    ParserContactFeatureTag featureData;
    RvBool                  isFeatureData;
    ParserContactFeatureTag featureControl;
    RvBool                  isFeatureControl;
    ParserContactFeatureTag featureMobility;
    RvBool                  isFeatureMobility;
    ParserContactFeatureTag featureDescription;
    RvBool                  isFeatureDescription;
    ParserContactFeatureTag featureEvents;
    RvBool                  isFeatureEvents;
    ParserContactFeatureTag featurePriority;
    RvBool                  isFeaturePriority;
    ParserContactFeatureTag featureMethods;
    RvBool                  isFeatureMethods;
    ParserContactFeatureTag featureSchemes;
    RvBool                  isFeatureSchemes;
    ParserContactFeatureTag featureApplication;
    RvBool                  isFeatureApplication;
    ParserContactFeatureTag featureVideo;
    RvBool                  isFeatureVideo;
    ParserContactFeatureTag featureLanguage;
    RvBool                  isFeatureLanguage;
    ParserContactFeatureTag featureType;
    RvBool                  isFeatureType;
    ParserContactFeatureTag featureIsFocus;
    RvBool                  isFeatureIsFocus;
    ParserContactFeatureTag featureActor;
    RvBool                  isFeatureActor;
    ParserContactFeatureTag featureText;
    RvBool                  isFeatureText;
    ParserContactFeatureTag featureExtensions;
    RvBool                  isFeatureExtensions;

    ParserContactFeatureTag featureSipInstance;
    RvBool                  isFeatureSipInstance;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

    void                    *exten;
    RvBool                 isExtention;

}ParserContactParams;

typedef struct
{
    ParserNameAddr      nameAddr;
    RvBool             isParams;
    ParserContactParams params;

}ParserContactHeader;

typedef struct
{
    RvBool             isCompact;
    RvBool             isStar;
    ParserContactHeader header;
}ParserContactHeaderValues;


/*-----------------------------------------------------------------------*/
/* G E N E R A L  H E A D E R                                            */
/*-----------------------------------------------------------------------*/
typedef struct
{
    ParserBasicToken name;
    ParserBasicToken value;
}ParserOtherHeader;


/*-----------------------------------------------------------------------*/
/* C O N T E N T  T Y P E                                                */
/*-----------------------------------------------------------------------*/

#ifndef RV_SIP_PRIMITIVES

typedef struct
{
    RvSipMediaType   type;
    ParserBasicToken other;
}ParserMediaType;

typedef struct
{
    RvSipMediaSubType type;
    ParserBasicToken  other;
}ParserMediaSubType;

#else /*  RV_SIP_PRIMITIVES */

typedef ParserBasicToken ParserMediaType;
typedef ParserBasicToken ParserMediaSubType;

#endif /* RV_SIP_PRIMITIVES */



typedef ParserBasicToken ParserGenericParamList;

#ifndef RV_SIP_PRIMITIVES

typedef ParserBasicToken ParserBoundary;
typedef ParserBasicToken ParserVersion;
typedef ParserBasicToken ParserBase;

#endif /* RV_SIP_PRIMITIVES */

/* Multipart/related parameter */ 
typedef struct 
{
	ParserMediaType         mediaType;
    ParserMediaSubType      mediaSubType;
} ParserContentTypeParamType; 

typedef struct
{
    RvBool                isBoundary;
    RvBool                isVersion;
    RvBool                isBase;
	/* Multipart/related parameters */ 
	RvBool				  isType; 
	RvBool				  isStart;
    RvBool                isOldAddrSpec;
#ifndef RV_SIP_PRIMITIVES
    ParserBoundary         boundary;
    ParserVersion          version;
    ParserBase             base;
	
	/* Multipart/related parameters */ 
	ParserContentTypeParamType  type;
	ParserExUri					start;
#endif /* RV_SIP_PRIMITIVES */
}ParserContentTypeParams;


typedef struct
{
    ParserMediaType         mediaType;
    ParserMediaSubType      mediaSubType;
#ifndef RV_SIP_PRIMITIVES
    ParserContentTypeParams params;
#endif /* RV_SIP_PRIMITIVES */
    RvBool                 isCompactForm;
    void                    *genericParamList;
}ParserContentType;


typedef ParserBasicToken ParserMediaTypeParamAttribute;
typedef ParserBasicToken ParserMediaTypeParamValue;

typedef struct
{
    ParserMediaTypeParamAttribute attribute;
    ParserMediaTypeParamValue     value;
}MediaTypeParam;

typedef ParserBasicToken ParserGenericParamDataVal;
typedef ParserBasicToken ParserGenericParamDataValOptional;
typedef ParserBasicToken ParserGenericParamData;

/*-----------------------------------------------------------------------*/
/* C O N T E N T - I D                                                   */
/*-----------------------------------------------------------------------*/

#ifndef RV_SIP_PRIMITIVES
typedef struct
{
    ParserExUri         addrSpec;
    RvBool              isOldAddrSpec;
} ParserContentID;
#endif /* RV_SIP_PRIMITIVES */

/*-----------------------------------------------------------------------*/
/*                      SUBS-REFER STRUCTURES                            */
/*                     ------------------------                          */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/* R E F E R - T O                                                       */
/*-----------------------------------------------------------------------*/
#ifndef RV_SIP_PRIMITIVES /* defined by express users */

/*typedef ParserExUri ParserReferToHeader ; */

typedef struct
{
    ParserNameAddr nameAddr;
    RvBool         isCompact;

    void          *exten;
    RvBool         isExtention;

}ParserReferToHeader;
#endif /* RV_SIP_PRIMITIVES */
#endif /* #ifdef RV_SIP_SUBS_ON - ReferTo structure */

/*-----------------------------------------------------------------------*/
/* R E F E R R E D - B Y                                                 */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES
typedef struct
{
    ParserNameAddr          referrerAddrSpec;    /* referrer-url: SIP-URL address */
    RvBool                  isCompact;
    ParserBasicToken        cidParam;
    RvBool                  isCid;
    void                   *genericParams;

}ParserReferredByHeader;

#endif /* RV_SIP_PRIMITIVES definition for express only */
#endif /* #ifdef RV_SIP_SUBS_ON - ReferrredBy structure */

/*-----------------------------------------------------------------------*/
/*      E V E N T                                                        */
/*-----------------------------------------------------------------------*/

#ifndef RV_SIP_PRIMITIVES

typedef ParserBasicToken ParserEventPackage;
typedef ParserBasicToken ParserEventTemplate;

typedef enum
{
    PARSER_EVENT_TYPE_ID,
        PARSER_EVENT_TYPE_OTHER
} ParserEventParamType;

typedef struct
{
    ParserEventParamType    eType;
    ParserBasicToken        paramValue;
    RvBool                 isExten;
    void                    *exten;
}ParserEventParam;

typedef struct
{
    ParserBasicToken        eventId;
    RvBool                 isEventId;
    void                    *exten;
    RvBool                 isExtention;
}ParserEventParams;

typedef struct
{
    ParserEventPackage  package;

    RvBool             isTemplate;
    ParserEventTemplate templateVal;
}ParserEventType;

typedef struct
{
    RvBool             isCompact;
    ParserEventType     eventType;
    ParserEventParams   eventParams;
    RvBool             isEventParams;
}ParserEvent;

#endif /* RV_SIP_PRIMITIVES - Event */

/*-----------------------------------------------------------------------*/
/*      A L L O W - E V E N T S                                          */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES
typedef struct
{
    ParserEventType     eventType;
    RvBool             isCompact;
}ParserAllowEvents;

#endif /* RV_SIP_PRIMITIVES - Allow-Events */
#endif /* #ifdef RV_SIP_SUBS_ON - AllowEvents structures */

/*-----------------------------------------------------------------------*/
/*      S U B S C R I P T I O N - S T A T E                              */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES
typedef enum
{
    PARSER_SUBS_STATE_TYPE_REASON,
        PARSER_SUBS_STATE_TYPE_EXPIRES,
        PARSER_SUBS_STATE_TYPE_RETRY_AFTER,
        PARSER_SUBS_STATE_TYPE_OTHER
} ParserSubsStateParamType;

typedef enum {
    PARSER_SUBS_STATE_VAL_ACTIVE,
        PARSER_SUBS_STATE_VAL_PENDING,
        PARSER_SUBS_STATE_VAL_TERMINATED,
        PARSER_SUBS_STATE_VAL_OTHER
} ParserSubsStateVal;

typedef enum {
    PARSER_SUBS_STATE_REASON_DEACTIVATED,
        PARSER_SUBS_STATE_REASON_PROBATION,
        PARSER_SUBS_STATE_REASON_REJECTED,
        PARSER_SUBS_STATE_REASON_TIMEOUT,
        PARSER_SUBS_STATE_REASON_GIVEUP,
        PARSER_SUBS_STATE_REASON_NORESOURCE,
        PARSER_SUBS_STATE_REASON_OTHER
} ParserSubsStateReasonTypes;

typedef struct
{
    ParserSubsStateParamType eType;
    ParserSubsStateReasonTypes eReasonType;
    ParserBasicToken        paramValue;
    RvBool                 isExten;
    void                    *exten;
}ParserSubsStateParam;

typedef struct
{
    ParserBasicToken        expires;
    RvBool                 isExpires;
    ParserBasicToken        retryAfter;
    RvBool                 isRetryAfter;
    ParserSubsStateReasonTypes eReasonType;
    ParserBasicToken        otherReason;
    RvBool                 isReason;
    void                    *exten;
    RvBool                 isExtention;
}ParserSubsStateParams;

typedef struct
{
    ParserSubsStateVal substateVal;
    ParserBasicToken   otherSubstateVal;
}ParserSubsStateValue;

typedef struct
{
    ParserSubsStateReasonTypes substateReason;
    ParserBasicToken           otherSubstateReason;
}ParserSubsStateReason;

typedef struct
{
    ParserSubsStateValue substateValue;
    RvBool              isParams;
    ParserSubsStateParams substateParams;

}ParserSubscriptionState;
#endif /* #ifndef RV_SIP_PRIMITIVES - Subscription State */
#endif /* #ifdef RV_SIP_SUBS_ON - SubscriptionState strucutres */

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/* W W W - A U T H E N T I C A T E / W W W - P R O X Y - A U T H E N T I C A T E  */
/* H E A D E R                                                           */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvBool             isAuth;
    RvBool             isAuthInt;
    RvBool             isOther;
    ParserBasicToken    strOther;
}ParserAuthQopOptions;

typedef struct
{
    RvBool          isDigest;
    ParserBasicToken otherScheme;
}ParserAuthScheme;

typedef struct
{
    RvSipAuthQopOption          eAuthQop;
    RvBool                     isOther;
    void                        *qop; /* optional field given by the server */
}ParserQopValue;

typedef struct
{
    RvSipAuthAlgorithm          eAlgorithm;		/* optional field given by the server */
    ParserBasicToken            algorithm;		/* algorithm string */
	ParserBasicToken            nAKAVersion;	/* aka version string */
	RvBool						isAKAVersion;	/* boolean for aka version */
}ParserAlgorithm;

typedef enum
{
   PARSER_DIGEST_TYPE_REALM,
   PARSER_DIGEST_TYPE_DOMAIN,
   PARSER_DIGEST_TYPE_NONCE,
   PARSER_DIGEST_TYPE_CNONCE,
   PARSER_DIGEST_TYPE_OPAQUE,
   PARSER_DIGEST_TYPE_STALE,
   PARSER_DIGEST_TYPE_ALGORITHM,
   PARSER_DIGEST_TYPE_OPQ_OPTIONS,
   PARSER_DIGEST_TYPE_USERNAME,
   PARSER_DIGEST_TYPE_DIGEST_URI,
   PARSER_DIGEST_TYPE_DIGEST_RESPONSE,
   PARSER_DIGEST_TYPE_MESSAGE_QOP,
   PARSER_DIGEST_TYPE_NONCE_COUNT,
   PARSER_DIGEST_TYPE_AUTH_PARAM,
   PARSER_DIGEST_TYPE_AUTS,
   PARSER_DIGEST_TYPE_INTEGRITY_PROTECTED,
   PARSER_DIGEST_TYPE_INTEGRITY_KEY,
   PARSER_DIGEST_TYPE_CIPHER_KEY,
   PARSER_DIGEST_TYPE_NEW_CHALLENGE
}ParserDigestChallengeType;

typedef struct
{
    ParserDigestChallengeType	eType;
    ParserBasicToken			challenge;
    RvSipAuthStale				eStale;
	RvSipAuthIntegrityProtected eProtected;
    ParserAlgorithm				algorithm;
    ParserQopValue				qopVal;
    ParserExUri					uri;
    ParserAuthQopOptions		authQop;
}ParserDigestChallenge;

typedef struct
{
    RvSipAuthScheme			eAuthScheme;	/* authentication scheme */
    ParserBasicToken		authScheme;		/* authentication scheme string */

    RvBool					isRealm;
    ParserBasicToken		realm;			/* this field must be present */

    RvBool					isDomain;
    ParserBasicToken		domain;			/* optional field given by the server */

    RvBool					isNonce;
    ParserBasicToken		nonce;			/* this field must be present */

    RvBool					isOpaque;
    ParserBasicToken		opaque;			/* optional field given by the server */

    RvSipAuthStale			eStale;			/* optional field given by the server */

    ParserAlgorithm			algorithm;

	RvBool					isAKAVersion;
    ParserBasicToken		nAKAVersion;	/* aka version specified inside algorithm param */

    ParserQopValue			qopValue;
	
	RvBool					isIntegrityKey;
    ParserBasicToken		integrityKey;	/* optional field given by the server */

	RvBool					isCipherKey;
    ParserBasicToken		cipherKey;		/* optional field given by the server */
	
    RvBool					isAuthParam;
    ParserBasicToken		authParam;		/* optional field given by the server */

    RvBool					isAuthParamList;
    void					*authParamList;

}ParserAuthenticationHeader;

/*-----------------------------------------------------------------------
 * A U T H O R I Z A T I O N / P R O X Y - A U T H O R I Z A T I O N
 * H E A D E R
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'Authorization' header or
   'Proxy-Authorization' header.
 */
typedef struct
{
    RvSipAuthScheme				eAuthScheme;/* authentication scheme */
    ParserBasicToken			authScheme;	/* authentication scheme string */

    RvBool						isUsername;
    ParserBasicToken			userName;	/* this field must be present */

    RvBool						isRealm;
    ParserBasicToken			realm;		/* this field must be present */

    RvBool						isNonce;
    ParserBasicToken			nonce;		/* this field must be present */

    RvBool						isUri;
    ParserExUri					hDigestUri;	/* SIP-URL or ABS-URI address, must be present */

    RvBool						isResponse;
    ParserBasicToken			response;	/* this field must be present */

    ParserAlgorithm				algorithm;	/* algorithm string */
	
	RvBool						isCnonce;
    ParserBasicToken			cnonce;		/* optional field given by the server */

    RvBool						isOpaque;
    ParserBasicToken			opaque;		/* optional field given by the server */

    RvBool						isNonceCount;
    ParserBasicToken			nonceCount;	/* optional field given by the server */

	RvBool						isAuts;
    ParserBasicToken			auts;		/* optional field given by the server */

    RvSipAuthIntegrityProtected	eProtected;	/* optional field given by the server */
	
    RvSipAuthQopOption			eAuthQop;	/* optional field */

    RvBool						isAuthParam;
    ParserBasicToken			authParam;	/* optional field given by the server */

    RvBool						isAuthParamList;
    void						*authParamList;
    
	RvSipAuthorizationHeaderType eAuthHeaderType; /* WWW-Authorizarion or PROXY-Authorizarion */

	/*patch - RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    RvChar * uriBegin;
    RvChar * uriEnd;
	
}ParserAuthorizationHeader;
#endif /* #ifdef RV_SIP_AUTH_ON */


/*-----------------------------------------------------------------------
 *  R E C O R D   R O U T E     A N D     R O U T E
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'Route' header or
   'Record-Route' header.
 */

typedef struct
{
    ParserNameAddr      nameAddr;
    RvBool             isParams;
    void*               routeParams;
    SipParseType        eRouteType;

}ParserRouteHeader;


#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------*/
/* R E T R Y - A F T E R                                                 */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvBool            isDeltaSeconds;
    ParserDeltaSeconds deltaSeconds;
    void               *genericParams;

}ParserRetryParam;


typedef struct
{
    RvBool             isSipDate;
    ParserSipDate       sipDate;

    ParserDeltaSeconds  deltaSeconds;

    RvBool             isComment;
    ParserComment       comment;

    ParserRetryParam    retryParam;


}ParserRetryAfter;
#endif /* RV_SIP_PRIMITIVES*/

#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------
 *  R S E C
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'Rseq' header
 */

typedef ParserBasicToken ParserRSeq;
typedef ParserBasicToken ParserResponseNum;


/*-----------------------------------------------------------------------
 *  R A C K
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'Rack' header
 */


typedef struct
{
    ParserResponseNum      responseNum;
    ParserSequenceNumber   sequenceNumber;
    ParserMethod           method;
}ParserRack;

#endif /* rack and rseq RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------
 *  O P T I O N   T A G
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'optionTag' for
   'Proxy-Require', 'Supported', 'Require', Unsupported' headers
 */


typedef struct
{
    SipParseType     headerType;
    ParserBasicToken optionTag;
}ParserOptionTag;

#endif /* RV_SIP_PRIMITIVES - option tag */

#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------*/
/* C O N T E N T  D I S P O S I T I O N                                  */
/*-----------------------------------------------------------------------*/
 /* Content-Disposition = "Content-Disposition" ":"
                          disposition-type *( ";" disposition-param )
    disposition-type = "render" | "session" | "icon" | "alert"
                        | disp-extension-token
    disposition-param = "handling" "="
                        ( "optional" | "required" | other-handling )
                        | generic-param
    other-handling = token
    disp-extension-token = token */

typedef ParserBasicToken ParserOtherDispositionType;
typedef ParserBasicToken ParserOtherHandlingParam;

typedef enum {
    PARSER_DISPOSITION_TYPE_RENDER,
    PARSER_DISPOSITION_TYPE_SESSION,
    PARSER_DISPOSITION_TYPE_ICON,
    PARSER_DISPOSITION_TYPE_ALERT,
    PARSER_DISPOSITION_TYPE_SIGNAL,
    PARSER_DISPOSITION_TYPE_OTHER
}ParserDispositionType;

typedef struct {
    ParserDispositionType      eDispositionType;
    ParserOtherDispositionType otherDispositionType;
}ParserDisposition;

typedef enum {
    PARSER_DISPOSITION_PARAM_HANDLING_OPTIONAL,
    PARSER_DISPOSITION_PARAM_HANDLING_REQUIRED,
    PARSER_DISPOSITION_PARAM_HANDLING_OTHER,
    PARSER_DISPOSITION_PARAM_HANDLING_UNKNOWN
}ParserDispositionHandlingParam;


typedef struct {
    ParserDispositionHandlingParam     eParam;
    ParserOtherHandlingParam           otherHandlingParam;
}ParserDispositionParam;

typedef struct {
    ParserDispositionParam             handlingParam;
    void                               *genericParamList;
    RvBool                            isGenericParam;
}ParserDispositionParams;


typedef struct {
    ParserDisposition       dispositionType;
    ParserDispositionParams dispositionParams;
}ParserContentDisposition;

#endif /* RV_SIP_PRIMITIVES - Content-Disposition */

#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------*/
/*      S E S S I O N  T I M E R                                         */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvBool                          isCompact;
    ParserDeltaSeconds               deltaSeconds;
    RvSipSessionExpiresRefresherType refresherType;
    void                             *exten;
    RvBool                          isExtention;

}ParserSessionExpiresHeader;

typedef struct
{
    ParserDeltaSeconds              deltaSeconds;
    void                            *exten;
    RvBool                         isExtention;

}ParserMinSEHeader;

/*-----------------------------------------------------------------------*/
/* R E P L A C E S    P A R A M                                          */
/*-----------------------------------------------------------------------*/
typedef enum
{
    ParserReplacesEarlyFlagParamUndefined = 0,
    ParserReplacesEarlyFlagParamEmpty,
    ParserReplacesEarlyFlagParam1,
    ParserReplacesEarlyFlagParamTrue
}ParserReplacesEarlyFlagParamType;

typedef enum
{
    PARSER_REPLACES_PARAM_TYPE_TO_TAG,
    PARSER_REPLACES_PARAM_TYPE_FROM_TAG,
    PARSER_REPLACES_PARAM_TYPE_EARLY_FLAG,
    PARSER_REPLACES_PARAM_TYPE_OTHER
} ParserParamTypeReplaces;


typedef struct
{
    ParserParamTypeReplaces paramType;
    ParserBasicToken        paramVal;
    RvBool                 isOther;
    void                    *otherParam;
    ParserReplacesEarlyFlagParamType eEarlyFlagType;
}ParserReplacesParam;

/*-----------------------------------------------------------------------*/
/* R E P L A C E S   P A R A M S                                         */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvBool           isToTag;
    ParserBasicToken  toTag;

    RvBool           isFromTag;
    ParserBasicToken  fromTag;

    ParserReplacesEarlyFlagParamType eEarlyFlagType;

    RvBool          isOtherParams;
    void             *otherParam;
}ParserReplacesParams;



#endif /* RV_SIP_PRIMITIVES */


#ifdef RV_SIP_AUTH_ON

/*-----------------------------------------------------------------------
 * A U T H E N T I C A T I O N - I N F O   H E A D E R
 *-----------------------------------------------------------------------*/
/*
 * This object is used to create and store 'Authentication-Info' header
 */
typedef struct
{
	/* message qop */
    RvBool                       isMsgQop;
	RvSipAuthQopOption           eMsgQop;
    ParserBasicToken             strMsgQop;

	/* nonce-count */
    RvBool                       isNonceCount;
    ParserBasicToken             strNonceCount;

	/* next-nonce */
    RvBool                       isNextNonce;
    ParserBasicToken             strNextNonce;

	/* cnonce */
    RvBool                       isCNonce;
    ParserBasicToken             strCNonce;

	/* response-auth */
    RvBool                       isRspAuth;
    ParserBasicToken             strRspAuth;

}ParserAuthenticationInfoHeader;
#endif /*RV_SIP_AUTH_ON*/

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
/*-----------------------------------------------------------------------
 * R E A S O N   H E A D E R
 *-----------------------------------------------------------------------*/

/* structure of parameters */
typedef struct
{
    /* cause */
    RvBool                       isCause;
    ParserBasicToken             strCause;

	/* text */
    RvBool                       isText;
    ParserQuotedString           strText;

	/* Other params */
	void                        *strOtherParam;
}ParserReasonParams;

/* reason header object */
typedef struct
{
	/* protocol */
    RvBool                       isProtocol;
	RvSipReasonProtocolType      eProtocol;
    ParserBasicToken             strProtocol;

	/* parameters */
	ParserReasonParams           params;
}ParserReasonHeader;

/*-----------------------------------------------------------------------
 * W A R N I N G   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* warn-code */
	ParserStatusCode             warnCode;

	/* warn-agent */
	ParserBasicToken             strWarnAgent;

	/* warn-text */
	ParserBasicToken             strWarnText;

}ParserWarningHeader;

/*-----------------------------------------------------------------------
 * T I M E S T A M P   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* Timestamp val (has two parts: integer and decimal parts) */
	RvBool                       bIsTimestampInt;
	ParserBasicToken             strTimestampInt;
	RvBool                       bIsTimestampDec;
	ParserBasicToken             strTimestampDec;

	/* Delay val (has two parts: integer and decimal parts) */
	RvBool                       bIsDelayInt;
	ParserBasicToken             strDelayInt;
	RvBool                       bIsDelayDec;
	ParserBasicToken             strDelayDec;
	
}ParserTimestampHeader;

/*-----------------------------------------------------------------------
 * I N F O   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* Address spec */
	ParserExUri          exUri;

	/* Other params */
	RvBool               isOtherParams;
    void                *otherParams;
	
}ParserInfoHeader;

/*-----------------------------------------------------------------------
 * A C C E P T   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* media */
	ParserMediaType         mediaType;
    ParserMediaSubType      mediaSubType;

	/* q-val */
	RvBool                  isQVal;
	ParserQVal              qVal;
		
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserAcceptHeader;

/*-----------------------------------------------------------------------
 * A C C E P T - E N C O D I N G   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* coding */
	ParserBasicToken        coding;

	/* q-val */
	RvBool                  isQVal;
	ParserQVal              qVal;
		
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserAcceptEncodingHeader;

/*-----------------------------------------------------------------------
 * A C C E P T - L A N G U A G E   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* coding */
	ParserBasicToken        language;

	/* q-val */
	RvBool                  isQVal;
	ParserQVal              qVal;
		
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserAcceptLanguageHeader;

/*-----------------------------------------------------------------------
 * R E P L Y - T O   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* name-addr */
	ParserNameAddr          nameAddr;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserReplyToHeader;

/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*-----------------------------------------------------------------------
 * P - U R I   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* name-addr */
	ParserNameAddr          nameAddr;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserPUriHeader;

/*-----------------------------------------------------------------------
 * P - V I S I T E D - N E T W O R K - I D   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{	
	ParserBasicToken		vnetworkSpec;

	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserPVisitedNetworkIDHeader;

/*-----------------------------------------------------------------------
 * P - A C C E S S - N E T W O R K - I N F O   H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_ACCESS_TYPE_IEEE_802_11A,
	PARSER_ACCESS_TYPE_IEEE_802_11B,
	PARSER_ACCESS_TYPE_3GPP_GERAN,
	PARSER_ACCESS_TYPE_3GPP_UTRAN_FDD,
	PARSER_ACCESS_TYPE_3GPP_UTRAN_TDD,
	PARSER_ACCESS_TYPE_3GPP_CDMA2000,
	PARSER_ACCESS_TYPE_ADSL,
	PARSER_ACCESS_TYPE_ADSL2,
	PARSER_ACCESS_TYPE_ADSL2_PLUS,
	PARSER_ACCESS_TYPE_RADSL,
	PARSER_ACCESS_TYPE_SDSL,
	PARSER_ACCESS_TYPE_HDSL,
	PARSER_ACCESS_TYPE_HDSL2,
	PARSER_ACCESS_TYPE_G_SHDSL,
	PARSER_ACCESS_TYPE_VDSL,
	PARSER_ACCESS_TYPE_IDSL,
	PARSER_ACCESS_TYPE_3GPP2_1X,
	PARSER_ACCESS_TYPE_3GPP2_1X_HRPD,
    PARSER_ACCESS_TYPE_DOCSIS,
    PARSER_ACCESS_TYPE_IEEE_802_11,
	PARSER_ACCESS_TYPE_IEEE_802_11G,
	PARSER_ACCESS_TYPE_OTHER
} ParserAccessType;

typedef struct
{
    ParserAccessType	type;
    ParserBasicToken	other;
}ParserAccess;

typedef struct
{
	ParserAccess			accessType;

	/* Specific Params */
    RvBool                  bNetworkProvided;

	RvBool                  isCgi3gpp;
	ParserBasicToken		cgi3gpp;
	
	RvBool                  isUtranCellId3gpp;
	ParserBasicToken		utranCellId3gpp;

  	RvBool                  isIWlanNodeID;
	ParserBasicToken		iWlanNodeID;

	RvBool                  isDslLocation;
	ParserBasicToken		dslLocation;

	RvBool                  isCi3gpp2;
	ParserBasicToken		ci3gpp2;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserPAccessNetworkInfoHeader;

/*-----------------------------------------------------------------------
 * P - C H A R G I N G - F U N C T I O N - A D D R E S S E S   H E A D E R
 *-----------------------------------------------------------------------*/

typedef enum
{
	PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_CCF,
	PARSER_CHARGING_FUNCTION_ADDRESSES_LIST_TYPE_ECF
} ParserPChargingFunctionAddressesListType;

typedef struct
{
	ParserPChargingFunctionAddressesListType	eListType;
	ParserBasicToken							value;
}ParserPChargingFunctionAddressesListElement;

typedef struct
{
	RvBool                  isCcfList;
	RvSipCommonListHandle	ccfList;
	
	RvBool                  isEcfList;
	RvSipCommonListHandle	ecfList;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserPChargingFunctionAddressesHeader;

/*-----------------------------------------------------------------------
 * P - C H A R G I N G - V E C T O R   H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_INFO_LIST_TYPE_PDP,
	PARSER_INFO_LIST_TYPE_DSL_BEARER
} ParserInfoListType;

typedef struct
{
	ParserInfoListType			eListType;
	ParserBasicToken			nItem;
	RvBool						bSig;
	ParserBasicToken			cid;
	
	RvBool						isFlowID;
	ParserBasicToken			flowID;
}ParserPChargingVectorInfoListElement;

typedef struct
{
	ParserBasicToken			icidValue;
								
	RvBool						isIcidGenAddr;
	ParserBasicToken			icidGenAddr;
								
	RvBool						isOrigIoi;
	ParserBasicToken			origIoi;
								
	RvBool						isTermIoi;
	ParserBasicToken			termIoi;
								
	RvBool						isGgsn;
	ParserBasicToken			ggsn;
	ParserBasicToken			gprsAuthToken;
	RvBool						isPdpInfoList;
	RvSipCommonListHandle		pdpInfoList;
								
	RvBool						isBras;
	ParserBasicToken			bras;
	ParserBasicToken			xdslAuthToken;
	RvBool						isDslBearerInfoList;
	RvSipCommonListHandle		dslBearerInfoList;
	
	RvBool						bWLanChargingInfo;
    
    RvBool						bPacketcableChargingInfo;
    ParserBasicToken			bcid;
    RvBool						isBCid;

	/* Other params */	
	RvBool						isOtherParams;
    void						*otherParams;
	
}ParserPChargingVectorHeader;

/*-----------------------------------------------------------------------
 * P - M E D I A - A U T H O R I Z A T I O N   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	ParserBasicToken		token;
}ParserPMediaAuthorizationHeader;

/*-----------------------------------------------------------------------
 * S E C U R I T Y   H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_MECHANISM_TYPE_DIGEST,
	PARSER_MECHANISM_TYPE_TLS,
	PARSER_MECHANISM_TYPE_IPSEC_IKE,
	PARSER_MECHANISM_TYPE_IPSEC_MAN,
	PARSER_MECHANISM_TYPE_IPSEC_3GPP,
	PARSER_MECHANISM_TYPE_OTHER
} ParserMechanismType;

typedef struct
{
    ParserMechanismType	type;
    ParserBasicToken	other;
}ParserMechanism;

typedef enum
{
	PARSER_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96,
	PARSER_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96
} ParserSecurityAlgorithmType;

typedef enum
{
	PARSER_SECURITY_PROTOCOL_TYPE_ESP,
	PARSER_SECURITY_PROTOCOL_TYPE_AH
} ParserSecurityProtocolType;

typedef enum
{
	PARSER_SECURITY_MODE_TYPE_TRANS,
	PARSER_SECURITY_MODE_TYPE_TUN,
	PARSER_SECURITY_MODE_TYPE_UDP_ENC_TUN
} ParserSecurityModeType;

typedef enum
{
	PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC,
	PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC,
	PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL
} ParserSecurityEncryptAlgorithmType;

typedef struct
{
	ParserMechanism						mechanism;
										
	RvBool								isPreference;
	ParserQVal							preference;
	
	RvBool								isDigestAlgorithm;
	ParserAlgorithm						digestAlgorithm;
	RvBool								isAKAVersion;
    ParserBasicToken					nAKAVersion;	/* aka version specified inside algorithm param */

/*
	ParserBasicToken					digestAlgorithm;
*/
										
	RvBool								isDigestQop;
	ParserBasicToken					strDigestQop;
	RvSipAuthQopOption					eDigestQop;

	RvBool								isDigestVerify;
	ParserBasicToken					digestVerify;
										
	RvBool								isAlgorithm;
	ParserSecurityAlgorithmType			algorithm;
										
	RvBool								isProtocol;
	ParserSecurityProtocolType			protocol;
										
	RvBool								isMode;
	ParserSecurityModeType				mode;
										
	RvBool								isEncryptAlgorithm;
	ParserSecurityEncryptAlgorithmType	encryptAlgorithm;
										
	RvBool								isSpiC;
	ParserBasicToken					spiC;

	RvBool								isSpiS;
	ParserBasicToken					spiS;

	RvBool								isPortC;
	ParserPort							portC;
											
	RvBool								isPortS;
	ParserPort							portS;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserSecurityHeader;

/*-----------------------------------------------------------------------
 * P - P R O F I L E - K E Y   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* name-addr */
	ParserNameAddr          nameAddr;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
} ParserPProfileKeyHeader;

/*-----------------------------------------------------------------------
 * P - U S E R - D A T A B A S E   H E A D E R
 *-----------------------------------------------------------------------*/

typedef struct
{
	/* Diameter URI */
	ParserExUri          exUri;
} ParserPUserDatabaseHeader;

/*-----------------------------------------------------------------------
 * P - A N S W E R - S T A T E  H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_ANSWER_TYPE_CONFIRMED,
	PARSER_ANSWER_TYPE_UNCONFIRMED,
	PARSER_ANSWER_TYPE_OTHER
} ParserAnswerType;

typedef struct
{
    ParserAnswerType	type;
    ParserBasicToken	other;
}ParserAnswer;

typedef struct
{
	ParserAnswer			answerType;

	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
	
}ParserPAnswerStateHeader;

/*-----------------------------------------------------------------------
 * P - S E R V E D - U S E R  H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_SESSION_CASE_TYPE_ORIG,
	PARSER_SESSION_CASE_TYPE_TERM
} ParserSessionCaseType;

typedef enum
{
    PARSER_REGISTRATION_STATE_TYPE_UNREG,
    PARSER_REGISTRATION_STATE_TYPE_REG
} ParserRegistrationStateType;

typedef struct
{
    /* name-addr */
	ParserNameAddr              nameAddr;
	
	ParserSessionCaseType       sessionCase;
    RvBool                      isSessionCase;
    
    ParserRegistrationStateType registrationState;
    RvBool                      isRegistrationState;
    
	/* Other params */
	RvBool                      isOtherParams;
    void                       *otherParams;
	
}ParserPServedUserHeader;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/*-----------------------------------------------------------------------
 * P - D C S - T R A C E - P A R T Y - I D   H E A D E R
 *-----------------------------------------------------------------------*/
typedef struct
{
	/* name-addr */
	ParserNameAddr          nameAddr;
}ParserPDCSTracePartyIDHeader;

/*-----------------------------------------------------------------------
 * P - D C S - O S P S   H E A D E R
 *-----------------------------------------------------------------------*/
typedef enum
{
	PARSER_OSPS_TAG_TYPE_BLV,
	PARSER_OSPS_TAG_TYPE_EI,
	PARSER_OSPS_TAG_TYPE_RING,
	PARSER_OSPS_TAG_TYPE_OTHER
} ParserOSPSTagType;

typedef struct
{
    ParserOSPSTagType	type;
    ParserBasicToken	other;
}ParserOSPSTag;

typedef struct
{
	ParserOSPSTag          tag;
}ParserPDCSOSPSHeader;

/*-----------------------------------------------------------------------
 * P - D C S - B I L L I N G - I N F O   H E A D E R
 *-----------------------------------------------------------------------*/
typedef struct
{
	ParserBasicToken		billingCorrelationID;
	ParserBasicToken		feid;
	ParserBasicToken		feidHost;

	RvBool                  isRKSGroupID;
	ParserBasicToken		rksGroupID;

	RvBool                  isChargeUri;
	ParserExUri				chargeUri;

	RvBool                  isCallingUri;
	ParserExUri				callingUri;
	
	RvBool                  isCalledUri;
	ParserExUri				calledUri;
	
	RvBool                  isRoutingUri;
	ParserExUri				routingUri;
	
	RvBool                  isLocRoutingUri;
	ParserExUri				locRoutingUri;

	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
}ParserPDCSBillingInfoHeader;

/*-----------------------------------------------------------------------
 * P - D C S - L A E S   H E A D E R
 *-----------------------------------------------------------------------*/
typedef struct
{
	ParserHostName			laesSigHost;
	
	RvBool                  isLaesSigPort;
	ParserPort				laesSigPort;
	
	RvBool                  isLaesContentHost;
	ParserBasicToken		laesContentHost;
	
	RvBool                  isLaesContentPort;
	ParserPort				laesContentPort;
	
	RvBool                  isLaesKey;
	ParserBasicToken		laesKey;
	
	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
}ParserPDCSLAESHeader;

/*-----------------------------------------------------------------------
 * P - D C S - R E D I R E C T   H E A D E R
 *-----------------------------------------------------------------------*/
typedef struct
{
	ParserExUri				calledIDUri;

	RvBool                  isRedirectorUri;
	ParserExUri				redirectorUri;

	RvBool                  isCount;
	ParserBasicToken		count;

	/* Other params */
	RvBool                  isOtherParams;
    void                   *otherParams;
}ParserPDCSRedirectHeader;

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
/*-----------------------------------------------------------------------
 *  B A D - S Y N T A X
 *-----------------------------------------------------------------------*/

#ifndef RV_SIP_PRIMITIVES
typedef struct
{
    ParserBasicToken        headerName;
    ParserBasicToken        headerVal;
    SipParseType            eType;

}ParserBadSyntaxHeader;
#endif /*#ifndef RV_SIP_PRIMITIVES */


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define PARSERTYPES_H*/

