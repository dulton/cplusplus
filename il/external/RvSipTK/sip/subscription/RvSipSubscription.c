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
 *                         <RvSipSubscription.c>
 *
 * The Subscription functions of the RADVISION SIP stack enable you to create and
 * manage subscription and notification objects. Subscription functions may be used
 * for implementing event-notification feature.
 *
 * Subscription API functions are grouped as follows:
 *
 * The Subscription Manager API
 * -----------------------------
 * The subscription manager is in charge of all the subscriptions. It is used
 * to set the event handlers of the subscription module and to create
 * new subscriptions.
 *
 * The Subscription API
 * --------------------
 * A subscription represents a SIP subscription as defined in draft rfc 3265.
 * Using the subscription API, the user can initiate subscriptions, react
 * to incoming subscriptions, refresh subscriptions and ask for terminating a subscription.
 * Functions to set and access the subscription fields are also available in the API.
 * A subscription is a state-full object and has a set of states associated with it.
 *
 * The Notification API
 * --------------------
 * A notification represents a SIP notification as defined in draft rfc 3265.
 * Using the notification API, the user can create notifications, and react
 * to incoming notifications.
 * Functions to set and access the notification fields are also available in the API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                  June 2002
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_SUBS_ON
#ifndef RV_SIP_PRIMITIVES

#include "RvSipSubscription.h"
#include "SubsObject.h"
#include "SubsNotify.h"
#include "SubsRefer.h"
#include "SubsHighAvailability.h"
#include "RvSipCallLeg.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "RvSipPartyHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "_SipAddress.h"
#include "_SipCommonCore.h"
#include "SubsCallbacks.h"
#include "_SipSubs.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvBool IsSubsDialogEstablished(Subscription* pSubs);

static RvBool IsDialogEstablished(RvSipCallLegHandle hCallLeg,
                                  RvLogSource* pLogSource);

static RvBool IsLegalExpiresValue(RvInt32 expires);

static RvBool SubsBlockTermination(IN Subscription* pSubs);

/*-----------------------------------------------------------------------*/
/*                SUBSCRIPTION MANAGER  API                              */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for all subscription events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR        - Bad pointer to the event handler structure.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr        - Handle to the subscription manager.
 *            pEvHandlers - Pointer to structure containing application event
 *                          handler pointers.
 *            structSize  - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrSetEvHandlers(
                                   IN  RvSipSubsMgrHandle   hMgr,
                                   IN  RvSipSubsEvHandlers *pEvHandlers,
                                   IN  RvInt32              structSize)
{
    SubsMgr    *pMgr = (SubsMgr*)hMgr;
    RvUint32   minStructSize = RvMin((RvUint32)structSize,sizeof(RvSipSubsEvHandlers));

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
              "RvSipSubsMgrSetEvHandlers - Setting event handlers to subscription mgr"));

    memset(&(pMgr->subsEvHandlers),0,sizeof(RvSipSubsEvHandlers));
    memcpy(&(pMgr->subsEvHandlers),pEvHandlers,minStructSize);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/

    return RV_OK;
}


/***************************************************************************
 * RvSipSubsMgrCreateSubscription
 * ------------------------------------------------------------------------
 * General: Creates a new subscription and exchange handles with the
 *          application. The new subscription assumes the Idle state.
 *          You may supply a call-leg handle to create the subscription inside
 *          an already established call-leg. Otherwise, set NULL in the call-leg
 *          handle parameter.(established call-leg means it has a to-tag -->
 *          INVITE and 180 or 2xx response were already sent/received)
 *          To establish a new active subscription
 *          1. Create a new subscription with this function.
 *          2. Init subscription parameters.
 *          3. Call the Subscribe() function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR        -  The pointer to the subscription handle
 *                                          is invalid.
 *               RV_ERROR_OUTOFRESOURCES -  Subscription list is full,subscription was not
 *                                          created.
 *               RV_OK                   -  Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubsMgr - Handle to the subscription manager
 *          hCallLeg - Handle to a call-leg that own the subscription.
 *                     if NULL, the subscription will create a new dialog,
 *                     and will not be related to any call-leg.
 *          hAppSubs - Application handle to the newly created subscription.
 * Output:  phSubs   - RADVISION SIP stack handle to the subscription.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrCreateSubscription(
                                   IN  RvSipSubsMgrHandle  hSubsMgr,
                                   IN  RvSipCallLegHandle  hCallLeg,
                                   IN  RvSipAppSubsHandle  hAppSubs,
                                   OUT RvSipSubsHandle    *phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                   )
{
    RvStatus      rv;
    SubsMgr       *pMgr = (SubsMgr*)hSubsMgr;



    if(phSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phSubs = NULL;

    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hCallLeg != NULL)
    {
        RvSipCallLegState      eDialogState;

        rv = RvSipCallLegGetCurrentState(hCallLeg, &eDialogState);
        if(eDialogState == RVSIP_CALL_LEG_STATE_IDLE ||
           RV_FALSE == IsDialogEstablished(hCallLeg, pMgr->pLogSrc))
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "RvSipSubsMgrCreateSubscription - Illegal Action on non-established dialog 0x%p",
                hCallLeg));
            return RV_ERROR_ILLEGAL_ACTION;
        }
        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        cctContext=RvSipCallLegGetCct(hCallLeg);
        URI_cfg=RvSipCallLegGetURIFlag(hCallLeg);
#endif
        //SPIRENT_END
    }

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipSubsMgrCreateSubscription - Creating a new subscription. hCallLeg = 0x%p", hCallLeg));

    /*create a new subscription. This will lock the subscription manager*/
    rv = SubsMgrSubscriptionCreate(hSubsMgr, hCallLeg, hAppSubs, RVSIP_SUBS_TYPE_SUBSCRIBER, RV_FALSE, phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,cctContext,URI_cfg
#endif
//SPIRENT_END
      );
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipSubsMgrCreateSubscription - Failed, SubsMgr failed to create a subscription"));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipSubsMgrCreateOutOfBandSubscription
 * ------------------------------------------------------------------------
 * General: Creates a new out of band subscription and exchange handles with the
 *          application.
 *          out of band subscription, is a subscription created with no SUBSCRIBE
 *          request. The new subscription assumes the Active state.
 *          You may supply a call-leg handle to create the subscription inside
 *          an already established call-leg. Else - set NULL in the call-leg
 *          handle parameter. (established dialog means it has a to-tag -->
 *          INVITE and 180 or 2xx response were already sent/received)
 *          To establish a new out of band subscription
 *          1. Create a new subscription with this function.
 *          2. Init subscription parameters.
 *
 *          NOTE - if you don't supply call-leg handle to this created out-of-band
 *          subscription, on initialization stage, you will have to set the dialog
 *          call-id parameter first, and only then you can call to RvSipSubsInit().
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR        -  The pointer to the subscription handle
 *                                          is invalid.
 *               RV_ERROR_OUTOFRESOURCES -  Subscription list is full,subscription
 *                                          was not created.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr  - Handle to the subscription manager
 *            hCallLeg  - Handle to a call-leg that own the subscription.
 *                        if NULL, the subscription will create a new dialog,
 *                        and will not be related to any call-leg.
 *            hAppSubs  - Application handle to the newly created subscription.
 *            eSubsType - Subscriber or Notifier.
 * Output:    phSubs    - RADVISION SIP stack handle to the subscription.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrCreateOutOfBandSubscription(
                                       IN  RvSipSubsMgrHandle    hSubsMgr,
                                       IN  RvSipCallLegHandle    hCallLeg,
                                       IN  RvSipAppSubsHandle    hAppSubs,
                                       IN  RvSipSubscriptionType eSubsType,
                                       OUT RvSipSubsHandle      *phSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, IN int cctContext,
  IN int URI_cfg
#endif
//SPIRENT_END
                                       )

{
    RvStatus      rv;
    SubsMgr       *pMgr = (SubsMgr*)hSubsMgr;

    if(phSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phSubs = NULL;

    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hCallLeg != NULL)
    {
        RvSipCallLegState      eDialogState;

        rv = RvSipCallLegGetCurrentState(hCallLeg, &eDialogState);
        if(eDialogState == RVSIP_CALL_LEG_STATE_IDLE ||
            RV_FALSE == IsDialogEstablished(hCallLeg, pMgr->pLogSrc))
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "RvSipSubsMgrCreateOutOfBandSubscription - Illegal Action on non-established dialog 0x%p",
                hCallLeg));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipSubsMgrCreateOutOfBandSubscription - Creating a new subscription"));

    /*create a new subscription. This will lock the subscription manager*/
    rv = SubsMgrSubscriptionCreate(hSubsMgr,hCallLeg, hAppSubs, eSubsType, RV_TRUE, phSubs
 //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,cctContext,URI_cfg
#endif
//SPIRENT_END
      );
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "RvSipSubsMgrCreateOutOfBandSubscription - Failed, SubsMgr failed to create a subscription"));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipSubsMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this subscription
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr - Handle to the subscription manager
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                       handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrGetStackInstance(
                                   IN  RvSipSubsMgrHandle  hSubsMgr,
                                   OUT void              **phStackInstance)
{
    SubsMgr    *pMgr = (SubsMgr*)hSubsMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }

    *phStackInstance = NULL;

    if(pMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr); /*lock the manager*/

    *phStackInstance = pMgr->hStack;

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr); /*unlock the manager*/

    return RV_OK;
}

