/******************************************************************************
Filename    :rvsdpenums.h
Description : differents enumerations

  ******************************************************************************
  Copyright (c) 2005 RADVision Inc.
  ************************************************************************
  NOTICE:
  This document contains information that is proprietary to RADVision LTD.
  No part of this publication may be reproduced in any form whatsoever
  without written prior approval by RADVision LTD..
  
    RADVision LTD. reserves the right to revise this publication and make
    changes without obligation to notify any person of such revisions or
    changes.
 Author:Rafi Kiel
******************************************************************************/

#ifndef _file_rvsdpenums_h_
#define _file_rvsdpenums_h_

#include "rverror.h"

#define RV_SDP_DEFAULT_CLOCK_RATE 8000
#define RV_SDP_INT_NOT_SET          -1


/**************** SDP media types ************/
typedef enum {
    RV_SDPMEDIATYPE_NOTSET   = 0,
    RV_SDPMEDIATYPE_AUDIO,                /*  audio  */
    RV_SDPMEDIATYPE_NAS,                  /*  nas    */
    RV_SDPMEDIATYPE_VIDEO,                /*  video    */
    RV_SDPMEDIATYPE_APP,                  /*  application */
    RV_SDPMEDIATYPE_DATA,                 /*  data    */
    RV_SDPMEDIATYPE_IMAGE,                /*  image   */
    RV_SDPMEDIATYPE_CONTROL,              /*  control */
    RV_SDPMEDIATYPE_UNKNOWN
} RvSdpMediaType;

/**************** SDP network type ************/
typedef enum {
    RV_SDPNETTYPE_NOTSET    = 0,
    RV_SDPNETTYPE_IN,                    /*  IN     */
    RV_SDPNETTYPE_ATM,                   /*  ATM    */
    RV_SDPNETTYPE_LOCAL,                 /*  LOCAL  */
    RV_SDPNETTYPE_OTHER,                 /*  Other  */
    RV_SDPNETTYPE_TN,                    /*  TN     */
    RV_SDPNETTYPE_ANY,                   /*  ANY    */
    RV_SDPNETTYPE_IGNORE,                /*  IGNORE */
    RV_SDPNETTYPE_UNKNOWN
} RvSdpNetType;

/**************** SDP addresses ************/
typedef enum {
    RV_SDPADDRTYPE_NOTSET    = 0,
    RV_SDPADDRTYPE_IP4,                  /*  IP4    */
    RV_SDPADDRTYPE_IP6,                  /*  IP6    */
    RV_SDPADDRTYPE_ENDPOINT,             /*  epn    */
    RV_SDPADDRTYPE_NSAP,                 /*  nsap   */
    RV_SDPADDRTYPE_E164,                 /*  e164   */
    RV_SDPADDRTYPE_GWID,                 /*  GWID   */
    RV_SDPADDRTYPE_ALIAS,                /*  ALIAS  */
    RV_SDPADDRTYPE_RFC2543,              /*  RFC2543*/
    RV_SDPADDRTYPE_ANY,                  /*  ANY    */
    RV_SDPADDRTYPE_IGNORE,               /*  IGNORE */
    RV_SDPADDRTYPE_UNKNOWN
} RvSdpAddrType;

/**************** SDP encryption methods ********/
typedef enum
{
    RV_SDPENCRMTHD_NOTSET = 0,
    RV_SDPENCRMTHD_CLEAR,   /* clear  */
    RV_SDPENCRMTHD_BASE64,  /* base64 */
    RV_SDPENCRMTHD_URI,     /* uri    */
    RV_SDPENCRMTHD_PROMPT,  /* prompt */
    RV_SDPENCRMTHD_KEY,     /* key */    
    RV_SDPENCRMTHD_UNKNOWN
} RvSdpEncrMethod;

