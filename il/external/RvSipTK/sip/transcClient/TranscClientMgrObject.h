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
 *                              <TranscClientMgrObject.c>
 *     The Transc-client manager object holds all the register-client and publish-client module
 *   resources Including memory pool, pools of lists, locks and more. The global
 *   Call-Id of this Transaction client within a re-boot is held in the transaction-
 *   client manager. 
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/

#ifndef TRANSC_CLIENT_MGR_OBJECT_H
#define TRANSC_CLIENT_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "rvmutex.h"
#include "_SipTranscClientTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "AdsRlist.h"
#include "AdsRpool.h"
#include "RvSipAuthenticator.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/* TranscClientMgr
 * ---------------------------------------------------------------------------
 *   The transaction-client manager object holds all the transaction-clients module
 *   resources Including memory pool, pools of lists, locks and more such objects can be
 *   Reg-client and Pub-clients. 
 *   The global Call-Id of this Transc client within a re-boot is held in the transaction-
 *   client manager. 
 */
typedef struct
{
    RvLogMgr*                 pLogMgr;
    HRPOOL                    hGeneralMemPool;       /* The stack's general memory pool */
    HRPOOL                    hMsgMemPool;           /* The stack's message memory pool */
    HPAGE                     hMemoryPage;           /* A memory page to be used for personal memory */
	RvMutex                   hLock;                 /* The managers lock */
    RvLogSource*              pLogSrc;                 /* The module's log register id */
    RvSipTranscMgrHandle      hTranscMgr;            /* The transaction's manager */
    RvSipTransportMgrHandle      hTransportMgr;         /* The transport manager */
    RvSipMsgMgrHandle         hMsgMgr;               /* The message's manager */
    RvRandomGenerator        *seed;
    void*                     hStack;                /* Handle to the stack instance */
    RvSipAuthenticatorHandle  hAuthModule;           /* The authentication module */
    RvBool                   bIsPersistent;

#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle        hSecMgr;
    RvSipSecAgreeMgrHandle   hSecAgreeMgr;            /* Handle to the security-agreement manager */
#endif /* RV_SIP_IMS_ON */

	RvSelectEngine        *pSelect;		/*The select engine is used for alert timer.*/
} TranscClientMgr;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_MGR_OBJECT_H */






















