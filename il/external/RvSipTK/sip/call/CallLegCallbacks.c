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
 *                              <CallLegCallbacks.c>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/


#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "CallLegCallbacks.h"
#include "_SipCommonUtils.h"
#include "CallLegRefer.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV ConfirmCallbackCallLeg(IN CallLeg       *pCallLeg,
                                                  IN const RvChar  *callbackID);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           CALL CALLBACKS                              */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the call-leg manager.
*         hCallLeg    - The new sip stack call-leg handle
* Output:(-)
***************************************************************************/
void RVCALLCONV CallLegCallbackCreatedEv(IN  CallLegMgr                *pMgr,
                                         IN  RvSipCallLegHandle        hCallLeg)
{
    RvInt32                  nestedLock = 0;
    CallLeg                 *pCallLeg  = (CallLeg *)hCallLeg;
    RvSipAppCallLegHandle    hAppCall   = NULL;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;
    RvSipCallLegEvHandlers  *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    pTripleLock = pCallLeg->tripleLock;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "CallLegCallbackCreatedEv - call leg mgr notifies the application of a new call-leg 0x%p",
        pCallLeg));

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnCallLegCreatedEvHandler != NULL)
    {
		OBJ_ENTER_CB(pCallLeg, CB_CALL_CALL_CREATED);
        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegCallbackCreatedEv - call-leg 0x%p, Before callback",
                  pCallLeg));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnCallLegCreatedEvHandler(
                                        (RvSipCallLegHandle)pCallLeg,
                                        &hAppCall);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        pCallLeg->hAppCallLeg = hAppCall;
		
		OBJ_LEAVE_CB(pCallLeg, CB_CALL_CALL_CREATED);
                
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegCallbackCreatedEv - call-leg 0x%p, After callback, phAppCall 0x%p",
            pCallLeg, hAppCall));        
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegCallbackCreatedEv - call-leg 0x%p, Application did not registered to callback, default phAppCall = 0x%p",
              pCallLeg, hAppCall));
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif
}

/***************************************************************************
 * CallLegCallbackChangeCallStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg state and notify the application about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeCallStateEv(
                                 IN  CallLeg                      *pCallLeg,
                                 IN  RvSipCallLegState             eState,
                                 IN  RvSipCallLegStateChangeReason eReason)
{
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    if (ConfirmCallbackCallLeg(pCallLeg,"ChangeCallStateEv") != RV_OK)
    {
        return;
    }

    /*notify the application about the new state - only if this is not a hidden call-leg. */
    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnStateChangedEvHandler != NULL)
    {
        RvSipAppCallLegHandle   hAppCall = pCallLeg->hAppCallLeg;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;
        RvInt32                 nestedLock = 0;

        pTripleLock = pCallLeg->tripleLock;
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackChangeCallStateEv - call-leg 0x%p, Before callback",pCallLeg));

        OBJ_ENTER_CB(pCallLeg,CB_CALL_CALL_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnStateChangedEvHandler(
            (RvSipCallLegHandle)pCallLeg,
            hAppCall,
            eState,
            eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg, CB_CALL_CALL_STATE_CHANGED);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackChangeCallStateEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackChangeCallStateEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }

}

/*-----------------------------------------------------------------------*/
/*                           MODIFY CALLBACKS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackChangeModifyStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg connect sub state.
 *          The connect sub state is used for incoming and outgoing re-Invite
 *          messages.
 *          This callback is DEPRECATED.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eSubState - The Modify state.
 *          eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeModifyStateEv(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason)
{
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    if (ConfirmCallbackCallLeg(pCallLeg,"ChangeModifyStateEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnModifyStateChangedEvHandler != NULL)
    {
        RvSipAppCallLegHandle   hAppCall   = pCallLeg->hAppCallLeg;
        RvInt32                 nestedLock = 0;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pCallLeg->tripleLock;

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackChangeModifyStateEv - call-leg 0x%p, Before callback",pCallLeg));

        OBJ_ENTER_CB(pCallLeg,CB_CALL_MODIFY_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnModifyStateChangedEvHandler(
                                   (RvSipCallLegHandle)pCallLeg,
                                    hAppCall,
                                    eModifyState,
                                    eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_MODIFY_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackChangeModifyStateEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackChangeModifyStateEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }
}


/***************************************************************************
 * CallLegCallbackReInviteCreatedEv
 * ------------------------------------------------------------------------
 * General: Notify that a new re-invite object was created.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            pInvite  - The new invite object.
 * Output:    pResponseCode - application may decide how to reject the re-invite request.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackReInviteCreatedEv(
                                 IN  CallLeg        *pCallLeg,
                                 IN  CallLegInvite  *pInvite,
                                 OUT RvUint16        *pResponseCode)
{
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    if (ConfirmCallbackCallLeg(pCallLeg,"ReInviteCreatedEv") != RV_OK)
    {
        return;
    }

    *pResponseCode = 0;

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnReInviteCreatedEvHandler!= NULL)
    {
        RvSipAppCallLegHandle   hAppCall   = pCallLeg->hAppCallLeg;
        RvSipAppCallLegInviteHandle hAppInvite;
        RvInt32                 nestedLock = 0;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pCallLeg->tripleLock;

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackReInviteCreatedEv - call-leg 0x%p, re-invite 0x%p, Before callback",
                pCallLeg, pInvite));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_REINVITE_CREATED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnReInviteCreatedEvHandler(
                                   (RvSipCallLegHandle)pCallLeg,
                                    hAppCall,
                                    (RvSipCallLegInviteHandle)pInvite,
                                    &hAppInvite,
                                    pResponseCode);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_REINVITE_CREATED);
        pInvite->hAppInvite = hAppInvite;
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackReInviteCreatedEv - call-leg 0x%p, re-invite 0x%p, After callback. pResponseCode=%d", 
                pCallLeg, pInvite, *pResponseCode));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackReInviteCreatedEv - call-leg 0x%p, re-invite 0x%p, Application did not registered to callback",pCallLeg, pInvite));

    }
}

/***************************************************************************
 * CallLegCallbackChangeReInviteStateEv
 * ------------------------------------------------------------------------
 * General: Changes the re-invite state of an invite object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eSubState - The Modify state.
 *          eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeReInviteStateEv(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  CallLegInvite                 *pInvite,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason)
{
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    if (ConfirmCallbackCallLeg(pCallLeg,"ChangeReInviteStateEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnReInviteStateChangedEvHandler!= NULL)
    {
        RvSipAppCallLegHandle   hAppCall   = pCallLeg->hAppCallLeg;
        RvSipAppCallLegInviteHandle hAppInvite = pInvite->hAppInvite;
        RvInt32                 nestedLock = 0;
        SipTripleLock          *pTripleLock;
        RvThreadId              currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pCallLeg->tripleLock;

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackChangeReInviteStateEv - call-leg 0x%p, pInvite 0x%p Before callback",
                pCallLeg, pInvite));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_REINVITE_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnReInviteStateChangedEvHandler(
                                   (RvSipCallLegHandle)pCallLeg,
                                    hAppCall,
                                    (RvSipCallLegInviteHandle)pInvite,
                                    hAppInvite,
                                    eModifyState,
                                    eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_REINVITE_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackChangeReInviteStateEv - call-leg 0x%p pInvite 0x%p, After callback",
                pCallLeg, pInvite));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackChangeReInviteStateEv - call-leg 0x%p pInvite 0x%p, Application did not registered to callback",
            pCallLeg, pInvite));

    }
}

/***************************************************************************
 * CallLegCallbackModifyRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnModifyRequestRcvdEvHandler
 *          This callback is DEPRECATED
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackModifyRequestRcvdEv(IN  CallLeg *pCallLeg)
{
    RvInt32                 nestedLock = 0;
    RvSipCallLegEvHandlers *evHandlers = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    /*notify the application about modify request.*/
    if(evHandlers != NULL && (*evHandlers).pfnModifyRequestRcvdEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackModifyRequestRcvdEv - call-leg 0x%p, Before callback",pCallLeg));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnModifyRequestRcvdEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackModifyRequestRcvdEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackModifyRequestRcvdEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }
}

