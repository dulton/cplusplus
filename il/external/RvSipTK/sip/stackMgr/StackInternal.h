
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
 *                              StackInternal.h
 *
 *  This file provides internal definitions for the stack module of the sip stack.
 *  it contains definitions for initiating the entire stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20-Nov-2000
 *********************************************************************************/


#ifndef _STACK_INTERNAL_H
#define _STACK_INTERNAL_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "rvlog.h"
#include "rvloglistener.h"

#include "AdsRpool.h"

#include "RvSipStackTypes.h"
#include "RvSipTransactionTypes.h"
#include "RvSipTransportTypes.h"
#include "_SipResolverTypes.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLegTypes.h"
#include "RvSipRegClientTypes.h"
#endif /* RV_SIP_PRIMITIVES */
#include "RvSipAuthenticator.h"
#include "RvSipTransmitterTypes.h"
#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */
#ifdef RV_SIP_IMS_ON
#include "RvSipSecurityTypes.h"
#endif /* RV_SIP_IMS_ON */
#include "_SipTransactionTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/



/* Stack configuration Default values */

/*objects mumber*/
#define DEFAULT_MAX_CALL_LEGS          10
#define DEFAULT_MAX_REG_CLIENTS        2
#define DEFAULT_MAX_PUB_CLIENTS        2
#define DEFAULT_MAX_TRANSACTIONS       30
#define DEFAULT_MAX_SUBSCRIPTIONS      0
#define DEFAULT_MAX_LOCAL_ADDRESSES    10

/*memory pool sizes*/
#define DEFAULT_MSG_POOL_PAGE_SIZE     1536
#define DEFAULT_GENERAL_POOL_PAGE_SIZE 1024

/* SPIRENT_BEGIN */
/* #define DEFAULT_ELEMENT_POOL_PAGE_SIZE 250 */
#define DEFAULT_ELEMENT_POOL_PAGE_SIZE 256
/* SPIRENT_END */

/*timers*/
#define DEFAULT_PROXY_2XX_SEND_TIMER   0
#define DEFAULT_SESSION_EXPIRES_TIME   1800
#define DEFAULT_SUBS_ALERT_TIMER       5000
#define DEFAULT_CLIENT_ALERT_TIMER	   5000
#define DEFAULT_SUBS_NO_NOTIFY_TIMER   32000
#define DEFAULT_FORKED_CALL_ACK_TIMER  32000

/*other resources*/
#define DEFAULT_MAX_BUFFER_SIZE                  2048
#define DEFAULT_NUM_OF_PROCESSING_THREADS        0
#define DEFAULT_PROCESSING_THREAD_PRIORITY       100
#define DEFAULT_PROCESSING_THREAD_STACK_SIZE     30000

/* dns parameters */
#define DEFAULT_ARES_BUFFER_SIZE                 1024
#define DEFAULT_MAX_NUM_OF_ELEMENTS_IN_DNS_LIST  5
#define DEFAULT_MAX_DNS_DOMAINS  5
#define DEFAULT_MAX_DNS_SERVERS  5

/* enum for all modules that have a dedicated log source */
#define STACK_NUM_OF_LOG_SOURCE_MODULES 41

/* SCTP parameters */    
#define DEFAULT_SCTP_IN_STREAMS  10
#define DEFAULT_SCTP_OUT_STREAMS 1

/* Security parameters */
#define DEFAULT_MAX_SECURITY_OBJECTS  1
#define DEFAULT_MAX_SEC_AGREE_OBJECTS 0
#define DEFAULT_SPI_RANGE_START       0x100
#define DEFAULT_SPI_RANGE_END         0xFFFFFFFF
#define DEFAULT_MAX_OWNERS_OF_SEC_AGREE 3

