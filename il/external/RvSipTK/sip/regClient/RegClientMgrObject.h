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
 *                              <RegClientMgrObject.c>
 *     The register-client manager object holds all the register-client module
 *   resources Including memory pool, pools of lists, locks and more. The global
 *   Call-Id of this Register client within a re-boot is held in the register-
 *   client manager. The CSeq-Step counter is manage in the manager as well.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *********************************************************************************/

#ifndef REG_CLIENT_MGR_OBJECT_H
#define REG_CLIENT_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "rvmutex.h"
#include "RvSipRegClientTypes.h"
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


/* RegClientMgr
 * ---------------------------------------------------------------------------
 *   The register-client manager object holds all the register-client module
 *   resources Including memory pool, pools of lists, locks and more. The global
 *   Call-Id of this Register client within a re-boot is held in the register-
 *   client manager. The CSeq-Step counter is manage in the manager as well.
 */
typedef struct
{
    RvLogMgr*                 pLogMgr;
    RLIST_POOL_HANDLE         hRegClientListPool;    /* A pool of the register-clients list */
    RLIST_HANDLE              hRegClientList;        /* A list of all register-clients */
  /*    RLIST_POOL_HANDLE         hContactHeadersPool;   A pool of Contuct headers */
    HRPOOL                    hGeneralMemPool;       /* The stack's general memory pool */
    HRPOOL                    hMsgMemPool;           /* The stack's message memory pool */
    HPAGE                     hMemoryPage;           /* A memory page to be used for personal memory */
    RvInt32                   strCallId;             /* The register-client's Call-Id */
	SipCSeq					  cseq;		 			 /* The current CSeq-Step of register-client requests */
    RvMutex                   hLock;                 /* The managers lock */
    RvUint32                  maxNumOfRegClients;    /* The max number of register-clients at the same time*/
    RvSipRegClientEvHandlers  regClientEvHandlers;   /* The event handlers that will be set by the application
                                                        and called on relevant events */
	
	RvBool					  bHandleExpTimer;		 /*Indicate whether to start timers according to expires header*/
    RvLogSource*              pLogSrc;                 /* The module's log register id */
    RvSipTranscMgrHandle      hTranscMgr;            /* The transaction's manager */
    
	SipTranscClientMgrHandle  hTranscClientMgr;
	
	RvUint32                regClientAlertTimeOut; /*The time to alert before regClient Expires*/

    RvSipMsgMgrHandle         hMsgMgr;               /* The message's manager */
    RvRandomGenerator        *seed;
    void*                     hStack;                /* Handle to the stack instance */

#ifdef SIP_DEBUG
    RvChar                  *pCallId;
#endif
	RvSipTransportMgrHandle	 hTransportMgr;

} RegClientMgr;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_MGR_OBJECT_H */