/**************** Send mode  ********/
typedef enum
{
    RV_SDPCONNECTMODE_NOTSET    = 0,
    RV_SDPCONNECTMODE_SENDONLY,       /* "sendonly" */
    RV_SDPCONNECTMODE_RECVONLY,       /* "recvonly" */
    RV_SDPCONNECTMODE_SENDRECV,       /* "sendrecv" */
    RV_SDPCONNECTMODE_INACTIVE        /* "inactive" */
} RvSdpConnectionMode;

/**************** SDP media protocols ********/
typedef enum {
    RV_SDPPROTOCOL_NOTSET       = 0,
    RV_SDPPROTOCOL_RTP,           /*  RTP/AVP  */
    RV_SDPPROTOCOL_RTP_SAVP,      /*  RTP/SAVP  */
    RV_SDPPROTOCOL_LOCAL,         /*  LOCAL  */
    RV_SDPPROTOCOL_ATM,           /*  atm/avp  */
    RV_SDPPROTOCOL_UDP,
    RV_SDPPROTOCOL_UDP_T38,
    RV_SDPPROTOCOL_TCP,    
    RV_SDPPROTOCOL_AAL1ATMF,
    RV_SDPPROTOCOL_AAL1ITU, 
    RV_SDPPROTOCOL_AAL1CUSTOM,
    RV_SDPPROTOCOL_AAL2ATMF,  
    RV_SDPPROTOCOL_AAL2ITU,   
    RV_SDPPROTOCOL_AAL2CUSTOM,
    RV_SDPPROTOCOL_AAL5ATMF,  
    RV_SDPPROTOCOL_AAL5ITU,   
    RV_SDPPROTOCOL_AAL5CUSTOM,
    RV_SDPPROTOCOL_H323C,     
    RV_SDPPROTOCOL_UNKNOWN   
} RvSdpProtocol;


#ifdef RV_SDP_MEGACO_ENABLED
typedef enum
{
    RV_SDPVCID_NOTSET       = 0,
    RV_SDPVCID_VCCI,
    RV_SDPVCID_CID,
    RV_SDPVCID_BCG,
    RV_SDPVCID_PORT,
    RV_SDPVCID_VPI,
    RV_SDPVCID_VCI,
    RV_SDPVCID_VPCI,
    RV_SDPVCID_NSAP,
    RV_SDPVCID_E164,
    RV_SDPVCID_GWID,
    RV_SDPVCID_ALIAS,
    RV_SDPVCID_UNKNOWN
} RvSdpVcId;
#endif /*RV_SDP_MEGACO_ENABLED*/


#ifdef RV_SDP_KEY_MGMT_ATTR

/**************** SDP key mgmt attribute protocol id ********/
typedef enum {
    RV_SDPKEYMGMT_NOTSET       = 0,
    RV_SDPKEYMGMT_MIKEY,
    RV_SDPKEYMGMT_UNKNOWN
} RvSdpKeyMgmtPrtclType;    

#endif /*RV_SDP_KEY_MGMT_ATTR*/

typedef enum
{
    SDP_FIELDTYPE_NOT_SET = 0,
    SDP_FIELDTYPE_ORIGIN,
    SDP_FIELDTYPE_INFORMATION,
    SDP_FIELDTYPE_SESSION_ID,
    SDP_FIELDTYPE_URI,
    SDP_FIELDTYPE_EMAIL,
    SDP_FIELDTYPE_PHONE,
    SDP_FIELDTYPE_BANDWIDTH,
    SDP_FIELDTYPE_VERSION,
    SDP_FIELDTYPE_TIME,
    SDP_FIELDTYPE_TIME_ZONE_ADJUST,
    SDP_FIELDTYPE_KEY,
    SDP_FIELDTYPE_ATTRIBUTE,
    SDP_FIELDTYPE_CONNECTION,
    SDP_FIELDTYPE_REPEAT,
    SDP_FIELDTYPE_MEDIA,
    SDP_FIELDTYPE_RTP_MAP,
    SDP_FIELDTYPE_KEY_MGMT,
    SDP_FIELDTYPE_CRYPTO,
    SDP_FIELDTYPE_CONNECTION_MODE,
    SDP_FIELDTYPE_FRAMERATE,
    SDP_FIELDTYPE_FMTP,
    SDP_FIELDTYPE_MEDIA_ID,
    SDP_FIELDTYPE_MEDIA_GROUP,
    SDP_FIELDTYPE_PRECONDITION,
    SDP_FIELDTYPE_MSRP_ACC_TYPES,
    SDP_FIELDTYPE_MSRP_ACC_WRAPPED_TYPES,
    SDP_FIELDTYPE_MSRP_MAX_SIZE,
    SDP_FIELDTYPE_MSRP_PATH,
    SDP_FIELDTYPE_RTCP,    
    SDP_FIELDTYPE_UNKNOWN_TAG,
    SDP_FIELDTYPE_BAD_FIELD      
} RvSdpFieldTypes;