/***************************************************************************
 * RvSipSubsMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application subs manger object.
 *          You set this handle in the stack using the
 *          RvSipSubsMgrSetAppMgrHandle() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubsMgr    - Handle to the stack subs manager..
 * Output:    pAppSubsMgr - The application subs manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrGetAppMgrHandle(
                                   IN RvSipSubsMgrHandle hSubsMgr,
                                   OUT void**            pAppSubsMgr)
{
    SubsMgr  *pSubsMgr;
    if(hSubsMgr == NULL || pAppSubsMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *pAppSubsMgr = NULL;

    pSubsMgr = (SubsMgr *)hSubsMgr;

    RvMutexLock(&pSubsMgr->hMutex,pSubsMgr->pLogMgr); /*lock the manager*/

    *pAppSubsMgr = pSubsMgr->pAppSubsMgr;

    RvMutexUnlock(&pSubsMgr->hMutex,pSubsMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own subs manager handle.
 *          You can use the RvSipSubsMgrSetAppMgrHandle function to
 *          save this handle in the stack subs manager object.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hSubsMgr    - Handle to the stack subs manager.
 *           pAppSubsMgr - The application subs manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsMgrSetAppMgrHandle(
                                   IN RvSipSubsMgrHandle hSubsMgr,
                                   IN void*              pAppSubsMgr)
{
    SubsMgr  *pSubsMgr;
    if(pAppSubsMgr == NULL || hSubsMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pSubsMgr = (SubsMgr *)hSubsMgr;

    RvMutexLock(&pSubsMgr->hMutex,pSubsMgr->pLogMgr); /*lock the manager*/

    pSubsMgr->pAppSubsMgr = pAppSubsMgr;

    RvMutexUnlock(&pSubsMgr->hMutex,pSubsMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                 SUBSCRIPTION API                                      */
/*-----------------------------------------------------------------------*/

#ifdef SUPPORT_EV_HANDLER_PER_OBJ
/***************************************************************************
 * RvSipSubsSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for the give subscription events.
 *			Setting Ev handler for certain subscription will replace the general
 *			EvHandle that is found in the subscription manager.
 * Return Value: RV_OK for Success otherwise error.
 *				 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs       - Handle to the subscription.
 *            pEvHandlers - Pointer to structure containing application event
 *                          handler pointers.
 *            structSize  - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetEvHandlers(
                                   IN  RvSipSubsHandle		hSubs,
                                   IN  RvSipSubsEvHandlers *pEvHandlers,
                                   IN  RvInt32              structSize)
{
    Subscription    *pSubs = (Subscription*)hSubs;
    RvUint32   minStructSize = RvMin((RvUint32)structSize,sizeof(RvSipSubsEvHandlers));
	RvStatus		rv;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pEvHandlers == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

	/*lock the subscription*/
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsSetEvHandlers - Setting event handlers to subscription 0x%p", pSubs));

	memset(&(pSubs->objEvHandlers),0,sizeof(RvSipSubsEvHandlers));
    memcpy(&(pSubs->objEvHandlers),pEvHandlers,minStructSize);

	pSubs->subsEvHandlers = &pSubs->objEvHandlers;

    SubsUnLockAPI(pSubs); /* unlock the subscription */

    return RV_OK;
}
#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/


/***************************************************************************
 * RvSipSubsInit
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with mandatory parameters: To and From headers
 *          of the dialog, expiresVal of the subscription, and Event header of
 *          the subscription.
 *          If the subscription was created inside of a call-leg you should not
 *          set the To and From headers.
 *          This function initialized the subscription, but do not send any request.
 *          You should call RvSipSubsSubscribe() in order to send a Subscribe
 *          request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the subscription the user wishes to initialize.
 *          hFrom - Handle to a party header, contains the from header information.
 *          hTo   - Handle to a party header, contains the from header information.
 *          expiresVal - Expires value to be set in first SUBSCRIBE request - in seconds.
 *                  value of this argument must be smaller than 0xFFFFFFF/1000.
 *                 (This is not necessarily the final expires value. Notifier may
 *                  decide of a shorter expires value in the 2xx response.)
 *                  If equals to UNDEFINED, no expires header will be set in the
 *                  SUBSCRIBE request.
 *          hEvent - Handle to an Event header. this header identify the subscription.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsInit(
                                         IN  RvSipSubsHandle        hSubs,
                                         IN  RvSipPartyHeaderHandle hFrom,
                                         IN  RvSipPartyHeaderHandle hTo,
                                         IN  RvInt32                expiresVal,
                                         IN  RvSipEventHeaderHandle hEvent)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
         pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsInit - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(IsLegalExpiresValue(expiresVal) == RV_FALSE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsInit - subs 0x%p - too big expires value 0x%x",
            pSubscription,expiresVal));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInit - subscription 0x%p, init subscription",
              hSubs));

    rv = SubsSetInitialParams(pSubscription, hFrom, hTo, expiresVal, hEvent);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInit - Failed for subscription 0x%p",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInit - Subscription 0x%p was initialized",hSubs));


    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsInitStr
 * ------------------------------------------------------------------------
 * General: Init a subscription with mandatory parameters, given in a string format:
 *          To and From header value strings strings of the dialog, expiresVal of
 *          the subscription, and Event header string value of the subscription.
 *          If the subscription was created inside of a call-leg you should not
 *          set the To and From values.
 *          This function initialized the subscription, but do not send any request.
 *          You should call RvSipSubsSubscribe() in order to send a Subscribe
 *          request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the subscription the user wishes to initialize.
 *          strFrom - String of the From header value. for Example: "A<sip:176.56.23.4:4444>"
 *          strTo -  String of the To header value. for Example: "B<sip:176.56.23.4;transport=TCP>"
 *          expiresVal - Expires value to be set in first SUBSCRIBE request - in seconds.
 *                  value of this argument must be smaller than 0xFFFFFFF/1000.
 *                 (This is not necessarily the final expires value. Notifier may
 *                  decide of a shorter expires value in the 2xx response.)
 *                  If equals to UNDEFINED, no expires header will be set in the
 *                  SUBSCRIBE request.
 *          strEvent - String to the Event header value. this header identify the
 *                  subscription. for Example: "eventPackageX.eventTemplateY"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsInitStr(
                                         IN  RvSipSubsHandle hSubs,
                                         IN  RvChar*         strFrom,
                                         IN  RvChar*         strTo,
                                         IN  RvInt32         expiresVal,
                                         IN  RvChar*         strEvent)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;
    RvSipPartyHeaderHandle  hFrom, hTo;
    RvSipEventHeaderHandle  hEvent        = NULL;
    HRPOOL                  hpool;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
         pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsInitStr - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(IsLegalExpiresValue(expiresVal) == RV_FALSE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsInitStr - subs 0x%p - too big expires value 0x%x",
            pSubscription,expiresVal));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInitStr - subscription 0x%p, init subscription",
              hSubs));

    /* getting temp page */
    hpool = pSubscription->pMgr->hGeneralPool;

    /* from */
    if(strFrom != NULL)
    {
        rv = RvSipCallLegGetNewPartyHeaderHandle(pSubscription->hCallLeg, &hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to construct From header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipPartyHeaderParseValue(hFrom, RV_FALSE, strFrom);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to parse From header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hFrom = NULL;
    }

    /* to */
    if(strTo != NULL)
    {
        rv = RvSipCallLegGetNewPartyHeaderHandle(pSubscription->hCallLeg, &hTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to construct To header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipPartyHeaderParseValue(hTo, RV_TRUE, strTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to parse To header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hTo = NULL;
    }

     /* event */
    if(strEvent != NULL)
    {
        rv = RvSipEventHeaderConstruct(pSubscription->pMgr->hMsgMgr, hpool,
                                        pSubscription->hPage, &hEvent);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to construct Event header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipEventHeaderParseValue(hEvent, strEvent);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsInitStr - subscription 0x%p, failed to parse Event header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    rv = SubsSetInitialParams(pSubscription, hFrom, hTo, expiresVal, hEvent);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInitStr - subscription 0x%p, failed to init subscription",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsInitStr - Subscription 0x%p was initialized",hSubs));
    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsDialogInit
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with dialog parameters: To, From, remote
 *          contact and local contact headers.
 *          This function is relevant only for subscription that was created
 *          outside of a call-leg.
 *          This function initializes the dialog of the subscription, but do not
 *          initialize the subscription parameters.
 *          You should call RvSipSubsInit() or RvSipSubsReferInit() in order
 *          to initialize the subscription parameters.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action,
 *                                         or subscription inside call-leg.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM       - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR        - Bad pointer was given by the application.
 *               RV_OK                   - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the subscription the user wishes to initialize.
 *          hFrom - Handle to a party header, contains the from header information.
 *          hTo -   Handle to a party header, contains the from header information.
 *          hLocalContactAddr - Handle to address, contains the local contact
 *                  Address information.
 *          hRemoteContactAddr - Handle to address, contains the remote
 *                  contact address information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDialogInit(
                                         IN  RvSipSubsHandle        hSubs,
                                         IN  RvSipPartyHeaderHandle hFrom,
                                         IN  RvSipPartyHeaderHandle hTo,
                                         IN  RvSipAddressHandle     hLocalContactAddr,
                                         IN  RvSipAddressHandle     hRemoteContactAddr)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
         pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsDialogInit - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(RV_FALSE == SipCallLegIsHiddenCallLeg(pSubscription->hCallLeg))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsDialogInit - subs 0x%p - illegal action for subscription inside a call-leg",
            pSubscription));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }


    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInit - subscription 0x%p, initializing subscription dialog...",
              hSubs));

    rv = SubsSetInitialDialogParams(pSubscription, hFrom, hTo, hLocalContactAddr, hRemoteContactAddr);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInit - Failed for subscription 0x%p",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInit - Subscription 0x%p - dialog was initialized successfully",hSubs));


    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsDialogInitStr
 * ------------------------------------------------------------------------
 * General: Initiate a subscription with dialog parameters in string format:
 *          To, From, remote contact and local contact headers.
 *          This function is relevant only for subscription that was created
 *          outside of a call-leg.
 *          This function initializes the dialog if the subscription, but do not
 *          initialize the subscription parameters.
 *          You should call RvSipSubsInitStr() or RvSipSubsReferInitStr() in order
 *          to initialize the subscription parameters.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action,
 *                                         or subscription inside call-leg.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM       - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR        - Bad pointer was given by the application.
 *               RV_OK                   - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs   - Handle to the subscription the user wishes to initialize.
 *          strFrom - String of the From header value.
 *                    for Example: "A<sip:176.56.23.4:4444>"
 *          strTo   - String of the To header value.
 *                    for Example: "B<sip:176.56.23.4;transport=TCP>"
 *          strLocalRemote - String of the local contact address value,
 *                    for Example: "sip:176.56.23.4:4444"
 *          strRemoteContact - String of the remote contact address value.
 *                    for Example: "sip:176.56.23.4:4444"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDialogInitStr(
                                         IN  RvSipSubsHandle hSubs,
                                         IN  RvChar*         strFrom,
                                         IN  RvChar*         strTo,
                                         IN  RvChar*         strLocalContactAddr,
                                         IN  RvChar*         strRemoteContactAddr)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;
    RvSipPartyHeaderHandle  hFrom, hTo;
    RvSipAddressHandle      hLocalContactAddr, hRemoteContactAddr;
    RvSipAddressType        eAddrType;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
         pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsDialogInitStr - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(RV_FALSE == SipCallLegIsHiddenCallLeg(pSubscription->hCallLeg))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsDialogInitStr - subs 0x%p - illegal action for subscription inside a call-leg",
            pSubscription));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }


    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInitStr - subscription 0x%p, initializing subscription dialog...",
              hSubs));

    /* from */
    if(strFrom != NULL)
    {
        rv = RvSipCallLegGetNewPartyHeaderHandle(pSubscription->hCallLeg, &hFrom);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to construct From header in dialog",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipPartyHeaderParseValue(hFrom, RV_FALSE, strFrom);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to parse From header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hFrom = NULL;
    }

    /* to */
    if(strTo != NULL)
    {
        rv = RvSipCallLegGetNewPartyHeaderHandle(pSubscription->hCallLeg, &hTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to construct To header in dialog",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipPartyHeaderParseValue(hTo, RV_TRUE, strTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to parse To header ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hTo = NULL;
    }

    /* local contact */
    if(strLocalContactAddr != NULL)
    {
        eAddrType = SipAddrGetAddressTypeFromString(pSubscription->pMgr->hMsgMgr, strLocalContactAddr);
        rv = RvSipCallLegGetNewAddressHandle(pSubscription->hCallLeg, eAddrType, &hLocalContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to construct Local Contact address ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipAddrParse(hLocalContactAddr, strLocalContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to parse Local Contact address ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hLocalContactAddr = NULL;
    }

    /* remote contact */
    if(strRemoteContactAddr != NULL)
    {
        eAddrType = SipAddrGetAddressTypeFromString(pSubscription->pMgr->hMsgMgr, strRemoteContactAddr);
        rv = RvSipCallLegGetNewAddressHandle(pSubscription->hCallLeg, eAddrType, &hRemoteContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to construct Remote Contact address ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
        rv = RvSipAddrParse(hRemoteContactAddr, strRemoteContactAddr);
        if(rv != RV_OK)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsDialogInitStr - subscription 0x%p, failed to parse Remote Contact address ",hSubs));
            SubsUnLockAPI(pSubscription);
            return rv;
        }
    }
    else
    {
        hRemoteContactAddr = NULL;
    }


    rv = SubsSetInitialDialogParams(pSubscription, hFrom, hTo, hLocalContactAddr, hRemoteContactAddr);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInitStr - subscription 0x%p, failed to init subscription dialog",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsDialogInitStr - Subscription 0x%p - dialog was initialized successfully",hSubs));
    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsSubscribe
 * ------------------------------------------------------------------------
 * General: establish a subscription by sending a SUBSCRIBE request.
 *          This function may be called only after the subscription was initialized
 *          with To, From, Event and Expires parameters.
 *          (If this is a subscription of an already established call-leg, To and
 *          From parameters are taken from the call-leg).
 *          Calling this function causes a SUBSCRIBE to be sent out and the
 *          subscription state machine to progress to the Subs_Sent state.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Subscription failed to create a new transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                            message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the subscription the user wishes to subscribe.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSubscribe(IN  RvSipSubsHandle   hSubs)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    RvSipCallLegState       eCallLegState;
    RvBool                  bHidden;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubs->bOutOfBand == RV_TRUE)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsSubscribe - Cannot subscribe an out of band subscription 0x%p",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsSubscribe - subscribing for subscription 0x%p",pSubs));

    rv = RvSipCallLegGetCurrentState(pSubs->hCallLeg, &eCallLegState);

    /* subscribe is allowed on subscription state idle, unauth or redirected. */
    if(pSubs->eState != RVSIP_SUBS_STATE_IDLE &&
       pSubs->eState != RVSIP_SUBS_STATE_UNAUTHENTICATED &&
       pSubs->eState != RVSIP_SUBS_STATE_REDIRECTED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsSubscribe - Failed for subscription 0x%p: Illegal Action in subs state %s ",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* subscribe is allowed on call-leg idle state, only if this is a hidden call-leg,
       otherwise it will be allowed only in call-leg which is already an early dialog. */
    bHidden = SipCallLegIsHiddenCallLeg(pSubs->hCallLeg);
    if ((bHidden==RV_TRUE && eCallLegState != RVSIP_CALL_LEG_STATE_IDLE) ||
        (bHidden==RV_FALSE && RV_FALSE == IsSubsDialogEstablished(pSubs)))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsSubscribe - Failed for subscription 0x%p: Illegal Action in subs dialog state %s",
                  pSubs, SipCallLegGetStateName(eCallLegState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsSubscribe(pSubs, RV_FALSE);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsRefresh
 * ------------------------------------------------------------------------
 * General: Refresh a subscription by sending a SUBSCRIBE request in the
 *          subscription. This method may be called only on SUBS_ACTIVE state.
 *          Calling to refresh causes a SUBSCRIBE request to be sent out, and the
 *          subscription state machine to progress to the Subs_Refresing state.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Illegal subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Subscription failed to create a new transaction.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                            message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs             - Handle to the subscription the user wishes to refresh.
 *            refreshExpiresVal - Expires value to be set in the refresh request
 *                                in seconds.
 *                                value of this argument must be smaller than 0xFFFFFFF/1000.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRefresh(
                                         IN  RvSipSubsHandle   hSubs,
                                         IN  RvInt32           refreshExpiresVal)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubs->bOutOfBand == RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRefresh - Cannot refresh an out of band subscription 0x%p",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pSubs->eSubsType != RVSIP_SUBS_TYPE_SUBSCRIBER)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRefresh - subscription 0x%p - Illegal action for a Notifier",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* Change in the following 'if' condition - Enable Subscription */
    /* refresh with no Expires header */ 
    /* refreshExpiresVal <= 0 || */ 
    if(IsLegalExpiresValue(refreshExpiresVal) != RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRefresh - subscription 0x%p - Illegal action with expires parameter %d",
            pSubs, refreshExpiresVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }


    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRefresh - refreshing for subscription 0x%p",pSubs));

    /* refresh can be called only on subs state active */
   if(pSubs->eState != RVSIP_SUBS_STATE_ACTIVE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsRefresh - Failed for subscription 0x%p: Illegal Action in subs state %s",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsRefresh(pSubs,refreshExpiresVal);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}

/***************************************************************************
 * RvSipSubsUnsubscribe
 * ------------------------------------------------------------------------
 * General: Asks Notifier to terminate the subscription by sending a SUBSCRIBE
 *          request with 'Expires:0' header in the subscription.
 *          This method may be called on states SUBS_ACTIVE, SUBS_PENDING and
 *          SUBS_2XX_RCVD.
 *          The function progress the subscription state machine to the
 *          Subs_Unsubscribing state.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Subscription failed to create a new transaction.
 *               RV_ERROR_UNKNOWN        - An error occurred while trying to send the
 *                                         message (Couldn't send a message to the given
 *                                         Request-Uri).
 *               RV_OK                   - Subscribe message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the subscription the user wishes to unsubscribe.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsUnsubscribe(IN  RvSipSubsHandle   hSubs)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;


    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubs->bOutOfBand == RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsUnsubscribe - Illegal action for an out of band subscription 0x%p",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pSubs->eSubsType != RVSIP_SUBS_TYPE_SUBSCRIBER)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsUnsubscribe - subscription 0x%p - Illegal action for a Notifier",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsUnsubscribe - Send an UNSUBSCRIBE for subscription 0x%p",pSubs));

    /*unsubscribe can be called only on states active, pending and 2xxRcvd. */
    if(pSubs->eState != RVSIP_SUBS_STATE_ACTIVE &&
       pSubs->eState != RVSIP_SUBS_STATE_PENDING &&
       pSubs->eState != RVSIP_SUBS_STATE_2XX_RCVD)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsUnsubscribe - Failed for subscription 0x%p: Illegal Action in state %s",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsRefresh(pSubs,0);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsRespondAccept
 * ------------------------------------------------------------------------
 * General: Sends a 200 response on a subscribe request and updates the
 *          subscription state machine.
 *          You may use this function to accept an initial subscribe request,
 *          refresh or unsubscribe requests.
 *          Note that this function does not send a notify request!!!
 *          It is the responsibility of the application to create and send a
 *          notify(active) request, immediately after it accepts the
 *          subscribe request by calling to this function.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN        -  Failed to accept the call. (failed
 *                                          while trying to send the 200 response).
 *               RV_OK                   -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs      - Handle to the subscription the user wishes to accept.
 *          expiresVal - Expires value to be set in the 200 response - in seconds.
 *                       value of this argument must be smaller than 0xFFFFFFF/1000.
 *                       if UNDEFINED, the expires value will be the same as in
 *                       the subscribe request.
 *                       Note that if there was no expires header in the incoming
 *                       SUBSCRIBE request, you must not give an UNDEFINED
 *                       expires value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRespondAccept (
                                        IN  RvSipSubsHandle   hSubs,
                                        IN  RvInt32          expiresVal)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRespondAccept - sending 200 for subscription 0x%p",pSubs));

    /* Accept can be called only on subs state subs_rcvd, refresh_rcvd, unsubscribe_rcvd */

    if((pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_REFRESH_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsRespondAccept - Failed for subscription 0x%p: Illegal Action in subs state %s",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

     if(IsLegalExpiresValue(expiresVal) == RV_FALSE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondAccept - subs 0x%p - too big expires value 0x%x",
            pSubs,expiresVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pSubs->eState == RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD)
    {
        expiresVal = 0;
        if(pSubs->hActiveTransc == NULL)
        {
            /* user already respond to this unsubscribe request, but remain in same
            state until he send notify(terminated) */
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsRespondAccept - Failed for subscription 0x%p: User already respond on the unsubscribe request",
                pSubs));
            SubsUnLockAPI(pSubs);
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    else if(expiresVal == UNDEFINED && pSubs->requestedExpirationVal < 0)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondAccept - Failed for subscription 0x%p: Illegal expiresVal %d (requested expiresVal %d)",
            pSubs, expiresVal, pSubs->requestedExpirationVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_BADPARAM;
    }
    else if((pSubs->requestedExpirationVal < expiresVal &&
            pSubs->requestedExpirationVal > 0) )
            /* if the incoming expires val is 0, we can set a bigger expires val */
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondAccept - Failed for subscription 0x%p: Illegal expiresVal %d (bigger than requested expiresVal %d)",
            pSubs, expiresVal, pSubs->requestedExpirationVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_BADPARAM;
    }

    rv = SubsAccept(pSubs,200, expiresVal, RV_FALSE);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsRespondPending
 * ------------------------------------------------------------------------
 * General: Sends a 202 response for an initial subscribe request, and update
 *          the subscription state machine to Subs_Pending state.
 *          You may use this function only for an initial subscribe request.
 *          Note that this function does not send a notify request!!!
 *          It is user responsibility to create and send a notify(pending) request,
 *          immediately after he respondPending to the subscribe request!!!
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN        -  Failed to accept the call. (failed
 *                                          while trying to send the 200 response).
 *               RV_OK                   -  Accept final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs      - Handle to the subscription.
 *          expiresVal - Expires value to be set in the 202 response - in seconds.
 *                       value of this argument must be smaller than 0xFFFFFFF/1000.
 *                       if UNDEFINED, the expires value will be the same as in
 *                       the subscribe request.
 *                       Note that if there was no expires header in the incoming
 *                       SUBSCRIBE request, you must not give an UNDEFINED
 *                       expires value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRespondPending (
                                        IN  RvSipSubsHandle   hSubs,
                                        IN  RvInt32          expiresVal)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRespondPending - sending 202 for subscription 0x%p",pSubs));

    /* AcceptPending can be called only on subs state subs_rcvd */

    if(pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondPending - Failed for subscription 0x%p: Illegal Action in subs state %s",
            pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

     if(IsLegalExpiresValue(expiresVal) == RV_FALSE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondPending - subs 0x%p - too big expires value 0x%x",
            pSubs,expiresVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pSubs->requestedExpirationVal < expiresVal)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondPending - Failed for subscription 0x%p: Illegal expiresVal %d (bigger than requested expiresVal %d)",
            pSubs, expiresVal, pSubs->requestedExpirationVal));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_BADPARAM;
    }

    rv = SubsAccept(pSubs,202, expiresVal, RV_FALSE);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}

