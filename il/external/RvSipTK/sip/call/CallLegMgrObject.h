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
 *                              <CallLegMgrObject.h>
 *     The call-leg manager object holds all the call-leg module resources
 *   Including memory pool, pools of lists, locks and more.
 *   Its main functionality is to manage the call-leg list, to create new
 *   call-legs and to associate new transaction to a specific call-leg according
 *   to the transaction key.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef CALL_LEG_MGR_OBJECT_H
#define CALL_LEG_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "rvmutex.h"
#include "rvselect.h"
#include "rvrandomgenerator.h"
#include "RvSipCallLegTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "_SipTransactionTypes.h"
#include "RvSipAuthenticator.h"
#include "AdsRlist.h"
#include "AdsRpool.h"
#include "AdsHash.h"
#include "CallLegTypes.h"
#include "_SipCommonTypes.h"
#include "RvSipSubscriptionTypes.h"

#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* CallLegMgr
 * ---------------------------------------------------------------------------
 * The call-leg manager object holds the call-leg module configuration and
 * resources handles including the list of call-legs, the module memory pool,
 * lock pool and more.
 *
 * hCallLegPool     -- Handle to a pool of call-leg lists. only one list is
 *                     allocated from this pool.
 * hCallLegList     -- Handle to the call-leg list.
 * hGeneralPool     -- Handle to the stack general pool. Each call-leg
 *                     takes one page from this pool when it is initialized
 * hTranscHandlesPool- Handle to a pool of transaction lists of handles. Each
 *                     call-leg takes one list from this pool to handle its
 *                     transactions.
 * hLockPool        -- Handle to a pool of locks. Each call-leg uses one lock.
 * hProcessLockPool -- Processing locks pool
 * hApiLockPool        -- API/Processing synchronization mutexes pool
 * hMutex           -- A lock used to lock the manager object to avoid threads
 *                     collisions.
 * maxNumOfCalls    -- Max number of call in the call-leg list.
 * maxNumOfTransc   -- Max number of transaction in the transaction module.
 * callLegEvHandlers - Structure with application event handler for the module
 *                     events
 * hLog             -- Handle to the log module. Used for registering new object
 *                     to the log module
 * pLogSrc            -- The module log-id. Used for printing messages.
 * hTranscMgr       -- Handle to the transaction module manager.
 * hTransportMgr    -- Handle to the transport module manager.
 * hMsgMgr          -- Handle to the message module manager.
 * pAppCallMgr      -- Pointer to the application's CallMgr object.
 * statusReplaces   -- The status of the Replaces header. Required/Supported/Undefined
 * sessionExpires   -- The time at which an element will consider the call timed
 *                     out, if no successful INVITE transaction occurs beforehand.
 *                     (default value 1800 seconds)
 * minSE            -- minimum value for the session interval the application
 *                     is willing to accept.(default value 0)
 * bAddReqMinSE     -- Indication for a min-SE that was set by the application,
 *                     thus has to be added to each og req.
 * manualSessionTimer- Specify whether the session time performed manually by the
 *                     application (RV_TRUE) or automatically by the call-leg
 *                     (RV_FALSE). Default value: RV_FALSE - the session time is
 *                     sent automatically.
 * manualBehavior   -- If this flag is on the stack allows the application to send
 *                      messages manually:
 *                   1. answer final response to the invite transaction after receiving cancel
 *                      to the invite (the stack accepts the cancel automatically).
 *                   2. When the call receives 2xx response to an invite message after cancelling it
 *                      the application will manually send ACK (in the manual ACK is true), and
 *                      the stack will not send BYE automatically.
 * enableInviteProceedingTimeoutState - if true, the call will reflect the transaction's
 *                                      proceeding state
 * bDisableRefer3515Behavior -- Indicates how to handle an incoming REFER request.
 *                      If this parameter is set to RV_TRUE, the old REFER callback functions
 *                      of the call-leg layer, will be called for every REFER states.
 *                      If this parameter is set to RV_FALSE, a new subscription will be created
 *                      for every incoming REFER, and the subscription callback functions
 *                      will be called.
 *
 * bEnableSubsForking -- If set to RV_FALSE, all Notifies, that don't match
 *                      existing dialog, will be not handled by Call-Leg Layer.
 *                      As a result, they will become general transaction.
 *                      If set to RV_TRUE, such Notifies will be passed by
 *                      Call-Leg Layer to Subscription Layer. The last one will
 *                      check, if they were received as a result of SUBSCRIBE
 *                      request forking, and if they do, it will create forked
 *                      Subscription and Call-Leg, which become owner
 *                      of the transaction.
 * hSecMgr            -- Handle to the Security Manager
 * hSecAgreeMgr       -- Handle to the Security Agreement Manager
 * terminateMask      -- Mask that defines in which callbacks we block the usage of RvSipCallLegTerminate() API.
 * cancelMask         -- Mask that defines in which callbacks we block the usage of RvSipCallLegCancel() API.
 */
