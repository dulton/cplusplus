/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                               MsgTypes.h                                   *
 * This file defines the internal types and structs of the message module.    *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/


#ifndef SIPINTERNALTYPES_H
#define SIPINTERNALTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
/* This include MUSTN'T be here '#include "RvSipUserConfig.h"' (due to RV_NOUSRCONFIG), instead "RV_SIP_DEF.h" MUST be included */ 
   
#include "RvSipMsgTypes.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipSessionExpiresHeader.h"
#include "AdsRpool.h"
#include "AdsRlist.h"
#include "_SipParserManager.h"
#include "RvSipTransportTypes.h"
#include "_SipCommonUtils.h"
#include "RvSipCommonList.h"
#include "rvrandomgenerator.h"
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
#include "RvSipInfoHeader.h"
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "RvSipPUriHeader.h"
#include "RvSipSecurityHeader.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

/*-----------------------------------------------------------------------*/
/*                          GLOBAL VARAIABLES                            */
/*-----------------------------------------------------------------------*/
#define SIP_MSG_CALL_ID_HEADER_NAME "Call-ID"

#ifdef RV_SIP_JSR32_SUPPORT
/* This is the maximal length of all the supported enumeration 
   strings in the message library. */
#define MAX_LENGTH_OF_ENUMERATION_STRING 64
/* Define the maximal length of a string that is produced out of a float naumber */
#define MAX_LENGTH_OF_FLOAT 24
/* Define the maximal length of a string that is produced out of an integer naumber */
#define MAX_LENGTH_OF_INT 12
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/* MsgMgr
 * ----------------
 * The struct contains all the variables of the message module
 */
typedef struct
{
    RvLogMgr*				pLogMgr;
    RvLogSource*			pLogSrc;	/* The log registerNum. for printing to log file */
    RvRandomGenerator		*seed;

    SipParserMgrHandle		hParserMgr;

    RvSipTransportMgrHandle	hTransportMgr;

}MsgMgr;



/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
typedef enum {
    MSG_DATE_TIME_ZONE_UNDEFINED = -1,
    MSG_DATE_TIME_ZONE_GMT,
    MSG_DATE_TIME_ZONE_UTC,
    MSG_DATE_TIME_ZONE_GMST
} MsgDateTimeZone;


/*
 * MsgAddrUrlHandle
 * ----------------------
 * Handle of the url object
 */
RV_DECLARE_HANDLE(MsgAddrUrlHandle);


/*
 * SIP URLs are used within SIP message to represent SIP addresses.
 */
typedef struct
{
    RvInt32			strUser;         /* the user information , may be empty. if there is
                                        a password related to this user, it will be
                                        concatenated  with ":" between */
    RvInt32			strHost;         /* fully-qualified domain name or numeric
                                        IP address */
    RvInt32			portNum;         /* the port number to send a request to */

    RvSipTransport	eTransport;      /* determines the transport mechanism to be used
                                        for sending SIP requests and responses */
    RvInt32			strTransport;    /* the transport mechanism when eTransport is OTHER */

    RvInt32			strMaddrParam;   /* provides the server address to be contacted for
                                        this user. */
    RvSipUserParam  eUserParam;		 /* the user parameter "phone" indicates that the
                                        user part contains a telephone number. */
    RvInt32         strUserParam;    /* the user param, when eUserParam is OTHER */

    RvInt16         ttlNum;          /* the ttl parameter */
    RvSipUrlLrType  lrParamType;     /* indicates the lr parameter form (lr; lr=true; lr=1)*/
    RvSipMethodType eMethod;         /* the method param */
    RvInt32         strOtherMethod;  /* string method type */
    RvInt32         strUrlParams;    /* URL parameters concatenated into a string */
    RvInt32         strHeaders;		 /* URL headers concatenated into a string */
    RvBool          bIsSecure;       /* Indicates if the address is secure (e.g. sips) */
    /*RvSipUrlScheme	eUrlScheme;       indicate the scheme for the url i.e. "sip:", "sips:", etc. */
    RvInt32			strMessageTransport; /* holds the transport parameter the same way it was received in the message (only for transports that are
                                          part of the transport enum*/
    RvSipCompType	eComp;           /* The compression parameter type */
    RvInt32			strCompParam;    /* The compression parameter string */
	RvInt32         strSigCompIdParam;              /* The SigCompId string */
	RvInt32			strTokenizedByParam; /* tokenized-by parameter */
	RvBool			bOrigParam;		 /* orig parameter */
    RvBool          bOldAddrSpec;    /* Should we use old addr-spec defined in RFC 822 */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvSipUriCPCType eCPCType;        /* The cpc parameter */
    RvInt32         strCPC;          /* offset of cpc parameter string */

    RvBool          bGrParam;        /* The gr parameter for GRUU */
    RvInt32         strGrParam;      /* The gr parameter for GRUU */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef SIP_DEBUG
    RvChar			*pUser;
    RvChar			*pHost;
    RvChar			*pTransport;
    RvChar			*pMaddrParam;
    RvChar			*pUserParam;
    RvChar			*pUrlParams;
    RvChar			*pHeaders;
    RvChar			*pOtherMethod;
    RvChar			*pMessageTransport;
    RvChar			*pCompParam;
	RvChar          *pSigCompIdParam;
	RvChar			*pTokenizedByParam;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvChar          *pGrParam;
    RvChar          *pCPC;             /* Pointer to the cpc parameter string */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif /* SIP_DEBUG */

}MsgAddrUrl;


/*
 * Absolute URIs are used within SIP message to represent SIP addresses.
 */
typedef struct
{
    RvInt32          strScheme;         /* The scheme of the uri. for example: sip, http,
                                        tel */
    RvInt32        strIdentifier;   /* The address in a string format */

#ifdef SIP_DEBUG
    RvChar          *pScheme;
    RvChar           *pIdentifier;
#endif

}MsgAddrAbsUri;

/*
 * TEL URLIs are used within SIP message to represent telephone addresses.
 */
#ifdef RV_SIP_TEL_URI_SUPPORT
typedef struct
{
	RvInt32   strPhoneNumber;   /* offset of the number string */
    RvInt32   strExtension;     /* offset of the extension string */
	RvInt32   strIsdnSubAddr;   /* offset of the isdn-subaddress string */
	RvInt32   strPostDial;      /* offset of the post-dial string */
	RvInt32   strContext;       /* offset of the phone-context parameter string */
	RvInt32   strOtherParams;   /* offset of all general parameters string */

	RvBool    bIsGlobalPhoneNumber; /* boolean indicating if the phone number is local or global */

	RvSipTelUriEnumdiType eEnumdiType; /* An indication of the enumdi parameter */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvSipUriCPCType    eCPCType; /* The cpc parameter */
    RvInt32   strCPC;            /* offset of cpc parameter string */

    RvInt32   strRn;             /* offset of rn parameter */
    RvInt32   strRnContext;      /* offset of rn-context parameter */

    RvInt32   strCic;            /* offset of cic parameter */
    RvInt32   strCicContext;     /* offset of cic-context parameter */

    RvBool    bNpdi;             /* boolean indicating if npdi parameter exists */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef SIP_DEBUG
	RvChar*   pPhoneNumber;     /* Pointer to the number string */
    RvChar*   pExtension;       /* Pointer to the extension string */
	RvChar*   pIsdnSubAddr;     /* Pointer to the isdn-subaddress string */
	RvChar*   pPostDial;        /* Pointer to the post-dial string */
	RvChar*   pContext;         /* Pointer to the phone-context parameter string */
	RvChar*   pOtherParams;     /* Pointer to all general parameters string */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    RvChar*   pCPC;             /* Pointer to the cpc parameter string */
    RvChar*   pRn;              /* Pointer to the rn parameter string */
    RvChar*   pRnContext;       /* Pointer to the rn-context parameter string */
    RvChar*   pCic;             /* Pointer to the cic parameter string */
    RvChar*   pCicContext;      /* Pointer to the cic-context parameter string */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif /* SIP_DEBUG */

}MsgAddrTelUri;
#endif /* RV_SIP_TEL_URI_SUPPORT */

/*
 * DIAMETER URIs are used within SIP Authentication, Authorization and 
   Accounting (AAA) framework for applications such as network access or IP mobility.
 */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