/***************************************************************************
 * CallLegCallbackModifyResultRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnModifyResultRcvdEvHandler
 *          This callback is DEPRECATED
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackModifyResultRcvdEv(IN  CallLeg  *pCallLeg,
                                                  IN  RvUint16 responseStatusCode)
{
    RvInt32                 nestedLock        = 0;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle   hAppCallLeg       = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    /*notify the application about modify request.*/
    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnModifyResultRcvdEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackModifyResultRcvdEv - call-leg 0x%p, Before callback",pCallLeg));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnModifyResultRcvdEvHandler(
                                             (RvSipCallLegHandle)pCallLeg,
                                             hAppCallLeg,
                                             responseStatusCode);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackModifyResultRcvdEv - call-leg 0x%p, After callback", pCallLeg));

    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackModifyResultRcvdEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }
}

/*-----------------------------------------------------------------------*/
/*                           CALL TRANSACTION CALLBACKS                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* CallLegCallbackTranscCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegTranscCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - Handle to the call-leg.
*         hTransc    -  Handle to the transaction.
* Output:hAppTransc - Application handle to this transaction
*        bAppHandleTransc -
***************************************************************************/
void RVCALLCONV CallLegCallbackTranscCreatedEv(IN  RvSipCallLegHandle   hCallLeg,
                                                 IN  RvSipTranscHandle    hTransc,
                                              OUT RvSipAppTranscHandle *hAppTransc,
                                              OUT RvBool              *bAppHandleTransc)
{
    RvInt32                 nestedLock = 0;
    CallLeg                *pCallLeg = (CallLeg *)hCallLeg;
    RvSipAppCallLegHandle   hAppCall = pCallLeg->hAppCallLeg;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    if (ConfirmCallbackCallLeg(pCallLeg,"TranscCreatedEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnTranscCreatedEvHandler != NULL)
    {

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscCreatedEv - call-leg 0x%p hTransc 0x%p, Before callback",
                  pCallLeg, hTransc));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_TRANSC_CREATED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnTranscCreatedEvHandler(
                            (RvSipCallLegHandle)pCallLeg,
                            hAppCall,
                            hTransc,
                            hAppTransc,
                            bAppHandleTransc);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_TRANSC_CREATED);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscCreatedEv - call-leg 0x%p hTransc 0x%p, After callback",
                  pCallLeg, hTransc));

    }
    else
    { 
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscCreatedEv - call-leg 0x%p, Application did not registered to callback",pCallLeg));

    }
}