/********************************************************************************************
 * Definition of StackMgr:
 * hAppStack           - Application handle to the stack.
 * hCallLegMgr         - handle to the call leg manager
 * hRegClientMgr       - handle to the reg client manager
 * hPubClientMgr       - handle to the pub client manager
 * hSubsMgr            - handle to the subscription manager
 * hTranscMgr          - handle to the transaction manager
 * hTransport          - handle to the transport
 * hMsgMgr             - handle to the message manager
 * hParserMgr          - handle to the parser manager
 * hAuthentication     - handle to the authenticator
 * hCompartmentMgr     - handle to the compartment manager
 * hSecAgreeMgr        - handle to the security agreement manager
 * hSecMgr             - handle to the security manager
 * hMessagePool        - handle to the message rpool
 * hGeneralPool        - handle to the general rpool
 * hElementPool         - handle to the smallpages rpool
 * supportedExtensionList - array of strings indicate option-tags which
 *                          are upported by the SIP Stack.
 * extensionListSize  - the size of supportedExtensionList array.
 * pfnPrintLogEntryEvHandler - application log callback function.
 * logContext          - application context for log callback function.
 * pSelect             - the stack select engine in use
 * seed                - random number generator.
 * pLogSrc             - pointer to the stack manager log source
 * pLogSourceArrey     - array with all stack and coure log sources.
 * logMgr              - The log manager.
 * pLogMgr             - Pointer to the log manager or NULL if compiled with #if (RV_LOGMASK != RV_LOGLEVEL_NONE).
 * bLogFileListenerEnabled - indicates if the stack is using the default common core log
 *                       file listener.
 * logFileListener - The log file listener pointer.
 ********************************************************************************************/
typedef struct {
    RvSipAppStackHandle           hAppStack;
#ifndef RV_SIP_PRIMITIVES
    RvSipCallLegMgrHandle         hCallLegMgr;
	SipTranscClientMgrHandle       hTranscClientMgr;
    RvSipRegClientMgrHandle       hRegClientMgr;
	RvSipPubClientMgrHandle       hPubClientMgr;
#ifdef RV_SIP_SUBS_ON
    RvSipSubsMgrHandle            hSubsMgr;
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* RV_SIP_PRIMITIVES */
    RvSipResolverMgrHandle        hRslvMgr;
    RvSipTransmitterMgrHandle     hTrxMgr;
#ifndef RV_SIP_LIGHT
    RvSipTranscMgrHandle          hTranscMgr;
#endif /*#ifndef RV_SIP_LIGHT*/
    RvSipTransportMgrHandle       hTransport;
    RvSipMsgMgrHandle             hMsgMgr;
    SipParserMgrHandle            hParserMgr;
#ifdef RV_SIP_AUTH_ON
    RvSipAuthenticatorHandle      hAuthentication;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIGCOMP_ON
    RvSipCompartmentMgrHandle     hCompartmentMgr;
    RvSigCompMgrHandle            hSigCompMgr;
#endif /* RV_SIGCOMP_ON */
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle             hSecMgr;
    RvSipSecAgreeMgrHandle        hSecAgreeMgr;
#endif /* RV_SIP_IMS_ON */

    HRPOOL                        hMessagePool;
    HRPOOL                        hGeneralPool;
    HRPOOL                        hElementPool;

#ifndef RV_SIP_PRIMITIVES
    RvChar**                      supportedExtensionList;
    RvInt32                       extensionListSize;
#endif

    RvSipStackPrintLogEntryEv     pfnPrintLogEntryEvHandler;
    void*                         logContext;
    RvSelectEngine*               pSelect;
    RvThread                      thread;
    RvRandomGenerator             seed;
    RvLogSource*                  pLogSrc;
    RvLogSource*                  pLogSourceArrey[STACK_NUM_OF_LOG_SOURCE_MODULES];
    RvLogMgr                      logMgr;
    RvLogMgr*                     pLogMgr;
    RvBool                        bLogFileListenerEnabled;
    RvLogListener                 logListener;
    RvBool                        bTerminalListenerEnabled;
}StackMgr;




#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _STACK_INTERNAL_H*/