typedef struct
{
    RvInt32			strHost;         /* Fully-qualified domain name or numeric
                                        IP address */
    RvInt32			portNum;         /* the port number to send a request to */

    RvSipTransport	eTransport;      /* Determines the transport mechanism to be used */
    RvInt32			strTransport;    /* The transport mechanism when eTransport is OTHER */
	
    RvSipDiameterProtocol eProtocol; /* Determines the protocol to be used */
    
    RvBool          bIsSecure;       /* Indicates if the address is secure (i.e. aaas://) */
    RvInt32         strOtherParams;  /* Offset of all general parameters string */
    
#ifdef SIP_DEBUG
    RvChar			*pUser;
    RvChar			*pHost;
    RvChar			*pTransport;
	RvChar          *pOtherParams;   /* Pointer to all general parameters string */
#endif /* SIP_DEBUG */

}MsgAddrDiameterUri;
#endif /* RV_SIP_IMS_HEADER_SUPPORT */

typedef union
{
    MsgAddrUrlHandle    hUrl;
    MsgAddrAbsUri       *pAbsUri;

#ifdef RV_SIP_TEL_URI_SUPPORT
	MsgAddrTelUri       *pTelUri;
#endif /* RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    MsgAddrDiameterUri  *pDiameterUri;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
}MsgAddressDetails;

/*
 * RvSipAddress is a struct that rap the url struct, or other address object that
 * will be supported in the future.
 */
typedef struct
{
    RvSipAddressType  eAddrType;
    MsgAddressDetails uAddrBody;

#ifdef RV_SIP_JSR32_SUPPORT
	RvInt32           strDisplayName;

#ifdef SIP_DEBUG
	RvChar           *pDisplayName;
#endif
#endif

     /* page and rpool for allocation */
    MsgMgr*         pMsgMgr; /* message manager for logId etc. */
    HPAGE           hPage;
    HRPOOL          hPool;

}MsgAddress;





#ifndef RV_SIP_PRIMITIVES

/*
 * MsgAllowHeader object purpose is to inform the recipient of valid methods
 * associated with the resource identified by the request-URI.
 * It is compose of enumerate variable (RvSipMethodType) which define the method type
 * and text string which holds a method type that is not one of the enumerate values
 */

typedef struct
{
    RvSipHeaderType eHeaderType;  /* type of the header (Allow) */
    RvSipMethodType    eMethod;      /* method type */
    RvInt32        strMethod;    /* text string method type if its not
                                     one of the enumerate values */

    MsgMgr*         pMsgMgr;      /* message manager for logId etc. */
    HPAGE           hPage;        /* page for allocation */
    HRPOOL          hPool;
    RvInt32        strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar        *pMethod;
    RvChar        *pBadSyntax;
#endif

}MsgAllowHeader;

#endif /*RV_SIP_PRIMITIVES */

/*
 * MsgDate is used as a part of the Expires header or expires parameter
 * of the Contact header. This struct implement the SIP-Date structure
 * as defined in the protocol
 */
typedef struct
{
    RvSipHeaderType       eHeaderType;    /* type of the header (Date) */
    RvSipDateWeekDay      eWeekDay; /* The enumeration of the week day */
    RvInt8               day;      /* The day number within a month (for
                                       example 20 in "20 Jan 2000") */
    RvSipDateMonth        eMonth;   /* The enumeration of the month */
    RvInt16              year;     /* Year */
    RvInt8               hour;     /* Hour */
    RvInt8               minute;   /* Minute */
    RvInt8               second;   /* Second */
    MsgDateTimeZone       zone;     /* Zone   */

    MsgMgr*               pMsgMgr;      /* message manager for logId etc. */
    HPAGE                 hPage;        /* Page for memory allocation */
    HRPOOL                hPool;        /* Memory pool */
    RvInt32              strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*              pBadSyntax;
#endif

}MsgDateHeader;

/*
 * MsgInterval
 * -----------
 * Internal structure - in use from Expires and Retry-After headers
 */
typedef struct
{
    RvSipExpiresFormat    eFormat;      /* The Interval format:
                                           Date or Delta-seconds */
    RvSipDateHeaderHandle hDate;         /* A date structure */
    RvUint32             deltaSeconds; /* Delta-seconds */

    MsgMgr*               pMsgMgr;      /* message manager for logId etc. */
    HPAGE                 hPage;        /* Page for memory allocation */
    HRPOOL                hPool;        /* Memory pool */
}MsgInterval;
/*
 * MsgExpires object is used to create an Expires header. This header
 * gives the date and time after which the message content expires
 */
typedef struct
{
    RvSipHeaderType       eHeaderType;  /* type of the header (Expires) */
    MsgInterval*          pInterval;

    MsgMgr*               pMsgMgr;      /* message manager for logId etc. */
    HPAGE                 hPage;        /* Page for memory allocation */
    HRPOOL                hPool;        /* Memory pool */
    RvInt32              strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*              pBadSyntax;
#endif

}MsgExpiresHeader;

/*
 * MsgContactHeader object can appear in INVITE, OPTION, ACK and REGISTER requests,
 * and in 1XX, 2XX, 3XX and 485 responses. In general, it provides a URL where the user
 * can be reached for further communications.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;       /* type of the header (Contact) */
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32                 strDisplayName;     /* the name of the person to
                                                   contact with*/
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvSipAddressHandle         hAddrSpec;       /* url address */
    RvInt32                    strContactParams;/* Contact parameters concatenated
                                                   into a string, with ';' between
                                                   the different parameters */
    RvSipExpiresHeaderHandle   hExpiresParam;   /* The expires parameter */
    RvInt32                    strQVal;         /* The 'q' parameter */
    RvSipContactAction         eAction;         /* The action parameter (proxy / redirect) */
    RvBool                     bIsStar;         /* If true the contact header is
                                                   a star : Meaning - " Contact: * "    */

#ifdef RV_SIP_IMS_HEADER_SUPPORT 
    RvInt32                    strTempGruu;     /* The temp-gruu parameter */
    RvInt32                    strPubGruu;      /* The pub-gruu parameter */
    RvInt32			           regIDNum;        /* The reg-id parameter */

    /* The next 40 parameters belong to the Feature Tag Parameter */
    RvBool                     bIsAudio;        /* Should the Audio parameter appear */
    RvInt32                    strAudio;        /* The Audio parameter */
    RvBool                     bIsAutomata;     /* Should the Automata parameter appear */
    RvInt32                    strAutomata;     /* The Automata parameter */
    RvBool                     bIsClass;        /* Should the Class parameter appear */
    RvInt32                    strClass;        /* The Class parameter */
    RvBool                     bIsDuplex;       /* Should the Duplex parameter appear */
    RvInt32                    strDuplex;       /* The Duplex parameter */
    RvBool                     bIsData;         /* Should the Data parameter appear */
    RvInt32                    strData;         /* The Data parameter */
    RvBool                     bIsControl;      /* Should the Control parameter appear */
    RvInt32                    strControl;      /* The Control parameter */
    RvBool                     bIsMobility;     /* Should the Mobility parameter appear */
    RvInt32                    strMobility;     /* The Mobility parameter */
    RvBool                     bIsDescription;  /* Should the Description parameter appear */
    RvInt32                    strDescription;  /* The Description parameter */
    RvBool                     bIsEvents;       /* Should the Events parameter appear */
    RvInt32                    strEvents;       /* The Events parameter */
    RvBool                     bIsPriority;     /* Should the Priority parameter appear */
    RvInt32                    strPriority;     /* The Priority parameter */
    RvBool                     bIsMethods;      /* Should the Methods parameter appear */
    RvInt32                    strMethods;      /* The Methods parameter */
    RvBool                     bIsSchemes;      /* Should the Schemes parameter appear */
    RvInt32                    strSchemes;      /* The Schemes parameter */
    RvBool                     bIsApplication;  /* Should the Application parameter appear */
    RvInt32                    strApplication;  /* The Application parameter */
    RvBool                     bIsVideo;        /* Should the Video parameter appear */
    RvInt32                    strVideo;        /* The Video parameter */
    RvBool                     bIsLanguage;     /* Should the Language parameter appear */
    RvInt32                    strLanguage;     /* The Language parameter */
    RvBool                     bIsType;         /* Should the Type parameter appear */
    RvInt32                    strType;         /* The Type parameter */
    RvBool                     bIsIsFocus;      /* Should the IsFocus parameter appear */
    RvInt32                    strIsFocus;      /* The IsFocus parameter */
    RvBool                     bIsActor;        /* Should the Actor parameter appear */
    RvInt32                    strActor;        /* The Actor parameter */
    RvBool                     bIsText;         /* Should the Text parameter appear */
    RvInt32                    strText;         /* The Text parameter */
    RvBool                     bIsExtensions;   /* Should the Extensions parameter appear */
    RvInt32                    strExtensions;   /* The Extensions parameter */

    RvBool                     bIsSipInstance;  /* Should the SipInstance parameter appear */
    RvInt32                    strSipInstance;  /* The SipInstance parameter */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    
	MsgMgr*                    pMsgMgr;         /* message manager for logId etc. */
    HPAGE                      hPage;           /* page for allocation */
    HRPOOL                     hPool;
    RvBool                     bIsCompact;      /* indicates whether to use a compact form when encoding */
    RvInt32                    strBadSyntax;    /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
    RvChar                    *pDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvChar                    *pContactParams;
    RvChar                    *pQVal;

