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
 *                              <SipCallLegMgr.h>
 *
 *  The SipCallLegMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the Call-leg module and getting the module
 *  resource status.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/


#ifndef SIP_CALL_LEG_MGR_H
#define SIP_CALL_LEG_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipResourcesTypes.h"
#include "RvSipCallLegTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "_SipTransactionTypes.h"
#include "AdsRlist.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvLogMgr*                 pLogMgr;
    RvLogSource*              moduleLogId;
    RvSipTransportMgrHandle   hTransport;
    RvSipTranscMgrHandle      hTranscMgr;
    RvSipMsgMgrHandle         hMsgMgr;
    RvSipTransmitterMgrHandle hTrxMgr;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    HRPOOL                    hElementPool;
    RvInt32                   maxNumOfCalls;
    RvInt32                   maxNumOfTransc;
    RvRandomGenerator*        seed;
    RvSipAuthenticatorHandle hAuthMgr;
    RvBool                   manualAckOn2xx;
    RvBool                   manualPrack;
    RvBool                   manualSessionTimer;
    RvInt32                  sessionExpires;
    RvInt32                  MinSE;
    RvChar**                 supportedExtensionList;
    RvInt32                  extensionListSize;
    RvBool                   addSupportedListToMsg;
    RvBool                   manualBehavior;
    RvBool                   bIsPersistent;
    RvBool                   enableInviteProceedingTimeoutState; /* reflects the transaction's
                                                                     ProceedingTimeout State */
#ifdef RV_SIP_SUBS_ON
    RvInt32                 maxSubscriptions;
    /*new SUBS-REFER parameters for version 3.1*/
    RvBool                  bEnableSubsForking;
    RvBool                  bDisableRefer3515Behavior;
#endif /* #ifdef RV_SIP_SUBS_ON */

   /*new parameters for version 3.1*/
    RvBool                  bEnableForking;
    RvInt32                 forkedAckTrxTimeout;
    RvInt32                 forked1xxTimerTimeout;
    
    /* new for version 4.0 */
    RvInt32                 inviteLingerTimeout;
    RvBool                  bOldInviteHandling;
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle       hSecMgr;
#endif

} SipCallLegMgrCfg;

/***************************************************************************
 * SipCallLegMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new call-leg manager. The call-leg manager is
 *          encharged of all the calls-legs. The manager holds a list of
 *          call-legs and is used to create new call-legs.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the call-leg mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   call-leg manager.
 *               RV_OK -        Call-leg manager was created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog          - Handle to the log module.
 *            moduleLogId   - The module log id used for printing.
 *          hTransport    - The transport manager handle.
 *            hTranscMgr    - The transaction manager handle.
 *          hAuthMgr      - The authentication manager handle
 *          hGeneralPool  - Pool used bye each call-leg for internal memory
 *                          allocations.
 *            maxNumOfCalls - Max number of concurrent calls.
 *            maxNumOfTransc -Max number of concurrent transactions.
 *          hStack        - A handle to the stack manager.
 * Output:     *phCallLegMgr - Handle to the newly created call leg manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrConstruct(
                            IN  SipCallLegMgrCfg          *pCfg,
                            IN  void                      *hStack,
                            OUT RvSipCallLegMgrHandle     *phCallLegMgr);


/***************************************************************************
 * SipCallLegMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the call-leg manager freeing all resources.
 * Return Value:  RV_OK -  Call-leg manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLegMgr - Handle to the manager to destruct.
 ***************************************************************************/

RvStatus RVCALLCONV SipCallLegMgrDestruct(
                                           IN RvSipCallLegMgrHandle hCallLegMgr);

/***************************************************************************
 * SipCallLegMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the call module. It includes the call-leg list, the transactions
 *          handles lists and the list items.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrGetResourcesStatus (
                                 IN  RvSipCallLegMgrHandle  hMgr,
                                 OUT RvSipCallLegResources  *pResources);

/***************************************************************************
 * SipCallLegMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of call-legs that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLegMgr - The call-leg manager.
 ***************************************************************************/
void RVCALLCONV SipCallLegMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipCallLegMgrHandle  hCallLegMgr);

