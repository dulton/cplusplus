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
 *                              <PubClientMgrObject.c>
 *     The publish-client manager object holds all the publish-client module
 *   resources Including memory pool, pools of lists, locks and more. The global
 *   Call-Id of this Publish client within a re-boot is held in the publish-
 *   client manager. The CSeq-Step counter is manage in the manager as well.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/

#ifndef PUB_CLIENT_MGR_OBJECT_H
#define PUB_CLIENT_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "rvmutex.h"
#include "RvSipPubClientTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "_SipTranscClientTypes.h"
#include "AdsRlist.h"
#include "AdsRpool.h"
#include "RvSipAuthenticator.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/* PubClientMgr
 * ---------------------------------------------------------------------------
 *   The publish-client manager object holds all the publish-client module
 *   resources Including memory pool, pools of lists, locks and more. The global
 *   Call-Id of this Publish client within a re-boot is held in the publish-
 *   client manager. The CSeq-Step counter is manage in the manager as well.
 */
typedef struct
{
    RvLogMgr*                 pLogMgr;
    RLIST_POOL_HANDLE         hPubClientListPool;    /* A pool of the publish-clients list */
    RLIST_HANDLE              hPubClientList;        /* A list of all publish-clients */
    HRPOOL                    hGeneralMemPool;       /* The stack's general memory pool */
    HRPOOL                    hMsgMemPool;           /* The stack's message memory pool */
    HPAGE                     hMemoryPage;           /* A memory page to be used for personal memory */
    SipCSeq					  cseq;		 			 /* The current CSeq-Step of publish-client requests */
    RvMutex                   hLock;                 /* The managers lock */
    RvUint32                  maxNumOfPubClients;    /* The max number of publish-clients at the same time*/
    RvSipPubClientEvHandlers  pubClientEvHandlers;   /* The event handlers that will be set by the application
                                                        and called on relevant events */
	
	RvBool					  bHandleExpTimer;		 /*Indicate whether to start timers according to expires header*/
	
	RvUint32                   pubClientAlertTimeOut; /*The time alert before expiration.*/
    RvLogSource*              pLogSrc;                 /* The module's log publish id */
    RvSipTranscMgrHandle      hTranscMgr;            /* The transaction's manager */
    
	SipTranscClientMgrHandle  hTranscClientMgr;

    RvSipMsgMgrHandle         hMsgMgr;               /* The message's manager */
    RvRandomGenerator        *seed;
    void*                     hStack;                /* Handle to the stack instance */

	RvSipTransportMgrHandle	 hTransportMgr;

} PubClientMgr;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef PUB_CLIENT_MGR_OBJECT_H */

