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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/

/*********************************************************************************
 *                              <RvSipUserConfig.h>
 *
 * The file enables the user to configure the stack compilation flags.
 * The configuration flags defined here will override any compilation flag
 * supplied in the compilation process.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                Jan-2004
 *********************************************************************************/

#ifndef _RV_SIP_USER_CONFIG
#define _RV_SIP_USER_CONFIG

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rvtypes.h"

/*--------------------RV_SIGCOMP_ON-----------------------*/
/*Enabling   the SigComp module*/
#ifndef RV_SIGCOMP_ON

#define RV_SIGCOMP_ON

#endif /*RV_SIGCOMP_ON*/




/*----------RV_DNS_ENHANCED_FEATURES_SUPPORT------------*/

/*Enabling the enhanced DNS feature*/
#ifndef RV_DNS_ENHANCED_FEATURES_SUPPORT
#define RV_DNS_ENHANCED_FEATURES_SUPPORT
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/



/*----------------RV_DEPRECATED_CORE---------------------*/
/*Enabling usage of the deprecated core API. This is for backwards
  compatability only. For os services use the mid layer.*/
#ifndef RV_DEPRECATED_CORE
/*
#define RV_DEPRECATED_CORE
*/
#endif /*RV_DEPRECATED_CORE*/



/*----------------RV_SIP_PRIMITIVES---------------------*/
#ifndef RV_SIP_PRIMITIVES
/*
#define RV_SIP_PRIMITIVES
*/
#endif /*RV_SIP_PRIMITIVES*/


/*------------RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY-------*/
/*
#define RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
*/

/*----------------RV_SIP_IMS_ON---------------------*/
#ifndef RV_SIP_IMS_ON

#define RV_SIP_IMS_ON

#endif

/*----------------RV_SIP_HIGH_AVAL_3_0---------------------*/
/*
#define RV_SIP_HIGH_AVAL_3_0
*/
/*----------------RV_SIP_SUBS_ON---------------------*/
#ifndef RV_SIP_SUBS_ON
#define RV_SIP_SUBS_ON
#endif
/*----------------RV_SIP_AUTH_ON---------------------*/
#ifndef RV_SIP_AUTH_ON
#define RV_SIP_AUTH_ON
#endif
/*----------------RV_SIP_HIGHAVAL_ON---------------------*/
#ifndef RV_SIP_HIGHAVAL_ON
#define RV_SIP_HIGHAVAL_ON
#endif

/*----------------RV_SIP_TEL_URI_SUPPORT---------------------*/
#ifndef RV_SIP_TEL_URI_SUPPORT
#define RV_SIP_TEL_URI_SUPPORT
#endif

/*----------------RV_SIP_OTHER_URI_SUPPORT---------------------*/
#ifndef RV_SIP_OTHER_URI_SUPPORT
#define RV_SIP_OTHER_URI_SUPPORT
#endif

/*----------------RV_SIP_UNSIGNED_CSEQ---------------------*/
/*handles cseq as RvUint32 enables CSeq to range from 0-2^32-1.
  Note - RFC3261 defines that CSeq should be from 0-2^31-1 therefore it
  is not recommended to use this flag.*/

/*#define RV_SIP_UNSIGNED_CSEQ*/



/*----------------RV_SIP_LIGHT---------------------*/
#ifndef RV_SIP_LIGHT
/*
#define RV_SIP_LIGHT
*/
#endif




/*--------RV_SIP_IMS_HEADER_SUPPORT ( P-Headers Support)-------------*/
#ifndef RV_SIP_IMS_HEADER_SUPPORT

#define RV_SIP_IMS_HEADER_SUPPORT

#endif

/*--------RV_SIP_IMS_DCS_HEADER_SUPPORT ( Cable Headers Support)-------------*/
#ifndef RV_SIP_IMS_DCS_HEADER_SUPPORT
/*
#define RV_SIP_IMS_DCS_HEADER_SUPPORT
*/
#endif

/*----------------RV_SIP_IPSEC_NEG_ONLY---------------------*/

#ifndef RV_SIP_IPSEC_NEG_ONLY
/*
#define RV_SIP_IPSEC_NEG_ONLY
*/
#endif


/*The following flags should not be used with the C sip toolkit*/

/*----------------RV_SSL_SESSION_STATUS----------------*/
/* enabling session status callback*/
/*#define RV_SSL_SESSION_STATUS*/

/*----------------RV_SIP_DISABLE_LISTENING_SOCKET------------*/


/*
#define RV_SIP_DISABLE_LISTENING_SOCKETS
*/



/*----------------RV_SIP_JSR32_SUPPORT---------------------*/

#ifndef RV_SIP_JSR32_SUPPORT
/*
#define RV_SIP_JSR32_SUPPORT
*/
#endif

/*----------------RV_SIP_EXTENDED_HEADER_SUPPORT---------------------*/