#ifdef RV_SIP_IMS_HEADER_SUPPORT 
    RvChar                    *pTempGruu;
    RvChar                    *pPubGruu;

    RvChar                    *pAudio;
    RvChar                    *pAutomata;
    RvChar                    *pClass;
    RvChar                    *pDuplex;
    RvChar                    *pData;
    RvChar                    *pControl;
    RvChar                    *pMobility;
    RvChar                    *pDescription;
    RvChar                    *pEvents;
    RvChar                    *pPriority;
    RvChar                    *pMethods;
    RvChar                    *pSchemes;
    RvChar                    *pApplication;
    RvChar                    *pVideo;
    RvChar                    *pLanguage;
    RvChar                    *pType;
    RvChar                    *pIsFocus;
    RvChar                    *pActor;
    RvChar                    *pText;
    RvChar                    *pExtensions;

    RvChar                    *pSipInstance;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    
    RvChar                    *pBadSyntax;
#endif

}MsgContactHeader;

/*
 * MsgCSeqHeader is used when clients send a request message, they must add CSeq
 * (command sequence) to the general-header field.
 * The header field in a request contains the request method and a single decimal
 * sequence number chosen by the requesting client, unique within a single call leg.
 * The ACK and CANCEL requests must contain the same CSeq value as the INVITE request
 * that it refers to, while a BYE request canceling an invitation must have a higher
 * sequence number.
 */

typedef struct
{
    RvSipHeaderType     eHeaderType;  /* type of the header (CSeq) */
	SipCSeq				step;		  /* single decimal sequence number unique
                                         within a single call.*/
    RvSipMethodType        eMethod;         /* method type */
    RvInt32            strOtherMethod;  /* string method type */

    MsgMgr*             pMsgMgr;         /* message manager for logId etc. */
    HPAGE               hPage;           /* page for allocation */
    HRPOOL              hPool;
    RvInt32            strBadSyntax;    /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar             *pOtherMethod;
    RvChar             *pBadSyntax;
#endif

}MsgCSeqHeader;

/*
 * MsgOtherHeader is used to define headers that have no specific header
 * object.
 * It contains the header name and header value.
 */
typedef struct
{
    RvSipHeaderType eHeaderType;    /* type of the header(Other) */
     RvInt32         strHeaderName;
     RvInt32        strHeaderVal;

     MsgMgr*        pMsgMgr;        /* message manager for logId etc. */
     HPAGE          hPage;          /* page for allocation */
     HRPOOL         hPool;
     RvInt32       strBadSyntax;    /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
     RvChar*        pHeaderName;
     RvChar*        pHeaderVal;
     RvChar*        pBadSyntax;
#endif

}MsgOtherHeader;

/*
 * This object is used to create and store 'From' header, indicating the initiator
 * of the request or 'To' header, indicating the desired recipient of the request.
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;     /* type of the header (To/From) */
    RvSipAddressHandle    hAddrSpec;     /* SIP-URL address */
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32            strDisplayName;   /* optional field rendered by a human-user
                                            interface */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvInt32            strTag;           /* this field must be present when it is
                                            possible that two instances of the user
                                            sharing a SIP address can make call
                                            invitations with the same CALL-ID */
#ifdef RFC_2543_COMPLIANT
    RvBool          bEmptyTag;           /* stack must support the obsolete RFC, and treat a response
                                            without a tag, as if it has an empty tag. this influence the
                                            ability to match requests to the call-leg with the empty tag... */
#endif /*#ifdef RFC_2543_COMPLIANT*/
    RvInt32            strOtherParams;   /* text string of all generic parameters.
                                            this is a list of parameters with ';'
                                            between them. */

    MsgMgr*             pMsgMgr;         /* message manager for logId etc. */
    HPAGE               hPage;           /* page for allocation */
    HRPOOL              hPool;
    RvBool             bIsCompact;       /*indicates whether to use a compact form when encoding*/
    RvInt32            strBadSyntax;     /* header value in string, in case of bad-syntax */
    

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
    RvChar            *pDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvChar            *pTag;
    RvChar            *pOtherParams;
    RvChar            *pBadSyntax;
#endif

}MsgPartyHeader;


#ifndef RV_SIP_PRIMITIVES

/*
 * This object is used to create and store 'Replaces' header
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;    /* type of the header (Replaces) */
    MsgMgr*             pMsgMgr;        /* message manager for logId etc. */
    RvInt32            strFromTag;
    RvInt32            strToTag;
    RvInt32            strCallID;
    RvInt32            strOtherParams;  /* text string of all generic parameters.
                                            this is a list of parameters with ';'
                                            between them. */

    RvSipReplacesEarlyFlagType  eEarlyFlagParamType; /* indicates the early-only parameter form
                                                      (early-only; early-onlylr=true; early-only=1)*/

    HPAGE               hPage;           /* page for allocation */
    HRPOOL              hPool;
    RvInt32            strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar            *pFromTag;
    RvChar            *pToTag;
    RvChar            *pCallID;
    RvChar            *pOtherParams;
    RvChar            *pBadSyntax;
#endif

}MsgReplacesHeader;

/*
 * MsgRSeqHeader is used when a reliable provisional response is sent
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;  /* type of the header (RSeq) */
	SipRSeq				respNum;

    MsgMgr*             pMsgMgr;      /* message manager for logId etc. */
    HPAGE               hPage;        /* page for allocation */
    HRPOOL              hPool;
    RvInt32            strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*            pBadSyntax;
#endif

}MsgRSeqHeader;


/*
 * MsgRAckHeader is used when a PRACK request is sent
 */
typedef struct
{
    RvSipHeaderType         eHeaderType;  /* type of the header (RAck) */
    SipRSeq					rseq;
    RvSipCSeqHeaderHandle   hCSeq;

    MsgMgr*                 pMsgMgr;      /* message manager for logId etc. */
    HPAGE                   hPage;        /* page for allocation */
    HRPOOL                  hPool;
    RvInt32                strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*                pBadSyntax;
#endif

}MsgRAckHeader;

/*
 * MsgRetryAfterHeader
 */
typedef struct
{
    RvSipHeaderType eHeaderType;  /* type of the header (Retry After) */
    RvInt32        strComment;
    RvInt32        duration;
    MsgInterval*    pInterval;
    RvInt32        strRetryAfterParams;

    MsgMgr*         pMsgMgr;      /* message manager for logId etc. */
    HPAGE           hPage;        /* Page for memory allocation */
    HRPOOL          hPool;        /* Memory pool */
    RvInt32        strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*        pComment;
    RvChar*        pRetryAfterParams;
    RvChar*        pBadSyntax;
#endif

}MsgRetryAfterHeader;

#endif /*#ifndef RV_SIP_PRIMITIVES*/

/*
 * MsgViaHeader object indicates the path taken by the request so far.
 * This prevents request looping and ensures replies take the same path as the
 * requests, which assists in firewall traversal and other unusual routing situation.
 */
typedef struct
{
    RvSipHeaderType eHeaderType;        /* type of the header (Via) */
    RvSipTransport  eTransport;         /* transport protocol */
    RvInt32         strTransport;       /* the transport mechanism when eTransport is OTHER */
    RvInt16         ttlNum;             /* The ttl value (UNDEFINED is possible)*/
    RvInt32         strHost;            /* fully-qualified domain name or numeric IP address */
    RvInt32         portNum;            /* port number  */
    RvInt32         strMaddrParam;      /* multi-cast address, under exceptional
                                           circumstances it may contain a uni-cast address */
    RvInt32         strReceivedParam;   /* Via headers can not be relied on the route
                                           replies, that why a proxy should check the top
                                           most Via headers field to ensure that it
                                           contains the sender correct network address.
                                           if the sender's address is incorrect, the
                                           proxy must add a "received" parameter to the
                                           Via header inserted by the previous hop. */
    RvInt32         rportParam;         /* Optional information to be filled by the Server
                                           side after receiving a request for returning
                                           responses to the source port. in case of
                                           empty rport parameter it's value is UNDEFINED.*/
    RvBool          bRportExists;       /* Does rport parameter appear in Via header */
    RvInt32         strBranchParam;     /* Branch parameter included by every proxy.
                                           it must be unique for each distinct request */
    RvSipCompType   eComp;              /* The compression type in the 'comp' parameter*/
    RvInt32         strCompParam;       /* Comp parameter that indicates the compression
                                           type the source endpoint supports */
	RvInt32         strSigCompIdParam;  /* The SigCompId string */
    RvBool          bIsHidden;          /* Indicates weather the Via should add ";hidden" to the
                                           via-params. */
    RvBool          bAlias;             /* Indicates weather the Via should add ";alias" to the
                                           via-params. */
    RvInt32         strViaParams;       /* Via parameters concatenated into a string */

    MsgMgr*         pMsgMgr;            /* message manager for logId etc. */

    /* page for allocation */
    HPAGE           hPage;
    HRPOOL          hPool;
    RvBool         bIsCompact;            /*indicates whether to use a compact form when encoding*/
    RvInt32        strBadSyntax;       /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar        *pTransport;
    RvChar        *pHost;
    RvChar        *pMaddrParam;
    RvChar        *pReceivedParam;
    RvChar        *pBranchParam;
    RvChar        *pViaParams;
    RvChar        *pBadSyntax;
    RvChar        *pCompParam;
	RvChar        *pSigCompIdParam;
#endif /* SIP_DEBUG */

}MsgViaHeader;