/**************** Typed-time units ********/
typedef enum
{
    RV_SDPTIMETYPE_DAY = 0,       /* "d" */
    RV_SDPTIMETYPE_HOUR,      /* "h" */
    RV_SDPTIMETYPE_MONTH,     /* "m" */
    RV_SDPTIMETYPE_SECOND,     /* "s" */ 
    RV_SDPTIMETYPE_NOT_SET        
} RvSdpTimeUnit;


/* Return codes */
typedef enum
{
    RV_SDPSTATUS_OK = RV_OK,
    RV_SDPSTATUS_ENCODEFAILBUF = -1,
    RV_SDPSTATUS_ALLOCFAIL = -2,
    RV_SDPSTATUS_PARSEFAIL = -3,
    RV_SDPSTATUS_ILLEGAL_SET = -4,
    RV_SDPSTATUS_NULL_PTR = -5,
    RV_SDPSTATUS_SET_FAILED = -6,
    RV_SDPSTATUS_SMALL_BUFFER = -7,
    RV_SDPSTATUS_NOT_EXISTS = -8
} RvSdpStatus;
 
typedef enum {
    RV_SDPPARSER_STOP_ZERO,
    RV_SDPPARSER_STOP_BLANKLINE,
    RV_SDPPARSER_STOP_DOTLINE,
    RV_SDPPARSER_STOP_CLOSEBRACE,
    RV_SDPPARSER_STOP_NOT_SET, /* keep this order
                                  insert new values here before RV_SDPPARSER_STOP_ALLOCFAIL
                                  remember not to use -2 and -3 values */
    RV_SDPPARSER_STOP_ALLOCFAIL = RV_SDPSTATUS_ALLOCFAIL,
    RV_SDPPARSER_STOP_ERROR = RV_SDPSTATUS_PARSEFAIL
} RvSdpParseStatus;