#ifndef RV_SIP_EXTENDED_HEADER_SUPPORT
/*
#define RV_SIP_EXTENDED_HEADER_SUPPORT
*/
#endif



/*-----------------------------------------------------*/
/*      INTERNAL DEFINITIONS - DO NOT CHANGE           */
/*-----------------------------------------------------*/
/*      Do not edit the file bellow this line          */

/*encode the header list before the fixed headers*/
#ifndef RV_SIP_ENCODE_HEADER_LIST_FIRST
/*#define RV_SIP_ENCODE_HEADER_LIST_FIRST*/
#endif
#if (RV_TLS_TYPE != RV_TLS_OPENSSL)
#undef RV_SSL_SESSION_STATUS
#endif /*(RV_TLS_TYPE != RV_TLS_OPENSSL)*/

#ifdef RV_SIP_PRIMITIVES
#undef RV_SIP_SUBS_ON
#undef RV_SIP_HIGHAVAL_ON
#undef RV_SIP_HIGH_AVAL_3_0
#undef RV_SIP_AUTH_ON
#define RV_SIP_AUTH_ON
#endif /*RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_LIGHT
#undef RV_SIP_AUTH_ON
#undef RV_SIP_SUBS_ON
#undef RV_SIP_HIGHAVAL_ON
#undef RV_SIP_TEL_URI_SUPPORT
#undef RV_DNS_ENHANCED_FEATURES_SUPPORT
#undef RV_SIGCOMP_ON
#undef RV_SIP_PRIMITIVES
#define RV_SIP_PRIMITIVES
#endif /*RV_SIP_LIGHT*/

#ifndef RV_SIP_HIGHAVAL_ON
#undef RV_SIP_HIGH_AVAL_3_0
#endif /* #ifndef RV_SIP_HIGHAVAL_ON*/

#ifdef RV_SIP_JSR32_SUPPORT
#define RV_SIP_EXTENDED_HEADER_SUPPORT
#define RV_SIP_SUBS_ON
#define RV_SIP_AUTH_ON
#define RV_SIP_TEL_URI_SUPPORT
#undef RV_SIP_OTHER_URI_SUPPORT
#undef RV_SIP_IMS_HEADER_SUPPORT
#undef RV_SIP_HIGH_AVAL_3_0
#undef RV_SIP_HIGHAVAL_ON
#undef RV_SIP_LIGHT
#undef RV_SIP_DISABLE_LISTENING_SOCKETS
#undef RV_SIP_UNSIGNED_CSEQ
#undef RV_SIGCOMP_ON
#undef RV_DEPRECATED_CORE
#undef RV_SIP_PRIMITIVES
#undef RV_DNS_ENHANCED_FEATURES_SUPPORT
#endif

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
#undef RV_SIP_IMS_HEADER_SUPPORT
#define RV_SIP_IMS_HEADER_SUPPORT
#undef RV_SIP_TEL_URI_SUPPORT
#define RV_SIP_TEL_URI_SUPPORT
#undef RV_SIP_AUTH_ON
#define RV_SIP_AUTH_ON
#undef RV_SIP_SUBS_ON
#define RV_SIP_SUBS_ON
#undef RV_SIP_PRIMITIVES
#undef RV_SIP_JSR32_SUPPORT
#undef RV_SIP_LIGHT
#undef RV_SIP_EXTENDED_HEADER_SUPPORT
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
#ifndef RV_SIP_IMS_ON
#define RV_SIP_IMS_ON
#endif
#undef RV_SIP_IPSEC_NEG_ONLY
#endif

#ifdef RV_SIP_IMS_ON
#undef RV_SIP_IMS_HEADER_SUPPORT
#define RV_SIP_IMS_HEADER_SUPPORT
#undef RV_SIP_TEL_URI_SUPPORT
#define RV_SIP_TEL_URI_SUPPORT
#undef RV_SIP_AUTH_ON
#define RV_SIP_AUTH_ON
#undef RV_SIP_SUBS_ON
#define RV_SIP_SUBS_ON
#undef RV_SIP_PRIMITIVES
#undef RV_SIP_JSR32_SUPPORT
#undef RV_SIP_LIGHT
#undef RV_SIP_EXTENDED_HEADER_SUPPORT
#else
#undef RV_SIP_IMS_HEADER_SUPPORT
#undef RV_SIP_IMS_DCS_HEADER_SUPPORT
#endif



/* RFC 3261: 
     12.1.1 UAS behavior
 ... A UAS MUST be prepared to receive a request without a tag in the From field, 
     in which case the tag is considered to have a value of null.
     
     12.1.2 UAC Behavior
...  A UAC MUST be prepared to receive a response without a tag in the To field, 
     in which case the tag is considered to have a value of null*/    
/*#define RFC_2543_COMPLIANT*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*#define _RV_SIP_USER_CONFIG*/

