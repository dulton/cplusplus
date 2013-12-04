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
 *                              <RvSipCallLeg.c>
 *
 * The Call-Leg functions of the RADVISION SIP stack enable you to create and
 * manage call-leg objects, connect and disconnect calls and control call-leg
 * parameters.
 * Call-Leg API functions are grouped as follows:
 * The Call-Leg Manager API
 * ------------------------
 * The call-leg manager is in charge of all the call-legs. It is used
 * to set the event handlers of the call-leg module and to create
 * new call-legs.
 *
 * The Call-Leg API
 * -----------------
 * A call-leg represents a SIP call leg as defined in RFC 2543. This
 * means that a call-leg is defined using the Call-ID, From and To
 * headers. Using the call-leg API, the user can initiate calls, react
 * to incoming calls and disconnect calls. Functions to set and
 * access the call-leg fields are also available in the API. A call-leg
 * is a state-full object and has a set of states associated with it.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipCallLegTypes.h"
#include "RvSipCallLeg.h"
#include "CallLegObject.h"
#include "CallLegSession.h"
#include "_SipTransport.h"
#include "RvSipReferToHeader.h"
#include "_SipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipReplacesHeader.h"
#include "CallLegRefer.h"
#include "CallLegReplaces.h"
#include "CallLegHighAvailability.h"
#include "CallLegAuth.h"
#include "CallLegSubs.h"
#include "CallLegInvite.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "CallLegSessionTimer.h"
#include "_SipReplacesHeader.h"
#include "_SipSubs.h"
#include "_SipCallLeg.h"
#include "RvSipSubscription.h"
#include "CallLegForking.h"
#ifdef RV_SIP_IMS_ON
#include "_SipAuthorizationHeader.h"
#include "CallLegSecAgree.h"
#endif /*#ifdef RV_SIP_IMS_ON*/
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
#include "CallLegDNS.h"
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

#include "_SipCommonUtils.h"
#include "_SipCommonCSeq.h"
#include "_SipCommonConversions.h"
#include "_SipHeader.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "CallLegRouteList.h"

#include "TransactionObject.h"
#include "TransmitterObject.h"
#include "RvSipTransmitter.h"

#include "rvexternal.h"

#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV CallLegNoActiveTransc(IN  CallLeg* pCallLeg);
static RvBool CallLegBlockTermination(IN CallLeg* pCallLeg);
static RvBool CallLegBlockCancellation(IN CallLeg* pCallLeg, RvChar* funcName);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
              C A L L  - L E G      M A N A G E R
 ------------------------------------------------------------------------*/


/***************************************************************************
 * RvSipCallLegMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for all call-leg events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the event handler structure.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the call-leg manager.
 *            pEvHandlers - Pointer to structure containing application event
 *                        handler pointers.
 *            structSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrSetEvHandlers(
                                   IN  RvSipCallLegMgrHandle  hMgr,
                                   IN  RvSipCallLegEvHandlers *pEvHandlers,
                                   IN  RvInt32               structSize)
{

    CallLegMgr *pMgr = (CallLegMgr*)hMgr;
    RvInt32 minStructSize = RvMin(structSize,((RvInt32)sizeof(RvSipCallLegEvHandlers)));

    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pEvHandlers == NULL)
    {
        return RV_ERROR_NULLPTR;
    }


    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/
    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipCallLegMgrSetEvHandlers - Setting event handlers to call-leg mgr"));

    memset(&(pMgr->callLegEvHandlers),0,sizeof(RvSipCallLegEvHandlers));
    memcpy(&(pMgr->callLegEvHandlers),pEvHandlers,minStructSize);

    if(pEvHandlers->pfnNestedInitialReqRcvdEvHandler != NULL)
    {
        pMgr->bEnableNestedInitialRequestHandling = RV_TRUE;
        SipTransacMgrSetEnableNestedInitialRequest(pMgr->hTranscMgr);
    }
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/

    return RV_OK;

}


/***************************************************************************
 * RvSipCallLegMgrCreateCallLeg
 * ------------------------------------------------------------------------
 * General: Creates a new Outgoing call-leg and exchange handles with the
 *          application.  The new call-leg assumes the Idle state.
 *          To establish a new session
 *          1. Create a new call-leg with this function.
 *          2. Set, at least, the To and From header.
 *          3. Call the Connect() function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the call-leg handle is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Call list is full,call-leg was not
 *                                   created.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - Handle to the call-leg manager
 *            hAppCallLeg - Application handle to the newly created call-leg.
 * Output:     RvSipCallLegHandle -   RADVISION SIP stack handle to the call-leg
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrCreateCallLeg(
                                   IN  RvSipCallLegMgrHandle hCallLegMgr,
                                   IN  RvSipAppCallLegHandle hAppCallLeg,
                                   OUT RvSipCallLegHandle    *phCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                   )
{

    CallLeg            *pCallLeg;
    RvStatus          rv;
    CallLegMgr *pMgr = (CallLegMgr*)hCallLegMgr;

    if(phCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phCallLeg = NULL;

    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipCallLegMgrCreateCallLeg - Creating a new call-leg"));

    /*create a new call-leg. This will lock the call-leg manager*/
    rv = CallLegMgrCreateCallLeg(pMgr,RVSIP_CALL_LEG_DIRECTION_OUTGOING,RV_FALSE,phCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipCallLegMgrCreateCallLeg - Failed, CallLegMgr failed to create a new call"));
        return rv;
    }
    pCallLeg = (CallLeg*)*phCallLeg;
    pCallLeg->hAppCallLeg = hAppCallLeg; /*set the application handle*/
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    pCallLeg->cctContext = cctContext;
    pCallLeg->URI_Cfg_Flag = URI_cfg;
#endif
//SPIRENT_END

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipCallLegMgrCreateCallLeg - New Call-Leg 0x%p was created successfully",*phCallLeg));
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application call-leg manger object.
 *          You set this handle in the stack using the
 *          RvSipCallLegMgrSetAppMgrHandle() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - Handle to the stack call-leg manager.
 * Output:     pAppCallLegMgr - The application call-leg manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrGetAppMgrHandle(
                                   IN RvSipCallLegMgrHandle hCallLegMgr,
                                   OUT void**               pAppCallLegMgr)
{
    CallLegMgr  *pCallMgr;
    if(hCallLegMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pCallMgr = (CallLegMgr *)hCallLegMgr;

    RvMutexLock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*lock the manager*/

    *pAppCallLegMgr = pCallMgr->pAppCallMgr;

    RvMutexUnlock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this call-leg
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr     - Handle to the stack call-leg manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrGetStackInstance(
                                   IN   RvSipCallLegMgrHandle   hCallLegMgr,
                                   OUT  void*       *phStackInstance)
{
    CallLegMgr  *pCallMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;
    if(hCallLegMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pCallMgr = (CallLegMgr *)hCallLegMgr;

    RvMutexLock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*lock the manager*/

    *phStackInstance = pCallMgr->hStack;

    RvMutexUnlock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own call-leg manager handle.
 *          You can use the RvSipCallLegMgrSetAppMgrHandle function to
 *          save this handle in the stack call-leg manager object.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr - Handle to the stack call-leg manager.
 *           pAppCallLegMgr - The application call-leg manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMgrSetAppMgrHandle(
                                   IN RvSipCallLegMgrHandle hCallLegMgr,
                                   IN void*               pAppCallLegMgr)
{
    CallLegMgr  *pCallMgr;
    if(hCallLegMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pCallMgr = (CallLegMgr *)hCallLegMgr;

    RvMutexLock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*lock the manager*/

    pCallMgr->pAppCallMgr = pAppCallLegMgr;

    RvMutexUnlock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/*-----------------------------------------------------------------------
       C A L L  - L E G:  S E S S I O N   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegMake
 * ------------------------------------------------------------------------
 * General: Set the To and From header in the call-leg and
 *          initiate an outgoing call. This function will cause
 *          an INVITE to be sent out and the call-leg state machine
 *          will progress to the Inviting state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to connect
 *          strFrom  - A string with the from party header for example
 *                     "From:sip:172.20.1.1:5060"
 *          strTo -   A string with the to party header for example:
 *                     "To:sip:172.20.5.5:5060"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegMake (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvChar*             strFrom,
                                         IN  RvChar*             strTo)
{
    RvStatus               rv;
    RvSipPartyHeaderHandle  hFrom;    /*handle to the from header*/
    RvSipPartyHeaderHandle  hTo;      /*handle to the to header*/
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL|| strFrom == NULL || strTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Call-leg 0x%p, making a call from %s to %s",
              pCallLeg,strFrom,strTo));

    /*-----------------------------------------
      get handles for the to and from headers
     ------------------------------------------*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Call-leg 0x%p, getting To and From party header handles",pCallLeg));
    rv = RvSipCallLegGetNewPartyHeaderHandle(hCallLeg,&hFrom);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Failed for Call-leg 0x%p, failed to get From party header handle",pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    rv = RvSipCallLegGetNewPartyHeaderHandle(hCallLeg,&hTo);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Failed for Call-leg 0x%p, failed to get To party header handle",pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    /*-----------------------------------------------
      Parse the given strings into the party headers
    -------------------------------------------------*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Call-leg 0x%p, Parsing To and From party headers",pCallLeg));

    rv = RvSipPartyHeaderParse(hTo,RV_TRUE,strTo);

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Failed for Call-leg 0x%p, To header has bad syntax",pCallLeg));

        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    rv = RvSipPartyHeaderParse(hFrom,RV_FALSE,strFrom);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Failed for Call-leg 0x%p, From header has bad syntax",pCallLeg));

        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    /*-------------------------------------------------------
       Set the header back in the call-Leg
      -------------------------------------------------------*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Call-leg 0x%p, Setting To and From party headers",pCallLeg));

    rv = RvSipCallLegSetToHeader(hCallLeg,hTo);
    if(rv != RV_OK)
    {
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    rv = RvSipCallLegSetFromHeader(hCallLeg,hFrom);
    if(rv != RV_OK)
    {
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    /*----------------------------------
       Connect the call
     *----------------------------------*/
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegMake - Call-leg 0x%p, Calling connect",pCallLeg));

    rv = RvSipCallLegConnect(hCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegConnect
 * ------------------------------------------------------------------------
 * General: Initiate an outgoing call. This method may be called
 *          only after the To, From fields have been set. Calling
 *          Connect causes an INVITE to be sent out and the
 *          call-leg state machine to progress to the Inviting state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to connect
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegConnect(
                                           IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
#ifdef RV_SIP_SUBS_ON
    if(pCallLeg->bIsReferCallLeg == RV_TRUE || pCallLeg->bFirstReferExists == RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegConnect - Failed for call-leg 0x%p: Illegal Action for refer call-leg",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegConnect - Connecting call-leg 0x%p",pCallLeg));


    /*connect can be called only on state idle or redirected, when the
      refer state is idle */
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE &&
       pCallLeg->eState != RVSIP_CALL_LEG_STATE_REDIRECTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegConnect - Failed for call-leg 0x%p: Illegal Action in state %s",
                  pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegConnect - Failed for call-leg 0x%p: Illegal Action for a 'refer call-leg'",
                  pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegSessionConnect(pCallLeg);
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegConnect - call-leg 0x%p returning from function with rv=%d",
               pCallLeg,rv));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}


/***************************************************************************
 * RvSipCallLegAccept
 * ------------------------------------------------------------------------
 * General: Called by the application to indicate that it is willing to accept
 *          an incoming call or an incoming re-Invite.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 200 response).
 *               RV_OK       -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to accept
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAccept (IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegAccept - call-leg 0x%p is no longer a valid call",pCallLeg));
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegAccept - Accepting call-leg 0x%p",pCallLeg));
    
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        /* the accept may be only on state offering - for initial invite,
           or it may be for a re-invite, and then the call must be on state connected */
        if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_OFFERING &&
           pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAccept - Failed to accept call-leg 0x%p: Illegal Action in state %s (old invite handling)",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }


    /*accept can be called only on offering - for initial invite, or
      on states connected, accepted and remote-accepted - for re-invite.*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING) ||
       (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CANCELLED && pCallLeg->pMgr->manualBehavior == RV_TRUE)||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_ACCEPTED  ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED ||
       (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ))
       /* we will check the re-invite states inside the session-accept function */
    {
        rv =  CallLegSessionAccept(pCallLeg);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegAccept - Failed to accept call-leg 0x%p: Illegal Action in state %s",
                  pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        rv = RV_ERROR_ILLEGAL_ACTION;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "RvSipSecAgree.h"

//Function to workaround the SipTransmitterSendMessage::SecObjGetSecureLocalAddressAndConnection 
//problem in the middle of re-registration when we are trying to reply through the SA 
//which has no server TCP connection open
RVAPI RvBool RVCALLCONV RvSipCallLegCanAccept (
                                        IN  RvSipCallLegHandle   hCallLeg,
                                        IN RvSipSecAgreeHandle auxsa1,
                                        IN RvSipSecAgreeHandle auxsa2,
                                        IN RvSipSecAgreeHandle auxsa3) {

  RvBool ret=RV_FALSE;
  
  CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
  if(pCallLeg) {
    if(pCallLeg->hActiveTransc) {
      
      Transaction *transc=(Transaction*)pCallLeg->hActiveTransc;
      Transmitter *trx=(Transmitter*)transc->hTrx;
      Transmitter *trxa=(Transmitter*)(pCallLeg->hActiveTrx);
      
      if(trx) {
        
        RvSipSecObjHandle sotrx=trx->hSecObj;
        RvSipSecObjHandle sotran=transc->hSecObj;
        RvSipSecObjHandle sotrxa=NULL;

        if(trxa) sotrxa=trxa->hSecObj;
        
        if(!sotran && !sotrx && !sotrxa && !auxsa1 && !auxsa2 && !auxsa3) {
          ret=RV_TRUE;
        } else {
          
          RvSipSecAgreeHandle sa=NULL;
          RvSipSecObjHandle soact=NULL;
          RvSipSecObjHandle soaux1=NULL;
          RvSipSecObjHandle soaux2=NULL;
          RvSipSecObjHandle soaux3=NULL;

          RvSipSecObjHandle sonew=NULL;
          
          RvSipTransactionGetSecAgree(pCallLeg->hActiveTransc,&sa);

          if(sa) RvSipSecAgreeGetSecObj(sa,&soact);
          if(auxsa1) RvSipSecAgreeGetSecObj(auxsa1,&soaux1);
          if(auxsa2) RvSipSecAgreeGetSecObj(auxsa2,&soaux2);
          if(auxsa3) RvSipSecAgreeGetSecObj(auxsa3,&soaux3);
          
          if(sotran && RvSipSecObjIsReady(sotran,1) && sotran!=sotrx) {
            sonew=sotran;
          } else if(!sotrx || !RvSipSecObjIsReady(sotrx,1)) {
            if(sotrxa && RvSipSecObjIsReady(sotrxa,1)) sonew=sotrxa;
            else if(soact && RvSipSecObjIsReady(soact,1)) sonew=soact;
            else if(soaux1 && RvSipSecObjIsReady(soaux1,1)) sonew=soaux1;
            else if(soaux2 && RvSipSecObjIsReady(soaux2,1)) sonew=soaux2;
            else if(soaux3 && RvSipSecObjIsReady(soaux3,1)) sonew=soaux3;

            if(sonew && sonew!=sotrx) {
              RvSipTransmitterSetSecObj((RvSipTransmitterHandle)trx,sonew);
              ret=RV_TRUE;
            }
          } else {
            ret=RV_TRUE;
          }
        }
      } else {
        //let RV stack handle this case
        ret=RV_TRUE;
      }
    } else {
      //let RV stack handle this case
      ret=RV_TRUE;
    }
  } else {
    //let RV stack handle this case
    ret=RV_TRUE;
  }

  return ret;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegByeAccept
 * ------------------------------------------------------------------------
 * General: Called by the application to indicate that it is willing to accept
 *          an incoming BYE.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_ILLEGAL_ACTION -  Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 200 response).
 *               RV_OK       -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to accept its BYE.
 *          hTransc  - handle to the BYE transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegByeAccept (IN  RvSipCallLegHandle   hCallLeg,
                                                  IN  RvSipTranscHandle    hTransc)
{

    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite* pInvite = NULL;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegByeAccept - Accepting call-leg 0x%p BYE transaction",pCallLeg));

    rv = CallLegSessionByeAccept(pCallLeg, hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegByeAccept - Failed, call-leg 0x%p",pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    /* after disconnect sequence has completed, we can kill the call */
    rv = CallLegInviteFindObjByState(pCallLeg, RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD, &pInvite);
    if(rv == RV_OK && pInvite != NULL)
    {
        /* If the application created a message in advance use it*/
        if (NULL != pCallLeg->hOutboundMsg)
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegByeAccept - Call-leg 0x%p - automatically responding to pending re-INVITE 0x%p with stack 487 response. Application outbound message is discarded. ",
                pCallLeg, pInvite));
            RvSipMsgDestruct(pCallLeg->hOutboundMsg);
            pCallLeg->hOutboundMsg = NULL;
        }
        rv = RvSipTransactionRespond(pInvite->hInviteTransc,487,NULL);
		/*detach the transaction for retransmissions*/
		if(rv == RV_OK)
		{
			CallLegInviteDetachInviteTransc(pCallLeg, pInvite, pCallLeg->hActiveTransc);
		}
    }

    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}




/***************************************************************************
 * RvSipCallLegReject
 * ------------------------------------------------------------------------
 * General: Can be used in the Offering state to reject an incoming call.
 *          This function can also be used to reject a modify (re-Invite)
 *          request received by a connected call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to reject
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReject (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16            status)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReject - Rejecting call-leg 0x%p",pCallLeg));

    /*invalid status code*/
    if (status < 300 || status >= 700)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegReject - Failed to reject Call-leg 0x%p. %d is not a valid error response code",
                  pCallLeg,status));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    switch (pCallLeg->eState) /* validity checks */
    {
    case RVSIP_CALL_LEG_STATE_OFFERING:
        break;
    case RVSIP_CALL_LEG_STATE_CANCELLED:
        if (RV_TRUE == pCallLeg->pMgr->manualBehavior)
        {
            break;
        }
        /* reject a re-invite */
    case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
        {
            CallLegInvite* pInvite = NULL;
            rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
            if (rv == RV_OK && pInvite != NULL &&
                (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD ||
                 pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED))
            {
                break;
            }
        }
        /* don't put "break" here */
    default:
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                         "RvSipCallLegReject - Failed to reject call-leg 0x%p: Illegal Action in state %s, no modify",
                      pCallLeg, CallLegGetStateName(pCallLeg->eState)));

            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
    } /* switch (pCallLeg->eState) */

    rv = CallLegSessionReject(pCallLeg,status);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegByeReject
 * ------------------------------------------------------------------------
 * General: Can be used in the Offering state to reject an incoming call.
 *          This function can also be used to reject a modify (re-Invite)
 *          request received by a connected call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to reject
 *          hTransc  - Handle to the BYE transaction
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegByeReject (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvSipTranscHandle    hTransc,
                                         IN  RvUint16            status)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegByeReject - Rejecting call-leg 0x%p BYE request",pCallLeg));

    /*invalid status code*/
    if (status < 300 || status >= 700)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegByeReject - Failed to reject Call-leg 0x%p BYE request. %d is not a valid error response code",
                  pCallLeg,status));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    rv = CallLegSessionByeReject(pCallLeg, hTransc, status);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}