/***************************************************************************
 * CallLegCallbackTranscStateChangedEv
 * ------------------------------------------------------------------------
 * General: Call the pfnTranscStateChangedEvHandler callback.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The sip stack call-leg handle
 *            hTransc -     The handle to the transaction.
 *          eTranscState - The call-leg transaction new state.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
void RVCALLCONV CallLegCallbackTranscStateChangedEv(
                           IN  RvSipCallLegHandle                hCallLeg,
                           IN  RvSipTranscHandle                 hTransc,
                           IN  RvSipCallLegTranscState           eTranscState,
                           IN  RvSipTransactionStateChangeReason eReason)
{

    RvInt32                 nestedLock = 0;
    CallLeg                *pCallLeg = (CallLeg *)hCallLeg;
    RvSipAppTranscHandle    hAppTransc = NULL;
    RvSipAppCallLegHandle   hAppCall = pCallLeg->hAppCallLeg;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegCallbackTranscStateChangedEv - Call-leg 0x%p, Transc 0x%p state changed to %s",
                  pCallLeg,hTransc,CallLegGetCallLegTranscStateName(eTranscState)));

    if (ConfirmCallbackCallLeg(pCallLeg,"TranscStateChangedEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnTranscStateChangedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscStateChangedEv - call-leg 0x%p hTransc 0x%p hAppTransc 0x%p, Before callback",
                 pCallLeg, hTransc, hAppTransc));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_TRANSC_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        (*callLegEvHandlers).pfnTranscStateChangedEvHandler(
                        (RvSipCallLegHandle)pCallLeg,
                        hAppCall,
                        hTransc,
                        hAppTransc,
                        eTranscState,
                        eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_TRANSC_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscStateChangedEv - call-leg 0x%p hTransc 0x%p hAppTransc 0x%p, After callback",
                 pCallLeg, hTransc, hAppTransc));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscStateChangedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }
}

/***************************************************************************
* CallLegCallbackTranscResolvedEv
* ------------------------------------------------------------------------
* General: Call the pfnTranscResolvedEvHandler callback.
*           This callback is DEPRECATED
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input:    hCallLeg    - The sip stack call-leg handle
*            hTransc     - The handle to the transaction that received the
*                         response
*           eStatus     - The resolution status. if the status is
*                         RVSIP_CALL_TRANSC_STATUS_RESPONSE_RECVD the
*                         responseCode parameter will contain the response
*                         message status code.
*            responseCode- The response status code.
***************************************************************************/
void RVCALLCONV CallLegCallbackTranscResolvedEv(IN  RvSipCallLegHandle               hCallLeg,
                                               IN  RvSipTranscHandle                hTransc,
                                               IN  RvSipCallLegTranscStatus         eStatus,
                                               IN  RvUint16                        responseCode)
{
    RvInt32                 nestedLock  = 0;
    CallLeg                *pCallLeg    = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers *evHandlers  = pCallLeg->callLegEvHandlers;
    RvSipAppTranscHandle    hAppTransc  = NULL;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);

    if (ConfirmCallbackCallLeg(pCallLeg,"TranscResolvedEv") != RV_OK)
    {
        return;
    }

    if(evHandlers != NULL && (*evHandlers).pfnTranscResolvedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscResolvedEv - call-leg 0x%p Transc 0x%p, Before callback",
                 pCallLeg, hTransc));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnTranscResolvedEvHandler(
                                        (RvSipCallLegHandle)pCallLeg,
                                        hAppCallLeg,
                                        hTransc,
                                        hAppTransc,
                                        eStatus,
                                        responseCode);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscResolvedEv - call-leg 0x%p Transc 0x%p, After callback",
                 pCallLeg, hTransc));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscResolvedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));

    }
}


/***************************************************************************
 * CallLegCallbackTranscRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: Notify the application of a new general transaction.
 *          This callback is DEPRECATED.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - the call-leg pointer
 *          hTransc     - the transaction handle
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackTranscRequestRcvdEv(
                                            IN  CallLeg           *pCallLeg,
                                            IN  RvSipTranscHandle  hTransc)
{
    RvSipCallLegEvHandlers *evHandlers = pCallLeg->callLegEvHandlers;
    RvInt32                nestedLock  = 0;
    RvSipAppTranscHandle   hAppTransc;
    RvSipAppCallLegHandle  hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock         *pTripleLock;
    RvThreadId             currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    if (ConfirmCallbackCallLeg(pCallLeg,"TranscRequestRcvdEv") != RV_OK)
    {
        return;
    }

    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);
     /* notify the application of the general request*/
    if(evHandlers != NULL && (*evHandlers).pfnTranscRequestRcvdEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackTranscRequestRcvdEv - call-leg 0x%p transc 0x%p, Before callback",
              pCallLeg, hTransc));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*evHandlers).pfnTranscRequestRcvdEvHandler((RvSipCallLegHandle)pCallLeg,
                                                 hAppCallLeg,
                                                 hTransc,
                                                 hAppTransc);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackTranscRequestRcvdEv - call-leg 0x%p transc 0x%p, After callback",
                  pCallLeg, hTransc));

    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackTranscRequestRcvdEv - call-leg 0x%p,Application did not registered to callback",
              pCallLeg));

    }
}

/*-----------------------------------------------------------------------*/
/*                           MESSAGE CALLBACKS                           */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* CallLegCallbackMsgReceivedEv
* ------------------------------------------------------------------------
* General: Call the pfnMsgReceivedEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackMsgReceivedEv(IN  RvSipCallLegHandle      hCallLeg,
                                                  IN  RvSipMsgHandle          hMsg)
{    RvInt32                 nestedLock  = 0;
    CallLeg                 *pCallLeg   = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers  *evHandlers = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle    hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;
    RvStatus                 rv = RV_OK;

    pTripleLock = pCallLeg->tripleLock;

    /* in case of a hidden call-leg, if subscription was not found, we might reach
    this state. and we should not pass this message to application */
    if (ConfirmCallbackCallLeg(pCallLeg,"MsgReceivedEv") != RV_OK)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(evHandlers != NULL && (*evHandlers).pfnMsgReceivedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackMsgReceivedEv - call-leg 0x%p, Before callback",
                 pCallLeg));
     
        OBJ_ENTER_CB(pCallLeg,CB_CALL_MSG_RCVD);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        /*notify the application that a message was recieced*/
        rv = (*evHandlers).pfnMsgReceivedEvHandler((RvSipCallLegHandle)pCallLeg,
                                            hAppCallLeg,
                                            hMsg);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_MSG_RCVD);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackMsgReceivedEv - call-leg 0x%p, rv=%d", 
                  pCallLeg, rv));

        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackMsgReceivedEv - call-leg 0x%p was terminated in cb. return failure", pCallLeg));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackMsgReceivedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
    return rv;
}