typedef enum
{
    RvSdpWarnNoWarning,
    RvSdpWarnInternErr,
    RvSdpWarnNoTag,
    RvSdpWarnNoEq,
    RvSdpWarnNoVal,
    RvSdpWarnMultNotAllowed,
    RvSdpWarnMissingTag,
    RvSdpWarnTagOutOfOrder,
    RvSdpWarnNotSdpLine,
    RvSdpWarnUnknownTagInMsg,
    RvSdpWarnUnknownTagInMedia,
    RvSdpWarnVersionFailed,
    RvSdpWarnOriginUserNameFailed,
    RvSdpWarnOriginSessIdTooLong,
    RvSdpWarnOriginSessIdFailed,
    RvSdpWarnOriginSessVersionFailed,
    RvSdpWarnOriginSessVersionTooLong,
    RvSdpWarnSessionNameFailed,
    RvSdpWarnSessionInfoFailed,
    RvSdpWarnConnectionNumOfAddrTooLong,
    RvSdpWarnConnectionNumOfAddrIllegal,
    RvSdpWarnConnectionTTLTooLong,
    RvSdpWarnConnectionTTLIllegal,
    RvSdpWarnEmailFailed,
    RvSdpWarnEmailValueIllegal,
    RvSdpWarnPhoneFailed,
    RvSdpWarnPhoneValueIllegal,
    RvSdpWarnBandwidthTypeIllegal,
    RvSdpWarnBandwidthValueIllegal,
    RvSdpWarnBandwidthValueTooLong,
    RvSdpWarnSessStartTimeTooLong,
    RvSdpWarnSessStartTimeIllegal,
    RvSdpWarnSessStopTimeTooLong,
    RvSdpWarnSessStopTimeIllegal,
    RvSdpWarnSessStartTimeTooSmall,
    RvSdpWarnSessStopTimeTooSmall,
    RvSdpWarnSessStartLessThanStop,
    RvSdpWarnRepeatIntervalValueIllegal,
    RvSdpWarnRepeatIntervalValueTooLong,
    RvSdpWarnRepeatDurationValueIllegal,
    RvSdpWarnRepeatDurationValueTooLong,
    RvSdpWarnRepeatOffsetValueIllegal,
    RvSdpWarnRepeatOffsetValueTooLong,
    RvSdpWarnRepeatNoOffsets,
    RvSdpWarnRepeatNoSessTime,
    RvSdpWarnTimeZoneIllegalTime,
    RvSdpWarnTimeZoneTimeTooLong,
    RvSdpWarnTimeZoneIllegalOffset,
    RvSdpWarnTimeZoneOffsetTooLong,
    RvSdpWarnTimeZoneNoData,
    RvSdpWarnKeyMethodIllegal,
    RvSdpWarnKeyValueNotSet,
    RvSdpWarnAttrNameIllegal,
    RvSdpWarnFailedToParseNetType,
    RvSdpWarnFailedToParseAddrType,
    RvSdpWarnFailedToParseAddress,
    RvSdpWarnIllegalMedia,
    RvSdpWarnIllegalMediaPort,
    RvSdpWarnMediaPortTooLong,
    RvSdpWarnIllegalMediaSubPort,
    RvSdpWarnMediaSubPortTooLong,
    RvSdpWarnIllegalMediaProtocol,
    RvSdpWarnNoFormatsForMedia,
    RvSdpWarnIllegalMediaFormat,
    RvSdpWarnTooManyMediaFormats,
    RvSdpWarnRtpMapNoPayload,
    RvSdpWarnRtpMapPayloadTooLong,
    RvSdpWarnRtpMapNoEncName,
    RvSdpWarnRtpMapBadEncRate,
    RvSdpWarnRtpMapEncRateTooLong,
    RvSdpWarnConnModeSetAlready,
    RvSdpWarnDomainAddressTooLong,
    RvSdpWarnIp4AddressTooLong,
    RvSdpWarnIp4AddressIllegal,
    RvSdpWarnDomainNameIllegal,
    RvSdpWarningAddressTooLong,
    RvSdpWarnIllegalURI,
#ifdef RV_SDP_MEGACO_ENABLED        
    RvSdpWarnIllegalVcid,
#endif /*RV_SDP_MEGACO_ENABLED*/

#ifdef RV_SDP_KEY_MGMT_ATTR    
    RvSdpWarnKeyMgmtIllegalPrtcl,
    RvSdpWarnKeyMgmtIllegalData,
#endif /*RV_SDP_KEY_MGMT_ATTR*/

#ifdef RV_SDP_CRYPTO_ATTR      
    RvSdpWarnCryptoIllegalTag,
    RvSdpWarnCryptoIllegalSuite,
    RvSdpWarnCryptoIllegalKeyMethod,
    RvSdpWarnCryptoIllegalKeyInfo,
    RvSdpWarnCryptoNoKeyParams,
    RvSdpWarnCryptoIllegalKeyParamsTooMany,
    RvSdpWarnCryptoIllegalSessParamsTooMany,
#endif /*RV_SDP_CRYPTO_ATTR*/
        
#ifdef RV_SDP_MEDIA_GROUPING_ATTR
    /*a=mid warnings*/
    RvSdpWarnMidNoValueForMidAttr,
    RvSdpWarnMidIllegalMidValue,
    RvSdpWarnMidIllegalMoreThenOneParam,
    
    /*a=group warnings*/
    RvSdpWarnMediaGroupNoMidIdParams,
    RvSdpWarnMediaGroupIllegalMidIdVal,
    RvSdpWarnMediaGroupMidIdParamsTooMany,
    RvSdpWarnMediaGroupNoSemanticsVal,
    RvSdpWarnMediaGroupIllegalSemantics,   
#endif /*RV_SDP_MEDIA_GROUPING_ATTR*/

    /*a=curr,a=des,a=conf warnings*/
#ifdef RV_SDP_PRECONDITIONS_ATTR
    RvSdpWarnPrecondNumOfParametersIllegal,
	RvSdpWarnPrecondMissingOneParameter,
    RvSdpWarnPrecondNoParameters,
    RvSdpWarnPrecondTooManyParameters,
#endif /* RV_SDP_PRECONDITIONS_ATTR */
   
    RvSdpWarnPrivateWarning
} RvSdpParseWarning;