/***************************************************************************
 * RvSipCallLegProvisionalResponse
 * ------------------------------------------------------------------------
 * General: Sends a provisional response (1xx class) to the remote party.
 *          This function can be called when ever a request is received for
 *          example in the offering state.
 * Return Value: RV_ERROR_UNKNOWN - Failed to send provisional response.
 *               RV_OK - Provisional response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 *            status -   The provisional response status code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegProvisionalResponse (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16 status)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = NULL;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegProvisionalResponse - Sending provisional response %d on call-leg 0x%p",status,pCallLeg));

    /*invalid status code*/
    if (status < 100 || status >= 200)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegProvisionalResponse - Failed for Call-leg 0x%p. %d is not a valid provisional response code",
                  pCallLeg,status));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegProvisionalResponse - Failed for Call-leg 0x%p. did not find invite object",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    /*check if the call state allows provisional responses*/
    
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING) ||
       (pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTED) ||
       ((pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ||
         pCallLeg->eState == RVSIP_CALL_LEG_STATE_ACCEPTED ||
         pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)&&
        pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD))
    {
        rv = CallLegSessionProvisionalResponse(pCallLeg,status,RV_FALSE);
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;

    }
    else if(NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        RvSipSubsState eReferState;

        eReferState = SipSubsGetCurrState(CallLegGetActiveReferSubs(pCallLeg));
        if(RVSIP_SUBS_STATE_SUBS_RCVD == eReferState)
        {
            /* 1xx on REFER request */
            rv = SipSubsRespondProvisional(CallLegGetActiveReferSubs(pCallLeg), status, NULL);
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegProvisionalResponse - Failed for call-leg 0x%p: Illegal Action in state %s modify state %s",
                  pCallLeg, CallLegGetStateName(pCallLeg->eState),
                  CallLegGetModifyStateName(pInvite->eModifyState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
/***************************************************************************
 * RvSipCallLegAck
 * ------------------------------------------------------------------------
 * General: This function sends ACK only for the initial invite.
 *          You may use it for re-invite, only if you configured your
 *          stack to work with the 'old invite' behavior!
 *
 *          The function sends an ACK request from the call-leg to the remote party.
 *          When the stack is configured to work in a manual ACK mode the
 *          call-leg will not send the ACK message after receive a 2xx
 *          response by itself. The application should use the RvSipCallLegAck()
 *          function to trigger the call-leg to send the ACK.
 *          This function can be called only in the Remote-Accepted state.
 *          or in the Modify Re-Invite Remote Accepted state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAck (IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus   rv = RV_OK;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegAck - Sending ACK on call-leg 0x%p",pCallLeg));

    if(pCallLeg->pMgr->manualAckOn2xx == RV_FALSE && pCallLeg->pMgr->manualBehavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegAck - Failed for call-leg 0x%p, manual ACK was not configured",pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_FALSE)
    {
        /* application wants to work with the new invite handling.
        in this case, this function sends ACK only on initial invite! */
        if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAck - Failed for call-leg 0x%p. Can't send ack in state %s. bOldInviteHandling=false",
                pCallLeg,CallLegGetStateName(pCallLeg->eState)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        /* The modify state will be checked inside the SessionAck function. */
        rv = CallLegSessionAck(pCallLeg, NULL);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAck - Failed for call-leg 0x%p",pCallLeg));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegAck - Failed for call-leg 0x%p. Can't send ack in state %s",
                  pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;


}



/***************************************************************************
 * RvSipCallLegProvisionalResponseReliable
 * ------------------------------------------------------------------------
 * General: Sends a reliable provisional response (1xx class other then 100)
 *          to the remote party.
 *          This function can be called when an Invite request is received for
 *          example in the Offering state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 *            status -   The provisional response status code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegProvisionalResponseReliable (
                                         IN  RvSipCallLegHandle   hCallLeg,
                                         IN  RvUint16            status)
{

    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = NULL;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegProvisionalResponseReliable - Sending provisional response %d on call-leg 0x%p",status,pCallLeg));

    /*invalid status code*/
    if (status <= 100 || status >= 200)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegProvisionalResponseReliable - Failed for Call-leg 0x%p. %d is not a valid reliable provisional response code",
                  pCallLeg,status));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegProvisionalResponseReliable - Failed for Call-leg 0x%p. did not find invite object",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    
    /*check if the call state allows provisional responses*/
    if((pCallLeg->eState == RVSIP_CALL_LEG_STATE_OFFERING) ||
       ((pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ||
         pCallLeg->eState == RVSIP_CALL_LEG_STATE_ACCEPTED  ||
         pCallLeg->eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED) &&
        pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD))

    {
        rv = CallLegSessionProvisionalResponse(pCallLeg,status,RV_TRUE);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegProvisionalResponseReliable - Failed for call-leg 0x%p: Illegal Action in state %s",
                  pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        rv = RV_ERROR_ILLEGAL_ACTION;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGet100RelStatus
 * ------------------------------------------------------------------------
 * General: Return the 100rel option tag status of a received Invite request.
 *          The 100rel option tag indicates whether the remote party
 *          Support/Require the PRACK extension. In case of a Require status
 *          the application should use the RvSipCallLegProvisionalResponseReliable
 *          and send a reliable provisional response.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The transaction handle.
 * Output:  relStatus - the reliable status received in the INVITE request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGet100RelStatus(
                                    IN RvSipCallLegHandle            hCallLeg,
                                    OUT RvSipTransaction100RelStatus *relStatus)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = NULL;

    *relStatus = RVSIP_TRANSC_100_REL_UNDEFINED;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
         return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->hActiveTransc == NULL)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_OK;
    }

    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGet100RelStatus - Failed for Call-leg 0x%p. did not find invite object. illegal action",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_OFFERING &&
        pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_OK;
    }

    rv = RvSipTransactionGet100RelStatus(pCallLeg->hActiveTransc,relStatus);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegSendPrack
 * ------------------------------------------------------------------------
 * General: When the SIP stack is configured to work in a manual PRACK
 *          mode the application is responsible for generating the PRACK
 *          message whenever a reliable provisional response is received.
 *          When a reliable provisional response is received the call-leg
 *          PRACK state machine assumes the REL_PROV_RESPONSE_RCVD.
 *          You should then call the RvSipCallLegSendPrack() function to
 *          send the PRACK message to the remote party. The call-leg PRACK
 *          state machine will then assume the PRACK_SENT state.
 *          Note: The stack is responsible for adding the RAck header
 *          to the PRACK message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - The transaction handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSendPrack(
                                    IN RvSipCallLegHandle            hCallLeg)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSendPrack - Sending PRACK call-leg 0x%p",pCallLeg));

    if (pCallLeg->pMgr->manualPrack == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegSendPrack - Failed for call-leg 0x%p: Can't send PRACK, manualPrack is false",
                  pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*send PRACK can be called only on PRACK state rel prev response rcvd or
      in state prack-final-response-rcvd to enable resending the PRACK when 401/407 was received.*/
    if(pCallLeg->ePrackState != RVSIP_CALL_LEG_PRACK_STATE_REL_PROV_RESPONSE_RCVD &&
       pCallLeg->ePrackState != RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSendPrack - Failed for call-leg 0x%p: Can't send PRACK, prack state of the call is %s",
              pCallLeg, CallLegGetPrackStateName(pCallLeg->ePrackState)));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegSessionSendPrack(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSendPrackResponse
 * ------------------------------------------------------------------------
 * General: When the SIP stack is configured to work in a manual PRACK
 *          mode the application is responsible to respond to any PRACK
 *          request that is received for a previously sent
 *          reliable provisional response.
 *          When a PRACK request is received the call-leg PRACK state
 *          machine assumes the PRACK_RCVD state.
 *          You should then call the RvSipCallLegSendPrackResponse() function to
 *          send a response to the PRACK request. The call-leg PRACK
 *          state machine will then assume the PRACK_FINAL_RESPONSE_SENT state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The transaction handle.
 *            responseCode - The response code to send.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSendPrackResponse(
                                    IN RvSipCallLegHandle            hCallLeg,
                                    IN RvUint16                     responseCode)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSendPrackResponse - Call-leg 0x%p sends response to PRACK.",pCallLeg));

    if (pCallLeg->pMgr->manualPrack == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegSendPrackResponse - Failed for call-leg 0x%p: Can't send response to PRACK, manualPrack is false",
                  pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*send PRACK response can be called only on PRACK state PRACK rcvd */
    if(pCallLeg->ePrackState != RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegSendPrackResponse - Failed for call-leg 0x%p: Can't send response to PRACK, prack state of the call is not PRACK rcvd",
                  pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(responseCode < 200 || responseCode > 699)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegSendPrackResponse - Failed for call-leg 0x%p: Can't send response to PRACK, response code must be between 200 to 699",
                  pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = CallLegSessionSendPrackResponse(pCallLeg, responseCode);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}



/***************************************************************************
 * RvSipCallLegGetCallLegMgr
 * ------------------------------------------------------------------------
 * General: Return the call leg manager handle
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The call leg handle.
 * Output:  phCallLegMgr - The call leg manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCallLegMgr(
                                    IN  RvSipCallLegHandle            hCallLeg,
                                    OUT RvSipCallLegMgrHandle         *phCallLegMgr)
{
    RvStatus                rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(phCallLegMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phCallLegMgr = NULL;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
         return RV_ERROR_INVALID_HANDLE;
    }
    *phCallLegMgr = (RvSipCallLegMgrHandle)pCallLeg->pMgr;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegDisconnect
 * ------------------------------------------------------------------------
 * General: Causes the call to disconnect.
 *          Disconnect() may be called in any state, but you may not use this
 *          function when the stack is in the middle of processing one of
 *          the following callbacks:
 *          pfnMsgReceivedEvHandler, pfnMsgToSendEvHandler,
 *          pfnFinalDestResolvedEvHandler, pfnNewConnInUseEvHandler.
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 *          The behavior of the function depends on the following call-leg states:
 *
 *          Connected, Accepted, Inviting
 *          -----------------------------
 *          Bye is sent and the call moves to the Disconnecting state.
 *
 *          Offering
 *          --------
 *          The incoming Invite is rejected with status code 603.
 *
 *          Idle, Disconnecting, Disconnected , Redirected ,Unauthenticated
 *          ---------------------------------------------------------------
 *          The call is terminated.
 *
 *          If the functions fail to send the BYE request, the call-leg will
 *          be terminated.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to disconnect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDisconnect (
                                            IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegDisconnect - Disconnecting call-leg 0x%p",pCallLeg));

    if(RV_TRUE == CallLegBlockCancellation(pCallLeg, "RvSipCallLegDisconnect"))
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_TRY_AGAIN;

    }

    /* If application already called RvSipCallLegDisconnect, the call-leg state
       is changed to DISCONNECTING only when the message sending is completed.
       Now, if on the middle of the sending the application calls RvSipCallLegDisconnect
       again (e.g in the newConInUse callback) we need to block the operation,
       so we won't have 2 BYE transactions in the call-leg */
    if(pCallLeg->eNextState == RVSIP_CALL_LEG_STATE_DISCONNECTING)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegDisconnect - Call-leg 0x%p is already in the middle of Disconnecting",pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_OK;
    }
    pCallLeg->eNextState = RVSIP_CALL_LEG_STATE_DISCONNECTING;
    
    rv = CallLegSessionDisconnect(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a call-leg without sending any messages (CANCEL or
 *          BYE). The call-leg will assume the Terminated state.
 *          Calling this function will cause an abnormal
 *          termination. All transactions related to the call-leg will be
 *          terminated as well.
 *            
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnCallLegCreatedEvHandler, pfnReInviteCreatedEvHandler,
 *          pfnTranscCreatedEvHandler, pfnByeCreatedEvHandler, pfnSessionTimerNegotiationFaultEvHandler
 *          pfnSessionTimerRefreshAlertEvHandler, pfnSigCompMsgNotRespondedEvHandler,
 *          pfnNestedInitialReqRcvdEvHandler, pfnNewConnInUseEvHandler, 
 *          pfnCallLegCreatedDueToForkingEvHandler, pfnProvisionalResponseRcvdEvHandler
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTerminate (
                                            IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    if(RV_TRUE == CallLegBlockTermination(pCallLeg))
    {    
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_TRY_AGAIN;

    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegTerminate - Terminating call-leg... 0x%p",pCallLeg));

#ifdef RV_SIP_SUBS_ON
    CallLegSubsTerminateAllSubs(pCallLeg);
#endif /* #ifdef RV_SIP_SUBS_ON */

    if (RVSIP_CALL_LEG_STATE_TERMINATED == pCallLeg->eState)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_OK;
    }

    CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegDetachOwner
 * ------------------------------------------------------------------------
 * General: Detach the Call-Leg owner. After calling this function the user
 *          will stop receiving events for this call-leg.
 *          This function can be called only when the object is in terminated
 *          state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to detach its owner.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDetachOwner(IN RvSipCallLegHandle  hCallLeg)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
          "RvSipCallLegDetachOwner - Call leg 0x%p is in state %s, and not in state terminated - can't detach owner",pCallLeg, CallLegGetStateName(pCallLeg->eState)));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegDetachOwner - Detaching owner of call-leg 0x%p",pCallLeg));

    pCallLeg->callLegEvHandlers = NULL;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegCancel
 * ------------------------------------------------------------------------
 * General: Cancels an INVITE request (or re-Invite request). Calling this
 *          function causes a CANCEL message to be sent to the remote party.
 *          You can call this function in the
 *          following call-leg states:
 *          -RVSIP_CALL_LEG_STATE_PROCEEDING
 *          -PROCEEDING_TIMEOUT
 *          and in the following MODIFY states:
 *          -RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING
 *          -RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnMsgReceivedEvHandler, pfnMsgToSendEvHandler,
 *          pfnFinalDestResolvedEvHandler.
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The handle to the call-leg the user wishes to cancel.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegCancel (
                                    IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = NULL;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    if(pCallLeg->eState !=  RVSIP_CALL_LEG_STATE_CONNECTED &&
       pCallLeg->bOriginalCall == RV_FALSE)
    {
        /* cancel on initial invite, in forked call-leg */
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegCancel - Failed to cancel call-leg 0x%p: Illegal Action for 'forked call-leg'",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(RV_TRUE == CallLegBlockCancellation(pCallLeg,"RvSipCallLegCancel"))
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_TRY_AGAIN;

    }

    rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
    if(rv != RV_OK || pInvite == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegCancel - Failed for Call-leg 0x%p. did not find invite object",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }
    
    switch (pCallLeg->eState)
    {
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
    case RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT:
        break;
	case RVSIP_CALL_LEG_STATE_CONNECTED:
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
        {
            if (pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING ||
                pInvite->eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT)
            {
                break;
            }
        }
        /* don't put "break" here */
    default: /* not valid */
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegCancel - Failed to cancel call-leg 0x%p: Illegal Action in state %s, (Modify state=%s)",
                   pCallLeg,CallLegGetStateName(pCallLeg->eState),
                   CallLegGetModifyStateName(pInvite->eModifyState)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegCancel - sending cancel on call-leg 0x%p",pCallLeg));

    rv = CallLegSessionCancel(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/*--------------------------------------------------------------------------
 *          R E - I N V I T E
 *-------------------------------------------------------------------------- */
/***************************************************************************
 * RvSipCallLegReInviteCreate
 * ------------------------------------------------------------------------
 * General: Creates a new re-INVITE object in a call-leg. 
 *          A re-invite object may be created on 2 conditions:
 *          1. The call-leg had already sent/received the 2xx response 
 *             for the initial INVITE request 
 *             (the call-leg state is CONNECTED or ACCEPTED or REMOTE-ACCEPTED),
 *          2. There are no other pending re-Invite transactions. (no other 
 *             re-invite object which waits for a final response).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to modify.
 *            hAppReInvite - Application handle to the new re-invite object.
 * Output:    phReInvite - Pointer to the new re-invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteCreate(
                                     IN   RvSipCallLegHandle          hCallLeg,
                                     IN   RvSipAppCallLegInviteHandle hAppReInvite,
                                     OUT  RvSipCallLegInviteHandle*   phReInvite)
{
    RvStatus  rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteCreate - Failed. call-leg 0x%p: Illegal Action. stack configured with old invite behavior",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteCreate - Failed. call-leg 0x%p: Illegal Action in state %s",
            pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReInviteCreate - creating a new re-invite object in call-leg 0x%p", pCallLeg));

    rv = CallLegInviteCreateObj(pCallLeg, hAppReInvite, RV_TRUE, 
                                RVSIP_CALL_LEG_DIRECTION_OUTGOING, (CallLegInvite**)phReInvite, NULL);


    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegReInviteRequest
 * ------------------------------------------------------------------------
 * General: Sends a re-INVITE request. 
 *			NOTE: This function doesn't refer to 
 *			Session-Timer Call-Leg's parameters. Thus, when used during a 
 *			Session Timer call, it turns off the Session Timer mechanism. 
 *			Consequently, in order to keep the mechanism up please use 
 *			the function RvSipCallLegSessionTimerInviteRefresh() instead.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call leg the user wishes to modify.
 *            hReInvite - Handle to the re-invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteRequest(
                                     IN   RvSipCallLegHandle         hCallLeg,
                                     IN   RvSipCallLegInviteHandle   hReInvite )
{
    RvStatus  rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = (CallLegInvite*)hReInvite;

    if(pCallLeg == NULL || hReInvite == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteRequest - Failed. call-leg 0x%p: Illegal Action. stack configured with old invite behavior",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReInviteRequest - sending re-invite from call-leg 0x%p, re-invite 0x%p",
              pCallLeg, pInvite));

    if(RV_FALSE == CallLegIsStateLegalForReInviteHandling(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteRequest - Call leg 0x%p, pInvite 0x%p illegal state (%s + active transc = 0x%p)", 
            pCallLeg, hReInvite, CallLegGetStateName(pCallLeg->eState), pCallLeg->hActiveTransc));
        
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if(pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_IDLE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "RvSipCallLegReInviteRequest - Failed send re-invite. call-leg 0x%p, re-invite 0x%p: re-invite state %s, active transc = 0x%p",
               pCallLeg,pInvite,CallLegGetModifyStateName(pInvite->eModifyState),pCallLeg->hActiveTransc));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }

    rv = CallLegSessionModify(pCallLeg, (CallLegInvite*)hReInvite);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegReInviteSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the re-invte application handle. Usually the application
 *          replaces handles with the stack in the RvSipCallLegReInviteCreatedEv()
 *          callback or the RvSipCallLegReInviteCreate() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call leg.
 *            hReInvite    - Handle to the re-invite object.
 *            hAppReInvite - Handle to the application re-invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteSetAppHandle (
                                IN  RvSipCallLegHandle          hCallLeg,
                                IN  RvSipCallLegInviteHandle    hReInvite,
                                IN  RvSipAppCallLegInviteHandle hAppReInvite)
{
    RvStatus   rv = RV_OK;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = (CallLegInvite*)hReInvite;
    
    if(pCallLeg == NULL || hReInvite == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    pInvite->hAppInvite = hAppReInvite;
    
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegReInviteAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request for 2xx response on re-invite.
 *          When the stack is configured to work in a manual ACK mode the
 *          call-leg will not send the ACK message after receive a 2xx
 *          response by itself. 
 *          The application should use the RvSipCallLegReInviteAck()
 *          function to trigger the call-leg to send the ACK.
 *          This function can be called only in the 
 *          Modify Re-Invite Remote Accepted state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call leg.
 *            hReInvite - Handle to the re-invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteAck (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite)
{
    RvStatus   rv = RV_OK;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = (CallLegInvite*)hReInvite;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteAck - Failed. call-leg 0x%p: Illegal Action. stack configured with old invite behavior",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(NULL == pInvite)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteAck - Failed for Call-leg 0x%p, no invite object was given."
            ,pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
            
    if(pCallLeg->pMgr->manualAckOn2xx == RV_FALSE && pCallLeg->pMgr->manualBehavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteAck - Failed for call-leg 0x%p, manual ACK was not configured",pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pInvite->eModifyState != RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteAck - Failed for Call-leg 0x%p, pInvite 0x%p - illegal action on state %s."
            ,pCallLeg, pInvite, CallLegGetModifyStateName(pInvite->eModifyState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegReInviteAck - Sending ACK on call-leg 0x%p, invite 0x%p",pCallLeg, hReInvite));
    
    
    rv = CallLegSessionAck(pCallLeg, pInvite);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegReInviteAck - Failed for call-leg 0x%p, invite 0x%p",
            pCallLeg, pInvite));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }
    
    
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;


}

/***************************************************************************
 * RvSipCallLegReInviteTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a re-invite object without sending any messages (CANCEL 
 *          or BYE). The re-invite object will assume the Terminated state.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 *            hReInvite - Handle to the re-invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteTerminate (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite)
{
    RvStatus   rv = RV_OK;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = (CallLegInvite*)hReInvite;

    if(pCallLeg == NULL || hReInvite == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegReInviteTerminate - Terminating invite 0x%p in call-leg 0x%p", hReInvite,pCallLeg));
    
    CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_UNDEFINED);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegReInviteGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the call-leg re-invite current state
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hCallLeg - The call-leg handle.
 *             hReInvite - Handle to the re-invite object.
 * Output:     peState -  The re-invite current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReInviteGetCurrentState (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipCallLegInviteHandle hReInvite,
                                            OUT RvSipCallLegModifyState  *peState)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite *pInvite = (CallLegInvite*)hReInvite;

    if(pCallLeg == NULL || hReInvite == NULL || peState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peState = pInvite->eModifyState;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipCallLegAuthenticate
 * ------------------------------------------------------------------------
 * General: Call this function to send a request with authentication information.
 *          If a call-leg receives a 401 or 407 response indicating that a
 *          request was not authenticated by the server or proxy, the
 *          call-leg assumes the Unauthenticated state.
 *          You may use RvSipCallLegAuthenticate() in the Unauthenticated
 *          state to re-send the request with authentication information.
 *          You can use this function to authenticate both the INVITE
 *          and BYE requests.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Cannot send request due to a resource problem.
 *               RV_ERROR_UNKNOWN - An error occurred. request was not sent.
 *               RV_OK - request was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to authenticate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthenticate(
                                           IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;
    RvBool   bReferAuth = RV_FALSE;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegAuthenticate - sending request for call-leg 0x%p",pCallLeg));

    /* SendRequest can be called only on state RequestAuthenticating */
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_UNAUTHENTICATED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegAuthenticate - Failed for call-leg 0x%p: Illegal Action in state %s",
                  pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE ||
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED )
    {
        /* this state is valid only when authenticating REFER request, with old refer behavior */
        if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE ||
           NULL == CallLegGetActiveReferSubs(pCallLeg))
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAuthenticate - Failed for call-leg 0x%p: Illegal Action in state %s (old refer=%d, hActiveReferSubs=%p)",
                pCallLeg,CallLegGetStateName(pCallLeg->eState),
                pCallLeg->pMgr->bDisableRefer3515Behavior, CallLegGetActiveReferSubs(pCallLeg)));
            CallLegUnLockAPI(pCallLeg);
            return RV_ERROR_ILLEGAL_ACTION;
        }
        bReferAuth = RV_TRUE;
        
    }

    if (bReferAuth == RV_FALSE)
    {
        switch (pCallLeg->eAuthenticationOriginState)
        {
            case RVSIP_CALL_LEG_STATE_INVITING:
            case RVSIP_CALL_LEG_STATE_PROCEEDING:
                {
                    rv = CallLegSessionConnect(pCallLeg);
                    break;
                }
            case RVSIP_CALL_LEG_STATE_DISCONNECTING:
                {
                    /*I don't call the disconnect function since it will disconnect
                      with no bye in the unauthenticated state since in the unauthenticated
                      state the application can decide to call disconnect and terminate the
                      call*/
                    rv = CallLegSessionSendBye(pCallLeg);
                    break;
                }
           default:
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                           "RvSipCallLegAuthenticate - unknown request type"));
                    CallLegUnLockAPI(pCallLeg);
                    return RV_ERROR_ILLEGAL_ACTION;
                }
        }
    }
#ifdef RV_SIP_SUBS_ON
    else 
    {
        rv = RvSipSubsRefer(pCallLeg->hActiveReferSubs);
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
    if (rv == RV_OK)
    {
        /* the request was sent successfully so the origin state returns to be undefined */
        pCallLeg->eAuthenticationOriginState = RVSIP_CALL_LEG_STATE_UNDEFINED;
    }

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegAuthenticate - Failed for call-leg 0x%p: rv=%d",
            pCallLeg,rv));
            
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}


/***************************************************************************
 * RvSipCallLegAuthBegin - Server authentication
 * ------------------------------------------------------------------------
 * General: Begin the server authentication process. challenge an incoming
 *          request.
 *          If the request is an active transaction (Invite, Bye),
 *          hCallLeg handle should be given, and hTransaction should be NULL.
 *          For general request, hTransaction should be given, and
 *          hCallLeg should be NULL.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to challenge
 *          hTransaction - Handle to the transaction user wishes to challenge
 *                         (in case of general request).
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegAuthBegin(
                                           IN  RvSipCallLegHandle hCallLeg,
                                           IN  RvSipTranscHandle  hTransaction)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegAuthBegin - Challenge an incoming request for call-leg 0x%p, transc 0x%p"
              ,pCallLeg, hTransaction));

    if(hTransaction != NULL)
    {
        rv = RvSipTransactionAuthBegin(hTransaction);
    }
#ifdef RV_SIP_SUBS_ON
    else if(CallLegGetActiveReferSubs(pCallLeg) != NULL)
    {
        /* authenticate the incoming REFER request */
        rv = RvSipSubsAuthBegin(CallLegGetActiveReferSubs(pCallLeg), NULL);
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
    else
    {
        if(pCallLeg->hActiveTransc == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegAuthBegin - Failed. Active transaction is NULL for call-leg 0x%p",
                   pCallLeg));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
        rv = RvSipTransactionAuthBegin(pCallLeg->hActiveTransc);
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegAuthProceed - Server authentication
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed in authentication procedure.
 *          actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD
 *          - Check the given authorization header, with the given password.
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct. (will cause AuthCompletedEv to be called, with status success)..
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop that searchs for authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure).
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the authentication
 *            procedure with next header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       The callLeg handle.
 *          hTransaction -   The transaction handle.
 *          action -         With which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USING_PASSWORD, else NULL.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthProceed(
                                   IN  RvSipCallLegHandle             hCallLeg,
                                   IN  RvSipTranscHandle              hTransaction,
                                   IN  RvSipTransactionAuthAction     action,
                                   IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                   IN  RvChar                        *password)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus rv;
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegAuthProceed - continue with action %d, for call-leg 0x%p, transc 0x%p"
              ,action, pCallLeg, hTransaction));

    if(hTransaction != NULL)
    {
        rv = SipTransactionAuthProceed(hTransaction, action, hAuthorization,
            password,(void*)pCallLeg, RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
            pCallLeg->tripleLock);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAuthProceed hCallLeg 0x%p: failed to authenticate transaction 0x%p (rv=%d:%s)",
                pCallLeg, hTransaction, rv, SipCommonStatus2Str(rv)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
    }
#ifdef RV_SIP_SUBS_ON
    else if(CallLegGetActiveReferSubs(pCallLeg) != NULL)
    {
        /* authenticate the incoming REFER request */
        rv = RvSipSubsAuthProceed(CallLegGetActiveReferSubs(pCallLeg), NULL,
                                  action, hAuthorization, password);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAuthProceed hCallLeg 0x%p: failed to authenticate subscription 0x%p (rv=%d:%s)",
                pCallLeg, CallLegGetActiveReferSubs(pCallLeg), rv, SipCommonStatus2Str(rv)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
    else
    {
        if(pCallLeg->hActiveTransc == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegAuthProceed - Failed. Active transaction is NULL for call-leg 0x%p",
                   pCallLeg));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_ILLEGAL_ACTION;
        }
        rv = SipTransactionAuthProceed(pCallLeg->hActiveTransc, action, hAuthorization,
            password,(void*)pCallLeg, RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
            pCallLeg->tripleLock);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegAuthProceed hCallLeg 0x%p: failed to authenticate transaction 0x%p (rv=%d:%s)",
                pCallLeg, pCallLeg->hActiveTransc, rv, SipCommonStatus2Str(rv)));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
#else
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(hTransaction);
    RV_UNUSED_ARG(action);
    RV_UNUSED_ARG(hAuthorization);
    RV_UNUSED_ARG(password);
    return RV_ERROR_NOTSUPPORTED;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/***************************************************************************
 * RvSipCallLegRespondUnauthenticated - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       The callLeg handle.
 *          hTransaction -   The transaction handle.
 *          responseCode -   401 or 407.
 *          strReasonPhrase - May be NULL, for default reason phrase.
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegRespondUnauthenticated(
                                   IN  RvSipCallLegHandle   hCallLeg,
                                   IN  RvSipTranscHandle    hTransaction,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar*             strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRespondUnauthenticated - call-leg 0x%p, transc 0x%p",
              pCallLeg, hTransaction));

#ifdef RV_SIP_SUBS_ON
    if(hTransaction == NULL && CallLegGetActiveReferSubs(pCallLeg) != NULL)
    {
        rv = RvSipSubsRespondUnauthenticated(
                CallLegGetActiveReferSubs(pCallLeg), NULL, responseCode,
                strReasonPhrase, headerType, hHeader);
    }
    else
#endif /* #ifdef RV_SIP_SUBS_ON */
    {
        rv = CallLegAuthRespondUnauthenticated(pCallLeg, hTransaction, responseCode, strReasonPhrase,
                                           headerType, hHeader, NULL, NULL, NULL, NULL,
                                           RV_FALSE, RVSIP_AUTH_ALGORITHM_UNDEFINED, NULL,
                                           RVSIP_AUTH_QOP_UNDEFINED, NULL);
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegRespondUnauthenticatedDigest - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Build an authentication header containing all given parameters,
 *          and add it to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       The callLeg handle.
 *          hTransaction -   The transaction handle.
 *          responseCode -   401 or 407
 *          strReasonPhrase - The reason phrase for this response code.
 *          strRealm -       mandatory.
 *          strDomain -      Optional string. may be NULL.
 *          strNonce -       Optional string. may be NULL.
 *          strOpaque -      Optional string. may be NULL.
 *          bStale -         TRUE or FALSE
 *          eAlgorithm -     Enumeration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
 *                           the algorithm value is taken from the the next argument.
 *          strAlgorithm -   String of algorithm. this parameters will be set only if
 *                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           Enumeration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
 *                           will be taken from the next argument.
 *          strQop -         String of qop. this parameter will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegRespondUnauthenticatedDigest(
                                   IN  RvSipCallLegHandle hCallLeg,
                                   IN  RvSipTranscHandle  hTransaction,
                                   IN  RvUint16         responseCode,
                                   IN  RvChar           *strReasonPhrase,
                                   IN  RvChar           *strRealm,
                                   IN  RvChar           *strDomain,
                                   IN  RvChar           *strNonce,
                                   IN  RvChar           *strOpaque,
                                   IN  RvBool           bStale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRespondUnauthenticatedDigest - call-leg 0x%p, transc 0x%p",
               pCallLeg, hTransaction));

#ifdef RV_SIP_SUBS_ON
    if(hTransaction == NULL && CallLegGetActiveReferSubs(pCallLeg) != NULL)
    {
        rv = RvSipSubsRespondUnauthenticatedDigest(
                CallLegGetActiveReferSubs(pCallLeg), NULL,responseCode,  strReasonPhrase,
                strRealm, strDomain,strNonce, strOpaque, bStale, eAlgorithm,
                strAlgorithm, eQop, strQop);
    }
    else