/***************************************************************************
* RvSipSubsRespondProvisional
* ------------------------------------------------------------------------
* General:  Send 1xx on subscribe request (initial subscribe, refresh
*           or unsubscribe).
*           Can be used in the RVSIP_SUBS_STATE_SUBS_RCVD, RVSIP_SUBS_STATE_REFRESH_RCVD
*           or RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD states.
*           Note - According to RFC 3265, sending provisional response on
*           SUBSCRIBE request is not allowed.
*           According to RFC 3515, only 100 response on REFER request is allowed.
* Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
*               RV_ERROR_BADPARAM          - The status code is invalid.
*               RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
*               RV_ERROR_UNKNOWN           - Failed to reject the subscription. (failed
*                                            while trying to send the reject response).
*               RV_OK                      - 1xx was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs      - Handle to the subscription the user wishes to send 1xx for.
*           statusCode - The 1xx value.
*           strReasonPhrase - Reason phrase to be set in the sent message.
*                        (May be NULL, to set a default reason phrase.)
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRespondProvisional(
                                       IN  RvSipSubsHandle  hSubs,
                                       IN  RvUint16         statusCode,
                                       IN  RvChar*          strReasonPhrase)
{
    RvStatus      rv;
    Subscription *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipSubsRespondProvisional(hSubs,statusCode,strReasonPhrase);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondProvisional - Failed for subscription 0x%p",pSubs));
        SubsUnLockAPI(pSubs);
        return rv;
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}

/***************************************************************************
* RvSipSubsRespondReject
* ------------------------------------------------------------------------
* General:  Reject an incoming subscribe request (initial subscribe, refresh
*           or unsubscribe).
*           Can be used in the RVSIP_SUBS_STATE_SUBS_RCVD, RVSIP_SUBS_STATE_REFRESH_RCVD
*           or RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD states.
* Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
*               RV_ERROR_BADPARAM          - The status code is invalid.
*               RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
*               RV_ERROR_UNKNOWN           - Failed to reject the subscription. (failed
*                                            while trying to send the reject response).
*               RV_OK                      - Reject final response was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs      - Handle to the subscription the user wishes to reject
*            statusCode - The rejection response code.
*            strReasonPhrase - Reason phrase to be set in the sent reject message.
*                        (May be NULL, to set a default reason phrase.)
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRespondReject (
                                         IN  RvSipSubsHandle  hSubs,
                                         IN  RvUint16         statusCode,
                                         IN  RvChar*          strReasonPhrase)
{
    RvStatus    rv;
    Subscription *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }



    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* 1xx and 2xx responses are not allowed in a reject function */
    if(statusCode < 300)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondReject - subscription 0x%p: Illegal Action with statusCode %d",
            pSubs,statusCode));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRespondReject - sending %d for subscription 0x%p",statusCode, pSubs));

    /* Reject can be called only on subs state subs_rcvd, refresh_rcvd, unsubscribe_rcvd */

    if((pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_REFRESH_RCVD) &&
        (pSubs->eState != RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondReject - Failed for subscription 0x%p: Illegal Action in subs state %s",
            pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsReject(pSubs,statusCode, strReasonPhrase);
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsAutoRefresh
 * ------------------------------------------------------------------------
 * General: Set the autoRefresh flag of a specific subscription.
 *          When autoRefresh is set to RV_TRUE, an automatic refreshing subscribe
 *          request is sent when needed.
 *          This parameter default value is taken from configuration.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs  - Handle to the subscription.
 *            bAutoRefresh - RV_TRUE to set on auto refresh, RV_FALSE to set it off.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsAutoRefresh (
                                         IN  RvSipSubsHandle  hSubs,
                                         IN  RvBool           bAutoRefresh)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsAutoRefresh - set auto refresh %d on subscription 0x%p",
              bAutoRefresh,pSubs));

    pSubs->autoRefresh = bAutoRefresh;

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsSetAlertTimer
 * ------------------------------------------------------------------------
 * General: Set the alert timer value of a specific subscription.
 *          Alert timer value defines when an alert before expiration of a
 *          subscription, is given.
 *          This parameter default value is taken from configuration.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs  - Handle to the subscription.
 *            alertValue - value in milliseconds.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetAlertTimer(
                                         IN  RvSipSubsHandle   hSubs,
                                         IN  RvInt32          alertValue)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsSetAlertTimer - set alert timer %d on subscription 0x%p",
              alertValue,pSubs));

    pSubs->alertTimeout = alertValue;

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
* RvSipSubsSetNoNotifyTimer
* ------------------------------------------------------------------------
* General: Set the no-notify timer value of a specific subscription.
*          No-Notify timer value defines when the subscription is terminated,
*          if no notify request is received and accepted after 2xx on
*          subscribe request was received.
*          This parameter default value is taken from configuration.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs         - Handle to the subscription.
*            noNotifyValue - Value in milliseconds.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetNoNotifyTimer(
                                          IN  RvSipSubsHandle  hSubs,
                                                  IN  RvInt32          noNotifyValue)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsSetNoNotifyTimer - set no-notify timer %d on subscription 0x%p",
              noNotifyValue,pSubs));

    pSubs->noNotifyTimeout = noNotifyValue;

    SubsUnLockAPI(pSubs); /*unlock the subs*/
    return RV_OK;
}


/***************************************************************************
 * RvSipSubsTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a subscription without sending any messages (unsubscribe or
 *          notify(terminated)). The subscription will assume the Terminated state.
 *          Calling this function will cause an abnormal termination.
 *          All notifications and transactions related to the subscription will be
 *          terminated as well.
 *          If this is subscription was created with no related call-leg,
 *          the subscription dialog will be terminated as well.
 *
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnSubsCreatedEvHandler, pfnNotifyCreatedEvHandler,
 *          pfnSubsCreatedDueToForkingEvHandler, pfnNewConnInUseEvHandler, 
 *          pfnSigCompMsgNotRespondedEvHandler, pfnReferNotifyReadyEvHandler 
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs  - Handle to the subscription the user wishes to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsTerminate (IN  RvSipSubsHandle   hSubs)
{
    RvStatus               rv = RV_OK;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    
    if(RV_TRUE == SubsBlockTermination(pSubs))
    {    
        SubsUnLockAPI(pSubs); /*unlock the subscription*/
        return RV_ERROR_TRY_AGAIN;

    }

    if(pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsTerminate - subscription 0x%p is already in state TERMINATED", pSubs));
    }
    else
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsTerminate - terminating subscription 0x%p", pSubs));

        /* terminate subscription */
        rv = SubsTerminate(pSubs, RVSIP_SUBS_REASON_SUBS_TERMINATED);
    }
    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
* RvSipSubsDetachOwner
* ------------------------------------------------------------------------
 * General: Detach the Subscription object from its owner. After calling this function
 *          the user will stop receiving events for this Subscription object. This function
 *          can be called only when the object is in terminated state.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs  - Handle to the subscription the user wishes to detach from owner.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDetachOwner (IN  RvSipSubsHandle   hSubs)
{
    RvStatus               rv = RV_OK;
    Subscription           *pSubs = (Subscription*)hSubs;


    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubs->eState != RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
          "RvSipSubsDetachOwner - Subscription object 0x%p is in state %s, and not in state terminated - can't detach owner",
          pSubs, SubsGetStateName(pSubs->eState)));

        SubsUnLockAPI(pSubs); /*unlock the subs*/
        return RV_ERROR_ILLEGAL_ACTION;
    }
    else
    {
        pSubs->subsEvHandlers = NULL;
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsDetachOwner - Subscription object 0x%p: was detached from owner",
               pSubs));
        rv = RV_OK;
    }

    SubsUnLockAPI(pSubs);
    return rv;
}

/***************************************************************************
 * RvSipSubsUpdateSubscriptionTimer
 * ------------------------------------------------------------------------
 * General: Sets the subscription timer. when RvSipSubsSubscriptionExpiredEv
 *          callback function is called, the subscription timer is no longer
 *          active. calling this function will activate this timer again.
 *          (on next expiration, RvSipSubsSubscriptionExpiredEv will be called again).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs           - Handle to the subscription object.
 *          expirationValue - The expiration value, in milli-seconds.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsUpdateSubscriptionTimer(
                                     IN  RvSipSubsHandle    hSubs,
                                     IN  RvInt32           expirationValue)
{
    RvStatus      rv;
    Subscription  *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(expirationValue <= 0)
    {

    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsObjectSetAlertTimer(pSubscription, expirationValue);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc ,(pSubscription->pMgr->pLogSrc ,
            "RvSipSubsUpdateSubscriptionTimer - Subscription 0x%p - Failed to set subscription timer",
            pSubscription));
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------
       S U B S C R I P T I O N - S R V   A U T H    F U N C T I O N S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsAuthBegin - Server authentication
 * ------------------------------------------------------------------------
 * General: Begin the server authentication process. challenge an incoming
 *          SUBSCRIBE, REFER or NOTIFY request.
 *          If the request is SUBSCRIBE or REFER, hSubs handle should be given,
 *          and hNotify should be NULL.
 *          If the request is NOTIFY, hNotify should be given, and
 *          hSubs should be NULL.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs   - Handle to the subscription the user wishes to challenge.
 *          hNotify - Handle to the notification user wishes to challenge.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipSubsAuthBegin(
                                           IN  RvSipSubsHandle    hSubs,
                                           IN  RvSipNotifyHandle  hNotify)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;
    RvSipTranscHandle       hTransc;

    if(pSubs == NULL && pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs == NULL)
    {
        /*lint -e413*/
        pSubs = pNotify->pSubs;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsAuthBegin - Challenge an incoming request for subscription 0x%p notification 0x%p",
              hSubs, hNotify));

    if(hNotify == NULL)
    {
        hTransc = pSubs->hActiveTransc;
    }
    else
    {
        hTransc = pNotify->hTransc;
    }
    if(hTransc == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsAuthBegin - Failed. transaction is NULL for subscription 0x%p", pSubs));
        SubsUnLockAPI(pSubs); /*unlock the subscription*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RvSipTransactionAuthBegin(hTransc);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc ,(pSubs->pMgr->pLogSrc ,
            "RvSipSubsAuthBegin - Failed for Subscription 0x%p",pSubs));
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}


/***************************************************************************
 * RvSipSubsAuthProceed - Server authentication
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed in authentication procedure.
 *          actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password.
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct. (will cause RvSipSubsAuthCompletedEv() to be called,
 *            with status success)..
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop that searchs for authorization headers.
 *            (will cause RvSipSubsAuthCompletedEv() to be called, with status failure).
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - Order to skip the given header, and continue the authentication
 *            procedure with next header (if exists).
 *            (will cause RvSipSubsAuthCredentialsFoundEv() to be called, or
 *            RvSipSubsAuthCompletedEv(failure) if there are no more
 *            authorization headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs -          Handle to the subscription user challenges.
 *          hNotify -        Handle to the notification user challenges.
 *          action -         With which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsAuthProceed(
                                   IN  RvSipSubsHandle                hSubs,
                                   IN  RvSipNotifyHandle              hNotify,
                                   IN  RvSipTransactionAuthAction     action,
                                   IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                   IN  RvChar                        *password)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;
    RvSipTranscHandle       hTransc;

    if(pSubs == NULL && pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs == NULL)
    {
        /*lint -e413*/
        pSubs = pNotify->pSubs;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsAuthProceed - continue with action %d, for subscription 0x%p notification 0x%p",
              action, hSubs, hNotify));

    if(hNotify == NULL)
    {
        hTransc = pSubs->hActiveTransc;
    }
    else
    {
        hTransc = pNotify->hTransc;
    }
    if(hTransc == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsAuthProceed - Failed. transaction is NULL for subscription 0x%p", pSubs));
        SubsUnLockAPI(pSubs); /*unlock the subscription*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SipTransactionAuthProceed(hTransc, action, hAuthorization,
        password,(void*)pSubs, RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION,
        pSubs->tripleLock);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsAuthProceed pSubs 0x%p: failed to authenticate transaction 0x%p (rv=%d:%s)",
            pSubs, hTransc, rv, SipCommonStatus2Str(rv)));
        SubsUnLockAPI(pSubs); /*unlock the subscription*/
        return rv;
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return RV_OK;
#else
    RV_UNUSED_ARG(hSubs);
    RV_UNUSED_ARG(hNotify);
    RV_UNUSED_ARG(action);
    RV_UNUSED_ARG(hAuthorization);
    RV_UNUSED_ARG(password);
    return RV_ERROR_NOTSUPPORTED;
#endif /* #ifdef RV_SIP_AUTH_ON */
}


/***************************************************************************
 * RvSipSubsRespondUnauthenticated - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response on a SUBSCRIBE, REFER or NOTIFY request.
 *          Add a copy of the given header to the response message.
 *          The given header should contain the authentication parameters.
 *          (should be of Authentication header type or Other header type)
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs -          Handle to the subscription..
 *          hNotify -        Handle to the notification.
 *          responseCode -   401 or 407.
 *          strReasonPhrase - May be NULL, for default reason phrase.
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipSubsRespondUnauthenticated(
                                   IN  RvSipSubsHandle      hSubs,
                                   IN  RvSipNotifyHandle    hNotify,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar*             strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;

    if(pSubs == NULL && pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs == NULL)
    {
        /*lint -e413*/
        pSubs = pNotify->pSubs;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRespondUnauthenticated - about to send %d response, for subscription 0x%p notification 0x%p",
              responseCode, hSubs, hNotify));

    rv = SubsRespondUnauth(pSubs, pNotify,  responseCode, strReasonPhrase, headerType, hHeader,
                            NULL, NULL, NULL, NULL, RV_FALSE, RVSIP_AUTH_ALGORITHM_UNDEFINED,
                            NULL, RVSIP_AUTH_QOP_UNDEFINED, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondUnauthenticated -  subscription 0x%p - Failed to send %d response (rv=%d)",
            pSubs, responseCode,rv));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_UNKNOWN;
    }

    SubsUnLockAPI(pSubs); /*unlock */
    return rv;
}


/***************************************************************************
 * RvSipSubsRespondUnauthenticatedDigest - Server authentication
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response on a SUBSCRIBE, REFER or NOTIFY request.
 *          Build an authentication header containing all given parameters,
 *          and add it to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs -          Handle to the subscription..
 *          hNotify -        Handle to the notification.
 *          responseCode -   401 or 407
 *          strReasonPhrase - May be NULL, for default reason phrase.
 *          strRealm -       mandatory.
 *          strDomain -      Optional string. may be NULL.
 *          strNonce -       Optional string. may be NULL.
 *          strOpaque -      Optional string. may be NULL.
 *          bStale -         TRUE or FALSE
 *          eAlgorithm -     enumeration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
 *                           the algorithm value is taken from the the next argument.
 *          strAlgorithm -   String of algorithm. this parameters will be set only if
 *                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           enumeration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
 *                           will be taken from the next argument.
 *          strQop -         String of qop. this parameter will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipSubsRespondUnauthenticatedDigest(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipNotifyHandle  hNotify,
                                   IN  RvUint16          responseCode,
                                   IN  RvChar           *strReasonPhrase,
                                   IN  RvChar           *strRealm,
                                   IN  RvChar           *strDomain,
                                   IN  RvChar           *strNonce,
                                   IN  RvChar           *strOpaque,
                                   IN  RvBool            bStale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;

    if(pSubs == NULL && pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs == NULL)
    {
        /*lint -e413*/
        pSubs = pNotify->pSubs;
    }

    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRespondUnauthenticatedDigest - about to send %d response, for subscription 0x%p notification 0x%p",
              responseCode, hSubs, hNotify));

    rv = SubsRespondUnauth(pSubs, pNotify,  responseCode, strReasonPhrase,
                           RVSIP_HEADERTYPE_UNDEFINED, NULL,
                           strRealm, strDomain, strNonce, strOpaque, bStale,
                           eAlgorithm, strAlgorithm, eQop, strQop);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRespondUnauthenticatedDigest -  subscription 0x%p - Failed to send %d response (rv=%d)",
            pSubs, responseCode,rv));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_UNKNOWN;
    }

    SubsUnLockAPI(pSubs); /* unlock */
    return rv;
}

#endif /* #ifdef RV_SIP_AUTH_ON */