/***************************************************************************
* CallLegCallbackMsgToSendEv
* ------------------------------------------------------------------------
* General: Call the pfnMsgToSendEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackMsgToSendEv(IN  RvSipCallLegHandle      hCallLeg,
                                                 IN  RvSipMsgHandle          hMsg)
{
    CallLeg                    *pCallLeg   = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers  *evHandlers = pCallLeg->callLegEvHandlers;
    RvStatus               rv = RV_OK;

    /* in case of a hidden call-leg, if subscription was not found, we might reach
    this state. and we should not pass this message to application */
    if (ConfirmCallbackCallLeg(pCallLeg,"MsgToSendEv") != RV_OK)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(evHandlers != NULL && (*evHandlers).pfnMsgToSendEvHandler != NULL)
    {
        RvSipAppCallLegHandle  hAppCall   = pCallLeg->hAppCallLeg;
        RvInt32                nestedLock = 0;
        SipTripleLock         *pTripleLock;
        RvThreadId             currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pCallLeg->tripleLock;

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackMsgToSendEv - call-leg 0x%p, Before callback",
                 pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_MSG_TO_SEND);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnMsgToSendEvHandler((RvSipCallLegHandle)pCallLeg,
                                                  hAppCall,
                                                  hMsg);
        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_MSG_TO_SEND);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackMsgToSendEv - call-leg 0x%p, After callback", 
                  pCallLeg));

        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackMsgToSendEv - call-leg 0x%p was terminated in cb. return failure", pCallLeg));
            rv = RV_ERROR_DESTRUCTED;
        }

    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegCallbackMsgToSendEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackMsgToSendEv - Call-Leg 0x%p - Failed in call-back pfnMsgToSendEvHandler"
            ,pCallLeg));
    }
    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           PRACK CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackPrackStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notify the application of a new prack state
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg.
 *            ePrackState       - The new prack state
 *            prackResponseCode - The response code that was received on a prack.
 *            hPrackTransc      - The handle for the prack transaction.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackPrackStateChangedEv(
                      IN  CallLeg                        *pCallLeg,
                      IN  RvSipCallLegPrackState         ePrackState,
                      IN  RvSipCallLegStateChangeReason  eReason,
                      IN  RvInt16                       prackResponseCode,
					  IN  RvSipTranscHandle             hPrackTransc)
{
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
	RvSipTranscHandle hPrackParent = NULL;
    RvSipTransaction100RelStatus e100RelStatus =
                    SipTransactionMgrGet100relStatus(pCallLeg->pMgr->hTranscMgr);

    if(e100RelStatus == RVSIP_TRANSC_100_REL_UNDEFINED)
    {
        return;
    }
	
	/*don't notify of PRACK states if there is no matching invite*/
	if(hPrackTransc != NULL)
	{
		SipTransactionGetPrackParent(hPrackTransc,&hPrackParent);
	}

    if ((RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD==ePrackState ||
         RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_SENT==ePrackState) &&
        hPrackParent == NULL)
    {
        return;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackPrackStateChangedEv - Call-leg 0x%p prack state changed to %s",
                   pCallLeg,CallLegGetPrackStateName(ePrackState)));

    if (ConfirmCallbackCallLeg(pCallLeg,"PrackStateChangedEv") != RV_OK)
    {
        return;
    }

    pCallLeg->ePrackState = ePrackState;

    if(callLegEvHandlers != NULL && (*(callLegEvHandlers)).pfnPrackStateChangedEvEvHandler != NULL)
    {
        RvSipAppCallLegHandle  hAppCall   = pCallLeg->hAppCallLeg;
        RvInt32                nestedLock = 0;
        SipTripleLock         *pTripleLock;
        RvThreadId             currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pCallLeg->tripleLock;

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackPrackStateChangedEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_PRACK_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnPrackStateChangedEvEvHandler(
                                    (RvSipCallLegHandle)pCallLeg,
                                    hAppCall,
                                    ePrackState,
                                    eReason,
                                    prackResponseCode);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_PRACK_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackPrackStateChangedEv - call-leg 0x%p, After callback", pCallLeg));

    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegCallbackPrackStateChangedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}

/*-----------------------------------------------------------------------*/
/*                           BYE CALLBACKS                               */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CallLegCallbackByeCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnByeCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg         - Handle to the call-leg.
*         hTransc    -  Handle to the bye transaction.
* Output:hAppTransc - Application handle to this transaction
*        bAppHandleTransc - indicates wh
***************************************************************************/
void RVCALLCONV CallLegCallbackByeCreatedEv(IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc,
                                           OUT RvSipAppTranscHandle *hAppTransc,
                                           OUT RvBool              *bAppHandleTransc)
{
    RvInt32                 nestedLock        = 0;
    CallLeg                *pCallLeg          = (CallLeg *)hCallLeg;
    RvSipAppCallLegHandle   hAppCall          = pCallLeg->hAppCallLeg;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegCallbackByeCreatedEv - New BYE transaction 0x%p was created for call-leg 0x%p - notifying application",
             hTransc,pCallLeg));

    if (ConfirmCallbackCallLeg(pCallLeg,"ByeCreatedEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnByeCreatedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackByeCreatedEv - call-leg 0x%p, Before callback",
                 pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_BYE_CREATED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnByeCreatedEvHandler(
                            (RvSipCallLegHandle)pCallLeg,
                            hAppCall,
                            hTransc,
                            hAppTransc,
                            bAppHandleTransc);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_BYE_CREATED);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackByeCreatedEv - call-leg 0x%p, After callback", 
                 pCallLeg));        
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegCallbackByeCreatedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}

/***************************************************************************
 * CallLegCallbackByeStateChangedEv
 * ------------------------------------------------------------------------
 * General: Call the pfnByeStateChangedEvHandler callback.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -    The sip stack call-leg handle
 *            hTransc -     The handle to the transaction.
 *          eTranscState - The call-leg bye transaction new state.
 *          eReason      - The reason for the new state.
 * Output: (-).
 ***************************************************************************/