#ifdef RV_SIP_AUTH_ON
/*
 * This object is used to create and store 'WWW-Authenticate' header or
   'Proxy-Authenticate' header.
 */
typedef struct
{
    RvSipHeaderType					eHeaderType;     /* type of the header (Authentication) */
    RvSipAuthScheme					eAuthScheme;     /* authentication scheme */
    RvInt32							strAuthScheme;   /* authentication scheme string */
    RvInt32							strRealm;        /* this field must be present */
    RvInt32							strDomain;       /* optional field given by the server */
    RvInt32							strNonce;        /* this field must be present */
    RvInt32							strOpaque;       /* optional field given by the server */
    RvSipAuthStale					eStale;          /* optional field given by the server */
    RvSipAuthAlgorithm				eAlgorithm;      /* optional field given by the server */
    RvInt32							strAlgorithm;    /* algorithm string */
    RvInt32							strQop;          /* optional field given by the server */
    RvSipAuthQopOption				eAuthQop;
    RvInt32							strAuthParam;    /* optional field given by the server */
    RvInt32							strUserName;     /* saving username for the next time */
    RvInt32							strPassword;     /* saving password for the next time */
	RvInt32							nAKAVersion;	 /* AKAv specified in algorithm parameter */
	RvInt32							strIntegrityKey; /* optional field */
	RvInt32							strCipherKey;	 /* optional field */

    MsgMgr*							pMsgMgr;         /* message manager for logId etc. */
    HPAGE							hPage;           /* page for allocation */
    HRPOOL							hPool;
    RvSipAuthenticationHeaderType	eAuthHeaderType; /* WWW-Authenticate or PROXY-Authenticate */
    RvInt32							strBadSyntax;    /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar						   *pAuthScheme;
    RvChar						   *pRealm;
    RvChar						   *pDomain;
    RvChar                         *pNonce;
    RvChar                         *pOpaque;
    RvChar						   *pAlgorithm;
    RvChar						   *pQop;
    RvChar						   *pAuthParam;
    RvChar						   *pUserName;
    RvChar						   *pPassword;
    RvChar						   *pIntegrityKey;
    RvChar						   *pCipherKey;
    RvChar						   *pBadSyntax;
#endif

}MsgAuthenticationHeader;

/*
 * This object is used to create and store 'Authorization' header or
   'Proxy-Authorization' header.
 */
typedef struct
{
    RvSipHeaderType					eHeaderType;	/* type of the header (Authorization) */
    RvSipAuthScheme					eAuthScheme;	/* authentication scheme */
    RvInt32							strAuthScheme;	/* authentication scheme string */
    RvInt32							strUserName;	/* this field must be present */
    RvInt32							strRealm;		/* this field must be present */
    RvInt32							strNonce;		/* this field must be present */
    RvSipAddressHandle				hDigestUri;		/* SIP-URL address, must be present */
    RvInt32							strResponse;	/* this field must be present */
    RvSipAuthAlgorithm				eAlgorithm;		/* optional field given by the server */
    RvInt32							strAlgorithm;	/* algorithm string */
    RvInt32							strCnonce;		/* optional field given by the server */
    RvInt32							strOpaque;		/* optional field given by the server */
    RvInt32							nonceCount;		/* optional field given by the server */
    RvSipAuthQopOption				eAuthQop;		/* optional field */
    RvInt32							strAuthParam;	/* optional field given by the server */
	RvInt32							strAuts;		/* optional field given by the server */
	RvSipAuthIntegrityProtected		eProtected;		/* optional field */
	RvInt32							nAKAVersion;	/* AKAv specified in algorithm parameter */

    MsgMgr*							pMsgMgr;		/* message manager for logId etc. */
    HPAGE							hPage;			/* page for allocation */
    HRPOOL							hPool;
    RvSipAuthorizationHeaderType	eAuthHeaderType;/* Authorization or Proxy-Authorization */
    RvInt32							strBadSyntax;	/* header value in string, in case of bad-syntax */
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    /*patch - save the uri string as is, in order to use it in the MD5 calculation.
        (come to solve the problem that we received 
        "uri="sip:3002@10.17.129.50:5060;user=phone;transport=tcp", and during MD5
        because we use address encode we use 
        "sip:3002@10.17.129.50:5060;transport=tcp;user=phone" */    
    RvInt32                     strOriginalUri;
/*#endif*/ /*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
	
#ifdef SIP_DEBUG
    RvChar						   *pAuthScheme;
    RvChar                         *pUserName;
    RvChar                         *pRealm;
    RvChar                         *pNonce;
    RvChar                         *pResponse;
    RvChar                         *pAlgorithm;
    RvChar                         *pCnonce;
    RvChar                         *pOpaque;
    RvChar                         *pAuthParam;
	RvChar                         *pAuts;
    RvChar                         *pBadSyntax;
#endif

}MsgAuthorizationHeader;
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
/*
 * MsgContentTypeHeader object indicates the type of the message body.
 */
typedef struct {
    RvSipHeaderType   eHeaderType; /* the type of the header (Content Type)*/
    RvSipMediaType    eMediaType;
    RvSipMediaSubType eMediaSubType;
    RvInt32          strMediaType;
    RvInt32          strMediaSubType;
    RvInt32          strBoundary;
    RvInt32          strVersion;
    RvInt32          strBase;
    RvInt32          strOtherParams;

    MsgMgr*           pMsgMgr;      /* message manager for logId etc. */
    HPAGE             hPage;
    HRPOOL            hPool;
    RvBool           bIsCompact;   /*indicates whether to use a compact form when encoding*/
    RvInt32          strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar          *pMediaType;
    RvChar          *pMediaSubType;
    RvChar          *pBoundary;
    RvChar          *pVersion;
    RvChar          *pBase;
    RvChar          *pOtherParams;
    RvChar          *pBadSyntax;
	RvChar          *pTypeParamMediaType;
    RvChar          *pTypeParamMediaSubType;
#endif
	/* multipart/related parameters */ 
	RvSipMediaType		eTypeParamMediaType;
    RvSipMediaSubType	eTypeParamMediaSubType;
    RvInt32				strTypeParamMediaType;
    RvInt32				strTypeParamMediaSubType;
	RvSipAddressHandle  hStartAddrSpec;     /* addr spec */
} MsgContentTypeHeader;

/*
 * MsgContentDispositionHeader object indicates the disposition type of
 * the message body.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;   /* type of the header (Content Disposition) */
    RvSipDispositionType     eType;
    RvSipDispositionHandling eHandling;
    RvInt32                 strType;
    RvInt32                 strHandling;
    RvInt32                 strOtherParams;

    MsgMgr*                  pMsgMgr;       /* message manager for logId etc. */
    HPAGE                    hPage;
    HRPOOL                   hPool;
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                 *pType;
    RvChar                 *pHandling;
    RvChar                 *pOtherParams;
    RvChar                 *pBadSyntax;
#endif

} MsgContentDispositionHeader;

/*
 * MsgContentIDHeader object indicates the ID of the message body.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (ContentID) */
    RvSipAddressHandle       hAddrSpec;			/* url address */

    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pBadSyntax;
#endif
}MsgContentIDHeader;

/*
 * Body object holds the message body and the Content-Type header of
 * a message or a body part.
 */