/*-----------------------------------------------------------------------
       S U B S C R I P T I O N - N O T I F Y  F U N C T I O N S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsCreateNotify
 * ------------------------------------------------------------------------
 * General: Creates a new notification object, related to a given subscription
 *          and exchange handles with the application notification object.
 *          For setting notify state information in the NOTIFY request, before
 *          sending, use RvSipNotifyGetOutboundMsg function.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_NULLPTR        - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs      - Handle to the subscription, relates to the new notification.
 *            hAppNotify - Handle to the application notification object.
 * Output:    phNotify   - Handle to the new created notification object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsCreateNotify(IN  RvSipSubsHandle      hSubs,
                                                 IN  RvSipAppNotifyHandle hAppNotify,
                                                 OUT RvSipNotifyHandle    *phNotify)
{
    RvStatus      rv;
    Subscription  *pSubs = (Subscription*)hSubs;

    if(phNotify == NULL )
    {
        return RV_ERROR_NULLPTR;
    }

    *phNotify = NULL;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs->eSubsType != RVSIP_SUBS_TYPE_NOTIFIER)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsCreateNotify - subscription 0x%p - Illegal action for Subscriber",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pSubs->eState == RVSIP_SUBS_STATE_SUBS_RCVD ||
        pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsCreateNotify - subscription 0x%p - Illegal state %s",
            pSubs, SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsCreateNotify - subscription 0x%p - Creating a new notification", pSubs));

    rv = SubsNotifyCreate(pSubs, hAppNotify, phNotify);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsCreateNotify - subscription 0x%p - Failed to create a notification (status %d)",
            pSubs, rv));
        SubsUnLockAPI(pSubs);
        return rv;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsCreateNotify - subscription 0x%p - New notification 0x%p was created successfully. hAppNotify = 0x%p",
              pSubs, *phNotify, hAppNotify));

    SubsUnLockAPI(pSubs);
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsNotifyCreate
 * ------------------------------------------------------------------------
 * General: This function is DEPRECATED!!!
 *          you should use RvSipSubsCreateNotify instead.
 *          note that the retrieved message handle is for one notify request,
 *          after sending this notify request you must not use this message handle
 *          again.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_NULLPTR        - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs       - Handle to the subscription, relates to the new notification.
 *          hAppNotify  - Handle to the application notification object.
 * Output:  phNotify    - Handle to the new created notification object.
 *          phNotifyMsg - Handle to the notify message that was constructed inside
 *                        the notification object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsNotifyCreate(
                                   IN  RvSipSubsHandle      hSubs,
                                   IN  RvSipAppNotifyHandle hAppNotify,
                                   OUT RvSipNotifyHandle    *phNotify,
                                   OUT RvSipMsgHandle       *phNotifyMsg)
{
    RvStatus      rv;
    Subscription  *pSubs = (Subscription*)hSubs;

    if(phNotify == NULL || phNotifyMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phNotify = NULL;
    *phNotifyMsg = NULL;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* create the notify object, using the new API function */
    rv = RvSipSubsCreateNotify(hSubs,hAppNotify,phNotify);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsNotifyCreate - subscription 0x%p - Failed to create a notification (status %d)",
            pSubs, rv));
        SubsUnLockAPI(pSubs);
        return rv;
    }
    /* create the outbound message */
    rv = RvSipNotifyGetOutboundMsg(*phNotify,phNotifyMsg);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsNotifyCreate - subscription 0x%p - Failed to create notification message (status %d)",
            pSubs, rv));
        SubsUnLockAPI(pSubs);
        return rv;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsNotifyCreate - subscription 0x%p - New notification 0x%p was created successfully",
              pSubs, *phNotify));

    SubsUnLockAPI(pSubs);
    return RV_OK;
}

/***************************************************************************
 * RvSipNotifySend
 * ------------------------------------------------------------------------
 * General: Sends the notify message placed in the notification object.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *               RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN        -  Failed to send the notify request.
 *               RV_OK                   -  Sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hNotify    - Handle to the notification object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifySend(IN  RvSipNotifyHandle hNotify)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pNotify->pSubs->eState == RVSIP_SUBS_STATE_SUBS_RCVD ||
        pNotify->pSubs->eState == RVSIP_SUBS_STATE_TERMINATED)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySend - subscription 0x%p - Illegal state %s",
            pNotify->pSubs, SubsGetStateName(pNotify->pSubs->eState)));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pNotify->pSubs->eSubsType == RVSIP_SUBS_TYPE_SUBSCRIBER)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySend - subscription 0x%p - Subscriber. Illegal action",
            pNotify->pSubs));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pNotify->eStatus == RVSIP_SUBS_NOTIFY_STATUS_MSG_SEND_FAILURE)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySend - subscription 0x%p - Illegal action in notification 0x%p state %s (use DNSContinue function instead)",
            pNotify->pSubs, pNotify, SubsNotifyGetStatusName(pNotify->eStatus)));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_IDLE)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySend - subscription 0x%p - Illegal action in notification 0x%p state %s",
            pNotify->pSubs, pNotify, SubsNotifyGetStatusName(pNotify->eStatus)));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifySend - subs 0x%p notification 0x%p - Sending notify request",
              pNotify->pSubs, pNotify));

    rv = SubsNotifySend(pNotify->pSubs, pNotify);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySend - subs 0x%p notification 0x%p - Failed to send notify request (status %d)",
            pNotify->pSubs, pNotify, rv));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}


/***************************************************************************
* RvSipNotifyAccept
* ------------------------------------------------------------------------
* General: Sends a 200 response on a NOTIFY request, and destruct the notification
*          object.
*          The function influence the subscription state machine as follows:
*          state SUBS_Sent + accepting Notify(active or pending) ->
*                                        moves to state SUBS_Notify_Before_2xx_Rcvd.
*          state SUBS_2xx_Rcvd + accepting Notify(pending) -> state SUBS_Pending.
*          state SUBS_2xx_Rcvd + accepting Notify(active) -> state SUBS_Active.
*          state SUBS_Pending + accepting Notify(active) -> state SUBS_Active.
*          all states + accepting Notify(terminated) -> state SUBS_Terminated.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
*               RV_ERROR_UNKNOWN        -  Failed to send the 200 response.
*               RV_OK                   -  Response was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hNotify    - Handle to the notification object.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyAccept(IN  RvSipNotifyHandle hNotify)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyAccept - subs 0x%p notification 0x%p - illegal action on state %s",
            pNotify->pSubs, pNotify, SubsNotifyGetStatusName(pNotify->eStatus)));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifyAccept - subs 0x%p notification 0x%p - Sending 200 on notify",
              pNotify->pSubs, pNotify));

    rv = SubsNotifyRespond(pNotify->pSubs, pNotify, 200);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyAccept - subs 0x%p notification 0x%p - Failed to send 200 on notify(status %d)",
            pNotify->pSubs, pNotify, rv));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}


/***************************************************************************
* RvSipNotifyReject
* ------------------------------------------------------------------------
* General: Sends a non-2xx response on a NOTIFY request, and destruct the
*          notification object.
*          This function does not influence the subscription state machine.
* Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
*               RV_ERROR_UNKNOWN        -  Failed to send the response.
*               RV_OK                   -  Response was sent successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hNotify    - Handle to the notification object.
*           statusCode - Response code.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyReject(IN  RvSipNotifyHandle hNotify,
                                             IN  RvUint16         statusCode)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyReject - subs 0x%p notification 0x%p - illegal action on state %s",
            pNotify->pSubs, pNotify, SubsNotifyGetStatusName(pNotify->eStatus)));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(statusCode < 300)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyReject - subs 0x%p notification 0x%p - illegal status code %d",
            pNotify->pSubs, pNotify, statusCode));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifyReject - subs 0x%p notification 0x%p - Sending %d on notify",
              pNotify->pSubs, pNotify, statusCode));

    rv = SubsNotifyRespond(pNotify->pSubs, pNotify, statusCode);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyReject - subs 0x%p notification 0x%p - Failed to send %d on notify(status %d)",
            pNotify->pSubs, pNotify, statusCode, rv));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}


/***************************************************************************
 * RvSipNotifyTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a notification object without sending any messages.
 *          The notification object will inform of the Terminated status.
 *          Calling this function will cause an abnormal termination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hNotify    - Handle to the notification object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyTerminate (IN  RvSipNotifyHandle hNotify)
{
    RvStatus      rv = RV_OK;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pNotify->eStatus == RVSIP_SUBS_NOTIFY_STATUS_TERMINATED)
    {
        RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyTerminate - subs 0x%p - notify 0x%p status is already terminated",
            pNotify->pSubs, pNotify));
        rv = RV_OK;
    }
    else
    {
        RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifyTerminate - subs 0x%p notification 0x%p - Terminating notification",
            pNotify->pSubs, pNotify));

        rv = SubsNotifyTerminate(pNotify, RVSIP_SUBS_NOTIFY_REASON_APP_COMMAND);
    }


    SubsUnLockAPI(pNotify->pSubs);
    return rv;
}

/***************************************************************************
* RvSipNotifyDetachOwner
* ------------------------------------------------------------------------
 * General: Detach the Notify object from its owner. After calling this function
 *          the user will stop receiving events for this Notify object. This function
 *          can be called only when the object is in terminated state.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hNotify    - Handle to the notification object.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyDetachOwner (IN  RvSipNotifyHandle hNotify)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pNotify->eStatus != RVSIP_SUBS_NOTIFY_STATUS_TERMINATED)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
          "RvSipNotifyDetachOwner - Notify object 0x%p is in state %s, and not in state terminated - can't detach owner",
          pNotify, SubsNotifyGetStatusName(pNotify->eStatus)));

        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    else
    {
        pNotify->pfnNotifyEvHandler = NULL;
        RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifyDetachOwner - Notify object 0x%p: was detached from owner",
               pNotify));
    }

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}

/***************************************************************************
 * RvSipNotifyGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message (request or response) that is going to be sent
 *          by the notification. You may get the message only in states where
 *          notification may send a message.
 *          You can call this function before you call a control API functions
 *          that sends a message (e.g. RvSipNotifySend() or RvSipNotifyAccept()).
 *          Note: The message you receive from this function is not complete.
 *          In some cases it might even be empty.
 *          You should use this function to add headers and body to the message
 *          before it is sent.
 *          To view the complete message use event message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hNotify -  The notification handle.
 * Output:  phMsg   -  pointer to the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyGetOutboundMsg(
                                     IN  RvSipNotifyHandle  hNotify,
                                     OUT RvSipMsgHandle        *phMsg)
{
    RvStatus       rv;
    Notification   *pNotify = (Notification*)hNotify;

    if(hNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phMsg = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsNotifyGetOutboundMsg(pNotify, phMsg);
    if (RV_OK != rv)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                   "RvSipNotifyGetOutboundMsg - notify 0x%p error in getting outbound message", pNotify));
        SubsUnLockAPI(pNotify->pSubs); /*unlock the subscription*/
        return rv;
    }

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifyGetOutboundMsg - notify 0x%p returned handle= 0x%p",
              pNotify,*phMsg));

    SubsUnLockAPI(pNotify->pSubs); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipNotifyGetSubsState
* ------------------------------------------------------------------------
* General: Returns the value of the Subscription-State header, that was
*          set by application to the NOTIFY message of this notify object.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hNotify    - Handle to the notification object.
* Output:   peSubsState - The SubsState: active, pending, terminated or other.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyGetSubsState (
                                    IN  RvSipNotifyHandle hNotify,
                                    OUT RvSipSubscriptionSubstate* peSubsState)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(peSubsState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peSubsState = pNotify->eSubstate;

    RvLogInfo(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
          "RvSipNotifyGetSubsState - Notify object 0x%p: return subsState %d",
           pNotify, *peSubsState));

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}

/***************************************************************************
 * RvSipNotifySetSubscriptionStateParams
 * ------------------------------------------------------------------------
 * General: Create and set the Subscription-State header in the notification object.
 *          This header will be set to the outbound NOTIFY request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *               RV_ERROR_UNKNOWN        -  General Failure.
 *               RV_OK                   -  Set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hNotify    - Handle to the notification object.
 *          eSubsState - SubsState to set in Subscription-State header.
 *          eReason    - reason to set in Subscription-State header
 *                       (RVSIP_SUBSCRIPTION_REASON_UNDEFINED for no reason).
 *          expiresParamVal - expires parameter value to set in Subscription-State
 *                       header (may be UNDEFINED)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifySetSubscriptionStateParams(
                                    IN  RvSipNotifyHandle         hNotify,
                                    IN  RvSipSubscriptionSubstate eSubsState,
                                    IN  RvSipSubscriptionReason   eReason,
                                    IN  RvInt32                   expiresParamVal)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

     if(IsLegalExpiresValue(expiresParamVal) == RV_FALSE)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySetSubscriptionStateParams - subs 0x%p - too big expires value 0x%x",
            pNotify->pSubs,expiresParamVal));
        SubsUnLockAPI(pNotify->pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsNotifySetSubscriptionStateParams(pNotify, eSubsState, eReason, expiresParamVal);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySetSubscriptionStateParams - subs 0x%p notification 0x%p - Failed to set notify info (status %d)",
            pNotify->pSubs, pNotify, rv));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
        "RvSipNotifySetSubscriptionStateParams - notify 0x%p, state %d, reason %d, expires %d, stack instance is 0x%p",
        hNotify, eSubsState, eReason, expiresParamVal, pNotify->pSubs->pMgr->hStack));

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}

/***************************************************************************
 * RvSipNotifyGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this notification
 *          object belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify         - The notification handle.
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                              handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifyGetStackInstance(
                                     IN  RvSipNotifyHandle   hNotify,
                                     OUT void                **phStackInstance)
{
    RvStatus               rv;
    Notification            *pNotify = (Notification*)hNotify;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }

    *phStackInstance = NULL;

    if(hNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "RvSipNotifyGetStackInstance - notify 0x%p, stack instance is 0x%p",
              hNotify,pNotify->pSubs->pMgr->hStack));

    *phStackInstance = pNotify->pSubs->pMgr->hStack;

    SubsUnLockAPI(pNotify->pSubs); /* unlock the subscription */
    return rv;
}

/*-----------------------------------------------------------------------
        D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsDNSGiveUp
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state of a subscription
 *          or notification object.
 *          Calling to this function, stops the sending of the request message,
 *          and returns the state machine to its previous state.
 *          You can use this function for a SUBSCRIBE, REFER or NOTIFY request.
 *          For SUBSCRIBE and REFER messages, use it if state was changed to
 *          MSG_SEND_FAILURE state. in this case set hNotify to be NULL.
 *          Use this function for a NOTIFY request, if you were informed of the
 *          MSG_SEND_FAILURE status in RvSipSubsNotifyEv() event. In this case,
 *          you should supply the notify handle.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handles to the objects are invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN        - General failure.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs    - Handle to the subscription that sent the request.
 *          hNotify  - Handle to the notify object, in case of notify request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDNSGiveUp (
                                              IN  RvSipSubsHandle   hSubs,
                                              IN  RvSipNotifyHandle hNotify)
{
    Subscription *pSubs = (Subscription*)hSubs;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus      rv    = RV_OK;
#endif

    if(NULL == pSubs)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (RV_TRUE == SipCallLegIsImsProtected(pSubs->hCallLeg))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSGiveUp - subscription 0x%p: illegal action for IMS Secure Subscription",
            pSubs));
	    SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    if(hNotify == NULL)
    {
        if(pSubs->eState != RVSIP_SUBS_STATE_MSG_SEND_FAILURE)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsDNSGiveUp - subscription 0x%p: Illegal action on state %s (not msg-send-failure state)" ,
                pSubs, SubsGetStateName(pSubs->eState)));
            SubsUnLockAPI(pSubs);
            return RV_ERROR_ILLEGAL_ACTION;
        }
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "RvSipSubsDNSGiveUp - subscription 0x%p: give up DNS" ,
        pSubs));

        rv = SubsDNSGiveUp(pSubs);
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSGiveUp - subscription 0x%p: give up DNS for notify 0x%p" ,
            pSubs, hNotify));

        rv = SubsNotifyDNSGiveUp(pSubs, (Notification*)hNotify);
    }
    SubsUnLockAPI(pSubs); /*unlock the subscription */
    return rv;
#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "RvSipSubsDNSGiveUp - subscription 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pSubs));
    RV_UNUSED_ARG(hNotify);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

}