#endif /* #ifdef RV_SIP_SUBS_ON */
    {
        rv = CallLegAuthRespondUnauthenticated(pCallLeg, hTransaction, responseCode, strReasonPhrase,
                                               RVSIP_HEADERTYPE_UNDEFINED, NULL, strRealm, strDomain,
                                               strNonce, strOpaque, bStale, eAlgorithm,
                                               strAlgorithm, eQop, strQop);
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * RvSipCallLegTranscCreate
 * ------------------------------------------------------------------------
 * General:Creates a new general transaction that is related to the supplied
 *         call-leg.
 *         The transaction will have the call-leg characteristics such as:
 *         To Header, From header, Call-ID and local and outbound addresses.
 *         The application can define an application handle to the transaction
 *         and supply it to the stack when calling this function.
 *         The application handle will be supplied back to the application
 *         when the  RvSipCallLegTranscStateChangedEv callback will be called.
 *         Creating a transaction and sending the request must be atomic since
 *         the cseq step is applied to the transaction on creation.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - Handle to the call leg.
 *          hAppTransc - Application handle to the new transaction
 * Output:  hTransc    - The handle to the newly created transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscCreate(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipAppTranscHandle hAppTransc,
                                           OUT RvSipTranscHandle  *hTransc)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *hTransc = NULL;

    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscCreate - Failed for Call-leg 0x%p - illegal state %s to create transaction",
            pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        *hTransc = NULL;
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegTranscCreate - call-leg 0x%p creating a new call-leg transaction",pCallLeg));

    /*-----------------------
     create a new transaction
    -------------------------*/
    rv = CallLegCreateTransaction(pCallLeg, RV_TRUE, hTransc);

    if (rv != RV_OK || NULL == *hTransc)
    {
        /*transc construction failed*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscCreate - Failed for Call-leg 0x%p - failed to create transaction",pCallLeg));
        *hTransc = NULL;
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    /*set the application as the initiator of the transaction*/
    SipTransactionSetAppInitiator(*hTransc);
    RvSipTransactionSetAppHandle(*hTransc, hAppTransc);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegTranscRequest
 * ------------------------------------------------------------------------
 * General: This function gets a Transaction related to the call-leg and
 *          sends a Request message with a given method. It can be used at
 *          any call-leg state. You can use this function for sending requests,
 *          such as INFO. The request will have the To, From and Call-ID of
 *          the call-leg and will be sent with a correct CSeq step. The request
 *          will be record routed if needed.
 *          Note:
 *          1. If you supply the function a NULL transaction, the stack
 *             will create a new call-leg transaction for you but you will not be
 *             able to replace handles with this transaction.
 *          2. A transaction that was supplied by the application will
 *             not be terminated on failure. It is the application
 *             responsibility to terminate the transaction using the
 *             RvSipCallLegTranscTerminated() function.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that will send the new Request.
 *          strMethod - A String with the request method.
 * Input Output:  hTransc - The handle to the newly created transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscRequest(
                                           IN  RvSipCallLegHandle     hCallLeg,
                                           IN  RvChar                *strMethod,
                                           INOUT RvSipTranscHandle    *hTransc)
{

    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || strMethod == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*don't allow general transaction in state Idle and Terminated*/
#if defined(UPDATED_BY_SPIRENT)
   // SPIRENT change: allow general transaction in state Idle (for PING request). 
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED
#else
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE
       || pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED
#endif
/* SPIRENT_END */
       || pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTING
       || pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTED
       || pCallLeg->eState == RVSIP_CALL_LEG_STATE_UNAUTHENTICATED
       || pCallLeg->eState == RVSIP_CALL_LEG_STATE_REDIRECTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegTranscRequest - call-leg 0x%p cannot send request at state %s",
                   pCallLeg,CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*don't allow sending bye or invite using this function*/
    if(SipCommonStricmp(strMethod,"INVITE") == RV_TRUE ||
        SipCommonStricmp(strMethod,"BYE") == RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegTranscRequest - cannot send method %s with this function",strMethod));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegTranscRequest - call-leg 0x%p is sending %s request",pCallLeg,strMethod));

    /*Sending this kind of request does no effect the call-leg state*/
    rv = CallLegSessionRequest(pCallLeg,strMethod,hTransc);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegTranscResponse
 * ------------------------------------------------------------------------
 * General: This function sends a response to a call-leg related transaction.
 *          When a call-leg receives a general request such as INFO
 *          (and other then BYE or PRACK) the call-leg first notifies the
 *          application that a new call-leg transaction was created using
 *          the RvSipCallLegTranscCreatedEv() callback. At this stage the
 *          application can specify whether or not it wishes to handle the
 *          transaction and can also replace handles with the stack.
 *          The call-leg will then notify the application about the new
 *          transaction state: "General Request Rcvd" using the callback
 *          RvSipCallLegTranscStateChangedEv().
 *          At this state the application should use the
 *          RvSipCallLegTranscResponse() function to send a response to the
 *          request.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that will send the new Request.
 *          hTransc - Handle to the transaction the request belongs to.
 *          status - The response code to the request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscResponse(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc,
                                           IN  RvUint16            responseCode)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegTranscResponse - call-leg 0x%p - responding to transc 0x%p",pCallLeg,hTransc));

    SipTransactionSetAppInitiator(hTransc);
    rv = RvSipTransactionRespond(hTransc,responseCode,NULL);
    if (rv != RV_OK)
    {
        /*transaction failed to send respond - terminate call-leg*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscResponse - Call-leg 0x%p - Failed to send response (rv=%d)",
            pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegTranscTerminate
 * ------------------------------------------------------------------------
 * General: 1. Terminate a General transaction inside a call-leg.
 *          2. Terminate a BYE transaction. Used by applications that work with
 *             the BYE stateChangedEv (RvSipCallLegByeStateChangedEv).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that the transaction terminated relates to.
 *          hTransc  - Handle to the transaction that is terminated.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscTerminate(
                                           IN  RvSipCallLegHandle   hCallLeg,
                                           IN  RvSipTranscHandle    hTransc)
{
    if(hCallLeg == NULL || hTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* The locks of the call-leg and transaction are made in RvSipTransactionTerminate */
    return RvSipTransactionTerminate(hTransc);
}


/***************************************************************************
 * RvSipCallLegReplacesGetMatchedCallExt
 * ------------------------------------------------------------------------
 * General: This function is called when the call leg is in the OFFERING state.
 *          This function searches for the call leg that has the same Call-ID,
 *          to tag and from tag as the Replaces header in the original call leg.
 *          If a matched call leg is found then this call leg is returned as
 *          the function output, otherwise this pointer will be NULL.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that received the INVITE with
 *                                   the Replaces header.
 * Output:  peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog doesn't fit.
 *                     application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces.
 *          hMatchedCallLeg - Handle to the call leg matched to the Replaces header.
 *                     If there is no such call leg, this handle will be NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesGetMatchedCallExt(
                                  IN  RvSipCallLegHandle         hCallLeg,
                                  OUT RvSipCallLegReplacesReason *peReason,
                                  OUT RvSipCallLegHandle        *hMatchedCallLeg)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    CallLeg    *pMatchedCallLeg;

    if(hMatchedCallLeg == NULL || pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(peReason == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_OFFERING)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pCallLeg->hReplacesHeader == NULL)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_OK;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReplacesGetMatchedCallExt - Searching for a matched Call Leg to Replaces header 0x%p",pCallLeg->hReplacesHeader));

    rv = CallLegReplacesGetMatchedCall(pCallLeg, (RvSipCallLegHandle *)&pMatchedCallLeg, peReason);
    *hMatchedCallLeg = (RvSipCallLegHandle)pMatchedCallLeg;
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegReplacesCompareReplacesToCallLeg
 * ------------------------------------------------------------------------
 * General: This function compares a Call leg to a Replaces header.
 *          The Call leg and Replaces header are equal if the Call-ID,
 *          from-tag and to-tag are equal.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg        - Handle to the call leg to compare with the Replaces header.
 *          hReplacesHeader - Handle to a Replaces header.
 * Output:  pbIsEqual       - The result of the comparison. RV_TRUE if the Call-leg and
 *                            Replaces header are equal, RV_FALSE - otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesCompareReplacesToCallLeg(IN  RvSipCallLegHandle        hCallLeg,
                                                                        IN  RvSipReplacesHeaderHandle hReplacesHeader,
                                                                        OUT RvBool                  *pbIsEqual)
{
      RvStatus  rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(hReplacesHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReplacesCompareReplacesToCallLeg - compares Call leg to Replaces header"));

    rv = CallLegReplacesCompareReplacesToCallLeg(pCallLeg, hReplacesHeader, pbIsEqual);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}



/*-----------------------------------------------------------------------
       C A L L  - L E G:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the call-leg page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information
 *         and set it back to the call-leg using the relevant 'set' function.
 *
 *         Note: You may use this function only on Idle state. On any other state you must 
 *         construct the header on an application page, and then set it to the stack object.
 *
 *         The function supports the following elements:
 *         Party - you should set these headers back with the  RvSipCallLegSetToHeader() 
 *                 or RvSipCallLegSetFromHeader() API functions.
 *         replaces - you should set this header back with the RvSipCallLegSetReplaces()
 *                  API function.
 *         Authorization - you should set this header back with the RvSipCallLegSetInitialAuthorization()
 *                         API function that is available in the IMS add-on only.
 *         Address of any type  - you should set the address back using the
 *                                RvSipCallLegSetRemoteContactAddress() or
 *                                RvSipCallLegSetLocalContactAddress() API functions.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg object.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewMsgElementHandle (
                                      IN   RvSipCallLegHandle   hCallLeg,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phHeader = NULL;

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if((eHeaderType != RVSIP_HEADERTYPE_TO &&
        eHeaderType != RVSIP_HEADERTYPE_FROM &&
#ifdef RV_SIP_IMS_ON
        eHeaderType != RVSIP_HEADERTYPE_AUTHORIZATION &&
#endif
        eHeaderType != RVSIP_HEADERTYPE_REPLACES) &&
        (eHeaderType == RVSIP_HEADERTYPE_UNDEFINED &&
         eAddrType == RVSIP_ADDRTYPE_UNDEFINED))
    {
        /* None of the supported headers, and not an address */
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetNewMsgElementHandle - Call-leg 0x%p - unsupported action for header %s address %s",
            pCallLeg, SipHeaderName2Str(eHeaderType), SipAddressType2Str(eAddrType)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewMsgElementHandle - getting new header-%s || addr-%s handle from call-leg 0x%p",
              SipHeaderName2Str(eHeaderType),SipAddressType2Str(eAddrType),pCallLeg));

    if(eHeaderType != RVSIP_HEADERTYPE_UNDEFINED)
    {
        rv = SipHeaderConstruct(pCallLeg->pMgr->hMsgMgr, eHeaderType,pCallLeg->pMgr->hGeneralPool,
                                pCallLeg->hPage, phHeader);
    }
    else /*address*/
    {
        rv = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool,
                                pCallLeg->hPage, eAddrType, (RvSipAddressHandle*)phHeader);
    }

    if (rv != RV_OK || *phHeader == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewMsgElementHandle - Call-leg 0x%p - Failed to create new header (rv=%d)",
              pCallLeg, rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_OUTOFRESOURCES;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}




/***************************************************************************
 * RvSipCallLegSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the call-leg call-id.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The Sip Stack handle to the call-leg
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCallId (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvChar              *strCallId)
{
    RvStatus               rv;
    RvInt32                offset;
    RPOOL_ITEM_HANDLE       ptr;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || strCallId == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetCallId - setting call id for call-leg 0x%p",pCallLeg));

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetCallId - Call-Leg 0x%p is not in state Idle", pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = RPOOL_AppendFromExternal(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                                  strCallId, (RvInt)strlen(strCallId)+1, RV_FALSE,
                                  &offset, &ptr);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetCallId - Call-Leg 0x%p Failed (rv=%d)", pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg);
        return rv;

    }


    pCallLeg->strCallId = offset;
#ifdef SIP_DEBUG
    pCallLeg->pCallId = (RvChar *)RPOOL_GetPtr(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                        pCallLeg->strCallId);
#endif
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegGetCallId
 * ------------------------------------------------------------------------
 * General:Copies the call-leg Call-Id into a given buffer.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the call-leg.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The call-leg does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The Sip Stack handle to the call-leg.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCallId (
                            IN  RvSipCallLegHandle   hCallLeg,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || pstrCallId == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegGetCallId - getting call id for call-leg 0x%p",pCallLeg));

    rv = CallLegGetCallId(pCallLeg,bufSize,pstrCallId,actualSize);

    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetCallId - call-leg 0x%p failed to get call id (rv=%d)",pCallLeg,rv));
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSetCSeq
 * ------------------------------------------------------------------------
 * General: Sets the call-leg outgoing CSeq step counter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The Sip Stack handle to the call-leg
 *            cseq - The cseq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCSeq (
                                      IN  RvSipCallLegHandle  hCallLeg,
                                      IN  RvInt32            cseq)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipCommonCSeqSetStep(cseq,&pCallLeg->localCSeq); 
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetCSeq - call-leg 0x%p failed to set CSeq (rv=%d)",pCallLeg,rv));
	}
    
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/

    return rv;
}

/***************************************************************************
 * RvSipCallLegGetCSeq
 * ------------------------------------------------------------------------
 * General: Gets the call-leg outgoing CSeq step counter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The Sip Stack handle to the call-leg.
 * Output:     cseq       - The cseq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCSeq (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *cseq)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipCommonCSeqGetStep(&pCallLeg->localCSeq,cseq); 
	if (rv != RV_OK) 
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetCSeq - call-leg 0x%p failed to get CSeq (rv=%d)",pCallLeg,rv));
	}

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegGetRemoteCSeq
 * ------------------------------------------------------------------------
 * General: Gets the call-leg incoming CSeq step counter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The Sip Stack handle to the call-leg.
 * Output:     cseq       - The remote cseq step.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetRemoteCSeq (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *cseq)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipCommonCSeqGetStep(&pCallLeg->remoteCSeq,cseq); 
	if (rv != RV_OK) 
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetRemoteCSeq - call-leg 0x%p failed to get remote CSeq (rv=%d)",pCallLeg,rv));
	}

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the from header associated with the call-leg. Note
 *          that attempting to alter the from header after call has
 *          been initiated might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 *            hFrom    - Handle to an application constructed from header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetFromHeader (
                                      IN  RvSipCallLegHandle      hCallLeg,
                                      IN  RvSipPartyHeaderHandle  hFrom)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hFrom == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetFromHeader - Failed for call-leg 0x%p. illegal Action in state %s",
              pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetFromHeader - Setting call-leg 0x%p From header",pCallLeg));

    rv = CallLegSetFromHeader(pCallLeg,hFrom);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;


}


/***************************************************************************
 * RvSipCallLegGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the call.
 *         Attempting to alter the from header after
 *         call left the Idle state might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 * Output:     phFrom -   Pointer to the call-leg From header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetFromHeader (
                                      IN    RvSipCallLegHandle      hCallLeg,
                                      OUT RvSipPartyHeaderHandle    *phFrom)
{
    RvStatus               rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phFrom == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phFrom = NULL;

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetFromHeader(pCallLeg,phFrom);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;


}

/***************************************************************************
 * RvSipCallLegSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the call-leg. Note
 *          that attempting to alter the To header after call left the Idle state
 *          might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 *            hTo      - Handle to an application constructed To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetToHeader (
                                      IN  RvSipCallLegHandle       hCallLeg,
                                      IN  RvSipPartyHeaderHandle   hTo)
{
    RvStatus               rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetToHeader - Failed for call-leg 0x%p. illegal Action in state %s",
              pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetToHeader - Setting call-leg 0x%p To header",pCallLeg));

    rv = CallLegSetToHeader(pCallLeg,hTo);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To address associated with the call.
 *          Attempting to alter the To address after
 *          call has been initiated might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 * Output:     phTo     - Pointer to the call-leg To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetToHeader (
                                      IN    RvSipCallLegHandle        hCallLeg,
                                      OUT   RvSipPartyHeaderHandle    *phTo)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phTo = NULL;
    rv = CallLegGetToHeader(pCallLeg,phTo);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipCallLegGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given CallLeg object.
 *          The call-id is kept on the Call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV RvSipCallLegGenerateCallId(IN RvSipCallLegHandle hCallLeg)
{
	RvStatus               rv;
	CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
  
	if(pCallLeg == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

	rv = CallLegLockAPI(pCallLeg); 
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    rv = SipCallLegGenerateCallId(hCallLeg);

	CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/

	return rv;
}

/***************************************************************************
 * RvSipCallLegGetReqURI 
 * ------------------------------------------------------------------------
 * General: Returns the Request URI address associated with the call.
 * Return Value: RV_InvalidHandle  - The call-leg handle is invalid.
 *               RV_BadPointer     - Bad pointer was given by the application. 
 *               RV_Success        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg - Handle to the call-leg.
 * Output: 	hReqUri  - Pointer to the Request URI address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReqURI (
                                      IN    RvSipCallLegHandle        hCallLeg,
                                      OUT   RvSipAddressHandle    *hReqUri)
{
	RvStatus               rv;
	CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
  
	if(pCallLeg == NULL || hReqUri == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

	*hReqUri = NULL;

	/*try to lock the call-leg*/
	rv = CallLegLockAPI(pCallLeg); 
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	rv = RvSipTransactionGetRequestUri(pCallLeg->hActiveTransc,hReqUri);
    
	CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
	return rv;

}


#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegSetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Set the contact address of the remote party. This is the address
 *          with which the remote party may be contacted with. This method
 *          may be used for outgoing calls when the user wishes to use a
 *          Request-URI that is different from the call-leg's To header.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 *            hContactAddress - Handle to an Address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetRemoteContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvSipAddressHandle   hContactAddress)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hContactAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetRemoteContactAddress - Setting call-leg 0x%p Remote contact address",pCallLeg));

    rv = CallLegSetRemoteContactAddress(pCallLeg,hContactAddress);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;


}
/***************************************************************************
 * RvSipCallLegGetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Get the remote party's contact address. This is the
 *          address supplied by the remote party by which it can be
 *          directly contacted in future requests.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg         - Handle to the call-leg.
 * Output:     phContactAddress - Pointer to the call-leg Remote Contact Address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetRemoteContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle   *phContactAddress)

{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phContactAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetRemoteContactAddress(pCallLeg,phContactAddress);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;


}

/***************************************************************************
 * RvSipCallLegSetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Sets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. The
 *          local contact address is used by the remote party to
 *          directly contact the local party.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New local contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg
 *            hContactAddress - Handle to the local contact address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetLocalContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvSipAddressHandle   hContactAddress)

{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hContactAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetLocalContactAddress - Setting call-leg 0x%p Local contact address",pCallLeg));

    rv = CallLegSetLocalContactAddress(pCallLeg,hContactAddress);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;


}


/***************************************************************************
 * RvSipCallLegGetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Gets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. If no value is
 *          supplied the From header's address part is taken. The
 *          local contact address is used by the remote party to
 *          directly contact the local party.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - Handle to the call-leg.
 * Output:     phContactAddress - Handle to the local contact address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetLocalContactAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle   *phContactAddress)

{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phContactAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetLocalContactAddress(pCallLeg,phContactAddress);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}


/***************************************************************************
 * RvSipCallLegSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Set the local address from which the call-leg will use to send outgoing
 *          requests.
 *          The SIP stack can be configured to listen to many local addresses.
 *          each local address has a transport type (TCP/UDP/TLS) and address type
 *          (IPv4/IPv6).
 *          When the SIP Stack sends an outgoing request, the local address (from where
 *          the request is sent) is chosen  according to the charecteristics of the
 *          remote address. Both the local and the remote addresses must have the
 *          same characteristics (transport type and address type).
 *          If several configured local addresses match the remote address chracteristics
 *          the first configured address is taken.
 *          You can use this function to force the call-leg to choose a specific local
 *          address for a specific transport and address type.
 *
 *          If the given ip address is 0.0.0.0 a default host is chosen
 *          according to the given port. To choose default ip and port use
 *          port 0.
 *          If localAddress is NULL or localAddress is empty, or eTransportType is
 *          RVSIP_TRANSPORT_UNDEFINED or eAddressType is RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED
 *          then the local addresses of the CallLeg will be the first address from
 *          each type from the configuration.
 *          You can use this function when the stack is listening to more then
 *          one IP address.
 *          strings that are used for the local address must match exactly to one of the
 *          strings that was inserted in the configuration struct in the initialization of the
 *          sip stack.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg
 *            eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType - The address type(ip or ip6).
 *            localAddress - The local address to be set to this call-leg.
 *            localPort - The local port to be set to this call-leg.
 *            if_name - the local interface name
 *            vdevblock  - hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                   *if_name
			    , IN  RvUint16               vdevblock
#endif
			    )
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvSipTransportLocalAddrHandle hLocalAddr = NULL;

    if ( pCallLeg == NULL || localAddress==NULL || localPort==(RvUint16)UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    if((eTransportType != RVSIP_TRANSPORT_TCP &&
#if (RV_TLS_TYPE != RV_TLS_NONE)
        eTransportType != RVSIP_TRANSPORT_TLS &&
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
        eTransportType != RVSIP_TRANSPORT_SCTP &&
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
        eTransportType != RVSIP_TRANSPORT_UDP) ||
       (eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP &&
        eAddressType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP6))
    {
        return RV_ERROR_BADPARAM;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetLocalAddress - Call-Leg 0x%p - Setting %s %s local address %s:%d",
               pCallLeg,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType),
               (localAddress==NULL)?"0":localAddress,localPort));

    rv = SipTransportLocalAddressGetHandleByAddress(pCallLeg->pMgr->hTransportMgr,
                                                    eTransportType,
                                                    eAddressType,
                                                    localAddress,
                                                    localPort,
#if defined(UPDATED_BY_SPIRENT)
						    if_name, 
						    vdevblock,
#endif
                                                    &hLocalAddr);

    if(hLocalAddr == NULL)
    {
          CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_NOT_FOUND;
    }
    if(rv != RV_OK)
    {
          CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    SipTransportLocalAddrSetHandleInStructure(pCallLeg->pMgr->hTransportMgr,
                                              &pCallLeg->localAddr,
                                              hLocalAddr,RV_FALSE);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the call-leg will use to send outgoing
 *          requests. This is the address the user set using the
 *          RvSipCallLegSetLocalAddress function. If no address was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg
 *          eTransportType - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type (ip4 or ip6).
 * Output:    localAddress - The local address this call-leg is using(must be longer than 48).
 *          localPort - The local port this call-leg is using.
 *          if_name - local interface name
 *          vdevblock - hardware port or else
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *localAddress,
                            OUT  RvUint16                *localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    )
{
    CallLeg                     *pCallLeg  = (CallLeg *)hCallLeg;
    RvStatus                   rv         = RV_OK;

    if (NULL == pCallLeg || localAddress==NULL || localPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetLocalAddress - Call-Leg 0x%p - Getting local %s,%s address",pCallLeg,
               SipCommonConvertEnumToStrTransportType(eTransportType),
               SipCommonConvertEnumToStrTransportAddressType(eAddressType)));


    rv = SipTransportGetLocalAddressByType(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->localAddr,
                                           eTransportType,
                                           eAddressType,
                                           localAddress,
                                           localPort
#if defined(UPDATED_BY_SPIRENT)
					   ,if_name
					   ,vdevblock
#endif
					   );
    if(RV_OK != rv)
    {
       RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegGetLocalAddress - Call-Leg 0x%p - Local %s %s address not found", pCallLeg,
                   SipCommonConvertEnumToStrTransportType(eTransportType),
                   SipCommonConvertEnumToStrTransportAddressType(eAddressType)));
       CallLegUnLockAPI(pCallLeg);
       return rv;
    }

    CallLegUnLockAPI(pCallLeg);
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetTranscCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address that is used by a specific call-leg
 *          transaction. You can supply a specific transaction handle or use
 *          NULL in order to get the address of the active transaction (INVITE
 *          transaction). In this version this function can be called
 *          only in the offering state to get the local address of an incoming INVITE
 *          transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          hTransc - Handle to a specific call-leg transaction or NULL to
 *                    indicate active transaction.
 * Output:  eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType  - The address type(IP4 or IP6).
 *          localAddress - The local address this transaction is using(must be longer than 48 bytes).
 *          localPort - The local port this transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetTranscCurrentLocalAddress(
                            IN  RvSipCallLegHandle        hCallLeg,
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *eTransportType,
                            OUT RvSipTransportAddressType *eAddressType,
                            OUT RvChar                   *localAddress,
                            OUT RvUint16                 *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT RvChar                   *if_name
			    , OUT RvUint16                 *vdevblock
#endif
			    )

{

    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || eTransportType == NULL || eAddressType == NULL ||
        localAddress == NULL || localPort == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hTransc == NULL && pCallLeg->hActiveTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetTranscCurrentLocalAddress - Call-Leg 0x%p - Failed. No active transaction in this call",
               pCallLeg));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    if(hTransc == NULL)
    {
        hTransc = pCallLeg->hActiveTransc;
    }

    rv = RvSipTransactionGetCurrentLocalAddress(hTransc,eTransportType,eAddressType,localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
						,if_name
						,vdevblock
#endif
						);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetTranscCurrentLocalAddress - Call-Leg 0x%p - Failed. (rv = %d)",
               pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}




/***************************************************************************
 * RvSipCallLegSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the Call-Leg object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          Request sent by this object will use the outbound detail specifications
 *          as a remote address. The Request-URI will be ignored.
 *          If the object has a Route Set, the outbound details will be ignored.
 *
 *          NOTE: If you specify both IP address and a host name in the
 *                Configuration structure, either of them will be set BUT
 *                the IP address is preferably used.
 *                If you do not specify port or transport, both will be determined
 *                according to the DNS procedures specified in RFC 3263
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - Handle to the call-leg.
 *            pOutboundCfg   - Pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundDetails(
                            IN  RvSipCallLegHandle              hCallLeg,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg)
{
    RvStatus   rv = RV_OK;
    CallLeg    *pCallLeg;

    RV_UNUSED_ARG(sizeOfCfg);
    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    
    /* Setting the Host Name of the outbound proxy BEFORE the ip address  */
    /* in order to assure that is the IP address is set it is going to be */
    /* even if the host name is set too (due to internal preference)      */
    if (pOutboundCfg->strHostName            != NULL &&
        strcmp(pOutboundCfg->strHostName,"") != 0)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetOutboundDetails - CallLeg 0x%p - Setting an outbound host name %s:%d ",
            pCallLeg, (pOutboundCfg->strHostName==NULL)?"NULL":pOutboundCfg->strHostName,pOutboundCfg->port));

        /*we set all info on the size an it will be copied to the Transaction on success*/
        rv = SipTransportSetObjectOutboundHost(pCallLeg->pMgr->hTransportMgr,
                                               &pCallLeg->outboundAddr,
                                               pOutboundCfg->strHostName,
                                               (RvUint16)pOutboundCfg->port,
                                               pCallLeg->pMgr->hGeneralPool,
                                               pCallLeg->hPage);
        if(RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegSetOutboundDetails - CallLeg 0x%p Failed (rv=%d)", pCallLeg,rv));
            CallLegUnLockAPI(pCallLeg); /*unlock the Call-Leg */;
            return rv;
        }
    }

	/* We are setting the transport type before calling the setObjectOutboundAddr since if the port is */
	/* undefined we are using the transport type in order to get the default port (5060/5061)          */
	pCallLeg->outboundAddr.ipAddress.eTransport = pOutboundCfg->eTransport;

    /* Setting the IP Address of the outbound proxy */
    if(pOutboundCfg->strIpAddress                            != NULL &&
       strcmp(pOutboundCfg->strIpAddress,"")                 != 0    &&
       strcmp(pOutboundCfg->strIpAddress,IPV4_LOCAL_ADDRESS) != 0    &&
       memcmp(pOutboundCfg->strIpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) != 0)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetOutboundDetails - CallLeg 0x%p - Setting an outbound address %s:%d ",
            pCallLeg, (pOutboundCfg->strIpAddress==NULL)?"0":pOutboundCfg->strIpAddress,pOutboundCfg->port));

        rv = SipTransportSetObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,
                                               &pCallLeg->outboundAddr,
                                               pOutboundCfg->strIpAddress,
                                               pOutboundCfg->port);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegSetOutboundDetails - Failed to set outbound address for CallLeg 0x%p (rv=%d)",
                pCallLeg,rv));
            CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg */
            return rv;
        }
    }

     /* Setting the Transport type of the outbound proxy */
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundDetails - CallLeg 0x%p - Setting outbound transport to %s",
               pCallLeg, SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));
    