typedef struct {
    MsgContentTypeHeader *pContentType;
    MsgContentIDHeader   *pContentID;
    RvInt32              strBody;
    RvUint32             length;
    RLIST_HANDLE          hBodyPartsList;

    MsgMgr*               pMsgMgr;  /* message manager for logId etc. */
    HPAGE                 hPage;
    HRPOOL                hPool;

#ifdef SIP_DEBUG
    RvChar              *pBody;
#endif

} Body;

typedef struct {
    Body                        *pBody;
    MsgContentDispositionHeader *pContentDisposition;
    RLIST_HANDLE                 hHeadersList;

    MsgMgr*                      pMsgMgr;  /* message manager for logId etc. */
    HPAGE                        hPage;
    HRPOOL                       hPool;

} BodyPart;

#endif /* RV_SIP_PRIMITIVES */


/*
 * This object is used to create and store Route or Recorde-Route headers,
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;      /* type of the header (Route Hop) */
    RvSipAddressHandle    hAddrSpec;          /* SIP-URL address */
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32            strDisplayName;   /* optional field rendered by a human-user
                                               interface */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvInt32            strOtherParams;   /* text string of all generic parameters.
                                             this is a list of parameters with ';'
                                             between them. */
    RvSipRouteHopHeaderType headerType;   /* Route or Record Route */

    MsgMgr*             pMsgMgr;          /* message manager for logId etc. */
    HPAGE               hPage;            /* page for allocation */
    HRPOOL              hPool;            /* pool the page belongs to */
    RvInt32            strBadSyntax;     /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
    RvChar            *pDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvChar            *pOtherParams;
    RvChar            *pBadSyntax;
#endif

}MsgRouteHopHeader;

#ifndef RV_SIP_PRIMITIVES

#ifdef RV_SIP_SUBS_ON

/*
 * This object is used to create and store 'Reffered-By' header, indicating the
 * initiator of the REFER request.
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;            /* type of the header (Referred By) */
    RvSipAddressHandle    hReferrerAddrSpec;      /* referrer-url: SIP-URL address */
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32            strReferrerDisplayName; /* optional field rendered by
                                                   a human-user interface */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvInt32            strCidParam;            /* the cid param */
    RvInt32            strOtherParams;         /* the general parameters string */

    MsgMgr*             pMsgMgr;      /* message manager for logId etc. */
    /* page for allocation */
    HPAGE               hPage;
    HRPOOL              hPool;
    RvBool             bIsCompact;   /*indicates whether to use a compact form when encoding*/
    RvInt32            strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
    RvChar            *pReferrerDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvChar            *pCidParam;
    RvChar            *pOtherParams;
    RvChar            *pBadSyntax;
#endif

}MsgReferredByHeader;

/*
 * This object is used to create and store 'Refer-To' header, used in REFER requests
 * to indicate the URL to initiate a call with.
 */
typedef struct
{
    RvSipHeaderType     eHeaderType;        /* type of the header (Refer To) */
    RvSipAddressHandle    hAddrSpec;            /* SIP-URL address */
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32            strDisplayName;     /* the name of the person to refer to */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvInt32            strOtherParams;     /* the generic params string */
    RvSipReplacesHeaderHandle hReplacesHeader; /* handle to the replaces header
                                            that is in the Refer-To header, could be NULL */

    MsgMgr*             pMsgMgr;  /* message manager for logId etc. */
    /* page for allocation */
    HPAGE               hPage;
    HRPOOL              hPool;
    RvBool             bIsCompact;  /*indicates whether to use a compact form when encoding*/
    RvInt32            strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
    RvChar            *pDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    RvChar            *pOtherParams;
    RvChar            *pBadSyntax;
#endif

}MsgReferToHeader;

/*
 * MsgAllowEventsHeader object purpose is to inform the recipient of valid events
 * associated with the resource identified by the request-URI.
 * It is compose of enumerate variable (RvSipMethodType) which define the method type
 * and text string which holds a method type that is not one of the enumerate values
 */

typedef struct
{
    RvSipHeaderType eHeaderType;  /* type of the header (AllowEvents) */
    RvInt32        strEventPackage;
    RvInt32        strEventTemplate;

    MsgMgr*         pMsgMgr;      /* message manager for logId etc. */
    HPAGE           hPage;        /* page for allocation */
    HRPOOL          hPool;
    RvBool         bIsCompact;   /*indicates whether to use a compact form when encoding*/
    RvInt32        strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*        pEventPackage;
    RvChar*        pEventTemplate;
    RvChar*        pBadSyntax;
#endif

}MsgAllowEventsHeader;

/*
 * MsgSubscriptionStateHeader object
 */

typedef struct
{
    RvSipHeaderType           eHeaderType;  /* type of the header (SubscriptionState) */
    RvSipSubscriptionSubstate eSubstate;
    RvInt32                  strSubstate;
    RvSipSubscriptionReason   eReason;
    RvInt32                  strReason;
    RvInt32                  expiresVal;
    RvInt32                  retryAfterVal;
    RvInt32                  strOtherParams;

    MsgMgr*                   pMsgMgr;      /* message manager for logId etc. */
    HPAGE                     hPage;        /* page for allocation */
    HRPOOL                    hPool;
    RvInt32                  strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                 *pSubstate;
    RvChar                 *pReason;
    RvChar                 *pOtherParams;
    RvChar                 *pBadSyntax;
#endif

}MsgSubscriptionStateHeader;
#endif /* #ifdef RV_SIP_SUBS_ON */

/*
 * MsgEventHeader object
 */

typedef struct
{
    RvSipHeaderType eHeaderType;  /* type of the header (Event) */
    RvInt32        strEventId;   /* text string event type. */
    RvInt32        strEventPackage;
    RvInt32        strEventTemplate;
    RvInt32        strEventParams;

    MsgMgr*         pMsgMgr;      /* message manager for logId etc. */
    HPAGE           hPage;        /* page for allocation */
    HRPOOL          hPool;
    RvBool         bIsCompact;   /*indicates whether to use a compact form when encoding*/
    RvInt32        strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*        pEventId;
    RvChar*        pEventPackage;
    RvChar*        pEventTemplate;
    RvChar*        pEventParams;
    RvChar*        pBadSyntax;
#endif

}MsgEventHeader;

/*
    MsgSessionExpiresHeader object
*/

typedef struct
{
    RvSipHeaderType       eHeaderType; /* type of the header (Session Expires) */
    RvInt32             deltaSeconds; /* Delta-seconds */
    RvSipSessionExpiresRefresherType eRefresherType; /* defines who will be the refresher usc/uas */
    RvInt32            strParams;    /* Contact parameters concatenated
                                         into a string, with ';' between
                                         the different parameters */
    RvBool             bIsCompact;   /* indicates whether to use a compact form when encoding*/

    MsgMgr*             pMsgMgr;      /* message manager for logId etc. */
    HPAGE               hPage;        /* Page for memory allocation */
    HRPOOL              hPool;        /* Memory pool */
    RvInt32            strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*            pBadSyntax;
#endif


}MsgSessionExpiresHeader;

/*
    MsgMinSEHeader object
*/

typedef struct
{
    RvSipHeaderType       eHeaderType;  /* type of the header (Session Expires) */
    RvInt32              deltaSeconds; /* Delta-seconds */
    RvInt32              strParams;    /* Contact parameters concatenated
                                          into a string, with ';' between
                                          the different parameters */

    MsgMgr*               pMsgMgr;      /* message manager for logId etc. */
    HPAGE                 hPage;        /* Page for memory allocation */
    HRPOOL                hPool;        /* Memory pool */
    RvInt32              strBadSyntax; /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*            pBadSyntax;
#endif

}MsgMinSEHeader;

#endif /*RV_SIP_PRIMITIVES*/


#ifdef RV_SIP_AUTH_ON
/*
 * This object is used to create and store 'Authentication-Info' header.
 */
typedef struct {
	RvSipHeaderType           eHeaderType;  /* type of the header (Authentication-Info) */

	RvInt32					  nonceCount;
	RvSipAuthQopOption        eMsgQop;
	RvInt32                   strMsgQop;
	RvInt32                   strNextNonce;
	RvInt32                   strCNonce;
	RvInt32                   strRspAuth;
	RvInt32                   strBadSyntax; /* header value in string, in case of bad-syntax */

	MsgMgr*                   pMsgMgr;      /* message manager for logId etc. */
    HPAGE                     hPage;        /* page for allocation */
    HRPOOL                    hPool;

#ifdef SIP_DEBUG
	RvChar                   *pMsgQop;
    RvChar                   *pNextNonce;
    RvChar                   *pCNonce;
    RvChar                   *pRspAuth;
	RvChar                   *pBadSyntax;
#endif

}MsgAuthenticationInfoHeader;
#endif /*RV_SIP_AUTH_ON*/

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