/***************************************************************************
 * RvSipSubsDNSContinue
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state of a subscription
 *          or notification object.
 *          The function creates a new cloned transaction, with a new DNS
 *          list, and terminates the old transaction. Application may get
 *          the outbound message at this stage, and set information to it
 *          (the same information that was set to the message of the original
 *          transaction). When message is ready, application should send
 *          the new message to the next ip address with RvSipSubsDNSReSendRequest()
 *          You can use this function for a SUBSCRIBE, REFER or NOTIFY request.
 *          For SUBSCRIBE and REFER messages, use it if state was changed to
 *          MSG_SEND_FAILURE state. in this case set hNotify to be NULL.
 *          Use this function for a NOTIFY request, if you were informed of the
 *          MSG_SEND_FAILURE status in RvSipSubsNotifyEv() event.
 *          In this case, you should supply the notify handle.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handles to the objects are invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN        - General failure.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs    - Handle to the subscription that sent the request.
 *          hNotify  - Handle to the notify object, in case of notify request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDNSContinue (IN  RvSipSubsHandle   hSubs,
                                                IN  RvSipNotifyHandle hNotify)
{
    Subscription *pSubs = (Subscription*)hSubs;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus      rv    = RV_OK;
#endif

    if(NULL == pSubs)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (RV_TRUE == SipCallLegIsImsProtected(pSubs->hCallLeg))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSContinue - subscription 0x%p: illegal action for IMS Secure Subscription",
            pSubs));
	    SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    if(hNotify == NULL)
    {
        if(pSubs->eState != RVSIP_SUBS_STATE_MSG_SEND_FAILURE)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsDNSContinue - subscription 0x%p: Illegal action on state %s (not msg-send-failure state)" ,
                pSubs, SubsGetStateName(pSubs->eState)));
            SubsUnLockAPI(pSubs);
            return RV_ERROR_ILLEGAL_ACTION;
        }
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSContinue - subscription 0x%p: continue DNS" ,
            pSubs));
        rv = SubsDNSContinue(pSubs);
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSContinue - subscription 0x%p: continue DNS for notify 0x%p" ,
            pSubs, hNotify));

        rv = SubsNotifyDNSContinue(pSubs, (Notification*)hNotify);
    }
    SubsUnLockAPI(pSubs); /*unlock the subscription */
    return rv;
#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "RvSipSubsDNSContinue - subscription 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pSubs));
    RV_UNUSED_ARG(hNotify);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
}

/***************************************************************************
 * RvSipSubsDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: This function is for use at MSG_SEND_FAILURE state of a subscription
 *          or notification object.
 *          Calling to this function, re-send the cloned transaction to the next ip
 *          address, and change state of the state machine back to the sending
 *          state.
 *          Note - Before calling this function, you should set all information
 *          that you set to the message at the first sending, (such as notify body)
 *          (You may use RvSipSubsGetOutboundMsg() API function, and then
 *          set your parameters to this message).
 *
 *          You can use this function for a SUBSCRIBE, REFER or NOTIFY request.
 *          For SUBSCRIBE and REFER messages, use it if state was changed to
 *          MSG_SEND_FAILURE state. in this case set hNotify to be NULL.
 *          Use this function for a NOTIFY request, if you were informed of the
 *          MSG_SEND_FAILURE status in RvSipSubsNotifyEv() event. In this case,
 *          you should supply the notify handle.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE - The handles to the objects are invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN        - General failure.
 *               RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs    - Handle to the subscription that sent the request.
 *          hNotify  - Handle to the notify object, in case of notify request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDNSReSendRequest(IN  RvSipSubsHandle   hSubs,
                                                    IN  RvSipNotifyHandle hNotify)
{
    Subscription *pSubs = (Subscription*)hSubs;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus      rv    = RV_OK;
#endif

    if(NULL == pSubs)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* No DNS activity should be done for objects, that uses IMS protected
       channels for message exchange */
#ifdef RV_SIP_IMS_ON
    if (RV_TRUE == SipCallLegIsImsProtected(pSubs->hCallLeg))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSReSendRequest - subscription 0x%p: illegal action for IMS Secure Subscription",
            pSubs));
	    SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    if(hNotify == NULL)
    {
        if(pSubs->eState != RVSIP_SUBS_STATE_MSG_SEND_FAILURE)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsDNSReSendRequest - subscription 0x%p: Illegal action on state %s (not msg-send-failure state)" ,
                pSubs, SubsGetStateName(pSubs->eState)));
            SubsUnLockAPI(pSubs);
            return RV_ERROR_ILLEGAL_ACTION;
        }
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSReSendRequest - subscription 0x%p: continue DNS" ,
            pSubs));
        rv = SubsDNSReSendRequest(pSubs);
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSReSendRequest - subscription 0x%p: continue DNS for notify 0x%p" ,
            pSubs, hNotify));

        rv = SubsNotifyDNSReSendRequest(pSubs, (Notification*)hNotify);
    }
    SubsUnLockAPI(pSubs); /*unlock the subscription */
    return rv;
#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "RvSipSubsDNSReSendRequest - subscription 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pSubs));
    RV_UNUSED_ARG(hNotify);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
}

/***************************************************************************
* RvSipSubsDNSGetList
* ------------------------------------------------------------------------
* General: Returns DNS list object from the transaction object.
*          You can use this function for a SUBSCRIBE, REFER or NOTIFY request.
*          For a SUBSCRIBE or REFER message, use it if state was changed to
*          MSG_SEND_FAILURE state. in this case set hNotify to be NULL.
*          Use this function for a NOTIFY request, if you were informed of the
*          MSG_SEND_FAILURE status in RvSipSubsNotifyEv() event.
*          In this case, you should supply the notify handle.
* Return Value: RV_OK or RV_ERROR_BADPARAM
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs     - Handle to the subscription that sent the request.
*          hNotify   - Handle to the notify object, in case of notify request.
* Output   phDnsList - DNS list handle
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsDNSGetList(
                                   IN  RvSipSubsHandle              hSubs,
                                   IN  RvSipNotifyHandle            hNotify,
                                   OUT RvSipTransportDNSListHandle *phDnsList)
{
    Subscription *pSubs = (Subscription*)hSubs;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvStatus      rv    = RV_OK;
#endif

    if(NULL == pSubs || NULL == phDnsList)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phDnsList = NULL;

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hNotify == NULL)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSGetList - subscription 0x%p: getting DNS list" ,
            pSubs));

		if(pSubs->eState != RVSIP_SUBS_STATE_MSG_SEND_FAILURE)
		{
			RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
				"RvSipSubsDNSGetList: Subscription 0x%p - Illigal action in state %s",
				pSubs, SubsGetStateName(pSubs->eState)));
			SubsUnLockAPI(pSubs); /*unlock the subscription */
            return RV_ERROR_ILLEGAL_ACTION;
		}


        if(pSubs->hActiveTransc == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
						"RvSipSubsDNSGetList - subscription 0x%p: illegal action for pSubs->hActiveTransc = NULL" ,
						pSubs));
			SubsUnLockAPI(pSubs); /*unlock the subscription */
            return RV_ERROR_ILLEGAL_ACTION;
        }

        rv = RvSipTransactionDNSGetList(pSubs->hActiveTransc, phDnsList);
        if (RV_ERROR_BADPARAM == rv) /* no active transaction. action not allowed at this stage */
        {
            rv = RV_ERROR_ILLEGAL_ACTION;
        }
    }
    else
    {
        Notification* pNotify = (Notification*)hNotify;

        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsDNSGetList - subscription 0x%p: getting DNS list for notify 0x%p" ,
            pSubs, pNotify));

        if(pNotify->hTransc == NULL)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsDNSGetList - subscription 0x%p, notify 0x%p: illegal action for notify transaction = 0x%p" ,
                pSubs, pNotify, pNotify->hTransc));
            SubsUnLockAPI(pSubs); /*unlock the subscription */
            return RV_ERROR_ILLEGAL_ACTION;
        }
        rv = RvSipTransactionDNSGetList(pNotify->hTransc, phDnsList);
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription */
    return rv;
#else /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
        "RvSipSubsDNSGetList - subscription 0x%p: Illegal action with no RV_DNS_ENHANCED_FEATURES_SUPPORT compilation" ,
        pSubs));
    RV_UNUSED_ARG(hNotify);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
}

/*-----------------------------------------------------------------------
       R E F E R   S U B S C R I P T I O N  A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsReferInit
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with it's refer parameters:
 *          refer-to and referred-by headers. (refer-to is mandatory).
 *          If the subscription was not created inside of a call-leg you must
 *          also initiate its dialog parameters first, by calling RvSipSubsDialogInit()
 *          before calling this function.
 *          This function initialized the refer subscription, but do not send
 *          any request. Use RvSipSubsRefer() for sending a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES    - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM          - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR           - Bad pointer was given by the application.
 *               RV_OK                      - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs       - Handle to the subscription the user wishes to initialize.
 *          hReferTo    - Handle to a refer-to header to set in the subscription.
 *          hReferredBy - Handle to a referred-by header to set in the subscription.
 *                        (If NULL - no referred-by header will be set to the message).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsReferInit(
                                 IN  RvSipSubsHandle             hSubs,
                                 IN  RvSipReferToHeaderHandle    hReferTo,
                                 IN  RvSipReferredByHeaderHandle hReferredBy)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubscription->pMgr->bDisableRefer3515Behavior == RV_TRUE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsReferInit - subs 0x%p - illegal action. (bDisableRefer3515Behavior = true)",
              pSubscription));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsReferInit(hSubs, hReferTo, hReferredBy, NULL, NULL, NULL);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsReferInit - Failed for subscription 0x%p",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsReferInitStr
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with it's refer parameters in a string format:
 *          refer-to and referred-by headers. (refer-to is mandatory), and optional
 *          replaces header, to be set in the refer-to header.
 *          If the subscription was not created inside of a call-leg you must
 *          also initiate its dialog parameters first, by calling RvSipSubsDialogInit()
 *          before calling this function.
 *          This function initialized the refer subscription, but do not send
 *          any request. Use RvSipSubsRefer() for sending a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs         - Handle to the subscription the user wishes to initialize.
 *          strReferTo    - String of the Refer-To header value.
 *                          for Example: "A<sip:176.56.23.4:4444;method=INVITE>"
 *          strReferredBy - String of the Referred-By header value.
 *                          for Example: "<sip:176.56.23.4:4444>"
 *                          (If NULL - no referred-by header will be set to the message).
 *          strReplaces  - The Replaces header to be set in the Refer-To header of
 *                         the REFER request. The Replaces header string doesn't
 *                           contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                           the subscription object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsReferInitStr(
                                 IN  RvSipSubsHandle hSubs,
                                 IN  RvChar         *strReferTo,
                                 IN  RvChar         *strReferredBy,
                                 IN  RvChar         *strReplaces)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubscription->pMgr->bDisableRefer3515Behavior == RV_TRUE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsReferInitStr - subs 0x%p - illegal action. (bDisableRefer3515Behavior = true)",
              pSubscription));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsReferInit(hSubs, NULL, NULL, strReferTo, strReferredBy, strReplaces);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsReferInitStr - Failed for subscription 0x%p",hSubs));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsRefer
 * ------------------------------------------------------------------------
 * General: Establishes a refer subscription by sending a REFER request.
 *          This function may be called only after the subscription was initialized
 *          with To, From, Refer-To and optional Referred-By parameters.
 *          (If this is a subscription of an already established call-leg, To and
 *          From parameters are taken from the call-leg).
 *          Calling this function causes a REFER to be sent out, and the
 *          subscription state machine to progress to the Subs_Sent state.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - Subscription failed to create a new transaction.
 *               RV_ERROR_UNKNOWN        - An error occurred while trying to send the
 *                                         message (Couldn't send a message to the given
 *                                         Request-Uri).
 *               RV_OK                   - Refer message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs - Handle to the subscription the user wishes to refer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRefer(IN  RvSipSubsHandle   hSubs)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    RvSipCallLegState       eCallLegState;
    RvBool                  bHidden;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(RV_TRUE == pSubs->bOutOfBand)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRefer - Cannot refer on an out of band subscription 0x%p",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(NULL == pSubs->pReferInfo)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsRefer - subscription 0x%p - No refer info in subscription",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRefer - refering for subscription 0x%p...",pSubs));

    rv = RvSipCallLegGetCurrentState(pSubs->hCallLeg, &eCallLegState);

    /* refer is allowed on subscription state idle, unauth or redirected. */
    if(pSubs->eState != RVSIP_SUBS_STATE_IDLE &&
       pSubs->eState != RVSIP_SUBS_STATE_UNAUTHENTICATED &&
       pSubs->eState != RVSIP_SUBS_STATE_REDIRECTED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsRefer - Failed for subscription 0x%p: Illegal Action in subs state %s ",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* subscribe is allowed on call-leg idle state, only if this is a hidden call-leg,
       otherwise it will be allowed only in call-leg which is already an early dialog. */
    bHidden = SipCallLegIsHiddenCallLeg(pSubs->hCallLeg);
    if ((bHidden==RV_TRUE && eCallLegState != RVSIP_CALL_LEG_STATE_IDLE) ||
        (bHidden==RV_FALSE && RV_FALSE == IsSubsDialogEstablished(pSubs)))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsRefer - Failed for subscription 0x%p: Illegal Action in subs dialog state %s",
                  pSubs, SipCallLegGetStateName(eCallLegState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SubsSubscribe(pSubs, RV_TRUE);

    SubsUnLockAPI(pSubs); /*unlock the subscription */
    return rv;
}


/***************************************************************************
 * RvSipSubsReferAccept
 * ------------------------------------------------------------------------
 * General: Sends a 202 response on a REFER request and updates the
 *          subscription state machine.
 *
 *          Accepting of the REFER request, creates a new object associated
 *          with the refer subscription.
 *          The type of the new object is according to the method parameter
 *          in the URL of the Refer-To header.
 *          if method=SUBSCRIBE or method=REFER, a new subscription object is created.
 *          if method=INVITE or no method parameter exists in the Refer-To URL,
 *          a new call-leg object is created.
 *          for all other methods - a new transaction object is created.
 *          The new created object contains the following parameters:
 *          Call-Id: The Call-Id of the REFER request,
 *          To:      The Refer-To header of the REFER request,
 *          From:    The local contact of the refer subscription.
 *          Referred-By: The Referred-By header of the RFEER request.
 *          Event:   In case of method = SUBSCRIBE.
 *          Refer-To: in case of method=REFER.
 *
 *          This function associates the refer subscription only with dialog-creating
 *          objects (call-leg or subscription). This means that the callback
 *          RvSipSubsReferNotifyReadyEv will be called only if the new object is
 *          call-leg or subscription. In case of a new transaction object, application
 *          has to save the association by itself.
 *
 *          application should check the method parameter, in order to know which
 *          application object to allocate (and to give in the hAppNewObj argument),
 *          and which handle type to supply this function as an output parameter.
 *
 *          To complete the acceptens of the REFER request the application must
 *          use the newly created object and establish it by calling to
 *          RvSipCallLegConnect in case of a new call-leg, RvSipSubsSubscribe in
 *          case of a new subscription, RvSipSubsRefer in case of a new refer
 *          subscription.
 *
 *          Note that this function does not send a notify request!!!
 *          It is the responsibility of the application to create and send an initializing
 *          notify(active) request. (application can do it from RvSipSubsReferNotifyReadyEv
 *          callback).
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the subscription is invalid.
 *               RV_ERROR_ILLEGAL_ACTION -  Invalid subscription state for this action.
 *               RV_ERROR_UNKNOWN        -  Failed to accept the request. (failed
 *                                          while trying to send the 202 response).
 *               RV_OK                   -  Accept. final response was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs      - Handle to the subscription the user wishes to accept.
 *          hAppNewObj - Handle to the new application object. (RvSipAppCallLegHandle
 *                       in case of method=INVITE or no method parameter,
 *                       RvSipAppSubsHandle in case of method=SUBSCRIBE/REFER
 *                       RvSipAppTranscHandle in all other cases).
 *          eForceObjType - In case application wants to force the stack to create
 *                       a specific object type (e.g. create a transaction and
 *                       not call-leg for method=INVITE in the refer-to url) it
 *                       can set the type in this argument. otherwise it should be
 *                       RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED.
 * Output:  peCreatedObjType - The type of object that was created by stack.
 *                       if application gave eForceObjType parameter, it will be
 *                       equal to eForceObjType. Otherwise it will be according
 *                       to the method parameter in the refer-to url.
 *          phNewObj   - Handle to the new object that was created by SIP Stack.
 *                       (RvSipCallLegHandle in case of method=INVITE or no
 *                       method parameter,
 *                       RvSipSubsHandle in case of method=SUBSCRIBE/REFER,
 *                       RvSipTranscHandle in all other cases).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsReferAccept (
                                IN    RvSipSubsHandle             hSubs,
                                IN    void*                       hAppNewObj,
                                IN    RvSipCommonStackObjectType  eForceObjType,
                                OUT   RvSipCommonStackObjectType *peCreatedObjType,
                                OUT   void*                      *phNewObj)
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;

    if(NULL == peCreatedObjType || phNewObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phNewObj = NULL;
    *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;

    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(NULL == pSubs->pReferInfo)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsReferAccept - Failed for subscription 0x%p: no refer info in subscription",pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsReferAccept - sending 202 for refer ... 0x%p",pSubs));

    /* Accept can be called only on subs state subs_rcvd, refresh_rcvd, unsubscribe_rcvd */

    if(pSubs->eState != RVSIP_SUBS_STATE_SUBS_RCVD)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsReferAccept - Failed for subscription 0x%p: Illegal Action in subs state %s",
                  pSubs,SubsGetStateName(pSubs->eState)));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* sends 202 response */
    rv = SubsAccept(pSubs,202, UNDEFINED, RV_TRUE);
    if(RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsReferAccept - subscription 0x%p: Failed to send 202 response (rv=%d)",
                  pSubs,rv));
        SubsUnLockAPI(pSubs);
        return rv;
    }

    /* SubsAccept() calls state changed callbacks, from which the application
       can terminate the subscription. Check this case. */
    if (RVSIP_SUBS_STATE_TERMINATED == pSubs->eState)
    {
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsReferAccept - subscription 0x%p is in TERMINATED state after 202 sending",
                   pSubs));
        SubsUnLockAPI(pSubs);
        return RV_ERROR_DESTRUCTED;
    }

    /*create and associate the new object */
    rv = SubsReferAttachNewReferResultObject(pSubs, hAppNewObj, eForceObjType, peCreatedObjType, phNewObj);
    if(RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RvSipSubsReferAccept - subscription 0x%p: Failed to associate new object (rv=%d)",
                  pSubs,rv));
        SubsUnLockAPI(pSubs);
        return rv;
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}