#ifdef RV_SIGCOMP_ON
    /* Setting the Compression type of the outbound proxy */
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundDetails - CallLeg 0x%p - Setting outbound compression to %s",
               pCallLeg, SipTransportGetCompTypeName(pOutboundCfg->eCompression)));
    pCallLeg->outboundAddr.eCompType = pOutboundCfg->eCompression;
	
	/* setting the sigcomp-id to the outbound proxy */
    if (pOutboundCfg->strSigcompId            != NULL &&
        strcmp(pOutboundCfg->strSigcompId,"") != 0)
	{
	    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
		          "RvSipCallLegSetOutboundDetails - CallLeg 0x%p - Setting outbound sigcomp-id to %s",
				  pCallLeg, (pOutboundCfg->strSigcompId==NULL)?"0":pOutboundCfg->strSigcompId));

		rv = SipTransportSetObjectOutboundSigcompId(pCallLeg->pMgr->hTransportMgr,
													&pCallLeg->outboundAddr,
													pOutboundCfg->strSigcompId,
													pCallLeg->pMgr->hGeneralPool,
													pCallLeg->hPage);
		if (RV_OK != rv)
		{
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegSetOutboundDetails - Failed to set outbound sigcomp-id for CallLeg 0x%p (rv=%d)",
                pCallLeg,rv));
            CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg */
            return rv;
		}
	}

#endif /* RV_SIGCOMP_ON */

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all outbound proxy details that the Call-Leg object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          If the outbound details were not set to the specific
 *          Call-Leg object but an outbound proxy was defined to the stack
 *          on initialization, the stack parameters will be returned.
 *          If the Call-Leg is not using an outbound address, NULL/UNDEFINED
 *          values are returned.
 *          NOTE: You must supply a valid consecutive buffer in the
 *                RvSipTransportOutboundProxyCfg structure to get the
 *                outbound strings (host name and ip address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - Handle to the call-leg.
 *          sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundDetails(
                            IN  RvSipCallLegHandle              hCallLeg,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg)
{
    CallLeg     *pCallLeg;
    RvStatus    rv;

    RV_UNUSED_ARG(sizeOfCfg);
    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetCallLegOutboundDetails(pCallLeg,sizeOfCfg, pOutboundCfg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "RvSipCallLegGetOutboundDetails - Failed for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg*/
        return rv;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy of the call-leg. All the requests sent
 *          by this call-leg will be sent directly to the call-leg
 *          outbound proxy address (unless the call-leg is using a
 *          Record-Route path).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg           - Handle to the call-leg
 *            strOutboundAddrIp    - The outbound proxy ip to be set to this
 *                               call-leg.
 *          outboundProxyPort  - The outbound proxy port to be set to this
 *                               call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundAddress(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvChar               *strOutboundAddrIp,
                            IN  RvUint16              outboundProxyPort)
{
    RvStatus           rv=RV_OK;
    CallLeg            *pCallLeg;

    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(strOutboundAddrIp == NULL || strcmp(strOutboundAddrIp,"") == 0 ||
       strcmp(strOutboundAddrIp,IPV4_LOCAL_ADDRESS) == 0 ||
       memcmp(strOutboundAddrIp, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) == 0)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegSetOutboundAddress - Failed, ip address  not supplied"));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

      RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundAddress - Call-Leg 0x%p - Setting an outbound address %s:%d ",
              pCallLeg, (strOutboundAddrIp==NULL)?"0":strOutboundAddrIp,outboundProxyPort));

    rv = SipTransportSetObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           strOutboundAddrIp,(RvInt32)outboundProxyPort);

    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RvSipCallLegSetOutboundAddress - Failed to set outbound address for call-leg 0x%p (rv=%d)",
                   pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}



/***************************************************************************
 * RvSipCallLegGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Gets the address of outbound proxy the call-leg is using. If an
 *          outbound address was set to the call-leg this address will be
 *          returned. If no outbound address was set to the call-leg
 *          but an outbound proxy was configured for the stack, the configured
 *          address is returned. '\0' is returned if the call-leg is not using
 *          an outbound proxy.
 *          NOTE: you must supply a valid consecutive buffer of size 48 to
 *          get the outboundProxyIp.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hCallLeg      - Handle to the call-leg
 * Output:
 *            outboundProxyIp   -  A buffer with the outbound proxy IP the call-leg.
 *                               is using(must be longer than 48).
 *          pOutboundProxyPort - The outbound proxy port the call-leg is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundAddress(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvChar              *outboundProxyIp,
                            OUT  RvUint16            *pOutboundProxyPort)
{
    CallLeg             *pCallLeg;
    RvStatus           rv;

    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(outboundProxyIp == NULL || pOutboundProxyPort == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegGetOutboundAddress - Failed, NULL pointer supplied"));
         CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

    rv = SipTransportGetObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           outboundProxyIp,
                                           pOutboundProxyPort);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetOutboundAddress - Failed to get outbound address of call-leg 0x%p,(rv=%d)",
            pCallLeg, rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "RvSipCallLegGetOutboundAddress - outbound address of call-leg 0x%p: %s:%d",
              pCallLeg, outboundProxyIp, *pOutboundProxyPort));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy of the call-leg when the proxy is a
 *          host name. All the requests sent
 *          by this call-leg will be sent to the call-leg
 *          outbound proxy address (unless the call-leg is using a
 *          Record-Route path).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg           - Handle to the call-leg
 *            strOutboundHost    - The outbound proxy host to be set to this
 *                               call-leg.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               call-leg. If you set the port to zero it
 *                               will be determined with the DNS procedure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundHostName(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvChar                *strOutboundHost,
                            IN  RvUint16              outboundPort)
{
    RvStatus               rv=RV_OK;
    CallLeg                 *pCallLeg;

    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (strOutboundHost == NULL ||
        strcmp(strOutboundHost,"") == 0)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegSetOutboundHostName - Failed, host name was not supplied"));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }

      RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundHostName - Call-Leg 0x%p - Setting an outbound host name %s:%d ",
              pCallLeg, (strOutboundHost==NULL)?"NULL":strOutboundHost,outboundPort));

    /*we set all info on the size an it will be copied to the call-leg on success*/
    rv = SipTransportSetObjectOutboundHost(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           strOutboundHost,
                                           outboundPort,
                                           pCallLeg->pMgr->hGeneralPool,
                                           pCallLeg->hPage);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegSetOutboundHostName - Call-Leg 0x%p Failed (rv=%d)", pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of outbound proxy the call-leg is using. If an
 *          outbound host was set to the call-leg this host will be
 *          returned. If no outbound host was set to the call-leg
 *          but an outbound host was configured for the stack, the configured
 *          address is returned. '\0' is returned if the call-leg is not using
 *          an outbound host.
 *          NOTE: you must supply a valid consecutive buffer to
 *          get the outboundProxy host name
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hCallLeg      - Handle to the call-leg
 * Output:
 *            srtOutboundHostName   -  A buffer with the outbound proxy host name
 *          pOutboundPort - The outbound proxy port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundHostName(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvChar              *strOutboundHostName,
                            OUT  RvUint16            *pOutboundPort)
{
    CallLeg    *pCallLeg;
    RvStatus     rv;
    pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (strOutboundHostName == NULL || pOutboundPort == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegGetOutboundHostName - Failed, NULL pointer supplied"));
        CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg*/
        return RV_ERROR_NULLPTR;
    }

    rv = SipTransportGetObjectOutboundHost(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           strOutboundHostName,
                                           pOutboundPort);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "RvSipCallLegGetOutboundHostName - Failed for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the CallLeg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the call-leg. If the outbound address
 *          of the call-leg is set then all the requests sent by this call-leg will be
 *          sent directly to the call-leg outbound proxy address (unless the call-leg is
 *          using a Record-Route path), with this transport.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg           - Handle to the call-leg
 *          eOutboundTransport - The outbound transport to be set
 *                               to this call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetOutboundTransport(
                            IN  RvSipCallLegHandle     hCallLeg,
                            IN  RvSipTransport         eOutboundTransport)
{
    RvStatus           rv = RV_OK;
    CallLeg            *pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

      RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundTransport - Call-Leg 0x%p - Setting outbound transport to %s",
              pCallLeg,SipCommonConvertEnumToStrTransportType(eOutboundTransport)));

    pCallLeg->outboundAddr.ipAddress.eTransport = eOutboundTransport;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport type of outbound proxy the call-leg is using. If no outbound
 *          transport was set UNDEFINED will be returned.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg           - Handle to the call-leg
 * Output: eOutboundTransport - The outbound proxy transport the call-leg is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOutboundTransport(
                            IN   RvSipCallLegHandle    hCallLeg,
                            OUT  RvSipTransport       *eOutboundProxyTransport)
{
    CallLeg             *pCallLeg = (CallLeg *)hCallLeg;
    RvStatus           rv;
    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(eOutboundProxyTransport == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegGetOutboundTransport - Failed, eOutboundProxyTransport is NULL"));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_NULLPTR;
    }


    rv = SipTransportGetObjectOutboundTransportType(pCallLeg->pMgr->hTransportMgr,
                                                    &pCallLeg->outboundAddr,
                                                    eOutboundProxyTransport);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegGetOutboundTransport - Failed for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
/***************************************************************************
 * RvSipCallLegSetPersistency
 * ------------------------------------------------------------------------
 * General: instructs the call-leg on whether to try and use persistent connection
 *          or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLeg - The call-leg handle
 *          bIsPersistent - Determines whether the call-leg will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetPersistency(
                           IN RvSipCallLegHandle       hCallLeg,
                           IN RvBool                  bIsPersistent)
{
    RvStatus  rv;

    CallLeg             *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the transaction*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetPersistency - Setting CallLeg 0x%p persistency to %d ",
              pCallLeg,bIsPersistent));

    if(pCallLeg->bIsPersistent == RV_TRUE && bIsPersistent == RV_FALSE &&
        pCallLeg->hTcpConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(pCallLeg->hTcpConn,
                                            (RvSipTransportConnectionOwnerHandle)hCallLeg);
           if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "RvSipCallLegSetPersistency - Failed to detach call-leg 0x%p from prev connection 0x%p",pCallLeg,pCallLeg->hTcpConn));
            CallLegUnLockAPI(pCallLeg);
            return rv;
        }
        pCallLeg->hTcpConn = NULL;
    }
    pCallLeg->bIsPersistent = bIsPersistent;
    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}
/***************************************************************************
 * RvSipCallLegGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns whether the call-leg is configured to try and use a
 *          persistent connection or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCallLeg - The call-leg handle
 * Output:  pbIsPersistent - Determines whether the call-leg uses
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetPersistency(
        IN  RvSipCallLegHandle                   hCallLeg,
        OUT RvBool                             *pbIsPersistent)
{

    CallLeg        *pCallLeg;


    pCallLeg = (CallLeg *)hCallLeg;
    *pbIsPersistent = RV_FALSE;

    if (NULL == pCallLeg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pbIsPersistent = pCallLeg->bIsPersistent;
    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetConnection
 * ------------------------------------------------------------------------
 * General: Set a connection to be used by the CallLeg. The CallLeg will
 *          supply this connection to all the transactions its creates.
 *          If the connection will not fit the transaction local and remote
 *          addresses it will be
 *          replaced. And the call-leg will assume the new connection.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransaction - Handle to the transaction
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetConnection(
                   IN  RvSipCallLegHandle                hCallLeg,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    RvStatus               rv;
    CallLeg        *pCallLeg= (CallLeg *)hCallLeg;
    RvSipTransportConnectionEvHandlers evHandler;
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler = CallLegConnStateChangedEv;
/*    evHandler.pfnConnStausEvHandler = CallLegConnStatusEv;*/

    if (NULL == pCallLeg)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetConnection - CallLeg 0x%p - Setting connection 0x%p", pCallLeg,hConn));

    if(pCallLeg->bIsPersistent == RV_FALSE)
    {
         RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "RvSipCallLegSetConnection - CallLeg 0x%p - Cannot set connection when the call-leg is not persistent", pCallLeg));
            CallLegUnLockAPI(pCallLeg);
         return RV_ERROR_ILLEGAL_ACTION;
    }

    if(hConn != NULL)
    {
        /*first try to attach the CallLeg to the connection*/
        rv = SipTransportConnectionAttachOwner(hConn,
                                 (RvSipTransportConnectionOwnerHandle)hCallLeg,
                                 &evHandler,sizeof(evHandler));

        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegSetConnection - Failed to attach CallLeg 0x%p to connection 0x%p",pCallLeg,hConn));
            CallLegUnLockAPI(pCallLeg);
            return rv;
        }
    }
    /*detach from the call-leg current connection if there is one and set the new connection
      handle*/
    CallLegDetachFromConnection(pCallLeg);
    pCallLeg->hTcpConn = hConn;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}
/***************************************************************************
 * RvSipCallLegGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeghTransc - Handle to the call-leg.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetConnection(
                            IN  RvSipCallLegHandle             hCallLeg,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if (NULL == pCallLeg || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phConn = NULL;
    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetConnection - pCallLeg 0x%p - Getting connection 0x%p",pCallLeg,pCallLeg->hTcpConn));

    *phConn = pCallLeg->hTcpConn;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegGetActiveTransmitter
 * ------------------------------------------------------------------------
 * General: Returns the transmitter that is currently being used by the
 *          call-leg. This function may be used only inside the final-dest-resolved
 *          callback.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeghTransc - Handle to the call-leg.
 * Output:    phTrx - Handle to the currently used transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetActiveTransmitter(
                            IN  RvSipCallLegHandle      hCallLeg,
                           OUT  RvSipTransmitterHandle *phTrx)
{
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if (NULL == pCallLeg || phTrx==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

     *phTrx = NULL;
    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetActiveTransmitter - pCallLeg 0x%p - Getting transmitter 0x%p",pCallLeg,pCallLeg->hActiveTrx));

    *phTrx = pCallLeg->hActiveTrx;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;

}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvSipCallLegGetActiveTransaction(
                            IN  RvSipCallLegHandle      hCallLeg,
                            OUT  RvSipTranscHandle *phTransc) {

  CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;
  
  if (NULL == pCallLeg || phTransc==NULL)
  {
    return RV_ERROR_NULLPTR;
  }
  
  *phTransc = NULL;
  if (RV_OK != CallLegLockAPI(pCallLeg))
  {
    return RV_ERROR_INVALID_HANDLE;
  }
  
  
  RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
    "%s - pCallLeg 0x%p - Getting transaction 0x%p",__FUNCTION__,pCallLeg,pCallLeg->hActiveTransc));
  
  *phTransc = pCallLeg->hActiveTransc;
  
  CallLegUnLockAPI(pCallLeg);
  return RV_OK;
}

RVAPI RvStatus RVCALLCONV RvSipCallLegGetMsgRcvdTransaction(
                            IN  RvSipCallLegHandle      hCallLeg,
                            OUT  RvSipTranscHandle *phTransc) {

  CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;
  
  if (NULL == pCallLeg || phTransc==NULL)
  {
    return RV_ERROR_NULLPTR;
  }
  
  *phTransc = NULL;
  if (RV_OK != CallLegLockAPI(pCallLeg))
  {
    return RV_ERROR_INVALID_HANDLE;
  }
  
  
  RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
    "%s - pCallLeg 0x%p - Getting transaction 0x%p",__FUNCTION__,pCallLeg,pCallLeg->hMsgRcvdTransc));
  
  *phTransc = pCallLeg->hMsgRcvdTransc;
  
  CallLegUnLockAPI(pCallLeg);
  return RV_OK;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipCallLegGetReplacesStatus
 * ------------------------------------------------------------------------
 * General: Return the replaces option tag status of a received Invite/Refer request.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The call-leg handle.
 * Output:  replacesStatus - The replaces status received in the INVITE/REFER request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReplacesStatus(
                                    IN  RvSipCallLegHandle             hCallLeg,
                                    OUT RvSipCallLegReplacesStatus    *replacesStatus)
{
    RvStatus  rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || replacesStatus == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = CallLegLockAPI(pCallLeg); /* try to lock the CallLeg */
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *replacesStatus = pCallLeg->statusReplaces;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegGetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Get the Replaces header from the Call-Leg. This function should be called
 *          before sending an INVITE - to get the replaces header set by the REFER message,
 *          if it exists, or when receiving an INVITE - from the evStateChanged, in order
 *          to know if the INVITE contained Replaces header.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 * Output:    hReplacesHeader - pointer to an handle to a Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReplacesHeader(IN  RvSipCallLegHandle        hCallLeg,
                                                         OUT RvSipReplacesHeaderHandle *phReplacesHeader)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phReplacesHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *phReplacesHeader = pCallLeg->hReplacesHeader;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegSetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Set the Replaces header in the Call-Leg. This function should be called
 *          before sending the INVITE request, when the call is in the idle state
 *          The application should call this function when it wants to add a Replaces
 *          header to the INVITE. If the application wants not to add a Replaces header
 *          that was received in the REFER message, and should be added to the INVITE
 *          triggered by the REFER, it should call this function with NULL as the
 *          replaces header
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *                 RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New remote contact was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 *            hReplacesHeader - Handle to a Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetReplacesHeader(IN  RvSipCallLegHandle        hCallLeg,
                                                         IN  RvSipReplacesHeaderHandle hReplacesHeader)
{
    RvStatus               rv = RV_OK;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_UNAUTHENTICATED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_REDIRECTED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetReplacesHeader - The call leg 0x%p is not in idle state",pCallLeg));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pCallLeg->pMgr->statusReplaces == RVSIP_CALL_LEG_REPLACES_UNDEFINED)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetReplacesHeader - replaces is not supported by the Stack"));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(hReplacesHeader == NULL)
    {
        pCallLeg->hReplacesHeader = NULL;
    }
    else
    {
        if(SipReplacesHeaderGetCallID(hReplacesHeader) == UNDEFINED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegSetReplacesHeader - Replaces header doesn't contain Call-ID"));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_BADPARAM;
        }
        if(SipReplacesHeaderGetToTag(hReplacesHeader) == UNDEFINED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegSetReplacesHeader - Replaces header doesn't contain to-tag"));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_BADPARAM;
        }
        if(SipReplacesHeaderGetFromTag(hReplacesHeader) == UNDEFINED)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegSetReplacesHeader - Replaces header doesn't contain from-tag"));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_BADPARAM;
        }
        if(pCallLeg->hReplacesHeader == NULL)
        {
            rv = RvSipReplacesHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                              pCallLeg->pMgr->hGeneralPool,
                                              pCallLeg->hPage,
                                              &(pCallLeg->hReplacesHeader));

            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "RvSipCallLegSetReplacesHeader - RvSipReplacesHeaderConstruct failed"));

                CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
                return rv;
            }
        }
        rv = RvSipReplacesHeaderCopy(pCallLeg->hReplacesHeader, hReplacesHeader);

    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}


/***************************************************************************
 * RvSipCallLegSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the call-leg application handle. Usually the application
 *          replaces handles with the stack in the RvSipCallLegCreatedEv()
 *          callback or the RvSipCallLegMgrCreateCallLeg() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - Handle to the call-leg.
 *            hAppCallLeg - A new application handle to the call-leg
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetAppHandle (
                                      IN  RvSipCallLegHandle       hCallLeg,
                                      IN  RvSipAppCallLegHandle    hAppCallLeg)
{
    RvStatus               rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetAppHandle - Setting call-leg 0x%p Application handle 0x%p",
              pCallLeg,hAppCallLeg));

    pCallLeg->hAppCallLeg = hAppCallLeg;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}

/***************************************************************************
 * RvSipCallLegGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 * Output:     phAppCallLeg     - The application handle of the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetAppHandle (
                                      IN    RvSipCallLegHandle        hCallLeg,
                                      OUT   RvSipAppCallLegHandle     *phAppCallLeg)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phAppCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phAppCallLeg = pCallLeg->hAppCallLeg;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegGetDirection
 * ------------------------------------------------------------------------
 * General: Queries the call-leg direction. A call-leg can be either an
 *          Incoming call or an Outgoing call. When you create a call,
 *          it is always an Outgoing call. If the call is created because
 *          an Invite has arrived, the call is an Incoming call.
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -     The call-leg handle.
 * Output:     *peDirection - The call-leg direction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetDirection (
                                 IN  RvSipCallLegHandle     hCallLeg,
                                 OUT RvSipCallLegDirection  *peDirection)
{
    RvStatus               rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || peDirection == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetDirection(pCallLeg,peDirection);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the call-leg current state
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     peState -  The call-leg current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrentState (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipCallLegState    *peState)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || peState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetCurrentState(pCallLeg,peState);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
/***************************************************************************
 * RvSipCallLegGetTranscByMsg
 * ------------------------------------------------------------------------
 * General: Gets the transaction of a given message
 *          Use this function to find the transaction handle of a specific
 *          message.
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 *          hMsg     - The message handle
 *          bIsMsgRcvd - RV_TRUE if this is a received message. RV_FALSE otherwise.
 * Output:     phTransc -  The handle of the transaction this message belongs to.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetTranscByMsg (
                                 IN  RvSipCallLegHandle     hCallLeg,
                                 IN  RvSipMsgHandle         hMsg,
                                 IN  RvBool                bIsMsgRcvd,
                                 OUT RvSipTranscHandle      *phTransc)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegGetTranscByMsg(pCallLeg,hMsg,bIsMsgRcvd,phTransc);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the call-leg. You can
 *          call this function from the state changed call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     phMsg    -  pointer to the received message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReceivedMsg(
                                            IN  RvSipCallLegHandle  hCallLeg,
                                             OUT RvSipMsgHandle       *phMsg)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phMsg = NULL;

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *phMsg = pCallLeg->hReceivedMsg;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the call-leg.
 *          You can call this function before you call API functions such as
 *          Connect(), Accept() Reject() etc, or from the state changed
 *          callback function when the new state indicates that a message
 *          is going to be sent.
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers to the message before
 *          it is sent. To view the complete message use event message to
 *          send.
 *          Note: The section of using RvSipCallLegGetOutboundMsg and sending
 *          the message is critical. For example, if you call
 *          RvSipCallLegGetOutboundMsg and Connect, your object must be locked.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:     phMsg   -  pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     OUT RvSipMsgHandle            *phMsg)
{
    CallLeg               *pCallLeg;
    RvSipMsgHandle         hOutboundMsg = NULL;
    RvStatus              rv;

    pCallLeg = (CallLeg *)hCallLeg;
    if(pCallLeg == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phMsg = NULL;

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(RV_TRUE  == CallLegGetIsHidden(pCallLeg) && 
	   RV_FALSE == CallLegGetIsRefer(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "RvSipCallLegGetOutboundMsg - call-leg= 0x%p illegal action for hidden call-leg",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
#ifdef RV_SIP_SUBS_ON
    else if(RV_TRUE == CallLegGetIsHidden(pCallLeg) &&
            RV_TRUE == CallLegGetIsRefer(pCallLeg)  &&
            NULL    != CallLegGetActiveReferSubs(pCallLeg))
    {
        /* need to get the outbound msg of the refer subscription */
        rv = RvSipSubsGetOutboundMsg(CallLegGetActiveReferSubs(pCallLeg), &hOutboundMsg);

        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetOutboundMsg - call-leg= 0x%p retrieving refer subs 0x%p outbound msg 0x%p",
              pCallLeg,CallLegGetActiveReferSubs(pCallLeg),hOutboundMsg));

        *phMsg = hOutboundMsg;

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */

    if (NULL != pCallLeg->hOutboundMsg)
    {
        hOutboundMsg = pCallLeg->hOutboundMsg;
    }
    else
    {
        rv = RvSipMsgConstruct(pCallLeg->pMgr->hMsgMgr,
                               pCallLeg->pMgr->hMsgMemPool,
                               &hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "RvSipCallLegGetOutboundMsg - call-leg= 0x%p error in constructing message",
                      pCallLeg));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return rv;
        }
        pCallLeg->hOutboundMsg = hOutboundMsg;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetOutboundMsg - call-leg= 0x%p returned handle= 0x%p",
              pCallLeg,hOutboundMsg));

    *phMsg = hOutboundMsg;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the call-leg to NULL. If the
 *          call-leg is about to send a message it will create its own
 *          message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegResetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg)
{
    CallLeg               *pCallLeg;
    RvStatus              rv;

    pCallLeg = (CallLeg *)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (NULL != pCallLeg->hOutboundMsg)
    {
        RvSipMsgDestruct(pCallLeg->hOutboundMsg);
    }
    pCallLeg->hOutboundMsg = NULL;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegResetOutboundMsg - call-leg= 0x%p resetting outbound message",
              pCallLeg));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT 