/*
    Reason-Header object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Reason) */

	RvSipReasonProtocolType eProtocol;     /* Protocol enumeration: SIP, Q850 or other */
	RvInt32                 strProtocol;   /* Protocol string */
	RvInt32                 strReasonText; /* The reason text parameter */
	RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	RvInt32                 protocolCause; /* The protocol cause parameter */

    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*                 pProtocol;
	RvChar*                 pReasonText;
	RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgReasonHeader;

/*
    Warning object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Warning) */

	RvInt16                 warnCode;          /* The 3-digit warning code */
	RvInt32                 strWarnAgent;      /* The warn-agent - hostport or token */
    RvInt32                 strWarnText;       /* The warn-text quoted string */

	MsgMgr*                 pMsgMgr;           /* message manager for logId etc. */
    HPAGE                   hPage;             /* Page for memory allocation */
    HRPOOL                  hPool;             /* Memory pool */
    RvInt32                 strBadSyntax;      /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*                 pWarnAgent;
	RvChar*                 pWarnText;
	RvChar*                 pBadSyntax;
#endif

}MsgWarningHeader;

/*
  Time stamp object
*/


typedef struct  
{
	RvInt32                 integerPart;  /* The integer part of the float value */
	RvInt32                 decimalPart;  /* The Decimal part of the float value*/
	
	
}timestampFloatVal;

typedef struct
{
    RvSipHeaderType         eHeaderType;       /* type of the header (Time stamp) */
	timestampFloatVal       timestampVal;
	timestampFloatVal       delayVal;

	MsgMgr*                 pMsgMgr;           /* message manager for logId etc */
    HPAGE                   hPage;             /* Page for memory allocation */
    HRPOOL                  hPool;             /* Memory pool */
    RvInt32                 strBadSyntax;      /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar*                 pBadSyntax;
#endif

}MsgTimestampHeader;

/*
    Info object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Info) */

	RvSipInfoHeaderType     eInfoType;     /* Alert-Info, Call-Info or Error-Info */
	
	RvSipAddressHandle      hAddrSpec;     /* The Addr-Spec of this Info header */
	RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	
    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgInfoHeader;

/*
    Accept-Encoding object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Accept-Encoding) */

	RvInt32                 strCoding;    /* The accepted codings (token / '*') */
	RvInt32                 strQVal;       /* The 'q' parameter */
    RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	
    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar*                 pCoding;
	RvChar*                 pQVal;
	RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgAcceptEncodingHeader;

/*
    Accept object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Accept) */

	RvSipMediaType          eMediaType;    /* The enumeration media type */
    RvSipMediaSubType       eMediaSubType; /* The enumeration media sub-type */
    RvInt32                 strMediaType;  /* The string media type */
    RvInt32                 strMediaSubType;/* The string media sub-type */
	
	RvInt32                 strQVal;       /* The 'q' parameter */
    RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	
    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar*                 pMediaType;
	RvChar*                 pMediaSubType;
	RvChar*                 pQVal;
	RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgAcceptHeader;

/*
    Accept-Language object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Accept-Language) */

	
	RvInt32                 strLanguage;   /* The accepted language (( 1*8ALPHA *( "-" 1*8ALPHA )) / '*') */
	RvInt32                 strQVal;       /* The 'q' parameter */
    RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	
    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar*                 pLanguage;
	RvChar*                 pQVal;
	RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgAcceptLanguageHeader;

/*
    Reply-To object
*/
typedef struct
{
    RvSipHeaderType         eHeaderType;   /* type of the header (Reply-To) */

	RvSipAddressHandle      hAddrSpec;     /* The Name-Addr/Addr-Spec of this Reply-To header */
	
#ifndef RV_SIP_JSR32_SUPPORT
    RvInt32                 strDisplayName;  /* optional field rendered by a human-user interface */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
	RvInt32                 strOtherParams;/* The string of all generic parameters.
											  this is a list of parameters with ';'
                                              between them */
	
    MsgMgr*                 pMsgMgr;       /* message manager for logId etc. */
    HPAGE                   hPage;         /* Page for memory allocation */
    HRPOOL                  hPool;         /* Memory pool */
    RvInt32                 strBadSyntax;  /* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
#ifndef RV_SIP_JSR32_SUPPORT
	RvChar*                 pDisplayName;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */	
    RvChar*                 pOtherParams;
	RvChar*                 pBadSyntax;
#endif

}MsgReplyToHeader;

/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*
 * MsgPUriHeader object.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (PUri) */
    RvInt32                  strDisplayName;	/* the Display name of the Address */
    RvSipAddressHandle       hAddrSpec;			/* url address */
    RvInt32                  strOtherParams;	/* Other parameters concatenated
												   into a string, with ';' between
                                                   the different parameters */
	RvSipPUriHeaderType		 ePHeaderType;		/* P-Header type:
												   - PAssociatedUri
												   - PCalledPartyID
												   - PAssertedIdentity
												   - PPreferredIdentity		*/
    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pDisplayName;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPUriHeader;

/*
 * MsgPVisitedNetworkIDHeader object.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (PVisitedNetworkID) */
    RvInt32                  strVNetworkSpec;	/* the Visited Network ID */
    RvInt32                  strOtherParams;	/* Other parameters concatenated
												   into a string, with ';' between
                                                   the different parameters */
    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pVNetworkSpec;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPVisitedNetworkIDHeader;

/*
 * MsgPAccessNetworkInfoHeader object.
 */