/***************************************************************************
 * RvSipNotifySetReferNotifyBody
 * ------------------------------------------------------------------------
 * General: Builds the body for a notify request of a refer subscription.
 *          The function sets the correct body and content-type header
 *          in the notify outbound message.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *               RV_ERROR_UNKNOWN        -  Failed to set the notify body.
 *               RV_OK                   -  set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hNotify    - Handle to the notification object.
 *          statusCode - status code to be set in NOTIFY message body.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipNotifySetReferNotifyBody(
                                            IN  RvSipNotifyHandle   hNotify,
                                            IN  RvInt16            statusCode)
{
    RvStatus      rv;
    Notification* pNotify = (Notification*)hNotify;

    if(pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SubsLockAPI(pNotify->pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pNotify->pSubs->pReferInfo->referFinalStatus = (RvUint16)statusCode;

    rv = SubsReferSetNotifyBody(pNotify, statusCode);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
            "RvSipNotifySetReferNotifyBody - subs 0x%p notification 0x%p - Failed to set notify info (status %d)",
            pNotify->pSubs, pNotify, rv));
        SubsUnLockAPI(pNotify->pSubs);
        return rv;
    }

    SubsUnLockAPI(pNotify->pSubs);
    return RV_OK;
}

/***************************************************************************
* RvSipSubsGetReferToHeader
* ------------------------------------------------------------------------
* General: Returns the Refer-To header associated with the subscription.
*
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - Refer-To header was returned successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs     - Handle to the subscription.
* Output:    phReferTo - Pointer to the subscription Refer-To header handle.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetReferToHeader (
                                IN  RvSipSubsHandle        hSubs,
                                OUT RvSipReferToHeaderHandle *phReferTo)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phReferTo == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phReferTo = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubscription->pReferInfo == NULL)
    {
        RvLogWarning(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetReferToHeader: subs 0x%p - no refer-info in subscription",
            hSubs));
        *phReferTo = NULL;
    }
    else
    {
        *phReferTo = pSubscription->pReferInfo->hReferTo;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipSubsGetReferredByHeader
* ------------------------------------------------------------------------
* General: Returns the Referred-By header associated with the subscription.
*
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - Referred-By header was returned successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs   - Handle to the subscription.
* Output:    phReferredBy - Pointer to the subscription Referred-By header handle.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetReferredByHeader (
                            IN  RvSipSubsHandle              hSubs,
                            OUT RvSipReferredByHeaderHandle *phReferredBy)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phReferredBy == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phReferredBy = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubscription->pReferInfo == NULL)
    {
        RvLogWarning(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetReferredByHeader: subs 0x%p - no refer-info in subscription",
            hSubs));
        *phReferredBy = NULL;
    }
    else
    {
        *phReferredBy = pSubscription->pReferInfo->hReferredBy;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipSubsGetReferFinalStatus
* ------------------------------------------------------------------------
* General: Returns the status code that was received in the body of the final
*          notify request of a refer subscription.
*
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - final status was returned successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:  hSubs             - Handle to the subscription.
* Output: pReferFinalStatus - Pointer to the refer final status value.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetReferFinalStatus (
                                        IN  RvSipSubsHandle  hSubs,
                                        OUT RvInt32        *pReferFinalStatus)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pReferFinalStatus == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *pReferFinalStatus = UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pSubscription->pReferInfo == NULL)
    {
        RvLogWarning(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetReferFinalStatus: subs 0x%p - no refer-info in subscription",
            hSubs));
    }
    else
    {
        *pReferFinalStatus = pSubscription->pReferInfo->referFinalStatus;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/*-----------------------------------------------------------------------
       S U B S C R I P T I O N:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
* RvSipSubsIsOutOfBand
* ------------------------------------------------------------------------
* General: Indicates whether the given subscription is an out-of-band subscription.
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_OK - Success.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs - Handle to the subscription.
* Output:  pbIsOutOfBand - A valid pointer which will be updated with the
*                          subscription out-of-band boolean.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsIsOutOfBand (IN  RvSipSubsHandle hSubs,
                                                 OUT RvBool*        bIsOutOfBand)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(bIsOutOfBand == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *bIsOutOfBand = pSubscription->bOutOfBand;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsSetEventHeader
 * ------------------------------------------------------------------------
 * General: Sets the event header associated with the subscription.
 *          Note that the event header identify the subscription, so you must not
 *          change it after subscription was initialized.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
 *               RV_OK                    - Event header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs  - Handle to the subscription.
 *            hEvent - Handle to an application constructed event header to be set
 *                     in the subscription object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetEventHeader (
                                      IN  RvSipSubsHandle         hSubs,
                                      IN  RvSipEventHeaderHandle  hEvent)
{
    RvStatus    rv;
    Subscription *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    /* you can set event value, only at initial state:
       idle for regular subscription, active for out of band subscription.*/
    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
         pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetEventHeader - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(SipCallLegIsHiddenCallLeg(pSubscription->hCallLeg) == RV_FALSE)
    {
        if(SubsFindSubsWithSameEventHeader(pSubscription->hCallLeg, pSubscription, hEvent) == RV_TRUE)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                "RvSipSubsSetEventHeader - subscription 0x%p - subscription with same Event header already exists",
                pSubscription));
            SubsUnLockAPI(pSubscription);
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
        "RvSipSubsSetEventHeader - subs 0x%p - Setting Event Header",
        pSubscription));

    rv = SubsSetEventHeader(pSubscription, hEvent);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetEventHeader - subs 0x%p - Failed to set event header (status %d)",
            pSubscription, rv));
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}


/***************************************************************************
 * RvSipSubsGetEventHeader
 * ------------------------------------------------------------------------
 * General: Returns the Event header associated with the subscription.
 *          Note that the event header identify the subscription,so you must not
 *          change it after subscription was initialized.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
 *               RV_OK                    - Event header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - Handle to the subscription.
 * Output:    phEvent - A valid pointer which will be updated with the Pointer to
 *                      the subscription Event header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetEventHeader (IN  RvSipSubsHandle         hSubs,
                                                   OUT RvSipEventHeaderHandle *phEvent)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phEvent == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phEvent = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *phEvent = pSubscription->hEvent;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}


/***************************************************************************
 * RvSipSubsSetExpiresVal
 * ------------------------------------------------------------------------
 * General: Sets the expires header value (in seconds) associated with the subscription.
 *          You may set the expires value, only before sending the first subscribe
 *          request. later on you should change it only with API functions of
 *          refresh. This is because the expires value is set according to the 2xx
 *          response, or notify messages.
 *          You must not change it after subscription was initialized.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_OK - Expires value was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs      - Handle to the subscription.
 *          expiresVal - time for subscription expiration - in seconds.
 *                       value of this argument must be smaller than 0xFFFFFFF/1000.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetExpiresVal (IN  RvSipSubsHandle   hSubs,
                                                  IN  RvInt32           expiresVal)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    /* you can set expiration value, only at initial states, after that updating expiration
    must be done with a notify requests, or refresh */
    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->eState != RVSIP_SUBS_STATE_SUBS_RCVD
        && pSubscription->bOutOfBand == RV_FALSE) ||
        (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
        pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetExpiresVal - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if(IsLegalExpiresValue(expiresVal) == RV_FALSE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetExpiresVal - subs 0x%p - too big expires value 0x%x",
            pSubscription,expiresVal));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
        "RvSipSubsSetExpiresVal - subs 0x%p - Setting Expires value %d",
        pSubscription,expiresVal));

    pSubscription->expirationVal = expiresVal;
    pSubscription->requestedExpirationVal = expiresVal;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}


/***************************************************************************
 * RvSipSubsGetExpiresVal
 * ------------------------------------------------------------------------
 * General: Returns the Expires value of the subscription, in seconds.
 *          For notifier subscription: The function retrieve the last expires
 *          value that was set by notifier in 2xx response,
 *          or in a notify request.
 *          Note - if the SUBSCRIBE request did not contain an Expires header,
 *          RVSIP_SUBS_EXPIRES_VAL_LIMIT is retrieved.
 *          If notifier wants the expires value from the incoming SUBSCRIBE
 *          request it should use RvSipSubsGetRequestedExpiresVal function.
 *          For subscriber subscription: The function retrieve the value that
 *          was set on initialization of the subscription, or updated
 *          value, in case notifier updated it already.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - Expires value was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - Handle to the subscription.
 * Output:    pExpires - Pointer to the Expires value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetExpiresVal (IN  RvSipSubsHandle   hSubs,
                                                  OUT RvInt32          *pExpires)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pExpires == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *pExpires = UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *pExpires = pSubscription->expirationVal;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetRequestedExpiresVal
 * ------------------------------------------------------------------------
 * General: Returns the requested expires value, in seconds.
 *          When a new SUBSCRIBE request is received, it's expires value
 *          can be retrieve using this function.
 *          If the SUBSCRIBE request did not contain an Expires header,
 *          RVSIP_SUBS_EXPIRES_VAL_LIMIT is retrieved.
 *          This function is relevant only for notifier subscription.
 *          (note - this function is different from RvSipSubsGetExpiresVal,
 *           because it only retrieve value, when there is an incoming SUBSCRIBE
 *           request, waiting for a response. RvSipSubsGetExpiresVal retrieve
 *           the last expires value that was set by notifier in 2xx response,
 *           or in a notify request).
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
 *               RV_OK                    - Expires value was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs             - Handle to the subscription.
 * Output:    pRequestedExpires - Pointer to the requested Expires value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetRequestedExpiresVal (
                                   IN  RvSipSubsHandle   hSubs,
                                   OUT RvInt32          *pRequestedExpires)
{
    RvStatus     rv;
    Subscription  *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRequestedExpires == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *pRequestedExpires = UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *pRequestedExpires = pSubscription->requestedExpirationVal;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetRemainingTime
 * ------------------------------------------------------------------------
 * General: Returns the remaining time of a subscription, in seconds.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
 *               RV_OK                    - Remaining time value was returned successfully.
 * Note -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - Handle to the subscription.
 * Output:    pRemainingTime - Pointer to the remaining time value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetRemainingTime (
                                       IN  RvSipSubsHandle   hSubs,
                                       OUT RvInt32          *pRemainingTime)
{
    RvStatus     rv;
    Subscription  *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRemainingTime == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *pRemainingTime = UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(!SipCommonCoreRvTimerStarted(&pSubscription->alertTimer))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetRemainingTime: pSubs 0x%p - alertTimer is NULL",hSubs));
        *pRemainingTime = UNDEFINED;
    }
    else
    {
        RvInt64 delay;
        RvTimerGetRemainingTime(&pSubscription->alertTimer.hTimer, &delay);
        *pRemainingTime = RvInt64ToRvInt32(RvInt64Div(delay, RV_TIME64_NSECPERSEC));
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return RV_OK;
}
/***************************************************************************
* RvSipSubsGetDialogObj
* ------------------------------------------------------------------------
* General: Returns handle to the dialog object, related to this subscription.
*          (the function is relevant for a subscription that was not created
*          in a connected call-leg).
*          The user can use this dialog handle to set or get parameters that
*          are kept inside the dialog, and not within the subscription
*          (such as call-Id, cseq, To, From and OutboundAddress).
*          You can get and set dialog parameters with the call-leg API functions.
*          Note - you must not change the dialog parameters after the subscription
*          was established.
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - Dialog object was returned successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs    - Handle to the subscription.
* Output:    phDialog - Pointer to the dialog handle.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetDialogObj (IN  RvSipSubsHandle     hSubs,
                                                 OUT RvSipCallLegHandle *phDialog)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phDialog == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phDialog = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetDialogObj - subscription 0x%p, dialog is 0x%p",pSubscription,pSubscription->hCallLeg));

    *phDialog = pSubscription->hCallLeg;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetRetryAfterInterval
 * ------------------------------------------------------------------------
 * General: Returns the Retry-after interval found in the last rejected response.
 *	        If the retry-after did not appear in the last response,
 *	        the return value is 0.
 *          When hSubs argument is given, the function retrieve the retry-after value
 *          from the response on SUBSCRIBE request.
 *          When hNotify argument is given, the function retrieve the retry-after value
 *          from the response on NOTIFY request.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
 *               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
 *               RV_OK                    - value was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs    - Handle to the subscription received the response.
 *            hNotify  - Handle to the notification received the response.
 * Output:    pRetryAfterInterval - The retry-after value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetRetryAfterInterval (
                                      IN  RvSipSubsHandle  hSubs,
                                      IN  RvSipNotifyHandle hNotify,
                                      OUT RvUint32       *pRetryAfterInterval)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;

    if(hSubs == NULL && hNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRetryAfterInterval == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *pRetryAfterInterval = 0;

    /*lock the subscription*/
    if(pSubscription == NULL)
    {
        /*lint -e413*/
        pSubscription = pNotify->pSubs;
    }
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pRetryAfterInterval = (hNotify != NULL)? pNotify->retryAfter : pSubscription->retryAfter;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;

}
/***************************************************************************
* RvSipSubsSetAppSubsHandle
* ------------------------------------------------------------------------
* General: Sets the application subscription handle.
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_OK                    - application handle was set successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs    - Handle to the subscription.
*            hAppSubs - Handle to the application subscription.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetAppSubsHandle (
                                          IN  RvSipSubsHandle     hSubs,
                                          IN  RvSipAppSubsHandle  hAppSubs)
{
    RvStatus     rv;
    Subscription  *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    pSubscription->hAppSubs = hAppSubs;
    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetAppSubsHandle - subscription 0x%p, app handle is 0x%p",pSubscription,pSubscription->hAppSubs));

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipSubsGetAppSubsHandle
* ------------------------------------------------------------------------
* General: Returns the application subscription handle.
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - application handle was returned successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSubs     - Handle to the subscription.
* Output:    phAppSubs - Handle to the application subscription.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetAppSubsHandle (
                                          IN  RvSipSubsHandle     hSubs,
                                          OUT RvSipAppSubsHandle *phAppSubs)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phAppSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phAppSubs = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetAppSubsHandle - subscription 0x%p, app handle is 0x%p",pSubscription,pSubscription->hAppSubs));

    *phAppSubs = pSubscription->hAppSubs;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipSubsGetSubsMgr
* ------------------------------------------------------------------------
* General: Returns the subs manager handle and the application subs manager
*          handle of the given subscription.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs        - Handle to the subscription.
* Output:   phSubsMgr    - The subs manager handle.
*           phSubsAppMgr - The application subs manager handle.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetSubsMgr (IN  RvSipSubsHandle     hSubs,
                                               OUT RvSipSubsMgrHandle *phSubsMgr,
                                               OUT void               **phSubsAppMgr)
{
    RvStatus     rv;
    Subscription  *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phSubsMgr == NULL || phSubsAppMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phSubsMgr = *phSubsAppMgr = NULL;

    rv = SubsLockAPI(pSubscription);/*lock the subscription*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *phSubsMgr = (RvSipSubsMgrHandle)pSubscription->pMgr;
    *phSubsAppMgr = pSubscription->pMgr->pAppSubsMgr;

    SubsUnLockAPI(pSubscription); /*unlock the subscription*/
    return RV_OK;
}