/***************************************************************************
 * RvSipCallLegMgrMatchTranscToCallLeg
 * ------------------------------------------------------------------------
 * General: The function checks if the given transaction matches one of the
 *          call-legs in the call-legs hash.
 *          If it matches, the function returns the handle of the call-leg.
 *          If not, the function returns NULL.
 *          The function handles only incoming transactions.
 *          This is an internal function for the RV SIP Server. no need to document.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc    - Handle to the transaction we want to check.          
 ***************************************************************************/
RVAPI RvSipCallLegHandle RVCALLCONV RvSipCallLegMgrMatchTranscToCallLeg(
                                   IN   RvSipCallLegMgrHandle hCallLegMgr,
                                   IN   RvSipTranscHandle      hTransc)
{
    CallLegMgr  *pCallMgr;
    SipTransactionKey hashKey;
    RvSipCallLegHandle hCallLeg = NULL;
    RvStatus rv;

    if(hCallLegMgr == NULL || hTransc == NULL)
    {
        return NULL;
    }
    
    pCallMgr = (CallLegMgr *)hCallLegMgr;
    
    RvMutexLock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*lock the manager*/
    
    /* fill the key parameters from transaction */
    rv = SipTransactionGetKeyFromTransc(hTransc, &hashKey);
    if(rv != RV_OK)
    {
        RvLogError(pCallMgr->pLogSrc,(pCallMgr->pLogSrc,
            "RvSipCallLegMgrMatchTranscToCallLeg - failed to get key from transc 0x%p",
             hTransc));
        RvMutexUnlock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*unlock the manager*/
        return NULL;
    }

    /* we search for a call-leg that suits an incoming transaction only! */ 
    CallLegMgrHashFind(pCallMgr, &hashKey, RVSIP_CALL_LEG_DIRECTION_INCOMING,
                       RV_TRUE /*bOnlyEstablishedCall*/,
                       (RvSipCallLegHandle**)&hCallLeg);
        
    RvMutexUnlock(&pCallMgr->hMutex,pCallMgr->pLogMgr); /*unlock the manager*/
    return hCallLeg;
}

#endif /*#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* RvSipCallLegGetSubscription
* ------------------------------------------------------------------------
* General: Gets a subscription handle from call-leg subscriptions list.
*          User may use the location and hItem parameters, to go over the list.
* Return Value: RV_ERROR_INVALID_HANDLE - if the given call leg handle is invalid
*               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
*               RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - The call-leg handle.
*          location - The location in list - next, previous, first or last.
*          hRelative - Handle to the current position in the list (a relative
*                      subscription from the list). Supply this value if you choose
*                      next or previous in the location parameter.
* Output:     phSubs   -  The handle of the returned subscription.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSubscription (
                                            IN  RvSipCallLegHandle     hCallLeg,
                                            IN  RvSipListLocation      location,
                                            IN  RvSipSubsHandle        hRelative,
                                            OUT RvSipSubsHandle        *phSubs)
{
    RvStatus    rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phSubs = NULL;

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetSubscription - Gets subscriptions of call-leg 0x%p",
              hCallLeg));

    if(pCallLeg->hSubsList == NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetSubscription - call-leg 0x%p subscriptions list is empty",
            hCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_OK;
    }

    rv = CallLegSubsGetSubscription(pCallLeg, location, hRelative, phSubs);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetSubscription - call-leg 0x%p - Failed in CallLegSubsGetSubscription",
            hCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_OK;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_TRUE)
    {
        RvSipSubsEventPackageType ePackType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_UNDEFINED;
        /* in this case the function should not retrieve refer subscription,
        because application does not know that refer is handled in subscription layer */
        while(*phSubs != NULL)
        {
            RvSipSubsGetEventPackageType(*phSubs, &ePackType);
            if(ePackType == RVSIP_SUBS_EVENT_PACKAGE_TYPE_REFER)
            {
                RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RvSipCallLegGetSubscription: call-leg 0x%p - found refer subs 0x%p. skip it",
                    pCallLeg, phSubs));
                rv = CallLegSubsGetSubscription (pCallLeg, RVSIP_NEXT_ELEMENT, *phSubs, phSubs);
                if(RV_OK != rv)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "RvSipCallLegGetSubscription - call-leg 0x%p - Failed in CallLegSubsGetSubscription(2)",
                        hCallLeg));
                    CallLegUnLockAPI(pCallLeg);
                    return RV_OK;
                }
            }
            else
            {
                break;
            }
        }
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegGetSubscription - call-leg 0x%p - returned subscription 0x%p",
        hCallLeg, *phSubs));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipCallLegGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this call-leg
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - Handle to the call-leg object.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetStackInstance(
                                   IN   RvSipCallLegHandle   hCallLeg,
                                   OUT  void*       *phStackInstance)
{
    CallLeg               *pCallLeg;
    RvStatus              rv;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    pCallLeg = (CallLeg *)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phStackInstance = pCallLeg->pMgr->hStack;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetForceOutboundAddrFlag 
 * ------------------------------------------------------------------------
 * General: Sets the force outbound addr flag. This flag forces the call-leg
 *          to send every request to the outbound address regardless of the
 *          message content or object state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg           - The call-leg handle.
 *     	    bForceOutboundAddr - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetForceOutboundAddrFlag(
                                IN  RvSipCallLegHandle  hCallLeg,
			                    IN  RvBool              bForceOutboundAddr)
{
	CallLeg               *pCallLeg;
	RV_Status              rv;

	pCallLeg = (CallLeg *)hCallLeg;
	if(pCallLeg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/*try to lock the call-leg*/
	rv = CallLegLockAPI(pCallLeg);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pCallLeg->bForceOutboundAddr = bForceOutboundAddr;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
		      "RvSipCallLegSetForceOutboundAddrFlag - call-leg= %x bForceOutboundAddr was set to %d",
			  pCallLeg,bForceOutboundAddr));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetAddAuthInfoToMsgFlag
 * ------------------------------------------------------------------------
 * General: Sets the AddAuthInfoToMsg Flag. If this flag is set to RV_FALSE,
 *          outgoing messages will not contain authorization information.
 *          The default value of this flag is RV_TRUE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hCallLeg           - The call-leg handle.
 *     	    bAddAuthInfoToMsg - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetAddAuthInfoToMsgFlag(
                                IN  RvSipCallLegHandle  hCallLeg,
			                    IN  RvBool              bAddAuthInfoToMsg)
{
	CallLeg               *pCallLeg;
	RV_Status              rv;

	pCallLeg = (CallLeg *)hCallLeg;
	if(pCallLeg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/*try to lock the call-leg*/
	rv = CallLegLockAPI(pCallLeg);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pCallLeg->bAddAuthInfoToMsg = bAddAuthInfoToMsg;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
		      "RvSipCallLegSetAddAuthInfoToMsgFlag - call-leg= %x bAddAuthInfoToMsg was set to %d",
			  pCallLeg,bAddAuthInfoToMsg));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegGetReceivedFromAddress
 * ------------------------------------------------------------------------
 * General: Gets the address from which the last message was received from.
 * Return Value: RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg  - The call-leg handle.
 * Output: pAddr     - Basic details about the received from address
 ***************************************************************************/
RVAPI RV_Status RVCALLCONV RvSipCallLegGetReceivedFromAddress(
										IN  RvSipCallLegHandle   hCallLeg,
										OUT RvSipTransportAddr  *pAddr)
{
    RvStatus        rv       = RV_OK;
    CallLeg*        pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || pAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    SipTransportConvertCoreAddressToApiTransportAddr(&pCallLeg->receivedFrom.addr,pAddr);
    pAddr->eTransportType = pCallLeg->receivedFrom.eTransport;

	/*unlock the call-leg*/
    CallLegUnLockAPI(pCallLeg);
    return rv;
}

#ifdef RV_SIP_HIGHAVAL_ON
/*-----------------------------------------------------------------------
       C A L L  - L E G   HIGH AVAILABILITY   F U N C T I O N S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegGetConnectedCallStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of a connected call.
 *          (The size of buffer, that should be supplied in RvSipCallLegStoreConnectedCall()).
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg .
 * Output:  len - the size of buffer. will be -1 in case of failure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetConnectedCallStorageSize(
                                       IN  RvSipCallLegHandle  hCallLeg,
                                       OUT RvInt32            *pLen)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || pLen == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetConnectedCallStorageSize - Gets storage size for call-leg 0x%p",
              hCallLeg));

    rv = CallLegGetCallStorageSize(hCallLeg, RV_TRUE, pLen);


    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
/***************************************************************************
 * RvSipCallLegStoreConnectedCall
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call leg.
 *          In order to store call-leg information the call leg must be in the
 *          connected state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg .
 *            memBuff  - The buffer that will be filled with the callLeg information.
 *          buffLen  - The length of the given buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegStoreConnectedCall(
                                       IN RvSipCallLegHandle hCallLeg,
                                       IN void*              memBuff,
                                       IN RvUint32           buffLen)
{
    RvStatus    rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvInt32     callLen;

    if(pCallLeg == NULL || memBuff == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegStoreConnectedCall - Stores call-leg 0x%p information",
              hCallLeg));

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegStoreConnectedCall - Cannot store call-leg 0x%p which isn't Connected",
              hCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegStoreCall(hCallLeg,RV_TRUE, buffLen, memBuff, &callLen);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegStoreConnectedCallExt
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call leg.
 *          In order to store call-leg information the call leg must be in the
 *          connected state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg .
 *            memBuff  - The buffer that will be filled with the callLeg information.
 *          buffLen  - The length of the given buffer.
 * Output   usedBuffLen - The number of bytes that were actually used in the buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegStoreConnectedCallExt(
                                       IN RvSipCallLegHandle hCallLeg,
                                       IN void*              memBuff,
                                       IN RvUint32           buffLen,
                                       OUT RvUint32          *pUsedBuffLen)
{
    RvStatus    rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvInt32     callLen;

    if(pCallLeg == NULL || memBuff == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *pUsedBuffLen = 0;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegStoreConnectedCallExt - Stores call-leg 0x%p information",
              hCallLeg));

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegStoreConnectedCallExt - Cannot store call-leg 0x%p which isn't Connected",
              hCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegStoreCall(hCallLeg,RV_TRUE, buffLen, memBuff, &callLen);
    *pUsedBuffLen = callLen;
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegRestoreConnectedCall
 * ------------------------------------------------------------------------
 * General: Restore all call-leg information into a given call-leg. The call-leg
 *          will assume the connected state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - Handle to the call-leg.
 *          memBuff    - The buffer that stores all the call-leg information
 *          buffLen    - The buffer size
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRestoreConnectedCall(
                                       IN RvSipCallLegHandle   hCallLeg,
                                       IN void*                memBuff,
                                       IN RvUint32             buffLen)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || memBuff == NULL || buffLen == 0)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRestoreConnectedCall - Restores information for call-leg 0x%p",
              hCallLeg));

    rv = CallLegRestoreCall(memBuff, buffLen, RV_TRUE, RVSIP_CALL_LEG_H_A_RESTORE_MODE_UNDEFINED, pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRestoreConnectedCall - Failed, CallLegMgr failed to restore call-leg 0x%p",
              hCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the manager*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the manager*/
    return RV_OK;

}

#ifdef RV_SIP_HIGH_AVAL_3_0
/***************************************************************************
 * RvSipCallLegRestoreOldVersionConnectedCall
 * ------------------------------------------------------------------------
 * General: The function is use to restore call-legs that were stored by
 *          stack version 3.0
 *          Restore all call-leg information into a given call-leg. The call-leg
 *          will assume the connected state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - Handle to the call-leg.
 *          memBuff    - The buffer that stores all the call-leg information
 *          buffLen    - The buffer size.
 *          eHAmode    - Define the exact mode, used to store the given buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRestoreOldVersionConnectedCall(
                                       IN RvSipCallLegHandle        hCallLeg,
                                       IN void*                     memBuff,
                                       IN RvUint32                  buffLen,
                                       IN RvSipCallLegHARestoreMode eHAmode)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || memBuff == NULL || buffLen == 0)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRestoreOldVersionConnectedCall - Restores information for call-leg 0x%p",
              hCallLeg));

    rv = CallLegRestoreCall(memBuff, buffLen, RV_TRUE, eHAmode, pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRestoreOldVersionConnectedCall - Failed, CallLegMgr failed to restore call-leg 0x%p",
              hCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the manager*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the manager*/
    return RV_OK;

}
#endif /*#ifdef RV_SIP_HIGH_AVAL_3_0*/

#endif /* #ifdef RV_SIP_HIGHAVAL_ON */

/*-----------------------------------------------------------------------
       C A L L  - L E G   SESSION TIMER   F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegSessionTimerInviteRefresh
 * ------------------------------------------------------------------------
 * General: Sends re-INVITE in order to refresh the session time.
 *          The refreshing re-invite request may be called in 2 conditions:
 *          1. The call-leg had already sent/received the 2xx response
 *             for the initial INVITE request
 *             (the call-leg state is CONNECTED or ACCEPTED or REMOTE-ACCEPTED),
 *          2. There are no other pending re-Invite transactions. (no other
 *             re-invite object which waits for a final response).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call leg the user wishes to refresh the call.
 *          hInvite  - Handle to the Re-Invite object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerInviteRefresh(
                               IN  RvSipCallLegHandle   hCallLeg,
                               IN  RvSipCallLegInviteHandle hInvite)
{
    RvStatus  rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hInvite == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "RvSipCallLegSessionTimerInviteRefresh - Refreshing call-leg 0x%p invite 0x%p", pCallLeg, hInvite));

    /*modify can be called only when the call is connected and no modify or
      modifying is taking place*/
    if((pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_ACCEPTED &&
        pCallLeg->eState != RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED) ||
		pCallLeg->hActiveTransc != NULL ||
        NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "RvSipCallLegSessionTimerInviteRefresh - Failed to refresh call-leg 0x%p: Illegal Action in state %s. active transc 0x%p",
               pCallLeg, CallLegGetStateName(pCallLeg->eState), pCallLeg->hActiveTransc));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    if(pCallLeg->pSessionTimer==NULL || pCallLeg->pNegotiationSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerInviteRefresh - Failed to refresh call-leg 0x%p: stack doesn't support session timer",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    rv = CallLegSessionTimerRefresh(pCallLeg, (CallLegInvite*)hInvite);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}
/***************************************************************************
* RvSipCallLegTranscSessionTimerGeneralRefresh
* ------------------------------------------------------------------------
* General: Creates a transaction related to the call-leg and sends a
*          Request message with the given method in order to refresh the call.
*          The only general transaction which allowed is "UPDATE".
*          The request will have the To, From and Call-ID of the call-leg and
*          will be sent with a current CSeq step. It will be record routed if needed.
* Return Value: RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
*               RV_ERROR_UNKNOWN - Failed to send.
*               RV_OK - Message was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg - Pointer to the call leg the user wishes to modify.
*          strMethod - A String with the request method.
*          sessionExpires - session time that will attach to this call.
*          minSE - minimum session expires time of this call
*          eRefresher - the refresher preference for this call
* Output:  hTransc - The handle to the newly created transaction
*                    if a transaction was supplied, this transaction will be used,
*                    and a new transaction will not be created
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerGeneralRefresh (
      IN  RvSipCallLegHandle                             hCallLeg,
      IN  RvChar                                        *strMethod,
      IN  RvInt32                                        sessionExpires,
      IN  RvInt32                                        minSE,
      IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher,
      INOUT RvSipTranscHandle                           *hTransc)
{
    RvStatus     rv=RV_OK;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || strMethod == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    if (minSE > sessionExpires)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerGeneralRefresh - call-leg 0x%p: the minSE value is bigger than the session expires value" ,
            pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pSessionTimer == NULL ||
       pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerGeneralRefresh - Failed to refresh call-leg 0x%p: stack doesn't support session timer",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    /*don't allow general transaction in state Idle and Terminated*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_UNAUTHENTICATED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_REDIRECTED ||
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /*don't allow general transaction inside a refer.*/
    if(NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*don't allow sending other transaction rather then UPDATE to refresh the call*/
    if(SipCommonStricmp(strMethod,CALL_LEG_UPDATE_METHOD_STR) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerGeneralRefresh - cannot send method %s with this function",strMethod));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegTranscSessionTimerGeneralRefresh - call-leg 0x%p is sending %s request",pCallLeg,strMethod));
    rv = CallLegSessionTimerGeneralRefresh (
            pCallLeg,strMethod,sessionExpires, minSE,eRefresher, hTransc);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
/***************************************************************************
 * RvSipCallLegSessionTimerSetPreferenceParams
 * ------------------------------------------------------------------------
 * General:Sets the preference Session Timer parameters associated with this call.
 *         These parameters may not be equal to the Session timer parameters
 *         of the call in the end of the negotiation.(after OK has been sent or
 *         received).
 *
 * NOTE: 1. If the sessionExpires is set to 0, the whole sessionTimer mechanism
 *			is turned off immediately. Moreover, the mechanism can be turned
 *			on by calling this function with non-zero sessionExpires value
 *			in the middle of a call.
 *		 2. If the input minSE parameter equals UNDEFINED, then a default value
 *       according to the standard (90) is set, and the Min-SE header won't
 *       be added to the CallLeg requests from now on except for a refresh,
 *       which is following 422 response.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - handle to the call-leg.
 *          sessionExpires - session time that will attach to this call.
 *          minSE          - minimum session expires time of this call
 *          eRefresher     - the refresher preference for this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerSetPreferenceParams (
         IN  RvSipCallLegHandle                            hCallLeg,
         IN  RvInt32                                       sessionExpires,
         IN  RvInt32                                       minSE,
         IN  RvSipCallLegSessionTimerRefresherPreference   eRefresher)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerSetPreferenceParams - call-leg 0x%p: set SE=%d, minSE=%d, refresher=%d" ,
            pCallLeg,sessionExpires,minSE,eRefresher));

    if (pCallLeg->pSessionTimer            == NULL ||
        pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerSetPreferenceParams - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }

    pCallLeg->pSessionTimer->bAddReqMinSE = (minSE == UNDEFINED) ? RV_FALSE : RV_TRUE;
    if (minSE == UNDEFINED || minSE < 0)
    {
        minSE = RVSIP_CALL_LEG_MIN_MINSE;
    }

    if (sessionExpires > 0 && minSE > sessionExpires)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerSetPreferenceParams - call-leg 0x%p: the minSE (%d) value is bigger than the session expires (%d)" ,
            pCallLeg, minSE, sessionExpires));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }

    pCallLeg->pNegotiationSessionTimer->sessionExpires       = sessionExpires;
    pCallLeg->pNegotiationSessionTimer->minSE                = minSE;
    pCallLeg->pNegotiationSessionTimer->eRefresherPreference = eRefresher;
    /* Update the CallLeg's Min-SE since it cannot be affected during negotiation */
    pCallLeg->pSessionTimer->minSE                           = minSE;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
* RvSipCallLegTranscSessionTimerSetPreferenceParams
* ------------------------------------------------------------------------
* General:Sets the preference Session Timer parameters associated with this
*         transaction.The only general transaction which allowed is "UPDATE".
*         These parameters may not be equal to the Session timer
*         parameters of the call in the end of the negotiation.
*         (after OK has been sent or received).
*
* NOTE: If the input minSE parameter equals UNDEFINED, then a default value
*       according to the standard (90) is set, and the Min-SE header won't
*       be added to the following CallLeg Transc request, except for a refresh,
*       which is following 422 response.
*
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hCallLeg       - handle to the call-leg.
*          hTransc        - Handle to the transaction the request belongs to.
*          sessionExpires - session time that will attach to this call.
*          minSE          - minimum session expires time of this call
*          eRefresher     - the refresher preference for this call
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerSetPreferenceParams (
                         IN  RvSipCallLegHandle                             hCallLeg,
                         IN  RvSipTranscHandle                              hTransc,
                         IN  RvInt32                                        sessionExpires,
                         IN  RvInt32                                        minSE,
                         IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher)
{
    RvStatus                         rv   = RV_OK;
    CallLegNegotiationSessionTimer  *info = NULL;
    CallLeg                         *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL  || hTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pCallLeg->pSessionTimer == NULL || pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerSetPreferenceParams - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    if (CallLegIsUpdateTransc(hTransc) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerSetPreferenceParams - call-leg 0x%p: transc 0x%p: the method have to be UPDATE ",
            pCallLeg,hTransc));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }

    if (minSE == UNDEFINED || minSE < 0)
    {
        minSE = RVSIP_CALL_LEG_MIN_MINSE;
    }
    else
    {
        /* Only if the different MinSE is set then its headers will appear */
        /* in the outgoing transc. The 'bAddOnceMinSE' MUST NOT be set     */
        /* to RV_FALSE since its value could be RV_TRUE due to 422         */
        /* received response */
        pCallLeg->pSessionTimer->bAddOnceMinSE = RV_TRUE;
    }

    if (minSE > sessionExpires)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegTranscSessionTimerSetPreferenceParams - call-leg 0x%p: the minSE (%d)value is bigger than the session expires (%d)" ,
            pCallLeg,minSE,sessionExpires));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    info = (CallLegNegotiationSessionTimer*)SipTransactionGetSessionTimerMemory(hTransc);
    if(info == NULL)
    {
        info = (CallLegNegotiationSessionTimer *)SipTransactionAllocateSessionTimerMemory(
                                                       hTransc,
                                                       sizeof(CallLegNegotiationSessionTimer));
        /* Initialize the structure in the default values from the call mgr*/
        CallLegSessionTimerSetMgrDefaults(pCallLeg->pMgr, RV_TRUE, info);
        info->bInRefresh = RV_FALSE;
    }

    info->sessionExpires       = sessionExpires;
    info->minSE                = minSE;
    info->eRefresherPreference = eRefresher;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSessionTimerGetNegotiationParams
 * ------------------------------------------------------------------------
 * General:Gets the negotiation Session Timer parameters associated with this call.
 *         These parameters may not be equal to the Session timer parameters
 *         of the call in the end of the negotiation.(after OK has been sent or
 *         received).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * OutPut:  sessionExpires - session time that will attach to this call.
 *          minSE - minimum session expires time of this call
 *          eRefresherType - the refresher type for this call
 *          eRefresherPref  - the refresher request for this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetNegotiationParams (
         IN   RvSipCallLegHandle                             hCallLeg,
         OUT  RvInt32                                       *sessionExpires,
         OUT  RvInt32                                       *minSE,
         OUT  RvSipSessionExpiresRefresherType              *eRefresherType,
         OUT  RvSipCallLegSessionTimerRefresherPreference   *eRefresherPref)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pCallLeg->pSessionTimer            == NULL ||
        pCallLeg->pNegotiationSessionTimer == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerGetNegotiationParams - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    *sessionExpires   = pCallLeg->pNegotiationSessionTimer->sessionExpires;
    *minSE            = pCallLeg->pNegotiationSessionTimer->minSE;
    *eRefresherType   = pCallLeg->pNegotiationSessionTimer->eRefresherType;
    *eRefresherPref   = pCallLeg->pNegotiationSessionTimer->eRefresherPreference;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