/***************************************************************************
 * SipCallLegMgrCreateCallLeg
 * ------------------------------------------------------------------------
 * General: Creates a new Outgoing call-leg and replace handles with the
 *          application.  The new call-leg assumes the Idle state.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create a new call-leg. Not
 *                                   enough resources.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - Handle to the call-leg manager
 *            eDirection - Direction (incoming/outgoing) of the new callLeg.
 *          bIsHidden   - is this is a hidden call-leg (for independent subscription)
 *                       or not.
 * Output:     phCallLeg  - sip stack handle to the new call-leg
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrCreateCallLeg(
                                        IN    RvSipCallLegMgrHandle hMgr,
                                        IN    RvSipCallLegDirection eDirection,
                                        IN    RvBool               bIsHidden,
                                        OUT   RvSipCallLegHandle   *phCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                        );

/***************************************************************************
 * SipCallLegMgrGetCallLegPool
 * ------------------------------------------------------------------------
 * General: Returns the call-leg pool handle.
 * Return Value: handle to the call-leg pool.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 ***************************************************************************/
RLIST_POOL_HANDLE RVCALLCONV SipCallLegMgrGetCallLegPool(IN  RvSipCallLegMgrHandle  hMgr);

/***************************************************************************
 * SipCallLegMgrGetCallLegList
 * ------------------------------------------------------------------------
 * General: Returns the call-leg list handle.
 * Return Value: handle to the call-leg list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 ***************************************************************************/
RLIST_HANDLE RVCALLCONV SipCallLegMgrGetCallLegList(IN  RvSipCallLegMgrHandle  hMgr);

/***************************************************************************
 * SipCallLegMgrSetSubsMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the subsMgr handle in call-leg mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr     - Handle to the call-leg manager.
 *          hSubsMgr - Handle to the subs manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrSetSubsMgrHandle(
                                        IN RvSipCallLegMgrHandle hCallLegMgr,
                                        IN RvSipSubsMgrHandle    hSubsMgr);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipCallLegMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in call-leg mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the call-leg manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrSetSecAgreeMgrHandle(
                                        IN RvSipCallLegMgrHandle     hCallLegMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

/***************************************************************************
 * SipCallLegMgrGetReplacesStatus
 * ------------------------------------------------------------------------
 * General: Get the replaces status of a Call-Leg mgr.
 * Return Value: RvSipCallLegReplacesStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 ***************************************************************************/
RvSipCallLegReplacesStatus RVCALLCONV SipCallLegMgrGetReplacesStatus(
                                            IN RvSipCallLegMgrHandle hCallLegMgr);

/******************************************************************************
 * SipCallLegMgrFindHiddenCallLeg
 * ----------------------------------------------------------------------------
 * General: search Call-Leg Manager database for Hidden Call-Leg with dialog
 *          identifiers, equal to those,supplied by Transaction Key parameter.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrFindHiddenCallLeg(
                                    IN  RvSipCallLegMgrHandle   hMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  RvSipCallLegDirection   eKeyDirection,
                                    OUT RvSipCallLegHandle      *phCallLeg);

/******************************************************************************
 * SipCallLegMgrFindOriginalHiddenCallLeg
 * ----------------------------------------------------------------------------
 * General: search Call-Leg Manager database for Hidden and Original Call-Leg
 *          with dialog identifiers, equal to those,supplied by Transaction Key
 *          parameter.
 *          Call-Leg is Hidden, if it serves out-of-dialog Subscription.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipCallLegMgrFindOriginalHiddenCallLeg(
                                    IN  RvSipCallLegMgrHandle   hMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  RvSipCallLegDirection   eKeyDirection,
                                    OUT RvSipCallLegHandle      *phCallLeg);

/******************************************************************************
 * SipCallLegMgrHashInsert
 * ----------------------------------------------------------------------------
 * General: Insert a Call-Leg into the Call-Leg Manager hash table.
 *          The key is generated in according to the data in Call-Leg object.
 * Return Value: RV_OK on success,
 *          error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 *****************************************************************************/
RvStatus SipCallLegMgrHashInsert(IN RvSipCallLegHandle hCallLeg);

/******************************************************************************
 * SipCallLegMgrLock
 * ----------------------------------------------------------------------------
 * General: locks Call-Leg Manager.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMgr - Handle to the Call-Leg Manager.
 * Output:  none.
 *****************************************************************************/
void SipCallLegMgrLock(IN RvSipCallLegMgrHandle hMgr);

/******************************************************************************
 * SipCallLegMgrUnlock
 * ----------------------------------------------------------------------------
 * General: unlocks Call-Leg Manager.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMgr - Handle to the Call-Leg Manager.
 * Output:  none.
 *****************************************************************************/
void SipCallLegMgrUnlock(IN RvSipCallLegMgrHandle hMgr);

#endif /* RV_SIP_PRIMITIVES */
#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_CALL_LEG_MGR_H */