/***************************************************************************
 * RvSipSubsGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the subscription current state
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given subscription handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - Handle to the subscription.
 * Output:    peState - The subscription current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetCurrentState (IN  RvSipSubsHandle    hSubs,
                                                    OUT RvSipSubsState    *peState)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(peState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *peState = RVSIP_SUBS_STATE_UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetCurrentState - subscription 0x%p, current state is %s",
             pSubscription,SubsGetStateName(pSubscription->eState)));

    *peState = pSubscription->eState;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
* RvSipSubsGetSubsType
* ------------------------------------------------------------------------
* General: Gets the subscription type. A subscription can be either a
*          Subscriber or a Notifier.
* Return Value: RV_ERROR_INVALID_HANDLE - if the given subscription handle is invalid
*               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
*               RV_OK.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs      - Handle to the subscription.
* Output:   peSubsType - The subscription type.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetSubsType (IN  RvSipSubsHandle        hSubs,
                                                OUT RvSipSubscriptionType  *peSubsType)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(peSubsType == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *peSubsType = RVSIP_SUBS_TYPE_UNDEFINED;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    *peSubsType = pSubscription->eSubsType;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the subscription. You can
 *          call this function from the state changed callback function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - Handle to the subscription.
 * Output:    phMsg   - Pointer to the received message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetReceivedMsg(
                                            IN  RvSipSubsHandle  hSubs,
                                            OUT RvSipMsgHandle  *phMsg)
{
    RvStatus       rv     = RV_OK;
    Subscription    *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL || phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phMsg = NULL;

    /*try to lock the subscription */
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if(pSubs->hActiveTransc != NULL)
    {
        rv = RvSipTransactionGetReceivedMsg(pSubs->hActiveTransc, phMsg);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "RvSipSubsGetReceivedMsg: Subs 0x%p - Failed to get received msg from hTransc 0x%p (rv= %d)",
                pSubs, pSubs->hActiveTransc, rv));
        }
    }
    else
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsGetReceivedMsg: Subs 0x%p - no valid transaction. illegal action.",
            pSubs));
        rv = RV_ERROR_ILLEGAL_ACTION;
    }

    SubsUnLockAPI(pSubs); /*unlock the subscription*/
    return rv;
}

/***************************************************************************
 * RvSipSubsGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message (request or response) that is going to be sent
 *          by the subscription. You should use this function to add headers 
 *          to the message before it is sent.
 *          You can call this function before you call a control API functions
 *          that sends a message (such as RvSipSubsSubscribe()), or from
 *          the ExpirationAlert call-back function when a refresh message is
 *          going to be sent.
 *          To view the complete message use event message to send.
 *          Note: The outbound message you receive is not complete.
 *          In some cases it might even be empty.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs   - The subscription handle.
 * Output:    phMsg   - Pointer to the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetOutboundMsg(
                                     IN  RvSipSubsHandle     hSubs,
                                     OUT RvSipMsgHandle     *phMsg)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;
    RvSipMsgHandle         hOutboundMsg = NULL;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phMsg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phMsg = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL != pSubscription->hOutboundMsg)
    {
        hOutboundMsg = pSubscription->hOutboundMsg;
    }
    else
    {
        rv = RvSipMsgConstruct(pSubscription->pMgr->hMsgMgr,
                               pSubscription->pMgr->hGeneralPool,
                               &hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                       "RvSipSubsGetOutboundMsg - subs 0x%p error in constructing message",
                      pSubscription));
            SubsUnLockAPI(pSubscription); /*unlock the subscription*/
            return rv;
        }
        pSubscription->hOutboundMsg = hOutboundMsg;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsGetOutboundMsg - subs 0x%p returned handle= 0x%p",
              pSubscription,hOutboundMsg));

    *phMsg = hOutboundMsg;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this subscription
 *          object belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs     - Handle to the subscription object.
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                        handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetStackInstance(
                                     IN  RvSipSubsHandle     hSubs,
                                     OUT void              **phStackInstance)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }

    *phStackInstance = NULL;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsGetStackInstance - subscription 0x%p, stack instance is 0x%p",pSubscription,pSubscription->pMgr->hStack));


    *phStackInstance = pSubscription->pMgr->hStack;

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}


/***************************************************************************
 * RvSipSubsGetEventPackageType
 * ------------------------------------------------------------------------
 * General: Returns the event package type of the given subscription: refer,
 *          presence, presence-winfo, etc. 
 *          Note that in the case of presence-winfo, the  Event header contains
 *          a package: presence, and a template: winfo, and they are both combined 
 *          here into a single subs event package PRESENCE_WINFO.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs         - Handle to the Subscription.
 * Output:  pePackageType - Pointer to the event package type.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetEventPackageType(
                                IN  RvSipSubsHandle             hSubs,
                                OUT RvSipSubsEventPackageType *pePackageType)
{
    RvStatus      rv;
    Subscription*   pSubs = (Subscription*)hSubs;
   
    if(pSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(NULL == pePackageType)
    {
        return RV_ERROR_NULLPTR;
    }
    *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_UNDEFINED;

    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pSubs->pReferInfo != NULL)
    {
        *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_REFER ;
    }
    else if (pSubs->hEvent == NULL)
    {
        *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_UNDEFINED;
    }
    else
    {
         RvChar      strPackage[10];
         RvChar      strTemplate[10];
         RvUint      actualLen;

        /* check presence and presence.winfo */
        rv = RvSipEventHeaderGetEventPackage(pSubs->hEvent, (RvChar*)strPackage, 10, &actualLen);
        if(rv != RV_OK)
        {
            *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_OTHER;
        }
        else if(SipCommonStricmp(strPackage, "presence") == RV_TRUE)
        {
            /* check the template for winfo now */
            rv = RvSipEventHeaderGetEventTemplate(pSubs->hEvent, (RvChar*)strTemplate, 10, &actualLen);
            if(rv != RV_OK)
            {
                *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_PRESENCE;
            }
            else
            {
                *pePackageType=(SipCommonStricmp(strTemplate,"winfo")==RV_TRUE)? \
                                RVSIP_SUBS_EVENT_PACKAGE_TYPE_PRESENCE_WINFO:RVSIP_SUBS_EVENT_PACKAGE_TYPE_PRESENCE;
            }
        }
        else /* not presence */
        {
            *pePackageType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_OTHER;
        }
       
    }

    SubsUnLockAPI(pSubs);
    return RV_OK;
}


/***************************************************************************
* RvSipSubsGetNewHeaderHandle
* ------------------------------------------------------------------------
* General: Allocates a new header on the subscription page, and returns the new
*         header handle.
*         The application may use this function to allocate a message header.
*         It should then fill the element information and set it back to
*         the subscription using the relevant 'set' (or 'init') function.
*         (e.g. after getting new event header handle, application should
*          fill it, and then set it back with RvSipSubsSetEventHeader()).
*
*         The function supports the following elements:
*         To/From - you should set these headers back with the RvSipSubsInit() 
*                   or RvSipSubsDialogInit() API functions.
*         Event   - you should set this header back with the RvSipSubsSetEventHeader()
*                   API function.
*         Refer-to - you should set this header back with the RvSipSubsReferInit()
*                   API function.
*         Referred-by - you should set this header back with the RvSipSubsReferInit()
*                   API function.
*
*          Note - You may use this function only on subscription initialization stage. 
*          After the initialization stage, you must allocate the header on 
*          application page, and then set it with the correct API function.
*
* Return Value: RV_ERROR_INVALID_HANDLE  - The subscription handle is invalid.
*               RV_ERROR_NULLPTR         - Bad pointer was given by the application.
*               RV_OK                    - Header was constructed successfully.
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSubs       - Handle to the subscription.
*           eHeaderType - Type of header to allocate.
* Output:   phHeader    - Pointer to the new header handle.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetNewHeaderHandle (
                            IN  RvSipSubsHandle  hSubs,
                            IN  RvSipHeaderType  eHeaderType,
                            OUT void*           *phHeader)
{
    RvStatus        rv;
    Subscription   *pSubscription = (Subscription*)hSubs;
    HRPOOL          hPool;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phHeader = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((pSubscription->bOutOfBand == RV_FALSE && pSubscription->eState != RVSIP_SUBS_STATE_IDLE) ||
       (pSubscription->bOutOfBand == RV_TRUE && pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetNewHeaderHandle - subs 0x%p - illegal action - you may use this function only on state IDLE!!!",
            pSubscription));
        SubsUnLockAPI(pSubscription); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    hPool = pSubscription->pMgr->hGeneralPool;
    switch (eHeaderType)
    {
    case RVSIP_HEADERTYPE_EVENT:
        rv = RvSipEventHeaderConstruct(pSubscription->pMgr->hMsgMgr, hPool,
                                pSubscription->hPage, (RvSipEventHeaderHandle*)phHeader);
        break;
    case RVSIP_HEADERTYPE_REFER_TO:
        rv = RvSipReferToHeaderConstruct(pSubscription->pMgr->hMsgMgr, hPool,
                                pSubscription->hPage, (RvSipReferToHeaderHandle*)phHeader);
        break;
    case RVSIP_HEADERTYPE_REFERRED_BY:
        rv = RvSipReferredByHeaderConstruct(pSubscription->pMgr->hMsgMgr, hPool,
                                pSubscription->hPage, (RvSipReferredByHeaderHandle*)phHeader);
        break;
    case RVSIP_HEADERTYPE_FROM:
    case RVSIP_HEADERTYPE_TO:
        rv = RvSipCallLegGetNewMsgElementHandle(pSubscription->hCallLeg, eHeaderType, 
                                 RVSIP_ADDRTYPE_UNDEFINED, phHeader);
        break;

    default:
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetNewHeaderHandle: subs 0x%p - illegal header type %d for this function",
            pSubscription, eHeaderType));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/***************************************************************************
 * RvSipSubsGetTranscCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address that is used by a specific subscription
 *          transaction.
 *          Supply a subscription handle to get local address of a subscribe transaction,
 *          or a notification handle to get local address of a notify transaction,.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs   - Handle to the subscription holding the transaction.
 *          hNotify - Handle to the notification holding the transaction.
 * Output:  eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType  - The address type(IP4 or IP6).
 *          localAddress  - The local address this transaction is using(must be longer than 48 bytes).
 *          localPort     - The local port this transaction is using.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipSubsGetTranscCurrentLocalAddress(
                            IN  RvSipSubsHandle            hSubs,
                            IN  RvSipNotifyHandle          hNotify,
                            OUT RvSipTransport            *eTransportType,
                            OUT RvSipTransportAddressType *eAddressType,
                            OUT RvChar                    *localAddress,
                            OUT RvUint16                  *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT RvChar                  *if_name
			    , OUT RvUint16                *vdevblock
#endif
			    )
{
    RvStatus               rv;
    Subscription           *pSubs = (Subscription*)hSubs;
    Notification           *pNotify = (Notification*)hNotify;
    RvSipTranscHandle       hTransc;

    if(pSubs == NULL && pNotify == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(eTransportType == NULL || eAddressType == NULL ||
        localAddress == NULL || localPort == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    *eAddressType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
    *localPort = (RvUint16)UNDEFINED;

    if(pSubs == NULL)
    {
        pSubs = pNotify->pSubs;
    }
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(hNotify == NULL)
    {
        hTransc = pSubs->hActiveTransc;
    }
    else
    {
        hTransc = pNotify->hTransc;
    }
    if(hTransc == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RvSipSubsGetTranscCurrentLocalAddress - subs 0x%p notify 0x%p - Failed. transaction is NULL", pSubs, pNotify));
        SubsUnLockAPI(pSubs); /*unlock the call leg*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RvSipTransactionGetCurrentLocalAddress(hTransc,eTransportType,eAddressType,localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
						,if_name
						,vdevblock
#endif
						);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsGetTranscCurrentLocalAddress - subs 0x%p notify 0x%p - Failed. (rv = %d)",
               hSubs,hNotify,rv));
         SubsUnLockAPI(pSubs); /*unlock the call leg*/
         return rv;
    }

    SubsUnLockAPI(pSubs); /*unlock the call leg*/
    return rv;
}