void RVCALLCONV CallLegCallbackByeStateChangedEv(
                           IN  RvSipCallLegHandle                hCallLeg,
                           IN  RvSipTranscHandle                 hTransc,
                           IN  RvSipCallLegByeState              eByeState,
                           IN  RvSipTransactionStateChangeReason eReason)
{

    RvInt32                 nestedLock        = 0;
    CallLeg                *pCallLeg          = (CallLeg *)hCallLeg;
    RvSipAppTranscHandle    hAppTransc        = NULL;
    RvSipAppCallLegHandle   hAppCallLeg       = pCallLeg->hAppCallLeg;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;

    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegCallbackByeStateChangedEv - Call-leg 0x%p, BYE Transc 0x%p new state is %s", 
           pCallLeg, hTransc, CallLegGetCallLegByeTranscStateName(eByeState)));

    if (ConfirmCallbackCallLeg(pCallLeg,"ByeStateChangedEv") != RV_OK)
    {
        return;
    }

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnByeStateChangedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackByeStateChangedEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_BYE_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        (*callLegEvHandlers).pfnByeStateChangedEvHandler(
                                            (RvSipCallLegHandle)pCallLeg,
                                            hAppCallLeg,
                                            hTransc,
                                            hAppTransc,
                                            eByeState,
                                            eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_BYE_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackByeStateChangedEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "CallLegCallbackByeStateChangedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}


/*-----------------------------------------------------------------------*/
/*                           SESSION TIMER CALLBACKS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* CallLegCallbackSessionTimerRefreshAlertEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerRefreshAlertEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerRefreshAlertEv(
                            IN  RvSipCallLegHandle      hCallLeg)
{
    RvInt32                 nestedLock  = 0;
    CallLeg                *pCallLeg    = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers *evHandlers  = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    RvStatus             rv = RV_OK;

    pTripleLock = pCallLeg->tripleLock;

    if (ConfirmCallbackCallLeg(pCallLeg,"SessionTimerRefreshAlertEv") != RV_OK)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(evHandlers != NULL && (*evHandlers).pfnSessionTimerRefreshAlertEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerRefreshAlertEv - call-leg 0x%p, Before callback",
                 pCallLeg));

        OBJ_ENTER_CB(pCallLeg,CB_CALL_SESSION_TIMER_REFRESH_ALERT);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        /*notify the application that a message was recieced*/
        rv = (*evHandlers).pfnSessionTimerRefreshAlertEvHandler(
                                            (RvSipCallLegHandle)pCallLeg,
                                            hAppCallLeg);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_SESSION_TIMER_REFRESH_ALERT);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerRefreshAlertEv - call-leg 0x%p, After callback", 
                 pCallLeg));
        
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackSessionTimerRefreshAlertEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }

    return rv;
}

/***************************************************************************
* CallLegCallbackSessionTimerNotificationEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerNotificationEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
 *       eReason     - The reason for this notify .
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerNotificationEv(
                IN  RvSipCallLegHandle                        hCallLeg,
                IN RvSipCallLegSessionTimerNotificationReason eReason)
{
    RvInt32                  nestedLock = 0;
    CallLeg                 *pCallLeg = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers  *evHandlers = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle    hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;

    RvStatus rv=RV_OK;

    pTripleLock = pCallLeg->tripleLock;

    if (ConfirmCallbackCallLeg(pCallLeg,"SessionTimerNotificationEv") != RV_OK)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(evHandlers != NULL && (*evHandlers).pfnSessionTimerNotificationEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerNotificationEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_SESSION_TIMER_NOTIFICATION);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        /*notify the application that a message was recieced*/
        rv = (*evHandlers).pfnSessionTimerNotificationEvHandler(
                                            (RvSipCallLegHandle)pCallLeg,
                                            hAppCallLeg,
                                            eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_SESSION_TIMER_NOTIFICATION);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerNotificationEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackSessionTimerNotificationEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }

    return rv;
}


/***************************************************************************
* CallLegCallbackSessionTimerNegotiationFaultEv
* ------------------------------------------------------------------------
* General: Call the pfnSessionTimerNegotiationFaultEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*        eReason     - The reason for this negotiation fault .
* Output:b_handleSessionTimer - RV_TRUE if the application wishes to operate
*                               the session time of this call.
*                               RV_FALSE if the application wishes not to
*                               operate the session timer to this call.
***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSessionTimerNegotiationFaultEv(
           IN  RvSipCallLegHandle                         hCallLeg,
           IN  RvSipCallLegSessionTimerNegotiationReason  eReason,
           OUT RvBool                                    *b_handleSessionTimer)
{
    RvInt32                 nestedLock  = 0;
    CallLeg                *pCallLeg    = (CallLeg *)hCallLeg;
    RvSipCallLegEvHandlers *evHandlers  = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
    RvStatus                rv          = RV_OK;

    pTripleLock = pCallLeg->tripleLock;

    if (ConfirmCallbackCallLeg(pCallLeg,"SessionTimerNegotiationFaultEv") != RV_OK)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(evHandlers != NULL && (*evHandlers).pfnSessionTimerNegotiationFaultEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerNegotiationFaultEv - call-leg 0x%p, Before callback",
                 pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_SESSION_TIMER_NEGOTIATION_FAULT);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        /*notify the application that a message was recieced*/
        rv = (*evHandlers).pfnSessionTimerNegotiationFaultEvHandler(
                                            (RvSipCallLegHandle)pCallLeg,
                                            hAppCallLeg,
                                            eReason,
                                            b_handleSessionTimer);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_SESSION_TIMER_NEGOTIATION_FAULT);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSessionTimerNegotiationFaultEv - call-leg 0x%p, After callback", 
                 pCallLeg));
        
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackSessionTimerNegotiationFaultEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }

    return rv;
}



#ifdef RV_SIP_SUBS_ON
/*-----------------------------------------------------------------------*/
/*                           REFER CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackChangeReferStateEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg refer state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new refer sub state.
 *            eReason  - The refer sub state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegCallbackChangeReferStateEv(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferState                eState,
                               IN  RvSipCallLegStateChangeReason         eReason)
{

    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    RvInt32                 nestedLock        = 0;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;

    pTripleLock = pCallLeg->tripleLock;

    /*notify the application about the new state*/
    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnReferStateChangedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackChangeReferStateEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_REFER_STATE_CHANGED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnReferStateChangedEvHandler(
                                   (RvSipCallLegHandle)pCallLeg,
                                   hAppCallLeg,
                                   eState,
                                   eReason);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_REFER_STATE_CHANGED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackChangeReferStateEv - call-leg 0x%p, After callback", pCallLeg));

    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackChangeReferStateEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}

/***************************************************************************
 * CallLegCallbackReferNotifyEv
 * ------------------------------------------------------------------------
 * General: Changes the call-leg notify state and notify the application
 *          about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new notify sub state.
 *            eReason  - The state change reason.
 *          status - Status of the connection attempt.
 *          cseq - The CSeq step of the associated REFER transaction.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackReferNotifyEv(
                               IN  CallLeg                              *pCallLeg,
                               IN  RvSipCallLegReferNotifyEvents         eState,
                               IN  RvSipCallLegStateChangeReason         eReason,
                               IN  RvInt16                              status,
                               IN  RvInt32                              cseq)
{

    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    RvInt32                 nestedLock = 0;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackReferNotifyEv - Call-leg 0x%p: Notification: %s, (reason = %s)"
              ,pCallLeg, CallLegReferGetNotifyEventName(eState),
              CallLegGetStateChangeReasonName(eReason)));

    /*notify the application about the new state*/
    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnReferNotifyEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackReferNotifyEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_REFER_NOTIFY);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnReferNotifyEvHandler(
                                   (RvSipCallLegHandle)pCallLeg,
                                   hAppCallLeg,
                                   eState,
                                   eReason,
                                   status,
                                   cseq);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_REFER_NOTIFY);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackReferNotifyEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackReferNotifyEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */


#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                           AUTHENTICATION CALLBACKS                    */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegCallbackAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  Calls to pfnAuthCredentialsFoundEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The sip stack transaction handle
 *          hCallLeg  - CallLeg handle for the transaction
 *          hAuthorization - Handle to the authorization header that was found.
 *          bCredentialsSupported - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackAuthCredentialsFoundEv(
                        IN    RvSipTranscHandle               hTransc,
                        IN    RvSipCallLegHandle              hCallLeg,
                        IN    RvSipAuthorizationHeaderHandle  hAuthorization,
                        IN    RvBool                         bCredentialsSupported)
{
    CallLeg                *pCallLeg    = (CallLeg*)hCallLeg;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    RvInt32                 nestedLock  = 0;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    pTripleLock = pCallLeg->tripleLock;
    if(callLegEvHandlers != NULL && (*(callLegEvHandlers)).pfnAuthCredentialsFoundEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackAuthCredentialsFoundEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_AUTH_CREDENTIALS_FOUND);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(callLegEvHandlers)).pfnAuthCredentialsFoundEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hTransc,
                                               hAuthorization,
                                               bCredentialsSupported);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_AUTH_CREDENTIALS_FOUND);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackAuthCredentialsFoundEv - call-leg 0x%p, After callback", pCallLeg));
    }
    else
    {
        /* This is an exception, because we can reach this callback, only
            if user asked for authentication procedure. and for that he must implement
            this callback */
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegCallbackAuthCredentialsFoundEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}

/***************************************************************************
* CallLegCallbackAuthCompletedEv
* ------------------------------------------------------------------------
* General:  call to pfnAuthCompletedEvHandler.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTransc      - The sip stack transaction handle
*          hCallLeg     - CallLeg handle for the transaction
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                        RV_FALSE if we did not.
***************************************************************************/
void RVCALLCONV CallLegCallbackAuthCompletedEv(
                                             IN    RvSipTranscHandle        hTransc,
                                             IN    RvSipCallLegHandle       hCallLeg,
                                             IN    RvBool                  bAuthSucceed)
{
    CallLeg                *pCallLeg    =  (CallLeg*)hCallLeg;
    RvInt32                 nestedLock  = 0;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    pTripleLock = pCallLeg->tripleLock;
    if(callLegEvHandlers != NULL &&
       (*(callLegEvHandlers)).pfnAuthCompletedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackAuthCompletedEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_AUTH_COMPLETED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(callLegEvHandlers)).pfnAuthCompletedEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hTransc,
                                               bAuthSucceed);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_AUTH_COMPLETED);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackAuthCompletedEv - call-leg 0x%p, After callback", pCallLeg));

    }
    else
    {
        /* This is an exception, because we can reach this callback, only
            if user asked for authentication procedure. and for that he must implement
            this callback */
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackAuthCompletedEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * CallLegCallbackTranscOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: calls to pfnOtherURLAddressFoundEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hCallLeg       - CallLeg handle for the transaction
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackTranscOtherURLAddressFoundEv(
                     IN  CallLeg                *pCallLeg,
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool               *bAddressResolved)
{
    RvInt32                 nestedLock = 0;
    RvStatus                rv         = RV_OK;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock          *pTripleLock;
    RvSipAppCallLegHandle   hAppCallLeg = pCallLeg->hAppCallLeg;
    RvSipAppTranscHandle    hAppTransc  = NULL;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;

    pTripleLock = pCallLeg->tripleLock;
    rv = RvSipTransactionGetAppHandle(hTransc,&hAppTransc);

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnOtherURLAddressFoundEvHandler!= NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscOtherURLAddressFoundEv - call-leg 0x%p, Before callback",pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_OTHER_URL_FOUND);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*(callLegEvHandlers)).pfnOtherURLAddressFoundEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hTransc,
                                               hAppTransc,
                                               hMsg,
                                               hAddress,
                                               hSipURLAddress,
                                               bAddressResolved);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_OTHER_URL_FOUND);
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscOtherURLAddressFoundEv - call-leg 0x%p, After callback", pCallLeg));

        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackTranscOtherURLAddressFoundEv - call-leg 0x%p was terminated in cb. return failure", pCallLeg));
            return RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
       *bAddressResolved = RV_FALSE;
       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackTranscOtherURLAddressFoundEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }

    return rv;
}