typedef struct
{
    RvLogMgr*                pLogMgr;
    RLIST_POOL_HANDLE        hCallLegPool;
    RLIST_POOL_HANDLE        hInviteObjsListPool;
    RLIST_HANDLE             hCallLegList;
    HASH_HANDLE              hHashTable;
    RvInt32                  hashSize;
    HRPOOL                   hGeneralPool;
    HRPOOL                   hMsgMemPool;
    HRPOOL                   hElementPool;
    RLIST_POOL_HANDLE        hTranscHandlesPool;
    RvMutex                  hMutex;
    RvInt32                  maxNumOfCalls;
    RvInt32                  numOfAllocatedCalls;
    RvInt32                  maxNumOfHiddenCalls;
    RvInt32                  numOfAllocatedHiddenCalls;
    RvInt32                  maxNumOfTransc;
    RvSipCallLegEvHandlers   callLegEvHandlers;
    RvLogSource*             pLogSrc;
    RvSipTranscMgrHandle     hTranscMgr;
    RvSipTransportMgrHandle  hTransportMgr;
    RvSipMsgMgrHandle        hMsgMgr;
    RvSipTransmitterMgrHandle hTrxMgr;
    void                     *pAppCallMgr;
    RvRandomGenerator        *seed;
    void*                    hStack;

    RvSipAuthenticatorHandle hAuthMgr;
#ifdef RV_SIP_SUBS_ON
    RvSipSubsMgrHandle       hSubsMgr;
#endif /* #ifdef RV_SIP_SUBS_ON */
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle        hSecMgr;
    RvSipSecAgreeMgrHandle   hSecAgreeMgr;
#endif /* RV_SIP_IMS_ON */
	RvBool                   manualAckOn2xx;
    RvBool                   manualPrack;
    RvBool                   manualBehavior;
    RvSipCallLegReplacesStatus statusReplaces;
    /* Session Timer parameters */
    RvBool                   manualSessionTimer;
    RvInt32                  sessionExpires;
    RvInt32                  MinSE;
    RvBool                   bAddReqMinSE;
    RvBool                   addSupportedListToMsg;
    CallLegSessionTimerStatus eSessiontimeStatus;
    RvBool                   bIsPersistent;
    /* patch for Customer */
    RvBool                   bEnableNestedInitialRequestHandling;
    RvBool                   enableInviteProceedingTimeoutState;

    /*new parameters for version 3.1*/
    RvBool                     bEnableForking;
    RvSipTransmitterEvHandlers trxEvHandlers;
    RvInt32                    ackTrxTimerTimeout;
    RvInt32                    forked1xxTimerTimeout;

    RvSelectEngine            *pSelect;
	RvBool                     bDisableRefer3515Behavior;
#ifdef RV_SIP_SUBS_ON
    RvBool                     bEnableSubsForking;
#endif /* #ifdef RV_SIP_SUBS_ON */
    
    /* new parameters for version 4.0 */
    RvInt32                    inviteLingerTimeout;
    RvBool                     bOldInviteHandling; /*Ack on 2xx is part of the tranasction*/
    
    /* new parameters for version 5.0 */
    RvInt32                    terminateMask;
    RvInt32                    cancelMask;
} CallLegMgr;




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * CallLegMgrCreateCallLeg
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
 *          isHidden   - is this is a hidden call-leg (for independent subscription)
 *                       or not.
 * Output:     phCallLeg  - sip stack handle to the new call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrCreateCallLeg(
                                        IN    CallLegMgr       *pMgr,
                                        IN    RvSipCallLegDirection eDirection,
                                        IN    RvBool                 isHidden,
                                        OUT   RvSipCallLegHandle *phCallLeg);


/***************************************************************************
* CallLegMgrNotifyCallLegCreated
* ------------------------------------------------------------------------
* General: Notifies the application/subscrition module, that a new call-leg
*          was created.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr       - Pointer to the call-leg manager.
*            hCallLeg   - The newly created call-leg
*               hTransc  - The transaction that caused this call leg to be created.
***************************************************************************/
RvStatus CallLegMgrNotifyCallLegCreated(IN  CallLegMgr*        pMgr,
                                      IN  RvSipCallLegHandle    hCallLeg,
                                      IN  RvSipTranscHandle     hTransc);

/***************************************************************************
 * CallLegMgrHashInit
 * ------------------------------------------------------------------------
 * General: Initialize the call-leg hash table. the number of entries is
 *          maxCallLegs*2+1.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to construct hash table
 *               RV_OK - hash table was constructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       The call-leg manager.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrHashInit(CallLegMgr *pMgr);


/***************************************************************************
 * CallLegMgrHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a call-leg into the hash table.
 *          The key is generated from the call-leg information and the
 *          call-leg handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to construct hash table
 *               RV_OK - Call-leg handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrHashInsert(IN RvSipCallLegHandle hCallLeg);

/***************************************************************************
 * CallLegMgrHashFind
 * ------------------------------------------------------------------------
 * General: find a call-leg in the hash table acording to the call-leg key.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr -       Handle of the call-leg manager
 *          pKey -       The call-leg key
 *          eDirection - the direction of the call.
 *          bOnlyEstablishedCall - Indicates if we search only call-legs with
 *                       to-tag, or also call-leg without to-tag.
 * Output:  phCallLeg -  Handle to the call-leg found in the hash or null
 *                       if the call-leg was not found.
 ***************************************************************************/