/******************************************************************************
 * RvSipSubsSetForkingEnabledFlag
 * ----------------------------------------------------------------------------
 * General: Sets the "Forking Enabled" flag to the subscription.
 *          In case the SUBSCRIBE request was forked by proxy to several
 *          notifiers, the subscription may get NOTIFY requests from several
 *          notifiers, which are referred to as "Forked NOTIFY requests".
 *          Setting the "Forking Enabled" flag of the subscription to RV_TRUE
 *          will create a new Subscription object for each Forked NOTIFY
 *          request received for the original subscription (the subscription
 *          that sent the SUBSCRIBE request).
 *          Setting the "Forking Enabled" flag of a subscription to RV_FALSE
 *          will cause the original subscription to reject with a 481 response
 *          (No Matching Call) for every Forked NOTIFY request that is received
 *          For forking details, see sections 3.3.3 and 4.4.9 of RFC 3265.
 *
 * Return Value: RV_OK on success,
 *               Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSubs   - Handle to the subscription that sent the request.
 *           bForkingEnabled - Forking-enabled flag value.
 * Output:   none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetForkingEnabledFlag(
                                                IN  RvSipSubsHandle hSubs,
                                                IN  RvBool          bForkingEnabled)
{
    RvStatus        rv;
    Subscription    *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = RvSipCallLegSetForkingEnabledFlag(pSubscription->hCallLeg, bForkingEnabled);
    if(RV_OK != rv)
    {
        RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsSetForkingEnabledFlag - RvSipCallLegSetForkingEnabledFlag(hCallLeg=0x%p) failed (rv=%d:%s)",
            pSubscription->hCallLeg, rv, SipCommonStatus2Str(rv)));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsSetForkingEnabledFlag - hSubs=0x%p - flag was set to %s",
              hSubs, (RV_TRUE==bForkingEnabled)?"TRUE":"FALSE"));

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/******************************************************************************
 * RvSipSubsGetForkingEnabledFlag
 * ----------------------------------------------------------------------------
 * General: Gets the "Forking Enabled" flag to the subscription.
 *          In case the SUBSCRIBE request was forked by proxy to several
 *          notifiers, the subscription may get NOTIFY requests from several
 *          notifiers, which are referred to as "Forked NOTIFY requests".
 *          Setting the "Forking Enabled" flag of the subscription to RV_TRUE
 *          will create a new Subscription object for each Forked NOTIFY
 *          request received for the original subscription (the subscription
 *          that sent the SUBSCRIBE request).
 *          Setting the "Forking Enabled" flag of a subscription to RV_FALSE
 *          will cause the original subscription to reject with a 481 response
 *          (No Matching Call) for every Forked NOTIFY request that is received
 *          For forking details, see sections 3.3.3 and 4.4.9 of RFC 3265.
 *
 * Return Value: RV_OK on success,
 *               Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSubs            - Handle to the subscription that sent the request.
 * Output:   pbForkingEnabled - A requested flag value.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetForkingEnabledFlag(
                                                IN  RvSipSubsHandle hSubs,
                                                OUT RvBool          *pbForkingEnabled)
{
    RvStatus        rv;
    Subscription    *pSubscription = (Subscription*)hSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pbForkingEnabled == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = RvSipCallLegGetForkingEnabledFlag(pSubscription->hCallLeg,pbForkingEnabled);
    if(RV_OK != rv)
    {
        RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetForkingEnabledFlag - RvSipCallLegGetForkingEnabledFlag(hCallLeg=0x%p) failed (rv=%d:%s)",
            pSubscription->hCallLeg, rv, SipCommonStatus2Str(rv)));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;
}

/******************************************************************************
* RvSipSubsGetOriginalSubs
* -----------------------------------------------------------------------------
* General:  Gets handle of the Original Subscription.
*           In case SUBSCRIBE request was forked by proxy to several notifiers,
*           subscription may get notify requests from several notifiers.
*           The original subscription, is the subscription that actually sent the
*           SUBSCRIBE request.
*           If no original subscription exists, NULL will be retrieved.
*           The function is intended for use with Forked Subscriptions.
*           For forking details see RFC 3265, sections 3.3.3 and 4.4.9.
*
* Return Value: RV_OK on success,
*               Error Code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    hSubs   - Handle to the subscription that sent the request.
* Output:   phOriginalSubs  - A valid pointer which will be updated with the
*                     original subscription handle.
******************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetOriginalSubs(
                                        IN  RvSipSubsHandle  hSubs,
                                        OUT RvSipSubsHandle *phOriginalSubs)
{
    RvStatus        rv;
    Subscription    *pSubscription = (Subscription*)hSubs;
    RvSipCallLegHandle hCallLegOrigSubs;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(phOriginalSubs == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *phOriginalSubs = NULL;

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == pSubscription->pOriginalSubs)
    {
        RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetOriginalSubs - Subs 0x%p - returned Original Subs NULL",
            hSubs));
        SubsUnLockAPI(pSubscription);
        *phOriginalSubs = NULL;
        return RV_OK;
    }

    rv = SipCallLegGetOriginalCallLeg(pSubscription->hCallLeg, &hCallLegOrigSubs);
    if (RV_OK != rv)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetOriginalSubs - Subs 0x%p - failed SipCallLegGetOriginalCallLeg(hCallLeg=0x%p) (rv=%d:%s)",
            hSubs, pSubscription->hCallLeg, rv, SipCommonStatus2Str(rv)));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return rv;
    }

    *phOriginalSubs = (RvSipSubsHandle)pSubscription->pOriginalSubs;

    if (hCallLegOrigSubs != pSubscription->pOriginalSubs->hCallLeg)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "RvSipSubsGetOriginalSubs - Subs 0x%p - Call-Leg 0x%p and Original Call-Leg 0x%p discrepancy",
            hSubs, hCallLegOrigSubs, pSubscription->pOriginalSubs->hCallLeg));
        SubsUnLockAPI(pSubscription);
        return RV_ERROR_DESTRUCTED;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
        "RvSipSubsGetOriginalSubs - Subs 0x%p - returned Original Subs 0x%p",
        hSubs, pSubscription->pOriginalSubs));

    SubsUnLockAPI(pSubscription);
    return RV_OK;
}
/***************************************************************************
 * RvSipSubsSetRejectStatusCodeOnCreation
 * ------------------------------------------------------------------------
 * General: This function can be used synchronously from the
 *          RvSipSubsCreatedEv callback to instruct the stack to
 *          automatically reject the request that created this subscription.
 *          In this function you should supply the reject status code.
 *          If you set this status code, the subscription will be destructed
 *          automatically when the RvSipSubsCreatedEv returns. The
 *          application will not get any further callbacks
 *          that relate to this subscription. The application will not get the
 *          RvSipSubsMsgToSendEv for the reject response message or the
 *          Terminated state for the subscription object.
 *          This function should not be used for rejecting a request in
 *          a normal scenario. For that you should use the RvSipSubsRespondReject()
 *          API function. You should use this function only if your application is
 *          incapable of handling this new subscription at all, for example in an
 *          application out of resource situation.
 *          Note: When this function is used to reject a request, the application
 *          cannot use the outbound message mechanism to add information
 *          to the outgoing response message. If you wish to change the response
 *          message you must use the regular reject function in the
 *          RVSIP_SUBS_STATE_SUBS_RCVD state.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSubs - Handle to the subscription object.
 *           rejectStatusCode - The reject status code for rejecting the request
 *                              that created this object. The value must be
 *                              between 300 to 699.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsSetRejectStatusCodeOnCreation(
                              IN  RvSipSubsHandle        hSubs,
                              IN  RvUint16               rejectStatusCode)
{
    RvStatus               rv;
    Subscription          *pSubscription = (Subscription*)hSubs;
    RvSipCallLegDirection  eDirection    = RVSIP_CALL_LEG_DIRECTION_UNDEFINED;

    if(hSubs == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /*lock the subscription*/
    rv = SubsLockAPI(pSubscription);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "RvSipSubsSetRejectStatusCodeOnCreation - subscription 0x%p, setting reject status to %d",
              pSubscription, rejectStatusCode));

    /*invalid status code*/
    if (rejectStatusCode < 300 || rejectStatusCode >= 700)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsSetRejectStatusCodeOnCreation - Failed. subscription 0x%p. %d is not a valid reject response code",
                  pSubscription,rejectStatusCode));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return RV_ERROR_BADPARAM;
    }

    RvSipCallLegGetDirection(pSubscription->hCallLeg, &eDirection);
    if(eDirection != RVSIP_CALL_LEG_DIRECTION_INCOMING ||
       pSubscription->eState != RVSIP_SUBS_STATE_IDLE)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                  "RvSipSubsSetRejectStatusCodeOnCreation(hSubs=0x%p) - Failed. Cannot be called in state %s or for %s subscriptions",
                  pSubscription,SubsGetStateName(pSubscription->eState),
                  SipCallLegGetDirectionName(eDirection)));
        SubsUnLockAPI(pSubscription); /* unlock the subscription */
        return RV_ERROR_BADPARAM;

    }
    pSubscription->createdRejectStatusCode = rejectStatusCode;
    SubsUnLockAPI(pSubscription); /* unlock the subscription */
    return rv;

}


#ifdef RV_SIP_HIGHAVAL_ON
/*-----------------------------------------------------------------------
       SUBSCRIPTION   HIGH AVAILABILITY   F U N C T I O N S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipSubsGetActiveSubsStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of an active subs.
 *          (The size of buffer, that should be supplied in RvSipSubsStoreActiveSubs()).
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs - Handle to the active subscription that will be stored.
 * Output:  pLen  - the size of buffer needed to store all subscription parameters.
 *                  UNDEFINED in case of failure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsGetActiveSubsStorageSize(
                                       IN  RvSipSubsHandle     hSubs,
                                       OUT RvInt32            *pLen)
{
    RvStatus        rv;
    Subscription   *pSubs = (Subscription*)hSubs;

    if(pSubs == NULL || pLen == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *pLen = 0;
    /*try to lock the subs*/
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsGetActiveSubsStorageSize - Gets storage size for subs 0x%p",
              pSubs));

    rv = SubsGetActiveSubsStorageSize(hSubs, RV_TRUE, pLen);
	if (rv != RV_OK)
	{
		RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
			"RvSipSubsGetActiveSubsStorageSize - Failed to get storage size for subs 0x%p",
			pSubs));
        SubsUnLockAPI(pSubs);
		return rv;
	}


    SubsUnLockAPI(pSubs); /*unlock the subs */
    return rv;
}
/***************************************************************************
 * RvSipSubsStoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Copies all subscription parameters from a given subscription to a
 *          given buffer.
 *          This buffer should be supplied when restoring the subscription.
 *          In order to store subs information the subscription must be in the
 *          active state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs    - Handle to the active subscription that will be stored.
 *          memBuff  - The buffer that will be filled with the subscription information.
 *          buffLen  - The length of the given buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsStoreActiveSubs(
                                       IN RvSipSubsHandle    hSubs,
                                       IN void*              memBuff,
                                       IN RvUint32           buffLen)
{
    RvStatus          rv;
    Subscription     *pSubs = (Subscription*)hSubs;
    RvUint32          subsLen;
    RvSipCallLegState eCallState;

    if(pSubs == NULL || memBuff == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the subs-leg*/
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsStoreActiveSubs - Stores subs 0x%p information",
              pSubs));

    /* If the Subs is a part of a CallLeg it mustn't be stored. */
    /* Only Subs that contain a CallLeg can be stored. In this  */
    /* case the CallLeg is in state RVSIP_CALL_LEG_STATE_IDLE   */
    rv = RvSipCallLegGetCurrentState(pSubs->hCallLeg,&eCallState);
    if (rv != RV_OK || eCallState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        SubsUnLockAPI(pSubs); /*unlock the subs */
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsStoreActiveSubs - Cannot store subs 0x%p inside a CallLeg 0x%p (State=%d)",
              pSubs,pSubs->hCallLeg,eCallState));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SipSubsStoreActiveSubs(hSubs,RV_TRUE, buffLen, memBuff, (RvInt32*)&subsLen);
    SubsUnLockAPI(pSubs); /*unlock the subs */
    return rv;
}

/***************************************************************************
 * RvSipSubsStoreActiveSubsExt
 * ------------------------------------------------------------------------
 * General: Copies all subscription parameters from a given subscription to a
 *          given buffer.
 *          This buffer should be supplied when restoring the subscription.
 *          In order to store subs information the subscription must be in the
 *          active state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs    - Handle to the active subscription that will be stored.
 *          memBuff  - The buffer that will be filled with the subscription information.
 *          buffLen  - The length of the given buffer.
 * Output   usedBuffLen - The number of bytes that were actually used in the buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsStoreActiveSubsExt(
                                       IN RvSipSubsHandle    hSubs,
                                       IN void*              memBuff,
                                       IN RvUint32           buffLen,
                                       OUT RvUint32          *pUsedBuffLen)
{
    RvStatus          rv;
    Subscription     *pSubs = (Subscription*)hSubs;
    RvUint32          subsLen;
    RvSipCallLegState eCallState;

    if(pSubs == NULL || memBuff == NULL || pUsedBuffLen == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the subs-leg*/
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pUsedBuffLen = 0;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsStoreActiveSubsExt - Stores subs 0x%p information",
              pSubs));

    /* If the Subs is a part of a CallLeg it mustn't be stored. */
    /* Only Subs that contain a CallLeg can be stored. In this  */
    /* case the CallLeg is in state RVSIP_CALL_LEG_STATE_IDLE   */
    rv = RvSipCallLegGetCurrentState(pSubs->hCallLeg,&eCallState);
    if (rv != RV_OK || eCallState != RVSIP_CALL_LEG_STATE_IDLE)
    {
        SubsUnLockAPI(pSubs); /*unlock the subs */
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsStoreActiveSubsExt - Cannot store subs 0x%p inside a CallLeg 0x%p (State=%d)",
              pSubs,pSubs->hCallLeg,eCallState));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SipSubsStoreActiveSubs(hSubs,RV_TRUE, buffLen, memBuff, (RvInt32*)&subsLen);
    *pUsedBuffLen = subsLen;
    SubsUnLockAPI(pSubs); /*unlock the subs */
    return rv;
}


/***************************************************************************
 * RvSipSubsRestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Restore all subscription information into a given subscription.
 *          The subscription will assume the active state and all subscription
 *          parameters will be initialized from the given buffer.
 *
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSubs   - Handle to the restored active subscription.
 *          memBuff - The buffer that stores all the subscription information
 *          buffLen - The buffer size
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSubsRestoreActiveSubs(
                                       IN RvSipSubsHandle    hSubs,
                                       IN void              *memBuff,
                                       IN RvUint32           buffLen)
{
    RvStatus       rv;
    Subscription  *pSubs = (Subscription*)hSubs;

    RV_UNUSED_ARG(buffLen);

    if(pSubs == NULL || memBuff == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /*try to lock the subs*/
    rv = SubsLockAPI(pSubs);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRestoreActiveSubs - Restores information for subs 0x%p",
              pSubs));

    rv = SubsRestoreActiveSubs(memBuff,buffLen,RV_TRUE, pSubs);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "RvSipSubsRestoreActiveSubs - Failed, CallLegMgr failed to create a new call"));
        SubsUnLockAPI(pSubs); /*unlock the subs*/
        return rv;
    }

    SubsUnLockAPI(pSubs); /*unlock the subs*/
    return RV_OK;
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* IsSubsDialogEstablished
* ------------------------------------------------------------------------
* General: Checks if the subscription dialog is already an established dialog.
*          (established dialog means it has a to-tag)
*          if so, we can send SUBSCRIBE/REFER request on this dialog.
*          The function is relevant only for subscription inside a call-leg.
* Return Value: RV_TRUE - .
*               RV_FALSE - before early dialog.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription.
***************************************************************************/
static RvBool IsSubsDialogEstablished(Subscription* pSubs)
{
    RvBool bEstablished;

    bEstablished = IsDialogEstablished(pSubs->hCallLeg, pSubs->pMgr->pLogSrc);

    if(RV_FALSE == bEstablished)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "IsSubsDialogEstablished: subs 0x%p - dialog is not an early dialog yet", pSubs));
    }
    return bEstablished;
}

/***************************************************************************
* IsDialogEstablished
* ------------------------------------------------------------------------
* General: Checks if the subscription dialog is already an established dialog.
*          (established dialog means it has a to-tag)
*          if so, we can send SUBSCRIBE/REFER request on this dialog.
*          The function is relevant only for subscription inside a call-leg.
* Return Value: RV_TRUE - .
*               RV_FALSE - before early dialog.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hSubs   - Handle to the subscription.
***************************************************************************/
static RvBool IsDialogEstablished(RvSipCallLegHandle hCallLeg,
                                  RvLogSource*       pLogSource)
{
    RvSipPartyHeaderHandle hToHeader;
    RvStatus               rv;
    RvSipCallLegState      eDialogState;

    rv = RvSipCallLegGetCurrentState(hCallLeg, &eDialogState);

    if(RVSIP_CALL_LEG_STATE_INVITING == eDialogState)
    {
        /* state inviting - no response or provisional response was received */
        RvLogDebug(pLogSource,(pLogSource,
            "IsDialogEstablished: call-leg 0x%p - dialog state INVITING/IDLE. not early dialog yet",
            hCallLeg));
        return RV_FALSE;
    }
    else if(RVSIP_CALL_LEG_STATE_OFFERING == eDialogState ||
            RVSIP_CALL_LEG_STATE_PROCEEDING == eDialogState)
    {
        /* if we have a to-tag, received in a provisional response,
        the dialog is an early dialog. */
        rv = RvSipCallLegGetToHeader(hCallLeg, &hToHeader);
        if(rv != RV_OK)
        {
            RvLogDebug(pLogSource,(pLogSource,
                "IsDialogEstablished: call-leg 0x%p - dialog state OFFERING. no to-tag. not early dialog yet",
                hCallLeg));
            return RV_FALSE;
        }
        if(SipPartyHeaderGetTag(hToHeader) != UNDEFINED)
        {
            /* to-tag exists -> this is an early dialog */
            return RV_TRUE;
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (RVSIP_CALL_LEG_STATE_REDIRECTED == eDialogState ||
             RVSIP_CALL_LEG_STATE_UNAUTHENTICATED == eDialogState)
    {
        RvLogDebug(pLogSource,(pLogSource,
            "IsDialogEstablished: call-leg 0x%p - dialog state REDIRECTED/UNAUTHENTICATED. not early dialog yet",
            hCallLeg));
        return RV_FALSE;
    }
	
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    return RV_TRUE;
}

/***************************************************************************
* IsLegalExpiresValue
* ------------------------------------------------------------------------
* General: check if the given expires value is in the supported limit.
*          values bigger than RVSIP_SUBS_EXPIRES_VAL_LIMIT (which is the biggest signed
*          number in 32 bits) are not allowed.
* Return Value: RV_TRUE - in limit.
*               RV_FALSE - not in limit.
* ------------------------------------------------------------------------
* Arguments:
* Input:   expires   - expires value to checks.
***************************************************************************/
static RvBool IsLegalExpiresValue(RvInt32 expires)
{
    if(expires == UNDEFINED)
    {
        return RV_TRUE;
    }

    if((RvUint)expires > (RvUint)RVSIP_SUBS_EXPIRES_VAL_LIMIT)
    {
        /* the number is so big, it became negative */
        return RV_FALSE;
    }
    return RV_TRUE;
}

/******************************************************************************
 * SubsBlockTermination
 * ----------------------------------------------------------------------------
 * General: The function checks if we need to block the terminate API because
 *          the call-leg is in the middle of a callback that does not allow termination.
 * Return Value: RV_TRUE - should block the Terminate function.
 *               RV_FALSE - should not block it.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pSubs   - Handle to the subscription.
 ******************************************************************************/
static RvBool SubsBlockTermination(IN Subscription* pSubs)
{
    RvInt32 res = 0;
    res = (pSubs->cbBitmap) & (pSubs->pMgr->terminateMask); 
    if (res > 0)
    {
        /* at least one of the bits marked in the mask, are also turned on
           in the cb bitmap */
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsBlockTermination - Subs 0x%p - can not terminate while inside callback %s",
                  pSubs, SubsGetCbName(res)));
        return RV_TRUE;
    }
    return RV_FALSE;
    
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/
#endif /* #ifdef RV_SIP_SUBS_ON */