/***************************************************************************
 * CallLegCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnFinalDestResolvedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - The CallLeg handle
 *            hTransc       - The transaction handle
 *            hMsg          - The message that is about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackFinalDestResolvedEv(
                     IN  CallLeg               *pCallLeg,
                     IN  RvSipTranscHandle      hTransc,
                     IN  RvSipMsgHandle         hMsg)
{

    RvInt32                  nestedLock        = 0;
    RvStatus                 rv                = RV_OK;
    RvSipCallLegEvHandlers  *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;
    RvSipAppCallLegHandle    hAppCallLeg = pCallLeg->hAppCallLeg;
    RvSipAppTranscHandle     hAppTransc  = NULL;

    pTripleLock = pCallLeg->tripleLock;
    RvSipTransactionGetAppHandle(hTransc,&hAppTransc);

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnFinalDestResolvedEvHandler != NULL)
    {
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegCallbackFinalDestResolvedEv - call-leg 0x%p, Transc=0x%p, appTransc=0x%p, hMsg=0x%p Before callback",
             pCallLeg,hTransc,hAppTransc,hMsg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_FINAL_DEST_RESOLVED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*(callLegEvHandlers)).pfnFinalDestResolvedEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hTransc,
                                               hAppTransc,
                                               hMsg);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_FINAL_DEST_RESOLVED);
        
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackFinalDestResolvedEv - call-leg 0x%p, After callback (rv=%d)", 
                 pCallLeg,rv));

        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackFinalDestResolvedEv - call-leg 0x%p was terminated in cb. return failure", pCallLeg));
            return RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackFinalDestResolvedEv - call-leg 0x%p, Transc=0x%p, appTransc=0x%p, hMsg=0x%p, Application did not register to callback",
              pCallLeg,hTransc,hAppTransc,hMsg));
    }
    return rv;
}


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * CallLegCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSigCompMsgNotRespondedEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - The CallLeg handle
 *            hTransc        - The transaction handle
 *            hAppTransc     - The transaction application handle.
 *            retransNo      - The number of retransmissions of the request
 *                             until now.
 *            ePrevMsgComp   - The type of the previous not responded request
 * Output:    peNextMsgComp  - The type of the next request to retransmit (
 *                            RVSIP_TRANSC_MSG_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackSigCompMsgNotRespondedEv(
                         IN  CallLeg                     *pCallLeg,
                         IN  RvSipTranscHandle            hTransc,
                         IN  RvSipAppTranscHandle         hAppTransc,
                         IN  RvInt32                      retransNo,
                         IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                         OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    SipTripleLock          *pTripleLock;
    RvThreadId              currThreadId; RvInt32 threadIdLockCount;
    RvInt32                 nestedLock        = 0;
    RvStatus                rv                  = RV_OK;
    RvSipCallLegEvHandlers *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    RvSipAppCallLegHandle   hAppCallLeg       = pCallLeg->hAppCallLeg;

    pTripleLock = pCallLeg->tripleLock;

    if(callLegEvHandlers                                       != NULL &&
       (*callLegEvHandlers).pfnSigCompMsgNotRespondedEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "CallLegCallbackSigCompMsgNotRespondedEv - call-leg 0x%p, Transc=0x%p, retrans=%d, prev=%s Before callback",
            pCallLeg,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_SIGCOMP_NOT_RESPONDED);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*(callLegEvHandlers)).pfnSigCompMsgNotRespondedEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hTransc,
                                               hAppTransc,
                                               retransNo,
                                               ePrevMsgComp,
                                               peNextMsgComp);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_SIGCOMP_NOT_RESPONDED);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackSigCompMsgNotRespondedEv - call-leg 0x%p, After callback (rv=%d)", 
                 pCallLeg, rv));

        
    }
    else
    {
       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackSigCompMsgNotRespondedEv - call-leg 0x%p, Transc=0x%p, retrans=%d, prev=%s, Application did not register to callback",
              pCallLeg,hTransc,retransNo,SipTransactionGetMsgCompTypeName(ePrevMsgComp)));
    }
    return rv;
}

#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * CallLegCallbackNestedInitialReqRcvdEv
 * ------------------------------------------------------------------------
 * General: calls to pfnInitialReqRcvdEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - The CallLeg handle
 *            hMsg          - The handle to the second INVITE message.
 * Output:    pbCreateCallLeg - RV_TRUE will cause the creation of a new call-leg.
 *                            RV_FALSE will cause the rejection of the second INIVTE.
 ***************************************************************************/
void RVCALLCONV CallLegCallbackNestedInitialReqRcvdEv(
                     IN  CallLeg               *pCallLeg,
                     IN  RvSipMsgHandle         hMsg,
                     OUT RvBool                *pbCreateNewCall)
{

    RvInt32                  nestedLock = 0;
    RvSipCallLegEvHandlers  *callLegEvHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;
    RvSipAppCallLegHandle    hAppCallLeg = pCallLeg->hAppCallLeg;

    pTripleLock = pCallLeg->tripleLock;

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnNestedInitialReqRcvdEvHandler != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackNestedInitialReqRcvdEv - call-leg 0x%p, hMsg=0x%p Before callback",
                 pCallLeg,hMsg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_NESTED_INITIAL_REQ);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*(callLegEvHandlers)).pfnNestedInitialReqRcvdEvHandler(
                                              (RvSipCallLegHandle)pCallLeg,
                                               hAppCallLeg,
                                               hMsg,
                                               pbCreateNewCall);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_NESTED_INITIAL_REQ);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackNestedInitialReqRcvdEv - call-leg 0x%p, After callback (bCreateNewCall=%d)",
                 pCallLeg, *pbCreateNewCall));
    }
    else
    {
       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackNestedInitialReqRcvdEv - call-leg 0x%p, hMsg=0x%p, Application did not register to callback",
              pCallLeg,hMsg));
    }
}

