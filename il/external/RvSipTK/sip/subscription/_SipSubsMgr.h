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
 *                              <_SipSubsMgrObject.h>
 *
 *     The subscription manager object holds all the event notification module
 *   configuration parameters, and a call-leg manager handle.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SIP_SUBS_MGR_OBJECT_H
#define SIP_SUBS_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipSubscriptionTypes.h"
#include "_SipCommonUtils.h"
#include "RvSipCallLegTypes.h"
#include "RvSipResourcesTypes.h"
#include "_SipTransactionTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
typedef struct
{
    RvLogMgr*                 pLogMgr;
    RvLogSource*              pLogSrc;
    RvSipMsgMgrHandle         hMsgMgr;
    RvSipCallLegMgrHandle     hCallLegMgr;
    RvSipTransportMgrHandle   hTransportMgr;
    RvSipTranscMgrHandle      hTransactionMgr;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hElementPool;
    RvBool                 autoRefresh;
    RvInt32                alertTimeout;
    RvInt32                noNotifyTimeout; /* if 0 - not setting noNotify timer */
    RvInt32                maxNumOfSubscriptions;
    void                    *hStack;
    RvRandomGenerator       *seed;
   /*new parameters for version 3.1*/
    RvBool                 bDisableRefer3515Behavior;

} SipSubsMgrCfg;




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipSubsMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new subscription manager.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the call-leg mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   call-leg manager.
 *               RV_OK -        Call-leg manager was created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     cfg - struct with configuration parameters.
 * Output:     *phSubsMgr - Handle to the newly created subs manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrConstruct(
                            IN  SipSubsMgrCfg          *cfg,
                            OUT RvSipSubsMgrHandle     *phSubsMgr);

/***************************************************************************
 * SipSubsMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the subscription manager freeing all resources.
 * Return Value:  RV_OK -  subscription manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubsMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrDestruct(
                                   IN RvSipSubsMgrHandle hSubsMgr);
/***************************************************************************
 * SipSubsMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the subscription module. It includes the subscription list, the
 *          notification lists and the list items.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrGetResourcesStatus (
                                 IN  RvSipSubsMgrHandle   hMgr,
                                 OUT RvSipSubsResources  *pResources);

/***************************************************************************
 * SipSubsMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of subscriptions that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The subscription manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrResetMaxUsageResourcesStatus (
			                         IN  RvSipSubsMgrHandle  hMgr);

/***************************************************************************
 * SipSubsMgrSubscriptionCreate
 * ------------------------------------------------------------------------
 * General: Creates a new subscription (regular subscriber subscription, or
 *          out of band subscription - subscriber/notifier):
 *          1. create subscription object.
 *          2. create a hidden call-leg, if needed.
 *          3. initiate subscription and set it in call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr      - Pointer to the subs manager.
 *          hCallLeg  - Handle to a connected call-leg, or NULL - to create a
 *                      hidden call-leg.
 *          hAppsubs   - Handle to the application subs object.
 *          eSubsType  - Subscriber or Notifier.
 *          bOutOfBand - a regular subscription, or out of band subscription.
 * Output:  phSubs     - sip stack handle to the new subscription
 ***************************************************************************/
RvStatus SipSubsMgrSubscriptionCreate( IN    RvSipSubsMgrHandle    hMgr,
                                       IN    RvSipCallLegHandle    hCallLeg,
                                       IN    RvSipAppSubsHandle    hAppSubs,
                                       IN    RvSipSubscriptionType eSubsType,
                                       IN    RvBool                bOutOfBand,
                                       OUT   RvSipSubsHandle      *phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                       );

/***************************************************************************
 * SipSubsMgrSubsTranscCreated
 * ------------------------------------------------------------------------
 * General: When a new SUBSCRIBE request is received the callLeg manager find the
 *          related call-leg (or create a hidden one),
 *          and then calls this function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr   - Handle to the subs manager
 *            hCallLeg   - Handle to the subscription call-leg.
 *            hTransc    - Handle to the new transaction.
 *            eMethod    - The method of the new transaction (SUBSCRIBE/REFER/NOTIFY)
 * Output:    pRejectCode - the reject-code to use, in case of transaction rejection.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrSubsTranscCreated(IN RvSipSubsMgrHandle     hSubsMgr,
                                                IN RvSipCallLegHandle     hCallLeg,
                                                IN RvSipTranscHandle      hTransc,
                                                IN SipTransactionMethod   eMethod,
                                                OUT RvUint16*             pRejectCode);

/***************************************************************************
 * SipSubsMgrNotifyTranscCreated
 * ------------------------------------------------------------------------
 * General: When a new NOTIFY request is received the callLeg manager find the
 *          related call-leg, and then calls this function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr   - Handle to the subs manager
 *            hCallLeg   - Handle to the subscription call-leg.
 *          hTransc    - Handle to the NOTIFY transaction.
 * Output:     bWasHandled - Was the NOTIFY transaction handled.
 ***************************************************************************/
RvStatus RVCALLCONV SipSubsMgrNotifyTranscCreated(
                                    IN    RvSipSubsMgrHandle     hSubsMgr,
                                    IN    RvSipCallLegHandle     hCallLeg,
                                    IN    RvSipTranscHandle      hTransc,
                                    OUT   RvBool                *bWasHandled);

/******************************************************************************
 * SipSubsMgrHandleUnmatchedNotify
 * ----------------------------------------------------------------------------
 * General: The function is called by Call-Leg Layer, when it handles NOTIFY,
 *          which doesn't match any existing Call-Leg. It checks, if the NOTIFY
 *          doesn't match any Call-Leg because it was sent in response to
 *          forked SUBSCRIBE with TO-tag, different from TO-tag of existing
 *          Call-Legs.
 *          See RFC 3265 sections 3.3.3 and 4.4.9 for Forked Subscriptions.
 *          If it indeed was sent in response to forked SUBSCRIBE, mapped to 
 *          existing Subscription (let's call it 'Original Subscription'),
 *          the function will create forked Subscription with underlying
 *          Call-Leg in state 2xx received, and will return handle of this
 *          hidden Call-Leg back to Call-Leg Layer. Call-Leg Layer will set
 *          this Call-Leg to be owner of the NOTIFY. As a result, the NOTIFY
 *          will be processed as a regular NOTIFY, arrived for existing
 *          Subscription on existing Call-Leg.
 *                  
 * Return Value: RV_OK on successful finish,
 *               error code, defined in RV_SIP_DEF.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr  - handle to Subscription Manager.
 *            hTtrans   - handle to Transaction, which handles incoming NOTIFY.
 * Output:    phCallLeg - pointer to the memory, where handle of the created
 *                        by function Call-Leg will be stored.
 *            pRejectStatus-pointer to the memory, where code of reject for 
 *                        incoming NOTIFY will be stored by the function.
 *****************************************************************************/
RvStatus SipSubsMgrHandleUnmatchedNotify(
                                IN    RvSipSubsMgrHandle     hSubsMgr,
                                IN    RvSipTranscHandle      hTransc,
                                OUT   RvSipCallLegHandle     *phCallLeg,
                                OUT   RvUint16               *pRejectStatus);

#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_SUBS_MGR_OBJECT_H */