* RvSipCallLegTranscSessionTimerGetNegotiationParams
* ------------------------------------------------------------------------
* General:Gets the negotiation Session Timer parameters associated with this
*         transaction.These parameters may not be equal to the Session timer
*         parameters of the call in the end of the negotiation.
*         (after OK has been sent or received).
*
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hCallLeg   - handle to the call-leg.
*          hTransc  - Handle to the transaction the request belongs to.
* OutPut:  sessionExpires - session time that will attach to this call.
*          minSE - minimum session expires time of this call
*          eRefresherType - the refresher type for this call
*          eRefresherPref  - the refresher request for this call
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegTranscSessionTimerGetNegotiationParams (
           IN  RvSipCallLegHandle                             hCallLeg,
           IN  RvSipTranscHandle                              hTransc,
           OUT RvInt32                                       *sessionExpires,
           OUT RvInt32                                       *minSE,
           OUT RvSipSessionExpiresRefresherType              *eRefresherType,
           OUT RvSipCallLegSessionTimerRefresherPreference   *eRefresherPref)
{
    RvStatus                       rv = RV_OK;
    CallLegNegotiationSessionTimer  *info = NULL;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL ||hTransc == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (pCallLeg->pSessionTimer==NULL ||pCallLeg->pNegotiationSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegTranscSessionTimerGetNegotiationParams - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    info = (CallLegNegotiationSessionTimer*)SipTransactionGetSessionTimerMemory(hTransc);
    if(info == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegTranscSessionTimerGetNegotiationParams - call-leg 0x%p: transc 0x%p session timer parameters not found" ,
                  pCallLeg,hTransc));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_INVALID_HANDLE;
    }
    *sessionExpires = info->sessionExpires;
    *minSE          = info->minSE;
    *eRefresherType = info->eRefresherType;
    *eRefresherPref = info->eRefresherPreference;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSessionTimerGetAlertTime
 * ------------------------------------------------------------------------
 * General: Return the alert time (the time in which refresh will be send
 *          before the session ends).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  alertTime  - the time in which refresh will be send
 *                       before the session ends.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetAlertTime (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *alertTime)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pCallLeg->pSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerGetAlertTime - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_UNKNOWN;
    }

    if (pCallLeg->pSessionTimer->alertTime > 0)
    {
        *alertTime = pCallLeg->pSessionTimer->alertTime;
    }
    else
    {
        *alertTime = pCallLeg->pSessionTimer->defaultAlertTime;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSessionTimerSetAlertTime
 * ------------------------------------------------------------------------
 * General: This function enables the application to modify the time in
 *          in which refresh will be send before the session ends.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  alertTime  - the time in which refresh will be send
 *                       before the session ends.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerSetAlertTime (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    IN  RvInt32             alertTime)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (alertTime < 0)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerSetAlertTime - call-leg 0x%p: alertTime is < 0" ,
            pCallLeg));
        return RV_ERROR_BADPARAM;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pCallLeg->pSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerSetAlertTime - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    /*printing AlertTime value to LOG file*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegSessionTimerSetAlertTime - call-leg 0x%p: alertTime = %d" ,
        pCallLeg,alertTime));

    pCallLeg->pSessionTimer->alertTime = alertTime;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSessionTimerGetRefresherType
 * ------------------------------------------------------------------------
 * General: Returns the refresher type of this call.
 *          The value of refresher type can be different from refresher
 *          preference (which is the application request).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  eRefresher  - the refresher request for this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetRefresherType (
                 IN  RvSipCallLegHandle               hCallLeg,
                 OUT RvSipSessionExpiresRefresherType *eRefresher)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pCallLeg->pSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerGetRefresherType - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
     *eRefresher = pCallLeg->pSessionTimer->eRefresherType;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSessionTimerGetMinSEValue
 * ------------------------------------------------------------------------
 * General:Returns the Min-SE value associated with this call.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  minSE - minimum session expires time of this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetMinSEValue (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *minSE)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pCallLeg->pSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerGetMinSEValue - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    *minSE = pCallLeg->pSessionTimer->minSE;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegSessionTimerGetSessionExpiresValue
 * ------------------------------------------------------------------------
 * General:Returns the Session-Expires value associated with this call
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - handle to the call-leg.
 * Output:  sessionExpires - session expires time of this call
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerGetSessionExpiresValue (
                                    IN  RvSipCallLegHandle   hCallLeg,
                                    OUT RvInt32             *sessionExpires)
{
    RvStatus               rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pCallLeg->pSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerGetSessionExpiresValue - call-leg 0x%p: stack doesn't support session timer" ,
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_UNKNOWN;
    }
    *sessionExpires = pCallLeg->pSessionTimer->sessionExpires;


    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegSessionTimerStopTimer
 * ------------------------------------------------------------------------
 * General: Turns off the loaded session timer (due to previous negotitation)
 *          In case of non-existent session timer this function would be
 *          meaningless.
 *          NOTE: It is highly recommanded to use this function carefully,
 *                when the local endpoint wishes to turn off the mechanism
 *                completely for example by sending a final response with
 *                no Session-Expires header. Otherwise, the stack might
 *                act inconsistently, regarding to the defined SIP standard.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - The timer was shut down successfully
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to refresh the call.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerStopTimer(
                               IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus  rv;

    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSessionTimerStopTimer - Stopping call-leg's 0x%p Session-Timer", pCallLeg));

    if(pCallLeg->pSessionTimer==NULL || pCallLeg->pNegotiationSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerStopTimer - Failed to stop call-leg's 0x%p Session-Timer: there's no support of session timer",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    CallLegSessionTimerReleaseTimer(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}

/*-----------------------------------------------------------------------
       C A L L  - L E G:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, delete the sending of the message, and
 *          change state of the state machine back to previous state.
 *          You can use this function for a call-leg request or for a general
 *          request:
 *          Use this function for INVITE, BYE, RE-INVITE and REFER messages,
 *          if state was changed to MSG_SEND_FAILURE state. For those transaction,
 *          you should set the transaction handle parameter to be NULL.
 *          Use this function for a general request, if you got MSG_SEND_FAILURE
 *          status in RvSipCallLegTranscResolvedEv event. For a general request,
 *          you should supply the general transaction handle.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that sent the request.
 *          hTransc  - Handle to the transaction, in case of general request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSGiveUp (
                                                  IN  RvSipCallLegHandle   hCallLeg,
                                                  IN  RvSipTranscHandle    hTransc)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pCallLeg->hSecAgree || NULL != pCallLeg->hSecObj)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSGiveUp - call-leg 0x%p: illegal action for IMS Secure call",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    if(hTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSGiveUp - call-leg 0x%p: give up DNS" ,
            pCallLeg));
        rv = CallLegDNSGiveUp(pCallLeg);
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSGiveUp - call-leg 0x%p: give up DNS for transaction 0x%p" ,
            pCallLeg, hTransc));

        rv = CallLegDNSTranscGiveUp(pCallLeg, hTransc);
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
#else
    RV_UNUSED_ARG(hTransc);
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegDNSGiveUp - call-leg 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pCallLeg));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipCallLegDNSContinue
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          Calling to this function, clone the failure transaction, with an updated
 *          dns list. (next step for user is to re-send the new transaction message
 *          to the next ip address, by using RvSipCallLegDNSReSendRequest API)
 *          You can use this function for a call-leg request or for a general
 *          request:
 *          Use this function for INVITE, BYE, RE-INVITE and REFER messages,
 *          if state was changed to MSG_SEND_FAILURE state. For those transaction,
 *          you should set the transaction handle parameter to be NULL.
 *          Use this function for a general request, if you got MSG_SEND_FAILURE
 *          status in RvSipCallLegTranscResolvedEv event. For a general request,
 *          you should supply the general transaction handle.
 *          Calling this function for a failure resulting from a 503 response,
 *          will send an ACK on the invite
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that sent the request.
 *          hTransc  - Handle to the transaction, in case of general request.
 * Output:  phClonedTransc - Handle to the cloned transaction - in case of general
 *                      transaction only.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSContinue (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipTranscHandle    hTransc,
                                            OUT RvSipTranscHandle*   phClonedTransc)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif
    if(NULL == pCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pCallLeg->hSecAgree || NULL != pCallLeg->hSecObj)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSContinue - call-leg 0x%p: illegal action for IMS Secure call",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    if(hTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSContinue - call-leg 0x%p: continue DNS" ,
            pCallLeg));

        rv = CallLegDNSContinue(pCallLeg);
    }
    else
    {
        if(phClonedTransc == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegDNSContinue (hCallLeg=0x%p, hTransc=0x%p)- no output parameter, phClonedTransc is NULL",
                pCallLeg, hTransc));
            CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
            return RV_ERROR_NULLPTR;
        }
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSContinue - call-leg 0x%p: continue DNS for transaction 0x%p" ,
            pCallLeg, hTransc));

        rv = CallLegDNSTranscContinue(pCallLeg, hTransc, phClonedTransc);
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
#else
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(phClonedTransc);
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegDNSContinue - call-leg 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pCallLeg));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipCallLegDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state.
 *          This function re-send the failure request.
 *          You should call this function after you have created the new
 *          transaction with RvSipCallLegDNSContinue, and after you set all
 *          needed parameters in the message.
 *          You can use this function for a call-leg request or for a general
 *          request:
 *          Use this function for INVITE, BYE, RE-INVITE and REFER messages,
 *          if state was changed to MSG_SEND_FAILURE state. For those transaction,
 *          you should set the transaction handle parameter to be NULL.
 *          Use this function for a general request, if you got MSG_SEND_FAILURE
 *          status in RvSipCallLegTranscResolvedEv event. For a general request,
 *          you should supply the general transaction handle.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that sent the request.
 *          hTransc  - Handle to the transaction, in case of general request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSReSendRequest (
                                            IN  RvSipCallLegHandle   hCallLeg,
                                            IN  RvSipTranscHandle    hTransc)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif
    if(NULL == pCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (NULL != pCallLeg->hSecAgree || NULL != pCallLeg->hSecObj)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSReSendRequest - call-leg 0x%p: illegal action for IMS Secure call",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    if(hTransc == NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSReSendRequest - call-leg 0x%p: re-send DNS" ,
            pCallLeg));

        rv = CallLegDNSReSendRequest(pCallLeg);
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSReSendRequest - call-leg 0x%p: re-send DNS for transaction 0x%p" ,
            pCallLeg, hTransc));

        rv = CallLegDNSTranscReSendRequest(pCallLeg,hTransc);
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
#else
    RV_UNUSED_ARG(hTransc);
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegDNSReSendRequest - call-leg 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pCallLeg));
    return RV_ERROR_ILLEGAL_ACTION;
#endif

}

/***************************************************************************
 * RvSipCallLegDNSGetList
 * ------------------------------------------------------------------------
 * General: retrieves DNS list object from the transaction object.
 *          You can use this function for a call-leg request or for a general
 *          request:
 *          Use this function for INVITE, BYE, RE-INVITE and REFER messages,
 *          if state was changed to MSG_SEND_FAILURE state. For those transaction,
 *          you should set the transaction handle parameter to be NULL.
 *          Use this function for a general request, if you got MSG_SEND_FAILURE
 *          status in RvSipCallLegTranscResolvedEv event. For a general request,
 *          you should supply the general transaction handle.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg that sent the request.
 *          hTransc  - Handle to the transaction, in case of general request.
 * Output   phDnsList - DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDNSGetList(
                              IN  RvSipCallLegHandle           hCallLeg,
                              IN  RvSipTranscHandle            hTransc,
                              OUT RvSipTransportDNSListHandle *phDnsList)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus rv = RV_OK;
#endif

    if(NULL == pCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if(phDnsList == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    *phDnsList = NULL;

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hTransc == NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSGetList - call-leg 0x%p: getting DNS list" ,
            pCallLeg));

        rv = CallLegDNSGetList(pCallLeg, phDnsList);
    }
    else
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegDNSGetList - call-leg 0x%p: getting DNS list for transaction 0x%p" ,
            pCallLeg, hTransc));

        rv = RvSipTransactionDNSGetList(hTransc, phDnsList);
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
#else
    RV_UNUSED_ARG(hTransc);
    RV_UNUSED_ARG(phDnsList);
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegDNSGetList - call-leg 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pCallLeg));
    return RV_ERROR_ILLEGAL_ACTION;
#endif
}

/***************************************************************************
 * RvSipCallLegNoActiveTransc
 * ------------------------------------------------------------------------
 * General: The function is for B2B usage.
 *          The function verify that there is no transaction in active state
 *          in the call-leg.
 *          For every transaction in the call-leg, that is not in active state,
 *          the function sets the AppTranscHandle to be NULL.
 *
 * Return Value: RV_OK - If there is no active transaction.
 *               RV_ERROR_UNKNOWN - If there is an active transaction.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegNoActiveTransc(
                              IN  RvSipCallLegHandle           hCallLeg)
{
    CallLeg*         pCallLeg = (CallLeg*)hCallLeg;
    RvStatus         rv = RV_OK;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED ||
       pCallLeg->hActiveTransc != NULL )
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegNoActiveTransc - call-leg 0x%p - Illegal state %s",
            pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegNoActiveTransc - call-leg 0x%p" ,pCallLeg));

    rv = CallLegNoActiveTransc(pCallLeg);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Copies an application constructed message into the outbound message
 *          of the call-leg. It is the application responsibility to free this
 *          message after calling the set function.
 *          It is not recommended to use this function and it is not documented.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 *             hMsg     - Handle to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegSetOutboundMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     IN  RvSipMsgHandle            hMsg)
{
    CallLeg               *pCallLeg;
    RvSipMsgHandle         hOutboundMsg = NULL;
    RvStatus              rv;

    pCallLeg = (CallLeg *)hCallLeg;
    if(pCallLeg == NULL || hMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(RV_TRUE  == CallLegGetIsHidden(pCallLeg) &&
	   RV_FALSE == CallLegGetIsRefer(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "RvSipCallLegSetOutboundMsg - call-leg= 0x%p illegal action for hidden call-leg",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    else if(RV_TRUE == CallLegGetIsHidden(pCallLeg) &&
            RV_TRUE == CallLegGetIsRefer(pCallLeg)  &&
            NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        /* need to get the outbound msg of the refer subscription */
        rv = SipSubsSetOutboundMsg(CallLegGetActiveReferSubs(pCallLeg), hMsg);

        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundMsg - call-leg= 0x%p set msg in refer subs 0x%p",
              pCallLeg,CallLegGetActiveReferSubs(pCallLeg)));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }
    /*copy the given message to the stack message pool*/
    rv = RvSipMsgConstruct(pCallLeg->pMgr->hMsgMgr,pCallLeg->pMgr->hMsgMemPool,
                           &hOutboundMsg);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetOutboundMsg - call-leg 0x%p failed to construct outbound message",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    rv = RvSipMsgCopy(hOutboundMsg, hMsg);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetOutboundMsg - call-leg= 0x%p error in constructing message",
            pCallLeg));
        RvSipMsgDestruct(hOutboundMsg);
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    if (NULL != pCallLeg->hOutboundMsg)
    {
        RvSipMsgDestruct(pCallLeg->hOutboundMsg);
        pCallLeg->hOutboundMsg = NULL;
    }

    pCallLeg->hOutboundMsg = hOutboundMsg;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetOutboundMsg - call-leg= 0x%p outbound message 0x%p was set",
              pCallLeg,pCallLeg->hOutboundMsg));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}


/***************************************************************************
 * RvSipCallLegSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the Call-Leg to a SigComp Compartment.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - The call-leg handle.
 *             hCompartment - Handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetCompartment(
                            IN RvSipCallLegHandle     hCallLeg,
                            IN RvSipCompartmentHandle hCompartment)
{
    RV_UNUSED_ARG(hCompartment);
    RV_UNUSED_ARG(hCallLeg);
    
    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipCallLegGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the associated the SigComp Compartment of a Call-Leg
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg      - The call-leg handle.
 *            phCompartment - Pointer of the handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCompartment(
                            IN  RvSipCallLegHandle      hCallLeg,
                            OUT RvSipCompartmentHandle *phCompartment)
{
    RV_UNUSED_ARG(hCallLeg);
    if (NULL != phCompartment)
    {
        *phCompartment = NULL;
    }

    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipCallLegDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a Call-Leg for outgoing
 *          requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCallLeg      - The call-leg handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegDisableCompression(
                                      IN RvSipCallLegHandle hCallLeg)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv       = RV_OK;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegDisableCompression - disabling the compression in call-leg 0x%p",
              pCallLeg));

    pCallLeg->bSigCompEnabled = RV_FALSE;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCallLeg);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipCallLegIsCompartmentRequired
 * ------------------------------------------------------------------------
 * General: Checks if the transaction has to be related to a compartment.
 *          The function is particularly useful for applications that would
 *          like to manage the stack Compartments manually. For instance,
 *          if you wish to relate an incoming CallLeg object, which handles
 *          SigComp messages to a specific Compartment, you can use this
 *          function in your implementation of a callback that notifies
 *          about newly created CallLeg object, before the stack attaches
 *          automatically this object to any Compartment.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg      - The call-leg handle.
 * Output:    pbRequired    - Indication if a compartment is required.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegIsCompartmentRequired(
                                      IN  RvSipCallLegHandle  hCallLeg,
                                      OUT RvBool             *pbRequired)
{
    RV_UNUSED_ARG(hCallLeg);
    RV_UNUSED_ARG(pbRequired);

    return RV_ERROR_ILLEGAL_ACTION;
}

/*-----------------------------------------------------------------------
       C A L L  - L E G:  F O R K I N G   A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCallLegGetOriginalCallLeg
 * ------------------------------------------------------------------------
 * General: An initial INVITE request, might be forked by a proxy. as a result,
 *          several 1xx and 2xx responses may be received from several UASs.
 *          The first incoming response will be mapped to the original call-leg,
 *          that sent the INVITE request. every other incoming response (with a
 *          different to-tag parameter), creates a new 'forked call-leg'.
 *
 *          This function returns the original call-leg of a given call-leg:
 *          If the given call-leg is a 'forked call-leg' the function will
 *          return the original call-leg handle.
 *          If the given call-leg is an 'original call-leg' the function will
 *          return the same call-leg handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg      - Handle to a call-leg.
 * Output:  phOrigCallLeg - Handle to the original call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetOriginalCallLeg(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    OUT RvSipCallLegHandle  *phOrigCallLeg)
{
    RvStatus   rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    CallLegMgr *pMgr;

    if(phOrigCallLeg == NULL  ||  pCallLeg == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    *phOrigCallLeg = NULL;
    pMgr = pCallLeg->pMgr;

    rv = SipCallLegGetOriginalCallLeg(hCallLeg,phOrigCallLeg);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipCallLegGetOriginalCallLeg - Call-leg 0x%p - failed SipCallLegGetOriginalCallLeg() (rv=%d:%s)",
            pCallLeg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "RvSipCallLegGetOriginalCallLeg - call-leg 0x%p - returned original call-leg 0x%p",
          pCallLeg, *phOrigCallLeg));

    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetForkingEnabledFlag
 * ------------------------------------------------------------------------
 * General: This function sets the 'forking-enabled' flag of the call-leg.
 *          This flag defines the call-leg behavior on receiving a 'forked'
 *          1xx/2xx response.
 *          If this flag is set to TRUE, then a new call-leg is created for
 *          every 1xx/2xx response with new to-tag.
 *          If this flag is set to FALSE, then the response will be mapped
 *          to the original call-leg. every 1xx response will update the
 *          call-leg to-tag parameter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForkingEnabledFlag (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvBool               bForkingEnabled)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvStatus   rv;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipCallLegSetForkingEnabledFlag(hCallLeg, bForkingEnabled);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetForkingEnabledFlag - Call-Leg 0x%p - failed to set flag (rv=%d:%s)",
            pCallLeg, rv, SipCommonStatus2Str(rv)));
         CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetForkingEnabledFlag - call-leg 0x%p - forking enabled flag was set to %d",
              pCallLeg, pCallLeg->bForkingEnabled));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSetRejectStatusCodeOnCreation
 * ------------------------------------------------------------------------
 * General: This function can be used synchronously from the
 *          RvSipCallLegCreatedEv callback to instruct the stack to
 *          automatically reject the request that created this call-leg.
 *          In this function you should supply the reject status code.
 *          If you set this status code, the call-leg will be destructed
 *          automatically when the RvSipCallLegCreatedEv returns. The
 *          application will not get any further callbacks
 *          that relate to this call-leg. The application will not get the
 *          RvSipCallLegMsgToSendEv for the reject response message or the
 *          Terminated state for the call-leg object.
 *          This function should not be used for rejecting a request in
 *          a normal scenario. For that you should use the RvSipCallLegReject()
 *          API function. You should use this function only if your application is
 *          incapable of handling this new call-leg at all, for example in an
 *          application out of resource situation.
 *          Note: When this function is used to reject a request, the application
 *          cannot use the outbound message mechanism to add information
 *          to the outgoing response message. If you wish to change the response
 *          message you must use the regular reject function in the Offering
 *          state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - Handle to the call-leg object.
 *           rejectStatusCode - The reject status code for rejecting the request
 *                              that created this object. The value must be
 *                              between 300 to 699.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetRejectStatusCodeOnCreation(
                              IN  RvSipCallLegHandle     hCallLeg,
                              IN  RvUint16               rejectStatusCode)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetRejectStatusCodeOnCreation - call-leg 0x%p, setting reject status to %d",
              pCallLeg, rejectStatusCode));

    /*invalid status code*/
    if (rejectStatusCode < 300 || rejectStatusCode >= 700)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegSetRejectStatusCodeOnCreation - Failed. Call-leg 0x%p. %d is not a valid reject response code",
                  pCallLeg,rejectStatusCode));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }
    if(pCallLeg->eDirection != RVSIP_CALL_LEG_DIRECTION_INCOMING ||
       pCallLeg->eState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegSetRejectStatusCodeOnCreation (hCallLeg=0x%p)- failed. Cannot be called in state %s or for %s call-legs",
                  pCallLeg,CallLegGetStateName(pCallLeg->eState),
                  CallLegGetDirectionName(pCallLeg->eDirection)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;

    }
    pCallLeg->createdRejectStatusCode = rejectStatusCode;
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;


}

