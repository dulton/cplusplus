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
 *                              <SubsMgrObject.h>
 *     The subscription manager object holds all the event notification module
 *   configuration parameters, and a call-leg manager handle.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SUBS_MGR_OBJECT_H
#define SUBS_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON

#include "rvmutex.h"
#include "rvselect.h"
#include "SubsObject.h"
#include "RvSipSubscriptionTypes.h"
#include "_SipCommonUtils.h"
#include "RvSipCallLegTypes.h"
#include "RvSipTransportTypes.h"
#include "AdsRlist.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define NOTIFY_PER_SUBSCRIPTION 5

/* SubsMgr
 * ---------------------------------------------------------------------------
 * The subscription manager object holds the eventNotification module configuration and
 * resources handles. most parameters can be access via call-leg manager handle.
 *
 * hCallLegMgr         -- Handle to a call-leg manager.
 * subsEvHandlers      -- Structure with application event handler for the module
 *                        events
 * hLog                -- Handle to the log module. Used for registering new object
 *                        to the log module
 * pLogSrc               -- The module log-id. Used for printing messages.
 * autoRefresh         -- AutoRefresh mode - sends automatic refresh request when alert is given.
 * alertTimeout        -- Value for the alert timer.
 * noNotifyTimeout     -- Value for the no-notify timer. value 0 means that this timer should not
 *                        be set. the timer is set when receiving 2xx response for a subscribe
 *                        request. the timer will be release when receiving first notify
 *                        request.
 * hStack              -- A handle to the stack instance.
 * pAppSubsMgr         -- A handle to the application subs maneger.
 * bDisableRefer3515Behavior -- Indicates how to handle an incoming REFER request.
 *                      If this parameter is set to RV_TRUE, the old REFER callback functions
 *                      of the call-leg layer, will be called for every REFER states.
 *                      If this parameter is set to RV_FALSE, a new subscription will be created
 *                      for every incoming REFER, and the subscription callback functions
 *                      will be called.
 * terminateMask       -- Mask that defines in which callbacks we block the usage of RvSipSubsTerminate() API.
 */
typedef struct SubsMgrStruct
{
    RvLogMgr*              pLogMgr;
    RvSipCallLegMgrHandle  hCallLegMgr;
    RvSipTransportMgrHandle hTransportMgr;
    RvSipMsgMgrHandle      hMsgMgr;
    RvSipTranscMgrHandle   hTransactionMgr;

    RvMutex                hMutex;
    HRPOOL                 hGeneralPool;
    HRPOOL                 hElementPool;
    RvSipSubsEvHandlers    subsEvHandlers;

    RvLogSource*           pLogSrc;
    RvRandomGenerator     *seed;

    RvInt32               maxNumOfSubs;
    RLIST_POOL_HANDLE      hSubsListPool;
    RLIST_HANDLE           hSubsList;
    RLIST_POOL_HANDLE      hNotifyListPool;
    RLIST_POOL_HANDLE      hTranscHandlesPool;

    RvBool                autoRefresh;
    RvInt32               alertTimeout;
    RvInt32               noNotifyTimeout; /* if 0 - not setting noNotify timer */

    void                  *hStack;
    void                  *pAppSubsMgr;
    RvBool                bDisableRefer3515Behavior;

    RvSelectEngine        *pSelect;
    /* new parameters for version 5.0 */
    RvInt32               terminateMask;
} SubsMgr;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsMgrSubscriptionCreate
 * ------------------------------------------------------------------------
 * General: Creates a new subscription (regular subscriber subscription, or
 *          out of band subscription - subscriber/notifier):
 *          1. create subscription object.
 *          2. create a hidden call-leg, if needed.
 *          3. initiate subscription and set it in call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr      - Pointer to the subs manager.
 *          hCallLeg  - Handle to a connected call-leg, or NULL - to create a
 *                      hidden call-leg.
 *          hAppsubs   - Handle to the application subs object.
 *            eSubsType  - Subscriber or Notifier.
 *          bOutOfBand - a regular subscription, or out of band subscription.
 * Output:     phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
RvStatus SubsMgrSubscriptionCreate(    IN    RvSipSubsMgrHandle    hMgr,
                                        IN    RvSipCallLegHandle    hCallLeg,
                                        IN    RvSipAppSubsHandle    hAppSubs,
                                        IN    RvSipSubscriptionType eSubsType,
                                        IN    RvBool               bOutOfBand,
                                        OUT   RvSipSubsHandle      *phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                        );

/***************************************************************************
 * SubsMgrSubscriptionObjCreate
 * ------------------------------------------------------------------------
 * General: Internal function. Creates a new subscription object.
 *          The call-leg was already created before calling to this function.
 *          1. create subscription in subs manager subscriptions list.
 *          2. initialize the subscription structure, and set triple lock in it.
 *          3. set the subscription in the call-leg subscriptions list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr       - pointer to the subs manager object
 *          hCallLeg   - Handle to the subscription call-leg (already exists!).
 *          hAppSubs   - Handle to the application subscription
 *          eSubsType  - Subscription type (notifier or subscriber)
 *          bOutOfBand - Is the subscription out-of-band or not.
 * Output:  phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
 RvStatus RVCALLCONV SubsMgrSubscriptionObjCreate(
                                        IN    SubsMgr*              pMgr,
                                        IN    RvSipCallLegHandle    hCallLeg,
                                        IN    RvSipAppSubsHandle    hAppSubs,
                                        IN    RvSipSubscriptionType eSubsType,
                                        IN    RvBool               bOutOfBand,
                                        OUT   RvSipSubsHandle      *phSubs);

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_MGR_OBJECT_H */