typedef struct
{
    RvSipHeaderType						eHeaderType;	    /* type of the header (PAccessNetworkInfo) */
	RvSipPAccessNetworkInfoAccessType	eAccessType;	    /* enum type of access type */
    RvInt32								strAccessType;	    /* the Stringed Access Type */
	RvInt32								strCgi3gpp;	        /* param for cgi-3gpp */
	RvInt32								strUtranCellId3gpp; /* param for UtranCellId3gpp */
	RvInt32								strIWlanNodeID;     /* param for IWlanNodeID */
	RvInt32								strDslLocation;		/* param for dsl-location */
	RvInt32								strCi3gpp2;		    /* param for ci-3gpp2 */
	RvBool					            bNetworkProvided;	/* param for network-provided */

	RvInt32								strOtherParams;	    /* Other parameters concatenated
															   into a string, with ';' between
															   the different parameters */
	
    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pAccessType;
	RvChar					*pCgi3gpp;
	RvChar					*pUtranCellId3gpp;
	RvChar					*pIWlanNodeID;
	RvChar					*pDslLocation;	
	RvChar					*pCi3gpp2;		
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPAccessNetworkInfoHeader;

/*
 * MsgPChargingFunctionAddressesListElem represents an element of the ccf/ecf params list
 * in the PChargingFunctionAddresses header. It contains: 
 *	- value		- for ccf/ecf param.
 */
typedef struct
{
	RvInt32		strValue;

	MsgMgr*		pMsgMgr;			/* message manager for logId etc. */
    HPAGE		hPage;				/* page for allocation */
    HRPOOL		hPool;

#ifdef SIP_DEBUG
    RvChar      *pValue;
#endif
}MsgPChargingFunctionAddressesListElem;
/*
 * MsgPChargingFunctionAddressesHeader object.
 */
typedef struct
{
    RvSipHeaderType			eHeaderType;	/* type of the header (PChargingFunctionAddresses) */
	RvSipCommonListHandle	ccfList;		/* List for ccf params */
	RvSipCommonListHandle	ecfList;		/* List for ecf params */
	RvInt32					strOtherParams;	/* Other parameters concatenated
												into a string, with ';' between
												the different parameters */

	MsgMgr*					pMsgMgr;		/* message manager for logId etc. */
	HPAGE					hPage;			/* page for allocation */
	HRPOOL					hPool;			
	RvInt32					strBadSyntax;	/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar					*pOtherParams;
	RvChar					*pBadSyntax;
#endif

}MsgPChargingFunctionAddressesHeader;

/*
 * MsgPChargingVectorInfoListElem represents an element of the pdp-info/dsl-bearer-info params list
 * in the PChargingVector header. It contains: 
 *	- nItem		- for pdp-item/dsl-bearer-item param.
 *	- bSig		- for pdp-sig/dsl-bearer-sig param.
 *	- strCid	- for gcid/dslcid param.
 *	- strFlowID	- flow-id param.
 */
typedef struct
{
	RvInt32		nItem;
	RvBool		bSig;
	RvInt32		strCid;
	RvInt32		strFlowID;

	MsgMgr*		pMsgMgr;			/* message manager for logId etc. */
    HPAGE		hPage;				/* page for allocation */
    HRPOOL		hPool;

#ifdef SIP_DEBUG
    RvChar      *pCid;
    RvChar      *pFlowID;
#endif
}MsgPChargingVectorInfoListElem;

/*
 * MsgPChargingVectorHeader object.
 */
typedef struct
{
    RvSipHeaderType			eHeaderType;		/* type of the header (PChargingVector) */
	RvInt32					strIcidValue;		/* param for icid-value */
	RvInt32					strIcidGenAddr;		/* param for icid-gen-addr */
	RvInt32					strOrigIoi;			/* param for orig-ioi */
	RvInt32					strTermIoi;			/* param for term-ioi */
	
	/* gprs-charging-info */
	RvInt32					strGgsn;			/* param for ggsn */
	RvInt32					strGprsAuthToken;	/* param for auth-token in gprs-charging-info */
	RvSipCommonListHandle	pdpInfoList;		/* list of pdp-info params */
	
	/* xdsl-charginf-info */
	RvInt32					strBras;			/* param for bras */
	RvInt32					strXdslAuthToken;	/* param for auth-token in xdsl-charging-info */
	RvSipCommonListHandle	dslBearerInfoList;	/* list of dsl-bearer-info params */

	/* i-wlan-charging-info */
	RvBool					bIWlanChargingInfo;	/* i-wlan-charging-info parameter ("pdg") */

	/* packetcable-charging-info */
	RvBool					bPacketcableChargingInfo;	/* packetcable-charging-info parameter ("packetcable-multimedia") */
	RvInt32					strBCid;			/* bcid parameter of packetcable-charging-info */

	RvInt32					strOtherParams;		/* Other parameters concatenated
													into a string, with ';' between
													the different parameters */

	MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar					*pIcidValue;
	RvChar					*pIcidGenAddr;
	RvChar					*pOrigIoi;
	RvChar					*pTermIoi;
	RvChar					*pGgsn;
	RvChar					*pGprsAuthToken;
	RvChar					*pBras;
	RvChar					*pXdslAuthToken;
	RvChar					*pBCid;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPChargingVectorHeader;

/*
 * Spi Value Object for Security Header
 */
typedef struct  
{
	RvUint32	value;			/* the value for the spi */
	RvBool		bInitialized;	/* was it initialized */
}SecuritySpiVal;

/*
 * MsgSecurityHeader object.
 */
typedef struct
{
	RvSipHeaderType						eHeaderType;		/* type of the header (PSecurity) */
	RvSipSecurityHeaderType				eSecurityHeaderType;/* Header type:
															   - SecurityClient
															   - SecurityServer
															   - SecurityVerify */
	RvSipSecurityMechanismType			eMechanismType;		/* Enumeration for the 
																security mechanism param */
	RvInt32								strMechanismType;	/* param for alternate mechanism types */
	RvInt32								strPreference;		/* param for preference */
	RvSipAuthAlgorithm					eDigestAlgorithm;	/* optional field given by the server */
	RvInt32								strDigestAlgorithm;	/* param for digest-algorithm */
	RvInt32								nAKAVersion;		/* AKAv specified in Digest algorithm parameter */
	RvSipAuthQopOption					eDigestQop;			/* enumeration for digest-qop */
	RvInt32								strDigestQop;		/* param for digest-qop */
	RvInt32								strDigestVerify;	/* param for digest-verify */
	RvSipSecurityAlgorithmType			eAlgorithm;			/* param for algorithm */
	RvSipSecurityProtocolType			eProtocol;			/* param for protocol */
	RvSipSecurityModeType				eMode;				/* param for mode */
	RvSipSecurityEncryptAlgorithmType	eEncryptAlgorithm;	/* param for encrypt-algorithm */
	SecuritySpiVal						spiC;				/* param for spiC */
	SecuritySpiVal						spiS;				/* param for spiS */
	RvInt32								portC;				/* param for portC */
	RvInt32								portS;				/* param for portS */
	RvInt32								strOtherParams;		/* Other parameters concatenated
																into a string, with ';' between
																the different parameters */
	
	MsgMgr*								pMsgMgr;			/* message manager for logId etc. */
	HPAGE								hPage;				/* page for allocation */
	HRPOOL								hPool;
	RvInt32								strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar					*pMechanismType;
	RvChar					*pPreference;
	RvChar					*pDigestAlgorithm;
	RvChar					*pDigestQop;
	RvChar					*pDigestVerify;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgSecurityHeader;

/*
 * MsgPMediaAuthorizationHeader object.
 */
typedef struct
{
    RvSipHeaderType			eHeaderType;	/* type of the header (PMediaAuthorization) */
	RvInt32					strToken;		/* param for PMediaAuthorizationToken */
	
    MsgMgr*					pMsgMgr;		/* message manager for logId etc. */
    HPAGE					hPage;			/* page for allocation */
    HRPOOL					hPool;
    RvInt32					strBadSyntax;	/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
	RvChar					*pToken;
    RvChar					*pBadSyntax;
#endif

}MsgPMediaAuthorizationHeader;

/*
 * MsgPProfileKeyHeader object.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (PProfileKey) */
    RvInt32                  strDisplayName;	/* the Display name of the Address */
    RvSipAddressHandle       hAddrSpec;			/* url address */
    RvInt32                  strOtherParams;	/* Other parameters concatenated
												   into a string, with ';' between
                                                   the different parameters */

    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pDisplayName;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPProfileKeyHeader;

/*
 * MsgPUserDatabaseHeader object.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (PUserDatabase) */
    RvSipAddressHandle       hAddrSpec;			/* url address (of type DiameterUri) */

    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pBadSyntax;
#endif
}MsgPUserDatabaseHeader;

/*
 * MsgPAnswerStateHeader object.
 */
typedef struct
{
    RvSipHeaderType						eHeaderType;	    /* type of the header (PAnswerState) */
	RvSipPAnswerStateAnswerType	        eAnswerType;	    /* enum type of Answer type */
    RvInt32								strAnswerType;	    /* the Stringed Answer Type */

	RvInt32								strOtherParams;	    /* Other parameters concatenated
															   into a string, with ';' between
															   the different parameters */
	
    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pAnswerType;
    RvChar                  *pOtherParams;
    RvChar                  *pBadSyntax;
#endif

}MsgPAnswerStateHeader;

/*
 * MsgPServedUserHeader object.
 */
typedef struct
{
    RvSipHeaderType                         eHeaderType;		/* type of the header (PServedUser) */
    RvInt32                                 strDisplayName;	    /* the Display name of the Address */
    RvSipAddressHandle                      hAddrSpec;			/* url address */
    RvSipPServedUserSessionCaseType         eSessionCaseType;
    RvSipPServedUserRegistrationStateType   eRegistrationStateType;
    RvInt32                                 strOtherParams; 	/* Other parameters concatenated
												                   into a string, with ';' between
                                                                   the different parameters */

    MsgMgr*                                 pMsgMgr;			/* message manager for logId etc. */
    HPAGE                                   hPage;				/* page for allocation */
    HRPOOL                                  hPool;
    RvInt32                                 strBadSyntax;		/* header value in string, in case of bad-syntax */
                            
#ifdef SIP_DEBUG            
    RvChar                     *pDisplayName;
    RvChar                     *pOtherParams;
    RvChar                     *pBadSyntax;
#endif

}MsgPServedUserHeader;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/*
 * MsgPDCSTracePartyIDHeader object.
 */
typedef struct
{
    RvSipHeaderType          eHeaderType;		/* type of the header (PDCSTracePartyID) */
    RvInt32                  strDisplayName;	/* the Display name of the Address */
    RvSipAddressHandle       hAddrSpec;			/* url address */

    MsgMgr*                  pMsgMgr;			/* message manager for logId etc. */
    HPAGE                    hPage;				/* page for allocation */
    HRPOOL                   hPool;
    RvInt32                  strBadSyntax;		/* header value in string, in case of bad-syntax */

#ifdef SIP_DEBUG
    RvChar                  *pDisplayName;
    RvChar                  *pBadSyntax;
#endif
}MsgPDCSTracePartyIDHeader;

/*
 * MsgPDCSOSPSHeader object.
 */
typedef struct
{
    RvSipHeaderType	     eHeaderType;		/* type of the header (PDCSOSPS) */
	RvSipOSPSTagType	 eOSPSTag;			/* Enumeration for the OSPS Tag */
    RvInt32			     strOSPSTag;		/* String for a proprietary OSPS tag */
					
    MsgMgr*			     pMsgMgr;			/* message manager for logId etc. */
    HPAGE			     hPage;				/* page for allocation */
    HRPOOL			     hPool;
    RvInt32			     strBadSyntax;		/* header value in string, in case of bad-syntax */
					
#ifdef SIP_DEBUG	
    RvChar			    *pOSPSTag;
    RvChar			    *pBadSyntax;
#endif
}MsgPDCSOSPSHeader;

/*
 * MsgPDCSBillingInfoHeader object.
 */
typedef struct
{
    RvSipHeaderType	     eHeaderType;				/* type of the header (PDCSBillingInfo) */
    RvInt32				 strBillingCorrelationID;	/* String for Billing Correlation ID */
	RvInt32			     strFEID;					/* String for FEID Hex */
	RvInt32			     strFEIDHost;				/* String for FEID Host */
	RvInt32			     strRKSGroupID;				/* String for RKSGroupID param */
	RvSipAddressHandle   hChargeAddr;				/* url address */
	RvSipAddressHandle   hCallingAddr;				/* url address */
	RvSipAddressHandle   hCalledAddr;				/* url address */
	RvSipAddressHandle   hRoutingAddr;				/* url address */
	RvSipAddressHandle   hLocRouteAddr;				/* url address */
	RvInt32				 strOtherParams;			/* Other parameters concatenated
														into a string, with ';' between
														the different parameters */
														
    MsgMgr*			     pMsgMgr;					/* message manager for logId etc. */
    HPAGE			     hPage;						/* page for allocation */
    HRPOOL			     hPool;						
    RvInt32			     strBadSyntax;				/* header value in string, in case of bad-syntax */
					
#ifdef SIP_DEBUG	
    RvChar			    *pBillingCorrelationID;
	RvChar			    *pFEID;
	RvChar			    *pFEIDHost;
	RvChar			    *pRKSGroupID;
	RvChar				*pOtherParams;
    RvChar			    *pBadSyntax;
#endif
}MsgPDCSBillingInfoHeader;

/*
 * MsgPDCSLAESHeader object.
 */
typedef struct
{
    RvSipHeaderType	     eHeaderType;				/* type of the header (PDCSLAES) */
    RvInt32				 strLaesSigHost;			/* String for Laes-sig Host */
	RvInt32				 laesSigPort;				/* Laes-sig Port number */
	RvInt32			     strLaesContentHost;		/* String for Laes-content Host */
	RvInt32				 laesContentPort;			/* Laes-content Port number */
	RvInt32			     strLaesKey;				/* String for Laes-key */
	RvInt32				 strOtherParams;			/* Other parameters concatenated
														into a string, with ';' between
														the different parameters */
														
    MsgMgr*			     pMsgMgr;					/* message manager for logId etc. */
    HPAGE			     hPage;						/* page for allocation */
    HRPOOL			     hPool;						
    RvInt32			     strBadSyntax;				/* header value in string, in case of bad-syntax */
					
#ifdef SIP_DEBUG	
    RvChar			    *pLaesSigHost;
	RvChar			    *pLaesContentHost;
	RvChar			    *pLaesKey;
	RvChar				*pOtherParams;
    RvChar			    *pBadSyntax;
#endif
}MsgPDCSLAESHeader;

/*
 * MsgPDCSRedirectHeader object.
 */
typedef struct
{
    RvSipHeaderType	     eHeaderType;				/* type of the header (PDCSLAES) */
    RvSipAddressHandle   hCalledIDAddr;				/* url address for Called-Id Parameter */
	RvSipAddressHandle   hRedirectorAddr;			/* url address for Redirector Parameter */
	RvInt32				 nRedirCount;				/* Number for redirections count */
	RvInt32				 strOtherParams;			/* Other parameters concatenated
														into a string, with ';' between
														the different parameters */
														
    MsgMgr*			     pMsgMgr;					/* message manager for logId etc. */
    HPAGE			     hPage;						/* page for allocation */
    HRPOOL			     hPool;						
    RvInt32			     strBadSyntax;				/* header value in string, in case of bad-syntax */
					
#ifdef SIP_DEBUG	
	RvChar				*pOtherParams;
    RvChar			    *pBadSyntax;
#endif
}MsgPDCSRedirectHeader;
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

/*
 * SipHeaderList represents an element of the headers list in the sip message.
 * It contains the header type which could be a specific header (RVSIP_HEADERTYPE_ALLOW,
 * RVSIP_HEADERTYPE_CONTACT,RVSIP_HEADERTYPE_VIA) or RVSIP_HEADERTYPE_OTHER which refers
 * to all other headers besides the headers (TO/FROM/CALL-ID/CSEQ/CONTENT-LENGTH/
 * CONTENT-TYPE) that appear by value in the message.
 */
typedef struct
{
     RvSipHeaderType   headerType;
     void*             hHeader;

}MsgHeaderListElem;


/*
 * RequestLine
 * ----------
 */
typedef struct
{
   RvSipMethodType      eMethod;
   RvInt32             strMethod;
   RvSipAddressHandle   hRequestUri;

#ifdef SIP_DEBUG
   RvChar             *pMethod;
#endif

}MsgRequestLine;

/*
 * Status line
 * -------------
 */
typedef struct
{
    RvInt16    statusCode;
    RvInt32    strReasonePhrase;

#ifdef SIP_DEBUG
    RvChar    *pReasonePhrase;
#endif

}MsgStatusLine;


/*
 * A SIP message object stores a SIP message in parsed format.
 * It allows easy access to the different message parts.
 * A SIP message is either a request from a client to a server,
 * or a response from a server to client.
 * The object is composed of
 * - start line (request line or response line)
 * - specific headers: To/From/CallId/Cseq/ContentLength/ContentTypes
 * - list of other headers
 * - body of the message (MIME/SDP)
 */

/*
 * MsgStartLine object stores the start line values:
 * startLine.requestLine stores the request line values (if this is a request),
 * and startLine.statusLine stores the status line values (if this is a response).
 */

typedef union
{
    MsgRequestLine            requestLine;            /* request line */
    MsgStatusLine            statusLine;                /* response line */

}MsgStartLine;

typedef struct
{
    /* start line */
    MsgStartLine                startLine;
    RvInt8                        bIsRequest;          /* RV_TRUE if the start line is request,
                                                     RV_FALSE if the start line is response,
                                                     UNDEFINED if not defined yet.*/
    RvInt32                    strBadSyntaxStartLine; /* UNDEFINED if message start-line
                                                       is correct. >0 if has a bad-syntax */
    RvInt32                    strBadSyntaxReasonPhrase; /* UNDEFINED if message is correct.
                                                     contains the first parser error string,
                                                     if message has a bad-syntax error */
    RvBool                      bBadSyntaxContentLength;

    /* headers */
    RvSipPartyHeaderHandle      hFromHeader;      /* From header */
    RvSipPartyHeaderHandle      hToHeader;          /* To header */
    RvSipCSeqHeaderHandle       hCSeqHeader;      /* Cseq header */
    RvSipOtherHeaderHandle      hCallIdHeader;    /* CallId header */
    RLIST_HANDLE                headerList;       /* List of other headers */

    /* message body */
    RvInt32                     contentLength;   /* ContentLength header */
    RvBool                      bCompContentLen; /* Is the ContentLength in compact form */
#ifndef RV_SIP_PRIMITIVES
    Body                       *pBodyObject;     /* body object */
#else
    RvInt32                     strContentType;  /* ContentType header */
    RvInt32                     strContentID;    /* ContentID header */
    RvInt32                     strBody;         /* message body container */
#endif /* #ifndef RV_SIP_PRIMITIVES */

    MsgMgr*  pMsgMgr;  /* message manager */
    RvBool   bForceCompactForm;

    /* page and pool for allocation */
    HPAGE                       hPage;
    HRPOOL                      hPool;

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	RvInt32                     terminationCounter;/* The message will not be deallocated as
												  long as terminationCounter > 0, i.e., RvSipMsgDistruct() 
												  will not be effective if terminationCounter > 0.
												  The terminationCounter can be increased via internal
												  API IncrementCounter() and will be decreased
												  with any call to RvSipMsgDistruct(). */
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

#ifdef SIP_DEBUG
    RvChar                       *pContentType;
    RvChar                       *pContentID;
    RvChar                       *pBody;
    RvChar                    *pBadSyntaxStartLine;
    RvChar                    *pBadSyntaxReasonPhrase;
#endif

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    RvChar                      *SPIRENT_msgBody;
    SPIRENT_RvSipMsgEncode_Type f_Encoder;
    SPIRENT_Encoder_Cfg_Type    Encoder_cfg;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
} MsgMessage;



#ifdef __cplusplus
}
#endif
#endif /* RVSIPTYPES_H */