void RVCALLCONV CallLegMgrHashFind(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eDirection,
                           IN  RvBool                 bOnlyEstablishedCall,
                           OUT RvSipCallLegHandle     **phCallLeg);


/***************************************************************************
 * CallLegMgrHashFindReplaces
 * ------------------------------------------------------------------------
 * General: find a call-leg in the hash table acording to the Replaces header parameters.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr                   - Handle of the call-leg manager
 *          hReplacesHeader        - Handle to the Replaces header.
 * Output:  phCallLeg              - Handle to the call-leg found in the hash or null
 *                                   if the call-leg was not found.
 *          peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog doesn't fit.
 *                     application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces.
 ***************************************************************************/
void RVCALLCONV CallLegMgrHashFindReplaces(
                           IN  CallLegMgr                *pMgr,
                           IN  RvSipReplacesHeaderHandle  hReplacesHeader,
                           OUT RvSipCallLegHandle       **phCallLeg,
                           OUT RvSipCallLegReplacesReason *peReason);

/***************************************************************************
 * CallLegMgrFindCallLegByMsg
 * ------------------------------------------------------------------------
 * General: The function search for a call-leg with same identifiers as in
 *          the given message (1xx, 2xx or ACK)
 * Return Value: RV_Status - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr   - The transaction manager handle.
 *          hAckMsg      - The handle to the received ACK message.
 * Output:  pbWasHandled - Indicates if the message was handled in the
 *                         callback.
 *          pCurIdentifier - The call-leg identifier before unlocking the call mgr.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrFindCallLegByMsg(
                     IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hMsg,
                     IN   RvSipCallLegDirection   eDirection,
                     IN   RvBool                  bOnlyEstablishedCall,
                     OUT  RvSipCallLegHandle     *phCallLeg,
                     OUT  RvInt32*               pCurIdentifier);

/******************************************************************************
 * CallLegMgrHashFindOriginal
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Original.
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
void RVCALLCONV CallLegMgrHashFindOriginal(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eKeyDirection,
                           OUT RvSipCallLegHandle     **phCallLeg);

/******************************************************************************
 * CallLegMgrHashFindHidden
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Hidden.
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
void RVCALLCONV CallLegMgrHashFindHidden(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eKeyDirection,
                           OUT RvSipCallLegHandle     **phCallLeg);

/******************************************************************************
 * CallLegMgrHashFindOriginalHidden
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Hidden and Original.
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
void RVCALLCONV CallLegMgrHashFindOriginalHidden(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eKeyDirection,
                           OUT RvSipCallLegHandle     **phCallLeg);

/******************************************************************************
 * CallLegMgrCheckObjCounters
 * ----------------------------------------------------------------------------
 * General: Checks if the CallLeg Mgr calls counters are valid, before
 *          a construction of new CallLeg or after a destruction of a CallLeg.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Handle of the Call-Leg Manager.
 *          bIsHidden   - Indicates if the constructed/destructed CallLeg
 *                        is hidden.
 *          bIncrement  - Indicates if the counters examination refers to
 *                        CallLeg construction(RV_TRUE)/destruction(RV_FALSE).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegMgrCheckObjCounters(
                                    IN  CallLegMgr *pMgr,
                                    IN  RvBool      bIsHidden,
                                    IN  RvBool      bIncrement);

/******************************************************************************
 * CallLegMgrUpdateObjCounters
 * ----------------------------------------------------------------------------
 * General: Updates the CallLeg Mgr calls counters are valid, after
 *          a construction of new CallLeg or after a destruction of a CallLeg.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Handle of the Call-Leg Manager.
 *          bIsHidden   - Indicates if the constructed/destructed CallLeg
 *                        is hidden.
 *          bIncrement  - Indicates if the counters eximination refers to
 *                        CallLeg construction(RV_TRUE)/destruction(RV_FALSE).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegMgrUpdateObjCounters(
                                    IN  CallLegMgr *pMgr,
                                    IN  RvBool      bIsHidden,
                                    IN  RvBool      bIncrement);

/******************************************************************************
 * CallLegMgrGetSubsMgr
 * ----------------------------------------------------------------------------
 * General: Retrieves the call-leg's mgr handle of subs mgr
 * Return Value: The call-leg's mgr handle of subs mgr.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr  - Handle of the Call-Leg Manager.
 *****************************************************************************/
RvSipSubsMgrHandle RVCALLCONV CallLegMgrGetSubsMgr(IN  CallLegMgr *pMgr);

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_MGR_OBJECT_H */