/***************************************************************************
 * CallLegCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg     - The CallLeg pointer
 *            hConn        - The new connection in use.
 *            bNewConnCreated - The connection is also a newly created connection
 *                               (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCallbackNewConnInUseEv(
                     IN  CallLeg*                       pCallLeg,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{
    RvInt32                   nestedLock    = 0;
    SipTripleLock            *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipAppCallLegHandle     hAppCallLeg   = pCallLeg->hAppCallLeg;
    RvSipCallLegEvHandlers*   evHandlers    = pCallLeg->callLegEvHandlers;
    RvStatus                  rv            = RV_OK;

    pTripleLock = pCallLeg->tripleLock;
    if(evHandlers != NULL && (*evHandlers).pfnNewConnInUseEvHandler != NULL)
    {
        OBJ_ENTER_CB(pCallLeg,CB_CALL_NEW_CONN_IN_USE);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegCallbackNewConnInUseEv: hCallLeg=0x%p, hAppCallLeg=0x%p, hConn=0x%p Before callback",
             pCallLeg,hAppCallLeg,hConn));

        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*evHandlers).pfnNewConnInUseEvHandler(
                                           (RvSipCallLegHandle)pCallLeg,
                                           hAppCallLeg,
                                           hConn,
                                           bNewConnCreated);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_NEW_CONN_IN_USE);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackNewConnInUseEv:hCallLeg= 0x%p,After callback, (rv=%d)",
                  pCallLeg, rv));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegCallbackNewConnInUseEv: hCallLeg=0x%p, hAppCallLeg=0x%p, hConn=0x%p,Application did not registered to callback",
                  pCallLeg,hAppCallLeg,hConn));

    }

    return rv;
}

/***************************************************************************
* CallLegCallbackCreatedDueToForkingEv
* ------------------------------------------------------------------------
* General: Call the pfnCallLegCreatedDueToForkingEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the call-leg manager.
*         hCallLeg    - The new sip stack call-leg handle
* Output:pbTerminated
***************************************************************************/
void RVCALLCONV CallLegCallbackCreatedDueToForkingEv(
                                         IN  CallLegMgr                *pMgr,
                                         IN  RvSipCallLegHandle        hCallLeg,
                                         OUT RvBool                    *pbTerminated)
{
    RvInt32                  nestedLock = 0;
    CallLeg                 *pCallLeg  = (CallLeg *)hCallLeg;
    RvSipAppCallLegHandle    hAppCall   = NULL;
    SipTripleLock           *pTripleLock;
    RvThreadId                currThreadId; RvInt32 threadIdLockCount;
    RvSipCallLegEvHandlers  *callLegEvHandlers = pCallLeg->callLegEvHandlers;

    pTripleLock = pCallLeg->tripleLock;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "CallLegCallbackCreatedDueToForkingEv - call leg mgr notifies the application of a new call-leg 0x%p",
        pCallLeg));

    if(callLegEvHandlers != NULL && (*callLegEvHandlers).pfnCallLegCreatedDueToForkingEvHandler != NULL)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegCallbackCreatedDueToForkingEv - call-leg 0x%p, Before callback",
                  pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_CREATED_DUE_TO_FORKING);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*callLegEvHandlers).pfnCallLegCreatedDueToForkingEvHandler(
                                        (RvSipCallLegHandle)pCallLeg,
                                        &hAppCall,
                                        pbTerminated);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_CREATED_DUE_TO_FORKING);
        pCallLeg->hAppCallLeg = hAppCall;
        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegCallbackCreatedDueToForkingEv - call-leg 0x%p, After callback, phAppCall=0x%p pbTerminate=%d",
            pCallLeg, hAppCall, *pbTerminated));
        
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "CallLegCallbackCreatedDueToForkingEv - call-leg 0x%p, Application did not registered to callback, default phAppCall = 0x%p",
              pCallLeg, hAppCall));
        /* application did not registered on this call-back, no need to continue with
           the new object */
        *pbTerminated = RV_TRUE;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif
}
/***************************************************************************
* CallLegCallbackProvisionalResponseRcvdEv
* ------------------------------------------------------------------------
* General: Call the pfnProvisionalResponseRcvdEvHandler callback.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hCallLeg    - The sip stack call-leg handle
*         hMsg     - Handle to the message that was received.
* Output:(-)
***************************************************************************/
void RVCALLCONV CallLegCallbackProvisionalResponseRcvdEv(
                                                             IN  CallLeg*          pCallLeg,
                                                             IN  RvSipTranscHandle hTransc,
                                                             IN  CallLegInvite*    pCallInvite,
                                                             IN  RvSipMsgHandle hMsg)
{    
    RvInt32                  nestedLock  = 0;
    RvSipCallLegEvHandlers  *evHandlers = pCallLeg->callLegEvHandlers;
    SipTripleLock           *pTripleLock;
    RvThreadId               currThreadId; RvInt32 threadIdLockCount;
    
    pTripleLock = pCallLeg->tripleLock;

    if(evHandlers != NULL && (*evHandlers).pfnProvisionalResponseRcvdEvHandler != NULL)
    {
        RvSipAppCallLegHandle       hAppCallLeg = pCallLeg->hAppCallLeg;
        RvSipAppTranscHandle        hAppTransc = NULL ;
        RvSipAppCallLegInviteHandle hAppInvite = NULL;
        
        if(hTransc != NULL)
        {
            RvSipTransactionGetAppHandle(hTransc,&hAppTransc);
        }

        if(NULL != pCallInvite)
        {
            hAppInvite = pCallInvite->hAppInvite;
        }

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackProvisionalResponseRcvdEv - call-leg 0x%p, Before callback",
                 pCallLeg));
        OBJ_ENTER_CB(pCallLeg,CB_CALL_PROV_RESP_RCVD);
        SipCommonUnlockBeforeCallback(pCallLeg->pMgr->pLogMgr,pCallLeg->pMgr->pLogSrc,
            pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        /*notify the application that a provisional response was recieved*/
        (*evHandlers).pfnProvisionalResponseRcvdEvHandler( (RvSipCallLegHandle)pCallLeg,
                                                           hAppCallLeg,
                                                           hTransc,
                                                           hAppTransc,
                                                           (RvSipCallLegInviteHandle)pCallInvite,
                                                           hAppInvite,
                                                           hMsg);

        SipCommonLockAfterCallback(pCallLeg->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pCallLeg,CB_CALL_PROV_RESP_RCVD);
        
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegCallbackProvisionalResponseRcvdEv - call-leg 0x%p, After callback",
                 pCallLeg));
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCallbackProvisionalResponseRcvdEv - call-leg 0x%p,Application did not registered to callback",pCallLeg));
    }
   
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* ConfirmCallbackCallLeg
* ------------------------------------------------------------------------
* General: Confirm that the given CallLeg is valid object to be used
*          in the CallLeg layer callbacks.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input: pCallLeg - A pointer to a CallLeg object.
*        strCbID  - A const string which identifies the callback that this
*                   function was called from.
* Output: (-)
***************************************************************************/
static RvStatus RVCALLCONV ConfirmCallbackCallLeg(IN CallLeg       *pCallLeg,
                                                  IN const RvChar  *strCbID)
{
#ifdef RV_SIP_SUBS_ON
    if(CallLegGetIsHidden(pCallLeg) == RV_TRUE &&
       CallLegGetIsRefer(pCallLeg)  == RV_FALSE)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "ConfirmCallbackCallLeg: Call-leg 0x%p - wrong callback (%s) for hidden call-leg",
            pCallLeg,strCbID));
        return RV_ERROR_ILLEGAL_ACTION;
    }
#else /* #ifdef RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(pCallLeg)
	RV_UNUSED_ARG(strCbID)
#endif /* #ifdef RV_SIP_SUBS_ON */
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(strCbID);
#endif

    return RV_OK;
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/