/***************************************************************************
 * RvSipCallLegGetForkingEnabledFlag
 * ------------------------------------------------------------------------
 * General: This function returns the 'forking-enabled' flag of the call-leg.
 *          This flag defines the call-leg behavior on receiving a 'forked'
 *          1xx/2xx response.
 *          If this flag is set to TRUE, then a new call-leg is created for
 *          every 1xx/2xx response with new to-tag.
 *          If this flag is set to FALSE, then the response will be mapped
 *          to the original call-leg. every 1xx response will update the
 *          call-leg to-tag parameter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetForkingEnabledFlag(
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvBool               *pbForkingEnabled)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvStatus   rv;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipCallLegGetForkingEnabledFlag(hCallLeg, pbForkingEnabled);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetForkingEnabledFlag - Call-Leg 0x%p - failed to get flag (rv=%d:%s)",
            pCallLeg, rv, SipCommonStatus2Str(rv)));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return rv;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}


/***************************************************************************
 * RvSipCallLegSetForkedAckTimerTimeout
 * ------------------------------------------------------------------------
 * General: The function is DEPRECATED. forked-ack-timer receives the
 *          value of the invite-linger-timer.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          timeout  - The timeout value for the Ack timer. (if 0, the timer will
 *                     not be set).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForkedAckTimerTimeout (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvInt32              timeout)
{
    RV_UNUSED_ARG(hCallLeg)
    RV_UNUSED_ARG(timeout)
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetForked1xxTimerTimeout
 * ------------------------------------------------------------------------
 * General: An initial INVITE request, might be forked by a proxy. as a result,
 *          several 1xx and 2xx responses may be received from several UASs.
 *          The first incoming response will be mapped to the original call-leg,
 *          that sent the INVITE request. every other incoming response (with a
 *          different to-tag parameter), creates a new 'forked call-leg'.
 *
 *          A forked call-leg, that received a 1xx response, sets a timer
 *          ('forked-1xx-timer'). This timer will be released when this
 *          call-leg receive a 2xx response.
 *          If it expires before 2xx was received, the call-leg is terminated.
 *          (usually the timer timeout value is taken from stack configuration).
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg - Handle to the call-leg.
 *          timeout  - The timeout value for the 1xx timer. (if 0, the timer will
 *                     not be set).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetForked1xxTimerTimeout (
                                    IN RvSipCallLegHandle   hCallLeg,
                                    IN RvInt32              timeout)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvStatus   rv;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pCallLeg->forked1xxTimerTimeout = timeout;

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSetForked1xxTimerTimeout - call-leg 0x%p - 1xx timer timeout was set to %d",
              pCallLeg, pCallLeg->forked1xxTimerTimeout));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegSetTranscTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the call-leg's transactions timers.
 *          If some of the fields in pTimers are not set (UNDEFINED), this
 *          function will calculate it, or take the values from configuration.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - Handle to the call-leg object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 *           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetTranscTimers(
                              IN  RvSipCallLegHandle     hCallLeg,
                              IN  RvSipTimers           *pTimers,
                              IN  RvInt32               sizeOfTimersStruct)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvStatus   rv;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = CallLegSetTimers(pCallLeg,pTimers);


    CallLegUnLockAPI(pCallLeg);
    RV_UNUSED_ARG(sizeOfTimersStruct)
    return rv;

}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipCallLegSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message,
 *          belonging to the CallLeg, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - Handle to the call-leg object.
 *           pParams  - Pointer to the struct that contains all the parameters.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSctpMsgSendingParams(
                    IN  RvSipCallLegHandle                 hCallLeg,
                    IN  RvUint32                           sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    CallLeg  *pCallLeg = (CallLeg*)hCallLeg;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(&params,pParams,minStructSize);

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetSctpMsgSendingParams - call-leg 0x%p - failed to lock the CallLeg",
            pCallLeg));
        return RV_ERROR_INVALID_HANDLE;
    }

    SipCallLegSetSctpMsgSendingParams(hCallLeg, &params);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegSetSctpMsgSendingParams - call-leg 0x%p - SCTP params(stream=%d, flag=0x%x) were set",
        pCallLeg, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}

/******************************************************************************
 * RvSipCallLegGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message, belonging
 *          to the CallLeg, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hCallLeg - Handle to the call-leg object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSctpMsgSendingParams(
                    IN  RvSipCallLegHandle                 hCallLeg,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    CallLeg  *pCallLeg = (CallLeg*)hCallLeg;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetSctpMsgSendingParams - call-leg 0x%p - failed to lock the CallLeg",
            pCallLeg));
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    SipCallLegGetSctpMsgSendingParams(hCallLeg, &params);
    memcpy(pParams,&params,minStructSize);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegGetSctpMsgSendingParams - call-leg 0x%p - SCTP params(stream=%d, flag=0x%x) were get",
        pCallLeg, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipCallLegSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Call-Leg.
 *          As a result of this operation, all messages, sent by this Call-Leg,
 *          will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hCallLeg - Handle to the Call-Leg object.
 *          hSecObj  - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSecObj(
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        IN  RvSipSecObjHandle     hSecObj)
{
    RvStatus rv;
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(NULL == pCallLeg)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegSetSecObj - call-leg 0x%p - Set Security Object %p",
        pCallLeg, hSecObj));

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetSecObj - call-leg 0x%p - failed to lock the CallLeg",
            pCallLeg));
        return RV_ERROR_INVALID_HANDLE;
    }
	
	if (pCallLeg->hSecAgree != NULL)
	{
		/* We do not allow setting security-object when there is a security-agreement attached to this call-leg */
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"RvSipCallLegSetSecObj - Call-leg 0x%p - Setting security-object (0x%p) is illegal when the call-leg has a security-agreement (security-agreement=0x%p)",
					pCallLeg, hSecObj, pCallLeg->hSecAgree));
		CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = CallLegSetSecObj(pCallLeg, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetSecObj - call-leg 0x%p - failed to set Security Object %p(rv=%d:%s)",
            pCallLeg, hSecObj, rv, SipCommonStatus2Str(rv)));
		CallLegUnLockAPI(pCallLeg);
        return rv;
    }

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipCallLegGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Call-Leg.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hCallLeg - Handle to the call-leg object.
 *  Output: phSecObj - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSecObj(
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        OUT RvSipSecObjHandle*    phSecObj)
{
    RvStatus rv;
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(NULL==pCallLeg || NULL==phSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegGetSecObj - call-leg 0x%p - Get Security Object",
        pCallLeg));

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetSecObj - call-leg 0x%p - failed to lock the CallLeg",
            pCallLeg));
        return RV_ERROR_INVALID_HANDLE;
    }

    *phSecObj = pCallLeg->hSecObj;

    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * RvSipCallLegUseFirstRouteForInitialRequest
 * ------------------------------------------------------------------------
 * General: Application may want to use a preloaded route header when sending
 *          an initial request. for this purpose, it should add the route
 *          header to the outbound message, and call this function to indicate
 *          the stack that it should send the request to the address of the
 *          first route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - Handle to the call-leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegUseFirstRouteForInitialRequest (
                                      IN  RvSipCallLegHandle       hCallLeg)
{
    RvStatus               rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegUseFirstRouteForInitialRequest - call-leg 0x%p will use preloaded route header",
              pCallLeg));

    pCallLeg->bUseFirstRouteForInitialRequest = RV_TRUE;

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;

}

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipCallLegGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: The function retrieve the auth-object that is currently being
 *          processed by the authenticator.
 *          (for application usage in the RvSipAuthenticatorGetSharedSecretEv
 *          callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg          - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrProcessedAuthObj (
                                      IN   RvSipCallLegHandle    hCallLeg,
                                      OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pAuthListInfo == NULL || pCallLeg->hListAuthObj == NULL)
    {
        *phAuthObj = NULL;
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetCurrProcessedAuthObj - call-leg 0x%p - pAuthListInfo=0x%p, hListAuthObj=0x%p ",
            pCallLeg, pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj ));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_NOT_FOUND;
    }

    *phAuthObj = pCallLeg->pAuthListInfo->hCurrProcessedAuthObj;
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegGetCurrProcessedAuthObj - call-leg 0x%p - currAuthObj=0x%p ",
        pCallLeg, *phAuthObj));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegAuthObjGet
 * ------------------------------------------------------------------------
 * General: The function retrieve auth-objects from the list in the call-leg.
 *          you may get the first/last/next object.
 *          in case of getting the next object, please supply the current
 *          object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg          - Handle to the call-leg.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthObjGet (
                                      IN   RvSipCallLegHandle     hCallLeg,
                                      IN   RvSipListLocation      eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj)
{
    RvStatus     rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || phAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pAuthListInfo == NULL || pCallLeg->hListAuthObj == NULL)
    {
        *phAuthObj = NULL;
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_NOT_FOUND;
    }

    rv = SipAuthenticatorAuthListGetObj(pCallLeg->pMgr->hAuthMgr, pCallLeg->hListAuthObj,
                                       eLocation, hRelativeAuthObj, phAuthObj);
    
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegAuthObjGet - call-leg 0x%p - got AuthObj=0x%p ",
        pCallLeg, *phAuthObj));

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an auth-obj from the list in the call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call-leg.
 *            hAuthObj - The Auth-Obj handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegAuthObjRemove (
                                      IN   RvSipCallLegHandle   hCallLeg,
                                      IN   RvSipAuthObjHandle   hAuthObj)
{
    RvStatus     rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL || hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pAuthListInfo == NULL || pCallLeg->hListAuthObj == NULL)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_NOT_FOUND;
    }

    rv = SipAuthenticatorAuthListRemoveObj(pCallLeg->pMgr->hAuthMgr,
                                           pCallLeg->hListAuthObj, hAuthObj);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipCallLegSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header in the outgoing request.
 *          An initial authorization header is a header that contains
 *          only the client private identity, and not real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The call-leg will set the initial header to the message, only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg       - The call-leg handle.
 *            hAuthorization - The initial Authorization header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetInitialAuthorization (
                                         IN RvSipCallLegHandle hCallLeg,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization)
{
    CallLeg          *pCallLeg;
    RvStatus            retStatus;

    if (NULL == hCallLeg || NULL == hAuthorization)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pCallLeg = (CallLeg *)hCallLeg;
    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegSetInitialAuthorization - Setting in call-leg 0x%p ",pCallLeg));

    /* Assert that the current state allows this action */
    if (RVSIP_CALL_LEG_STATE_IDLE != pCallLeg->eState &&
        RVSIP_CALL_LEG_STATE_UNAUTHENTICATED != pCallLeg->eState)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSetInitialAuthorization - call-leg 0x%p - illegal state %s",
            pCallLeg, CallLegGetStateName(pCallLeg->eState)));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Check if the given header was constructed on the call-leg
       object's page and if so attach it */
    if((SipAuthorizationHeaderGetPool(hAuthorization) == pCallLeg->pMgr->hGeneralPool) &&
       (SipAuthorizationHeaderGetPage(hAuthorization) == pCallLeg->hPage))
    {
        pCallLeg->hInitialAuthorization = hAuthorization;
    }
    else /* need to copy the given header to the call-leg page */
    {
        if(pCallLeg->hInitialAuthorization == NULL)
        {
            retStatus = RvSipAuthorizationHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                                        pCallLeg->pMgr->hGeneralPool,
                                                        pCallLeg->hPage,
                                                        &pCallLeg->hInitialAuthorization);
            if(retStatus != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RvSipCallLegSetInitialAuthorization - call-leg 0x%p - Failed to construct header (retStatus=%d)",
                    pCallLeg, retStatus));
                CallLegUnLockAPI(pCallLeg);
                return retStatus;
            }
        }
        retStatus = RvSipAuthorizationHeaderCopy(pCallLeg->hInitialAuthorization, hAuthorization);
        if(retStatus != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RvSipCallLegSetInitialAuthorization - call-leg 0x%p - Failed to copy header (retStatus=%d)",
                pCallLeg, retStatus));
            CallLegUnLockAPI(pCallLeg);
            return retStatus;
        }
    }
    CallLegUnLockAPI(pCallLeg);
    return RV_OK;
}

/***************************************************************************
 * RvSipCallLegSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this call-leg. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the call-leg will exploit this agreement and the data
 *          it brings. If not, the call-leg will use this security-agreement
 *          object to negotiate an agreement with the remote party.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg   - Handle to the call-leg.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSetSecAgree(
                    IN  RvSipCallLegHandle           hCallLeg,
                    IN  RvSipSecAgreeHandle          hSecAgree)
{
	CallLeg          *pCallLeg;
	RvStatus          rv;

    if (hCallLeg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pCallLeg = (CallLeg *)hCallLeg;

	if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* if the call-leg was terminated we do not allow the attachment */
	if (RVSIP_CALL_LEG_STATE_TERMINATED == pCallLeg->eState)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"RvSipCallLegSetSecAgree - Call-leg 0x%p - Attaching security-agreement 0x%p is forbidden in state Terminated",
					pCallLeg, hSecAgree));
		CallLegUnLockAPI(pCallLeg);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = CallLegSecAgreeAttachSecAgree(pCallLeg, hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"RvSipCallLegSetSecAgree - call-leg 0x%p - Failed to attach security-agreement=0x%p (retStatus=%d)",
					pCallLeg, hSecAgree, rv));
	}

	CallLegUnLockAPI(pCallLeg);

	return rv;
}

/***************************************************************************
 * RvSipCallLegGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the call-leg.
 *          The security-agreement object captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg    - Handle to the call-leg.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetSecAgree(
									IN   RvSipCallLegHandle      hCallLeg,
									OUT  RvSipSecAgreeHandle    *phSecAgree)
{
	CallLeg          *pCallLeg = (CallLeg *)hCallLeg;

    if (NULL == hCallLeg)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (NULL == phSecAgree)
	{
		return RV_ERROR_BADPARAM;
	}

    if (RV_OK != CallLegLockAPI(pCallLeg))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	*phSecAgree = pCallLeg->hSecAgree;

	CallLegUnLockAPI(pCallLeg);

	return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /*RV_SIP_PRIMITIVES */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * CallLegNoActiveTransc
 * ------------------------------------------------------------------------
 * General: The function is for B2B usage.
 *          The function verify that there is no transaction in active state
 *          in the call-leg.
 *          For every transaction in the call-leg, that is not in active state,
 *          the function sets the AppTranscHandle to be NULL.
 *
 * Return Value: RV_OK - If there is no active transaction.
 *               RV_ERROR_UNKNOWN - If there is an active transaction.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 ***************************************************************************/
static RvStatus RVCALLCONV CallLegNoActiveTransc(IN  CallLeg* pCallLeg)
{
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    CallLegInvite        *pInvite = NULL;
#ifdef RV_SIP_SUBS_ON
    RvSipSubsHandle      hSubs = NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
    RvStatus rv = RV_OK;
    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = pCallLeg->pMgr->maxNumOfTransc*2;  /*actual loops should be maximum - maxNumOfTransc*/

    /*------------------------------------------------------
       Search for an active transaction in general transaction list
      ------------------------------------------------------*/
    rv = CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_VERIFY_NO_ACTIVE, NULL);
    if (rv != RV_OK)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegNoActiveTransc - Call-leg 0x%p - Found an active transaction",
                  pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    /*------------------------------------------------------
       Search for an active transaction in invite objects list
      ------------------------------------------------------*/
    safeCounter = 0;
    pItem = NULL;

    /*get the first item from the general transactions list*/
    RLIST_GetHead(pCallLeg->pMgr->hInviteObjsListPool, pCallLeg->hInviteObjsList, &pItem);

    pInvite = (CallLegInvite*)pItem;

    /*go over all the transactions*/
    while(pInvite != NULL && pInvite->hInviteTransc != NULL && safeCounter < maxLoops)
    {
        /* check the transaction state */
        /* check the transaction state */
        if(RV_TRUE == SipTransactionIsInActiveState(pInvite->hInviteTransc))
        {
            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegNoActiveTransc - Call-leg 0x%p - Found an active INVITE transaction",
                  pCallLeg));
            return RV_ERROR_UNKNOWN;
        }

        /* get next transactions */
        RLIST_GetNext(pCallLeg->pMgr->hInviteObjsListPool, pCallLeg->hInviteObjsList,
                    pItem,
                    &pNextItem);
        pItem = pNextItem;
        pInvite = (CallLegInvite*)pItem;
        safeCounter++;
    }

    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegNoActiveTransc - Call-leg 0x%p - Reached MaxLoops %d, infinite loop",
                  pCallLeg,maxLoops));
        return RV_ERROR_UNKNOWN;
    }

     /*------------------------------------------------------
       Search for an active transaction in subscriptions list
      ------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
    /* check if there is any subscription - check every one for state terminated???? */
    RvSipCallLegGetSubscription((RvSipCallLegHandle)pCallLeg, RVSIP_FIRST_ELEMENT, hSubs, &hSubs);
    if (hSubs != NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegNoActiveTransc: call-leg 0x%p. subscription 0x%p was found",
            pCallLeg, hSubs));
        return RV_ERROR_UNKNOWN;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */


    /*------------------------------------------------------
       Set AppTranscHandle of all transactions in transaction list
       to be NULL.
      ------------------------------------------------------*/
    CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_RESET_APP_HANDLE_ALL_TRANSC, NULL);
    
    return RV_OK;
}

/******************************************************************************
 * CallLegBlockTermination
 * ----------------------------------------------------------------------------
 * General: The function checks if we need to block the terminate API because
 *          the call-leg is in the middle of a callback that does not allow termination.
 * Return Value: RV_TRUE - should block the Terminate function.
 *               RV_FALSE - should not block it.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg      - Pointer to the Call-Leg object.
 ******************************************************************************/
static RvBool CallLegBlockTermination(IN CallLeg* pCallLeg)
{
    RvInt32 res = 0;
    res = (pCallLeg->cbBitmap) & (pCallLeg->pMgr->terminateMask); 
    if (res > 0)
    {
        /* at least one of the bits marked in the mask, are also turned on
           in the cb bitmap */
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegBlockTermination - call-leg 0x%p - can not terminate while inside callback %s",
                  pCallLeg, CallLegGetCbName(res)));
        return RV_TRUE;
    }
    return RV_FALSE;
    
}

/******************************************************************************
 * CallLegBlockCancellation
 * ----------------------------------------------------------------------------
 * General: The function checks if we need to block the cancel API because
 *          the call-leg is in the middle of a callback that does not allow termination.
 * Return Value: RV_TRUE - should block the Terminate function.
 *               RV_FALSE - should not block it.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg      - Pointer to the Call-Leg object.
 ******************************************************************************/
static RvBool CallLegBlockCancellation(IN CallLeg* pCallLeg, RvChar* funcName)
{
    RvInt32 res = 0;
    res = (pCallLeg->cbBitmap) & (pCallLeg->pMgr->cancelMask); 
    if (res > 0)
    {
        /* at least one of the bits marked in the mask, are also turned on
           in the cb bitmap */
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "%s - call-leg 0x%p - can not cancel/disconnect while inside callback %s",
                  funcName, pCallLeg, CallLegGetCbName(res)));
        return RV_TRUE;
    }
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(funcName);
#endif

    return RV_FALSE;
    
}


/*-----------------------------------------------------------------------
       C A L L  - L E G:  D P R E C A T E D   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipCallLegModify
 * ------------------------------------------------------------------------
 * General: The function is deprecated.
 *          You should use RvSipCallLegReInviteCreate() and RvSipCallLegReInviteRequest().
 *			NOTE: This function doesn't refer to
 *			Session-Timer Call-Leg's parameters. Thus, when used during a
 *			Session Timer call, it turns off the Session Timer mechanism.
 *			Consequently, in order to keep the mechanism up please use
 *			the function RvSipCallLegSessionTimerInviteRefresh() instead.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegModify (IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus        rv;
    CallLeg         *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(RV_FALSE == CallLegIsStateLegalForReInviteHandling(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegModify - Call leg 0x%p illegal state (%s + active transc = 0x%p + oldInvite=%d)",
             pCallLeg, CallLegGetStateName(pCallLeg->eState), pCallLeg->hActiveTransc, pCallLeg->pMgr->bOldInviteHandling));

        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = CallLegSeesionCreateAndModify(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegGetCurrentModifyState
 * ------------------------------------------------------------------------
 * General: The function is deprecated.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetCurrentModifyState (
                            IN  RvSipCallLegHandle       hCallLeg,
                            OUT RvSipCallLegModifyState  *peModifyState)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    CallLegInvite*  pInvite = NULL;

    if(pCallLeg == NULL || peModifyState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bOldInviteHandling == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegGetCurrentModifyState - call-leg 0x%p illegal action when working with new invite",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED)
    {
        /* initial invite is still active */
        *peModifyState = RVSIP_CALL_LEG_MODIFY_STATE_UNDEFINED;
    }
    else if(pCallLeg->hActiveTransc != NULL)
    {
        /* a re-invite attempt might be active */
        rv = CallLegInviteFindObjByTransc(pCallLeg, pCallLeg->hActiveTransc, &pInvite);
        if(rv == RV_OK && pInvite != NULL && pInvite->bInitialInvite == RV_FALSE)
        {
            *peModifyState = pInvite->eModifyState;
            if(*peModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED ||
               *peModifyState == RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT ||
               *peModifyState == RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD)
            {
                *peModifyState = RVSIP_CALL_LEG_MODIFY_STATE_IDLE;
            }
        }
        else /* no re-invite object. application may modify the call now */
        {
            *peModifyState = RVSIP_CALL_LEG_MODIFY_STATE_IDLE;
        }
    }
    else
    {
        /* application may modify the call now. */
        *peModifyState = RVSIP_CALL_LEG_MODIFY_STATE_IDLE;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}

/***************************************************************************
 * RvSipCallLegGetOutboundAckMsg
 * ------------------------------------------------------------------------
 * General: The function is deprecated.
 *          Use the regular function - RvSipCallLegGetOutboundMsg() in order
 *          to get the ACK message, before sending the ACK.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - The call-leg handle.
 * Output:    phMsg   -  A pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipCallLegGetOutboundAckMsg(
                                     IN  RvSipCallLegHandle     hCallLeg,
                                     OUT RvSipMsgHandle         *phMsg)
{
    CallLeg               *pCallLeg;
    RvStatus               rv;

    pCallLeg = (CallLeg *)hCallLeg;
    if(pCallLeg == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phMsg = NULL;

    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = RvSipCallLegGetOutboundMsg(hCallLeg, phMsg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegSessionTimerRefresh
 * ------------------------------------------------------------------------
 * General: The function is DEPRECATED
 *          Use RvSipCallLegReInviteCreate() and
 *              RvSipCallLegSessionTimerInviteRefresh()
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegSessionTimerRefresh(
                               IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus  rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegSessionTimerRefresh - Refreshing call-leg 0x%p", pCallLeg));

    if(RV_FALSE == pCallLeg->pMgr->bOldInviteHandling)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerRefresh - call-leg 0x%p: Illegal Action with new invite handling",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*modify can be called only when the call is connected and no modify or
      modifying is taking place*/
    if(pCallLeg->eState != RVSIP_CALL_LEG_STATE_CONNECTED ||
       pCallLeg->hActiveTransc != NULL ||
       NULL != CallLegGetActiveReferSubs(pCallLeg))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "RvSipCallLegSessionTimerRefresh - Failed to refresh call-leg 0x%p: Illegal Action with active transc 0x%p",
               pCallLeg,pCallLeg->hActiveTransc));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    if(pCallLeg->pSessionTimer==NULL || pCallLeg->pNegotiationSessionTimer==NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RvSipCallLegSessionTimerRefresh - Failed to refresh call-leg 0x%p: stack doesn't support session timer",
            pCallLeg));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;

    }
    rv = CallLegSessionTimerRefresh(pCallLeg, NULL);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegReplacesPrepareHeaderFromCallLeg
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED. Application should build the replaces
 *          header by itself, considering the call-leg direction (incoming or
 *          outgoing) and whether the header should replace for the local
 *          call-leg, or for the remote peer call-leg.
 *
 *          This function prepares a Replaces header from the Call-ID, from-tag and
 *          to-tag of the given Call-Leg. the function takes the parameters of the
 *          given call-leg as is.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg        - Handle to the call leg to make the Replaces header from.
 *          hReplacesHeader - Handle to a Constructed Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesPrepareHeaderFromCallLeg(
                                      IN    RvSipCallLegHandle         hCallLeg,
                                      IN    RvSipReplacesHeaderHandle  hReplacesHeader)
{
    RvStatus  rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(hReplacesHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReplacesPrepareHeaderFromCallLeg - Prepares a Replaces header from Call Leg 0x%p",pCallLeg));

    rv = CallLegReplacesPrepareReplacesHeaderFromCallLeg(pCallLeg, hReplacesHeader);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * RvSipCallLegRouteListAdd
 * ------------------------------------------------------------------------
 * General: Given route value or values (the values are seperated by comma), 
 *          the function adds Route header(s) to the given hCallLeg.
 *          The header(s) are always inserted to the head of the route list.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 *            routeStr - Route header value.
 ***************************************************************************/
RVAPI RvStatus RvSipCallLegRouteListAdd(
                        IN  RvSipCallLegHandle      hCallLeg,
                        IN  RvChar                  *nexthop,
                        IN  RvChar                  *routeStr)       
{
    CallLeg                   *pCallLeg = (CallLeg*)hCallLeg;
    RvStatus                  rv;
    RvChar*                   pStr = routeStr;
    RvChar*                   token;

#if defined(UPDATED_BY_SPIRENT)
    // To prevent memory leak.
    if(pCallLeg->hListPage == NULL_PAGE)
    {
       rv = RPOOL_GetPage(pCallLeg->pMgr->hGeneralPool,0,&(pCallLeg->hListPage));
       if(rv != RV_Success)
       {
          pCallLeg->hListPage = NULL_PAGE;
          return RV_ERROR_OUTOFRESOURCES;
       }
    }
#else
   rv = RPOOL_GetPage(pCallLeg->pMgr->hGeneralPool,0,&(pCallLeg->hListPage));
   if(rv != RV_Success)
   {
      pCallLeg->hListPage = NULL_PAGE;
      return RV_ERROR_OUTOFRESOURCES;
   }
#endif

   /*--------------------------------------
      Construct the Route list if needed
   -----------------------------------------*/
   if( pCallLeg->hRouteList == NULL )
   {
      
      pCallLeg->hRouteList = RLIST_RPoolListConstruct(pCallLeg->pMgr->hGeneralPool,
                                                  pCallLeg->hListPage, sizeof(void*),
                                                  pCallLeg->pMgr->pLogSrc);
      if(pCallLeg->hRouteList == NULL)
      {
         RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "%s - Failed for Call-leg 0x%p, Failed to construct a new list",__FUNCTION__,pCallLeg));
         return RV_ERROR_UNKNOWN;
      }
   }

   if(nexthop && *nexthop) {
     rv = CallLegRouteListAdd( pCallLeg, pCallLeg->hListPage, pCallLeg->hRouteList, nexthop );
     if(rv != RV_Success)
     {
       RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "%s - Failed for Call-leg 0x%p, Failed to add to route list on page %d",
         __FUNCTION__,pCallLeg,pCallLeg->hListPage));
       return rv;
     }
   }

   // Parse route values and add to route list.
   while( pStr && *pStr)
   {
     char ch=0;
     token = strchr( pStr, ',' );
     if( token ) {
       ch=*token;
       *token = '\0';
     }

     rv = CallLegRouteListAdd( pCallLeg, pCallLeg->hListPage, pCallLeg->hRouteList, pStr );
     if(rv != RV_Success)
     {
       if(token) *token=ch;
       RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "%s - Failed for Call-leg 0x%p, Failed to add to route list on page %d",
         __FUNCTION__,pCallLeg,pCallLeg->hListPage));
       return rv;
     }
     
     if( token ) {
       *token=ch;
       pStr = token + 1;
     } else {
       pStr = NULL;
     }
   }

   return RV_OK;
}

int RvSipCallLegGetCct(IN  RvSipCallLegHandle hCallLeg) {

   if(!hCallLeg) return -1;
   else {
      CallLeg *cl = (CallLeg*)hCallLeg;
      if(cl->cctContext<0) {
         if(cl->hActiveTransc && RvSipTransactionGetCct(cl->hActiveTransc)>=0) {
            cl->cctContext = RvSipTransactionGetCct(cl->hActiveTransc);
         }
      }
      return cl->cctContext;
   }
}

void RvSipCallLegSetCct(IN  RvSipCallLegHandle hCallLeg,int cct) {
   if(hCallLeg) {
      CallLeg *cl=(CallLeg*)hCallLeg;
      cl->cctContext = cct;
      if(cct>=0) {
         if(cl->hActiveTransc) RvSipTransactionSetCct(cl->hActiveTransc,cl->cctContext);
      }
   }
}