/*media-group Enums*/
#ifdef RV_SDP_MEDIA_GROUPING_ATTR
typedef enum {
        RV_SDP_GROUP_SEMANTICS_NOTSET = 0,
        RV_SDP_GROUP_SEMANTICS_LS,      /*Lip Synchronization*/
        RV_SDP_GROUP_SEMANTICS_FID,     /*Flow Identification*/
        RV_SDP_GROUP_SEMANTICS_SRF,     /*Single (Resource) Reservation Flow*/
        RV_SDP_GROUP_SEMANTICS_UNKNOWN 
} RvSdpGroupSemanticsType;
#endif /*RV_SDP_MEDIA_GROUPING_ATTR*/




/*preconditions Enums*/
#ifdef RV_SDP_PRECONDITIONS_ATTR

typedef enum
{
        RV_SDP_PRECOND_NAME_CURRENT_ATTR =0,
        RV_SDP_PRECOND_NAME_DESIRED_ATTR,
        RV_SDP_PRECOND_NAME_CONFIRMED_ATTR,
        RV_SDP_PRECOND_NAME_ALL,
        RV_SDP_PRECOND_NAME_UNKNOWN

}RvSdpPrecondName;

typedef enum {
        RV_SDP_PRECOND_TYPE_NOTSET = 0, 
        RV_SDP_PRECOND_TYPE_QOS ,       /* QOS = quality Of Service */
        RV_SDP_PRECOND_TYPE_UNKNOWN 
} RvSdpPreconditionType;

typedef enum {
        RV_SDP_PRECOND_STATUS_TYPE_NOTSET = 0, 
        RV_SDP_PRECOND_STATUS_TYPE_E2E ,
        RV_SDP_PRECOND_STATUS_TYPE_LOCAL ,       
        RV_SDP_PRECOND_STATUS_TYPE_REMOTE ,       
        RV_SDP_PRECOND_STATUS_TYPE_UNKNOWN 
} RvSdpPreconditionStatusType;

typedef enum {
        RV_SDP_PRECOND_STRENGTH_NOTSET = 0, 
        RV_SDP_PRECOND_STRENGTH_NONE ,
        RV_SDP_PRECOND_STRENGTH_OPTIONAL ,       
        RV_SDP_PRECOND_STRENGTH_MANDATORY,  
        RV_SDP_PRECOND_STRENGTH_FAILURE,       
        RV_SDP_PRECOND_STRENGTH_UNKNOWN 
} RvSdpPreconditionStrengthTag;

typedef enum {
        RV_SDP_PRECOND_DIRECTION_NOTSET = 0, 
        RV_SDP_PRECOND_DIRECTION_NONE,
        RV_SDP_PRECOND_DIRECTION_SEND,
        RV_SDP_PRECOND_DIRECTION_RECV,       
        RV_SDP_PRECOND_DIRECTION_SENDRECV,  
        RV_SDP_PRECOND_DIRECTION_UNKNOWN 
} RvSdpPreconditionDirectionTag;


#endif /*RV_SDP_PRECONDITIONS_ATTR*/

#endif