int RvSipCallLegGetURIFlag(IN  RvSipCallLegHandle hCallLeg) {

   if(!hCallLeg) return 0xFF;
   else {
      CallLeg *cl = (CallLeg*)hCallLeg;
      return cl->URI_Cfg_Flag;
   }
}

void RvSipCallLegSetURIFlag(IN  RvSipCallLegHandle hCallLeg,int uri_cfg) {
   if(hCallLeg) {
      CallLeg *cl=(CallLeg*)hCallLeg;
      cl->URI_Cfg_Flag=uri_cfg;
      if(cl->hActiveTransc) RvSipTransactionSetURIFlag(cl->hActiveTransc,uri_cfg);
   }
}

const RvChar* RvSipCallLegGetLocalIpStrFromActiveTransc(IN RvSipCallLegHandle hCallLeg) {
   if(!hCallLeg) return NULL;
   else {
      CallLeg *cl=(CallLeg*)hCallLeg;
      if(!cl->hActiveTransc) return NULL;
      else {
         return RvSipTransactionGetLocalIpStr(cl->hActiveTransc);
      }
   }
}

RvInt32 RvSipCallLegGetLocalPortFromActiveTransc(IN RvSipCallLegHandle hCallLeg) {
   if(!hCallLeg) return -1;
   else {
      CallLeg *cl=(CallLeg*)hCallLeg;
      if(!cl->hActiveTransc) return -1;
      else {
         return RvSipTransactionGetLocalPort(cl->hActiveTransc);
      }
   }
}


#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/*- D P R E C A T E D   S E T / G E T   F U N C T I O N S --------------------*/
/***************************************************************************
 * RvSipCallLegGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg.
 * Output:     phParty - Handle to the newly created party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewPartyHeaderHandle (
                                      IN   RvSipCallLegHandle      hCallLeg,
                                      OUT  RvSipPartyHeaderHandle  *phParty)
{
    return RvSipCallLegGetNewMsgElementHandle(hCallLeg, RVSIP_HEADERTYPE_FROM,
                                          RVSIP_ADDRTYPE_UNDEFINED, (void**)phParty);
}

/***************************************************************************
 * RvSipCallLegGetNewReplacesHeaderHandle
 * ------------------------------------------------------------------------
 * General:DEPRECATED!!! use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call-leg.
 * Output:     phReplaces - Handle to the newly created Replaces header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReplacesHeaderHandle (
                                      IN   RvSipCallLegHandle         hCallLeg,
                                      OUT  RvSipReplacesHeaderHandle  *phReplaces)
{
    return RvSipCallLegGetNewMsgElementHandle(hCallLeg, RVSIP_HEADERTYPE_REPLACES,
                                          RVSIP_ADDRTYPE_UNDEFINED, (void**)phReplaces);
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetNewReferToHeaderHandle
 * ------------------------------------------------------------------------
 * General:This function is DEPRECATED!!! you should use the subscription refer API.
 *         In order to use it for code that written for earlier stack version,
 *         you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *         Allocates a new Refer-To header and returns the new Refer-To header
 *         handle.
 *         In order to set the Refer-To header of a call-leg ,the application
 *         should:
 *         1. Get a new Refer-To header handle using this function.
 *         2. Fill the Refer-To header information and set it back using
 *            RvSipCallLegRefer().
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer to party handle was given.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to allocate.
 *               RV_OK        - New party header was allocated successfully
 *                                   and its handle was returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call-leg.
 * Output:     phReferTo - Handle to the newly created Refer-To header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReferToHeaderHandle (
                                      IN   RvSipCallLegHandle         hCallLeg,
                                      OUT  RvSipReferToHeaderHandle  *phReferTo)
{
    RvStatus  rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phReferTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    
    *phReferTo = NULL;

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferToHeaderHandle - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferToHeaderHandle - getting new Refer-To handle from call-leg 0x%p",pCallLeg));

    rv = RvSipReferToHeaderConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool,
                                   pCallLeg->hPage, phReferTo);
    if (rv != RV_OK || *phReferTo == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferToHeaderHandle - Failed to create new Refer-To header (rv=%d)",
              rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_OUTOFRESOURCES;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetNewReferredByHeaderHandle
 * ------------------------------------------------------------------------
 * General:This function is DEPRECATED!!! you should use the subscription refer API.
 *         In order to use it for code that written for earlier stack version,
 *         you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *         Allocates a new Referred-By header and returns the new Referred-By header
 *         handle.
 *         In order to set the Referred-By header of a call-leg ,the application
 *         should:
 *         1. Get a new Referred-By header handle using this function.
 *         2. Fill the Referred-By header information and set it back using
 *            RvSipCallLegRefer().
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer to party handle was given.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to allocate.
 *               RV_OK        - New party header was allocated successfully
 *                                   and its handle was returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call-leg.
 * Output:     phReferredBy - Handle to the newly created Referred-By header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewReferredByHeaderHandle (
                                      IN   RvSipCallLegHandle            hCallLeg,
                                      OUT  RvSipReferredByHeaderHandle  *phReferredBy)
{
    RvStatus  rv;
    CallLeg  *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phReferredBy == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phReferredBy = NULL;

    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferredByHeaderHandle - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferredByHeaderHandle - getting new Referred-By handle from call-leg 0x%p",pCallLeg));

    rv = RvSipReferredByHeaderConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool,
                                   pCallLeg->hPage, phReferredBy);
    if (rv != RV_OK || *phReferredBy == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetNewReferredByHeaderHandle - Failed to create new Referred-By header (rv=%d)",
              rv));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_OUTOFRESOURCES;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetReferToAddress
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Gets the Refer-To address associated with the call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - Handle to the call-leg.
 * Output:     phReferToAddress - Handle to the Refer-To address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReferToAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle  *phReferToAddress)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvSipReferToHeaderHandle hReferTo;

    if(pCallLeg == NULL || phReferToAddress == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetReferToAddress - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL == pCallLeg->hActiveReferSubs)
    {
        *phReferToAddress = NULL;
    }
    else
    {
        rv = RvSipSubsGetReferToHeader(pCallLeg->hActiveReferSubs, &hReferTo);
        if(hReferTo != NULL)
        {
            *phReferToAddress = RvSipReferToHeaderGetAddrSpec(hReferTo);
        }
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegGetReferredByAddress
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Gets the Referred-By address associated with the call-leg.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The call-leg handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg        - Handle to the call-leg.
 * Output:     phReferredByAddress - Handle to the Referred-By address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetReferredByAddress (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      OUT RvSipAddressHandle  *phReferredByAddress)
{
    RvStatus               rv;
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL || phReferredByAddress == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the call-leg*/
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegGetReferredByAddress - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL == pCallLeg->hReferredByHeader)
    {
        *phReferredByAddress = NULL;
    }
    else
    {
        *phReferredByAddress = RvSipReferredByHeaderGetAddrSpec(pCallLeg->hReferredByHeader);
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RvSipCallLegGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General: DEPRECATED!!! use RvSipCallLegGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  - Handle to the call-leg.
 *            eAddrType - Type of address the application wishes to create.
 * Output:     phAddr    - Handle to the newly created address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegGetNewAddressHandle (
                                      IN   RvSipCallLegHandle      hCallLeg,
                                      IN  RvSipAddressType         eAddrType,
                                      OUT  RvSipAddressHandle      *phAddr)
{
    return RvSipCallLegGetNewMsgElementHandle(hCallLeg, RVSIP_HEADERTYPE_UNDEFINED,
                                          eAddrType, (void**)phAddr);
}

/*- D P R E C A T E D   R E F E R   F U N C T I O N S --------------------*/
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegRefer
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Sends a REFER associated with the call-leg. This function may be
 *          called only after the To and From header fields were set.
 *          Calling Refer causes a REFER request to be sent out and the
 *          call-leg refer state machine to progress to the Refer Sent
 *          state.
 *          This function is also used to send an authenticated refer
 *          request in the RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 *               RV_OK - REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - Handle to the call leg the user wishes to send REFER.
 *          hReferTo     - The Refer-To header to be sent in the REFER request.
 *                         The Refer-To header will be kept in the call-leg object, and can
 *                         contain Replaces header.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Refer-To header will
 *                         be taken from the call-leg object.
 *          hReferredBy  - The Referred-By header to be sent in the REFER request.
 *                         The Referred-By header will be kept in the call-leg object.
 *                         This parameter is optional. If not specified the call-leg
 *                         will use a default Referred-By header.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Referred-By header will
 *                         be taken from the call-leg object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRefer (
                                      IN  RvSipCallLegHandle          hCallLeg,
                                      IN  RvSipReferToHeaderHandle    hReferTo,
                                      IN  RvSipReferredByHeaderHandle hReferredBy)
{
    RvStatus   rv;
    CallLeg   *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRefer - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pCallLeg->pMgr->hSubsMgr == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRefer - call-leg 0x%p - illegal action. (subsMgr = NULL)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegReferCreateSubsAndRefer(pCallLeg, hReferTo, hReferredBy, NULL,NULL,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegRefer - Failed for call-leg 0x%p (rv=%d)",
              pCallLeg, rv));
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferStr
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Sends a REFER associated with the call-leg. This function may be
 *          called only after the To and From header fields were set.
 *          Calling Refer causes a REFER request to be sent out and the
 *          call-leg refer state machine to progress to the Refer Sent
 *          state.
 *          This function is also used to send an authenticated refer
 *          request in the RVSIP_CALL_LEG_REFER_STATE_REFER_UNAUTHENTICATED.
 *          Note: this function is equivalent to RvSipCallLegRefer.The difference
 *          is that in this function the Refer-To and Referred-By parameters
 *          are given as strings.
 *          The strings may be addresses (for example: "sip:bob@172.20.1.20:5060"),
 *          for backward compatability, or headers strings ( for example:
 *          "Refer-To: A<sip:172.20.1.20:5060>")
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid call-leg state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - REFER message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - Handle to the call leg the user wishes to send REFER.
 *          strReferTo   - The Refer-To address to be sent in the Refer-To
 *                         header of the REFER request. or the Refer-To header itself.
 *                         The Refer-To header will be kept in the call-leg object.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Refer-To header will
 *                         be taken from the call-leg object.
 *          strReferredBy - The Referred-By address to be sent in the
 *                         Referred-By header of the REFER request, or the
 *                         Referred-By header itself.
 *                         The Referred-By header will be kept in the call-leg object.
 *                         This parameter is optional. If not specified the call-leg
 *                         will note set a Referred-By header.
 *          strReplaces  - The Replaces header to be sent in the Refer-To
 *                         header of the REFER request. The Replaces header string doesn't
 *                         contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                         the call-leg object.
 *                         When using this function for authenticating or
 *                         redirecting a previously sent refer request you can
 *                         set this parameter to NULL. The Replaces header will
 *                         be taken from Refer-To header in the call-leg object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferStr (
                                      IN  RvSipCallLegHandle   hCallLeg,
                                      IN  RvChar              *strReferTo,
                                      IN  RvChar              *strReferredBy,
                                      IN  RvChar              *strReplaces)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferStr - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = CallLegReferCreateSubsAndRefer(pCallLeg, NULL, NULL, strReferTo,strReferredBy,strReplaces);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferStr - Failed for call-leg 0x%p (rv=%d)",
              pCallLeg, rv));
        CallLegUnLockAPI(pCallLeg);
        return rv;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferCancel
 * ------------------------------------------------------------------------
 * General: This function is obsolete. Cancel on REFER is illegal.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferCancel (
                                    IN  RvSipCallLegHandle   hCallLeg)
{
    RV_UNUSED_ARG(hCallLeg);
    return RV_ERROR_ILLEGAL_ACTION;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferAccept
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Call this function in the "Refer Received" state to accept the
 *          REFER request.
 *          This function will do the following:
 *          1. send a  202-Accepted response to the REFER request.
 *          2. create a new call-leg object.
 *          3. Set the following parameter to the new call-leg:
 *              * Call-Id: the Call-Id of the REFER request.
 *              * To header: The Refer-To header of the REFER request.
 *              * From header: The local contact
 *              * Referred-By: The Referred-By header of the RFEER request.
 *          After calling this function you should connect the new call-leg to
 *          the referenced user agent by calling RvSipCallLegConnect().
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the new call-leg handle is
 *                                  invalid.
 *                 RV_ERROR_ILLEGAL_ACTION -  Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN       -  Failed to accept the call. (failed
 *                                   while trying to send the 202 response, or
 *                                   to create and initialize the new call-leg).
 *               RV_OK       -  Accepted final response was sent successfully,
 *                                   and a new call-leg was created and initialized
 *                                   successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to accept refer from.
 *          hAppCallLeg - Application handle to the newly created call-leg.
 * Output:  phNewCallLeg - The new call-leg that is created and initialized by
 *                        this function.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferAccept (
                                        IN  RvSipCallLegHandle    hCallLeg,
                                        IN  RvSipAppCallLegHandle hAppCallLeg,
                                        OUT RvSipCallLegHandle   *phNewCallLeg)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferAccept - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferAccept - Accepting REFER request in call-leg 0x%p",pCallLeg));

    /*accept can be called only on offering and connected/modified*/
    if(((pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE) ||
        (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)) &&
        ((pCallLeg->hActiveReferSubs != NULL) &&
        (SipSubsGetCurrState(pCallLeg->hActiveReferSubs) == RVSIP_SUBS_STATE_SUBS_RCVD)))
    {
        rv = CallLegReferAccept(pCallLeg, hAppCallLeg, phNewCallLeg);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                     "RvSipCallLegReferAccept - call-leg 0x%p - Failed. Illegal Action in state %s, refer sub state %s, hActiveSubs = 0x%p",
                  pCallLeg, CallLegGetStateName(pCallLeg->eState),
                  CallLegReferGetSubsStateName(pCallLeg), pCallLeg->hActiveReferSubs));
        rv = RV_ERROR_ILLEGAL_ACTION;
    }

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferReject
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Can be used in the "Refer Received" refer state to reject an
 *          incoming REFER request.
 * Return Value: RV_ERROR_INVALID_HANDLE    -  The handle to the call-leg is invalid.
 *               RV_ERROR_BADPARAM - The status code is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid call-leg state for this action.
 *               RV_ERROR_UNKNOWN          - Failed to reject the call. (failed
 *                                     while trying to send the reject response).
 *               RV_OK -          Reject final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to reject
 *            status   - The rejection response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferReject (
                                         IN  RvSipCallLegHandle hCallLeg,
                                         IN  RvUint16           status)
{
    RvStatus   rv;
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferReject - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferReject - Rejecting REFER request for call-leg 0x%p",pCallLeg));

    /*invalid status code*/
    if (status < 300 || status >= 700)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegReferReject - Failed to reject REFER request for Call-leg 0x%p. %d is not a valid error response code",
                  pCallLeg,status));
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_BADPARAM;
    }


    if(((pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE) ||
        (pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED)) &&
        ((pCallLeg->hActiveReferSubs != NULL) &&
        (SipSubsGetCurrState(pCallLeg->hActiveReferSubs) == RVSIP_SUBS_STATE_SUBS_RCVD)))
    {
        rv = CallLegReferReject(pCallLeg,status);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegReferReject - Failed to reject REFER request for call-leg 0x%p: Illegal Action in state %s, refer state %s",
                  pCallLeg, CallLegGetStateName(pCallLeg->eState),
                  CallLegReferGetSubsStateName(pCallLeg)));
        rv = RV_ERROR_ILLEGAL_ACTION;
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferEnd
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Four call-legs are involved in a complete Refer operation:
 *          call-leg A  - the call-leg that sends the REFER request.
 *          call-leg B1 - the call-leg that receives the REFER request.
 *          call-leg B2 - a new call-leg created to contact the third party
 *          call-leg C  - The Refer-To call-leg.
 *
 *             A                       B1 B2               C
 *           |                        | |                |
 *           |   REFER                | |                |
 *           |----------------------->| |                |
 *           |        202 Accepted    | |                |
 *           |<-----------------------| | INVITE         |
 *           |                        | |--------------->|
 *           |                        | |  (whatever)    |
 *           |                        | |<---------------|
 *           |                        |
 *          Once it is known whether B2 succeeded or failed contacting C,
 *          B1 will get a "Notify Ready" notification.
 *          At this point B1 can notify A about the result of the refer by
 *          sending NOTIFY request.
 *          1. Call the RvSipCallLegReferEnd() function to indicate that you
 *          do NOT wish to send the NOTIFY request.
 *          Note: If you call this function with a call-leg in the Idle state,
 *          the call-leg will be terminated.
 *          2. You can also use RvSipCallLegReferEnd() in the
 *          Unauthenticated and Redirected refer states of the call-leg in order
 *          to complete the refer process without re-sending a refer request.
 *          Calling ReferEnd() from states Unauthenticated or Redirected
 *          will return the call-leg to
 *          Idle refer state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_OK - REFER process was completed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to send REFER.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferEnd (
                                         IN  RvSipCallLegHandle   hCallLeg)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferEnd - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RvSipCallLegReferEnd - call-leg 0x%p",pCallLeg));

    CallLegReferEnd(pCallLeg);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferNotify
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Sends a Refer NOTIFY request to the remote party (the Event
 *          header specifies the refer event).
 *          When the application receives the Notify Ready notification
 *          from the RvSipCallLegReferNotifyEv callback it must decide
 *          weather to send a NOTIFY request to the REFER sender or not.
 *          If the application decides to send a NOTIFY request to the
 *          REFER sender, it must call this function. This function is also
 *          used to authenticate a NOTIFY request.
 *          If you call this function you should not call the
 *          call-leg A  - the call-leg that sends the REFER request.
 *          call-leg B1 - the call-leg that receives the REFER request.
 *          call-leg B2 - a new call-leg created to contact the third party
 *          call-leg C  - The Refer-To call-leg.
 *
 *             A                       B1 B2               C
 *           |                        | |                |
 *           |   REFER                | |                |
 *           |----------------------->| |                |
 *           |        202 Accepted    | |                |
 *           |<-----------------------| | INVITE         |
 *           |                        | |--------------->|
 *           |                        | |  (whatever)    |
 *           |                        | |<---------------|
 *           |            NOTIFY      | |                |
 *           |<-----------------------| |                |
 *           |   200 OK               | |                |
 *           |----------------------->| |                |
 *           |                        |
 *           |                        |
 *          Once it is known whether B2 succeeded or failed contacting C,
 *          B1 will get a "Notify Ready" notification.
 *          By calling the RvSipCallLegReferNotify() B1 can send a NOTIFY
 *          message to A letting it know the REFER result.
 *          The NOTIFY message will include the "Event=refer" header.
 *          The body of the NOTIFY message will contain a SIP Response Status-Line
 *          with a status code given as a parameter to this function.
 *          If the application wishes to avoid sending the NOTIFY request is
 *          should call RvSipCallLegReferEnd() function instead.
 *          The RvSipCallLegReferNotify() function is also used to
 *          authenticate a previously sent NOTIFY request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the call-leg is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Call-leg failed to create a new
 *                                   transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - NOTIFY message was sent successfully(if needed).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to send REFER.
 *          status   - The status code that will be used to create a status
 *                     line for the NOTIFY request message body.
 *          cSeqStep - The Cseq step of the REFER transaction that this
 *                     NOTIFY relate to. This value will be set to the cseq
 *                     parameter of the Event header of the NOTIFY request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferNotify (
                                         IN  RvSipCallLegHandle  hCallLeg,
                                         IN  RvInt16             status,
                                         IN  RvInt32             cSeqStep)
{
    RvStatus               rv;
    CallLeg                 *pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = CallLegLockAPI(pCallLeg);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferNotify - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
      "RvSipCallLegReferNotify - call-leg 0x%p: Sending NOTIFY with status=%d, cseq=%d",pCallLeg,status, cSeqStep));

    if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

#if defined(UPDATED_BY_SPIRENT)
    if ( status >= 700 )
#else
    if (200 > status || 700 <= status)
#endif
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RvSipCallLegReferNotify - Failed for call-leg 0x%p: Illegal status code to notify with",pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* send NOTIFY. The call-leg will terminate in error cases in the Idle state */
    rv = CallLegReferNotifyRequest(pCallLeg, status, cSeqStep);

    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipCallLegReferGetCurrentState
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!! you should use the subscription refer API.
 *          In order to use it for code that written for earlier stack version,
 *          you must set the configuration parameter 'disableRefer3515Behavior' to true.
 *          Returns the Refer sub-state inside the call leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call leg the user wishes to get its Refer state.
 * Output:  peReferState - The Refer state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReferGetCurrentState(
                               IN  RvSipCallLegHandle      hCallLeg,
                               OUT RvSipCallLegReferState *peReferState)
{
    RvStatus  rv = RV_OK;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pCallLeg->pMgr->bDisableRefer3515Behavior == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferGetCurrentState - call-leg 0x%p - illegal action. (bDisableRefer3515Behavior = false)",
              pCallLeg));
        CallLegUnLockAPI(pCallLeg);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReferGetCurrentState - Gets Refer state from call-leg 0x%p",pCallLeg));
    if(pCallLeg->hActiveReferSubs == NULL)
    {
        *peReferState = RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE;
    }
    else
    {
        *peReferState = CallLegReferGetCurrentReferState(hCallLeg);
    }
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;
}
#endif /* #ifdef RV_SIP_SUBS_ON */


/***************************************************************************
 * RvSipCallLegReplacesDisconnect
 * ------------------------------------------------------------------------
 * General: This function is obsolete. In order to disconnect a replaces call
 *          use RvSipCallLegDisconnect.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  -  The handle to the call-leg is invalid.
 *               RV_ERROR_UNKNOWN        - Failed to send message (BYE or final
 *                                   rejection response).
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_OK        - BYE message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call leg the user wishes to disconnect.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesDisconnect(IN RvSipCallLegHandle hCallLeg)
{
    RvStatus  rv;

    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if(pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = CallLegLockAPI(pCallLeg); /*try to lock the call-leg*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "RvSipCallLegReplacesDisconnect - Disconnecting call-leg 0x%p",pCallLeg));

    rv = CallLegSessionDisconnect(pCallLeg);
    CallLegUnLockAPI(pCallLeg); /*unlock the call leg*/
    return rv;

}

/***************************************************************************
 * RvSipCallLegReplacesGetMatchedCall
 * ------------------------------------------------------------------------
 * General: This function is obsolete. use RvSipCallLegReplacesGetMatchedCallExt
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg               - Handle to the call leg that received the INVITE with
 *                                   the Replaces header.
 * Output:  pbFoundNonInviteDialog - RV_TRUE if we found matched dialog which was not established
 *                                   with INVITE. For example, Subscription. In this case
 *                                   hMatchedCallLeg will be NULL and the application should
 *                                   return 401/481/501 to the INVITE with the Replaces.
 *          hMatchedCallLeg        - Handle to the call leg matched to the Replaces header.
 *                                   If there is no such call leg, this handle will be NULL,
 *                                   and 481 response will be sent to the original call leg
 *                                   (hCallLeg)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegReplacesGetMatchedCall(
                                  IN  RvSipCallLegHandle         hCallLeg,
                                  OUT RvBool                   *pbFoundNonInviteDialog,
                                  OUT RvSipCallLegHandle        *hMatchedCallLeg)
{
    RvStatus  rv;
    RvSipCallLegReplacesReason eReason;

    if(NULL == pbFoundNonInviteDialog)
    {
        return RV_ERROR_NULLPTR;
    }
    *pbFoundNonInviteDialog = RV_FALSE;

    rv = RvSipCallLegReplacesGetMatchedCallExt(hCallLeg, &eReason, hMatchedCallLeg);
    if (rv == RV_OK)
    {
        if(eReason == RVSIP_CALL_LEG_REPLACES_REASON_FOUND_NON_INVITE_DIALOG)
        {
            *pbFoundNonInviteDialog = RV_TRUE;
        }
    }
    return rv;
}


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipCallLegRouteListEmpty
 * ------------------------------------------------------------------------
 * General: Return is Route list of call leg empty or not 
 *
 * Return Value: RvInt32
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg               - Handle to the call leg.
 * Return value: RV_TRUE when route list is empty
 *               RV_FALSE when route list is not empty
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipCallLegRouteListEmpty(IN  RvSipCallLegHandle         hCallLeg)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    if (pCallLeg && pCallLeg->hRouteList) {
        RLIST_ITEM_HANDLE  ElementPtr;
        RLIST_GetTail(NULL, pCallLeg->hRouteList, &ElementPtr);
        if (ElementPtr) {
            return RV_FALSE;
        }
    }
    return RV_TRUE;
}

/***************************************************************************
 * RvSipCallLegRouteListReset
 * ------------------------------------------------------------------------
 * General: Reset tail entry of route list of call leg with new route 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg               - Handle to the call leg.
 *          nexthop                - New route
 * Return value: RV_TRUE when route list is empty
 *               RV_FALSE when route list is not empty
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCallLegRouteListReset(IN  RvSipCallLegHandle      hCallLeg,
                                                     IN  RvChar                  *nexthop)
{
    CallLeg                   *pCallLeg = (CallLeg*)hCallLeg;
    if(NULL == pCallLeg) {
        return RV_ERROR_NULLPTR;
    }
    return CallLegRouteListReset(pCallLeg, nexthop);
}
#endif
/* SPIRENT_END */
#endif /* #ifndef RV_SIP_PRIMITIVES */




