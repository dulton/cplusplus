
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
 *                              <CallLegSubs.c>
 *
 *  Handles SUBSCRIBE process methods.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef  RV_SIP_SUBS_ON

#include "CallLegSubs.h"
#include "CallLegMsgLoader.h"
#include "CallLegRouteList.h"
#include "RvSipEventHeader.h"
#include "_SipEventHeader.h"
#include "RvSipSubscription.h"
#include "RvSipSubscriptionTypes.h"
#include "_SipSubs.h"
#include "_SipCommonUtils.h"


/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvBool IsReferWithNoIdEventHeader(RvSipEventHeaderHandle hEvent);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegSubsAddSubscription
 * ------------------------------------------------------------------------
 * General: Add a new subscription handle to the subscription list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer the call-leg holding the new subscription.
 *          hSubs    - Handle to the subscription to be added.
 * Output:  phItem   - the position item in call-leg subcription list.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSubsAddSubscription(
                                       IN RvSipCallLegHandle  hCallLeg,
                                       IN RvSipSubsHandle     hSubs,
                                       OUT RLIST_ITEM_HANDLE  *phItem)
{
    RvStatus rv;
    RvSipSubsHandle* phSubs;
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phItem = NULL;

    if(pCallLeg->hSubsList == NULL)
    {
        /* allocate a dynamic list (and not regular rlist) for subscriptions.
           This list is allocated on the call-leg regular page. */
        pCallLeg->hSubsList =  RLIST_RPoolListConstruct(
                                    pCallLeg->pMgr->hGeneralPool,
                                    pCallLeg->hPage, sizeof(void *),
                                    pCallLeg->pMgr->pLogSrc);
        if (NULL == pCallLeg->hSubsList)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSubsAddSubscription: call-leg 0x%p, failed to allocate subsList", pCallLeg));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    rv = RLIST_InsertTail(NULL, pCallLeg->hSubsList, phItem);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSubsAddSubscription: call-leg 0x%p, failed to add a new subscription for subsList (status %d)",
            pCallLeg, rv));
        return rv;
    }

    phSubs = (RvSipSubsHandle*)*phItem;
    *phSubs = hSubs;

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegSubsAddSubscription: call-leg 0x%p, new subs 0x%p was added to list (hItem=0x%p)",
         pCallLeg, hSubs, *phItem));

    return RV_OK;
}


/***************************************************************************
 * CallLegSubsFindSubscription
 * ------------------------------------------------------------------------
 * General: Find a subscription in the subscription list, by it's Event header.
 *          If the event header is empty, we search for the default value
 *          'Event:PINT'
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg holding the subscription.
 *          hEvent   - Event of subscription.
 *          eSubsType - type of subscription (subscriber or notifier).
 * Output:     phSubs - Pointer to the Found subscription handle.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSubsFindSubscription(
                                       IN  RvSipCallLegHandle     hCallLeg,
                                       IN  RvSipEventHeaderHandle hEvent,
                                       IN  RvSipSubscriptionType  eSubsType,
                                       OUT RvSipSubsHandle        *phSubs)
{
    RvStatus            rv = RV_OK;
    RvSipSubsHandle      hTempSubs;
    RvSipEventHeaderHandle hTempEvent;
    RvSipSubscriptionType eTempType;
    RvSipSubsState       eSubsState;
    RLIST_ITEM_HANDLE    hElement;
    RvUint32            safeCounter = 0;
    RvUint32            maxLoops = 10000;
    RvBool              isEqual;
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->hSubsList == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *phSubs = NULL;

    RLIST_GetHead(NULL, pCallLeg->hSubsList, &hElement);
    while (NULL != hElement)
    {
        hTempSubs = *(RvSipSubsHandle*)hElement;
        eSubsState = SipSubsGetCurrState(hTempSubs);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSubsFindSubscription - callLeg % x, Failed to get state of subs 0x%p",
                pCallLeg, hTempSubs));
        }
        else if(RVSIP_SUBS_STATE_TERMINATED != eSubsState)/* do not handle subscriptions in state terminated */
        {
            rv = RvSipSubsGetSubsType(hTempSubs, &eTempType);
            if(eTempType == eSubsType)
            {
                rv = RvSipSubsGetEventHeader(hTempSubs, &hTempEvent);
                if(rv != RV_OK)
                {
                    RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                        "CallLegSubsFindSubscription - callLeg % x, Failed to get Event header from subs 0x%p",
                        pCallLeg, hTempSubs));
                }
                else if(hEvent == NULL &&
                       (SipSubsIsEventHeaderExist(hTempSubs) == RV_FALSE))
                {
                    *phSubs = hTempSubs;
                    return RV_OK;
                }
                else if(hEvent != NULL)
                {
                    isEqual = RvSipEventHeaderIsEqual(hEvent, hTempEvent);
                    if(isEqual == RV_TRUE)
                    {
                        *phSubs = hTempSubs;
                        return RV_OK;
                    }
                    else
                    {
                        /* if the event="refer" with no id parameter, it is compare to the first
                        refer subscription in the call-leg */
                        if(IsReferWithNoIdEventHeader(hEvent) == RV_TRUE &&
                           SipSubsIsFirstReferSubs(hTempSubs) == RV_TRUE)
                        {
                            *phSubs = hTempSubs;
                            return RV_OK;
                        }
                    }
                }
            }
        }
        RLIST_GetNext(NULL, pCallLeg->hSubsList, hElement, &hElement);
        if (safeCounter > maxLoops)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                      "CallLegSubsFindSubscription - Error in loop - number of rounds is larger than number of subscriptions"));
            return RV_ERROR_UNKNOWN;
        }
        safeCounter++;
    }

    return RV_ERROR_NOT_FOUND;
}

/***************************************************************************
* CallLegSubsRemoveSubscription
* ------------------------------------------------------------------------
* General: Removes a subscription from the subscription list.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer the call-leg holding the new subscription.
*           hItem - Position of the subscription in call-leg list.
***************************************************************************/
RvStatus RVCALLCONV CallLegSubsRemoveSubscription(
                                                  IN RvSipCallLegHandle   hCallLeg,
                                                  IN RLIST_ITEM_HANDLE    hItem)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pCallLeg->hSubsList == NULL)
    {
        return RV_ERROR_NOT_FOUND;
    }
    RvLogDebug(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
        "CallLegSubsRemoveSubscription - Call-leg 0x%p - remove subs of hItem=0x%p",
        pCallLeg, hItem));
            

    RLIST_Remove(NULL, pCallLeg->hSubsList, hItem);

    return RV_OK;
}

/***************************************************************************
 * CallLegSubsTerminateAllSubs
 * ------------------------------------------------------------------------
 * General: Go over the call-leg subscription list, Terminate the subscription
 *          and remove it from the list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
void RVCALLCONV CallLegSubsTerminateAllSubs(IN  CallLeg*   pCallLeg)
{
    RLIST_ITEM_HANDLE    pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE    pNextItem   = NULL;     /*next item*/
    RvSipSubsHandle     *phSubs;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = pCallLeg->pMgr->maxNumOfTransc*2;  /*actual loops should be maximum - maxNumOfTransc*/

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegSubsTerminateAllSubs - Call-leg 0x%p - terminating all its subscriptions."
             ,pCallLeg));

    if(pCallLeg->hSubsList == NULL)
    {
        return;
    }

    /*get the first item from the list*/
    RLIST_GetHead(NULL, pCallLeg->hSubsList, &pItem);

    phSubs = (RvSipSubsHandle*)pItem;
    

    /*go over all the transactions*/
    while(phSubs != NULL && safeCounter < maxLoops)
    {
        /* destruct the subscription and removes it from call-leg subs list*/
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegSubsTerminateAllSubs - Call-leg 0x%p - terminate subs 0x%p (hItem=0x%p)."
             ,pCallLeg, *phSubs, pItem));

        SipSubsTerminate(*phSubs);

        RLIST_GetNext(NULL,
                      pCallLeg->hSubsList,
                      pItem,
                      &pNextItem);

        pItem = pNextItem;
        phSubs = (RvSipSubsHandle*)pItem;
        safeCounter++;
    }
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSubsTerminateAllSubs - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                  pCallLeg,maxLoops));
    }
    /* we don't have to destruct the subscription list - because the list is on a page
    that is going to be free in the CallLegFree function. */
}

/***************************************************************************
* CallLegSubsIsSubsListEmpty
* ------------------------------------------------------------------------
* General: indicate if the call-leg's subscription list is empty.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer the call-leg holding the subscription list.
***************************************************************************/
RvBool RVCALLCONV CallLegSubsIsSubsListEmpty(RvSipCallLegHandle hCallLeg)
{
    RLIST_ITEM_HANDLE    hElement;
    CallLeg*    pCallLeg = (CallLeg*)hCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_TRUE;
    }
    if(pCallLeg->hSubsList == NULL)
    {
        return RV_TRUE;
    }

    RLIST_GetHead(NULL, pCallLeg->hSubsList, &hElement);
    if (NULL != hElement)
    {
        return RV_FALSE;
    }
    else
    {
        return RV_TRUE;
    }
}

/***************************************************************************
* CallLegSubsInsertCallToHash
* ------------------------------------------------------------------------
* General: Insert a call-leg to the calls hash table.
*          This function is called when the first subscribe request is sent for
*          a hidden call-leg (function SubsSubscribe()).
*          Note that the function does not check if the call-leg is in deed hidden.
*          this checking should be done outside this function.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hCallLeg - Handle to the call-leg.
*           bCheckInHash - check call-leg in the hash before inserting the
*                       new call-leg. (relevant for out-of-band subscription).
*                       If found, return failure.
***************************************************************************/
RvStatus RVCALLCONV CallLegSubsInsertCallToHash(RvSipCallLegHandle hCallLeg,
                                                 RvBool            bCheckInHash)
{
    CallLeg*              pCallLeg = (CallLeg*)hCallLeg;
    RvStatus             rv;
    SipTransactionKey     pKey;
    CallLeg*              pFoundCallLeg;

    if(pCallLeg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* check if the call-leg is already in the hash */
    if(bCheckInHash == RV_TRUE)
    {
        memset(&pKey,0,sizeof(pKey));

        pKey.hFromHeader      = pCallLeg->hFromHeader;
        pKey.hToHeader        = pCallLeg->hToHeader;
        pKey.strCallId.offset = pCallLeg->strCallId;
        pKey.strCallId.hPage  = pCallLeg->hPage;
        pKey.strCallId.hPool  = pCallLeg->pMgr->hGeneralPool;

        CallLegMgrHashFind(pCallLeg->pMgr,
                           &pKey,
                           pCallLeg->eDirection,
                           RV_FALSE, /* bOnlyEstablishedCall */
                           (RvSipCallLegHandle**)&pFoundCallLeg);
        if(pFoundCallLeg != NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSubsInsertCallToHash: call-leg 0x%p - Failed. call-leg 0x%p was found in the hash.",
                pCallLeg, pFoundCallLeg));
            return RV_ERROR_UNKNOWN;
        }

    }

    rv = CallLegMgrHashInsert(hCallLeg);
    return rv;

}

/***************************************************************************
 * CallLegSubsLoadFromRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters for dialog from incoming SUBSCRIBE request
 *          message. the loading is done only for a subscribe that creates
 *          a dialog.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegSubsLoadFromRequestRcvdMsg(
                                IN  CallLeg                *pCallLeg,
                                IN  RvSipMsgHandle          hMsg)
{
    /* SUBSCRIBE in call idle state (=hidden call-leg), load contact address to remote contact,
      the request URI to the local contact and the record route list. */
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE)
    {
        CallLegMsgLoaderLoadDialogInfo(pCallLeg, hMsg);
    }
    else
    {
        /* if not hidden call-leg - load only the remote contact */
        CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg, hMsg);
    }
}

/***************************************************************************
 * CallLegSubsLoadFromResponseRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters from incoming SUBSCRIBE response message,
 *          For example in a 3xx response the contact address should be
 *          loaded.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegSubsLoadFromResponseRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)
{
    RvUint16 responseCode = RvSipMsgGetStatusCode(hMsg);
    RvStatus rv;

    /*for 3xx response, load contact address*/
    if(responseCode >= 300 && responseCode < 400)
    {
        CallLegMsgLoaderLoadContactToRedirectAddress(pCallLeg,hMsg);
    }
    else if(responseCode >= 200 && responseCode < 300)/*2xx*/
    {
        CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
    }

    /* for 2xx response, load route-list if does not exist yet */
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE &&
       responseCode>=200 && responseCode<300 &&
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/*
 * We're going to override pre-existing route set regardless of whether it has anything because of the
 * required actions in the RFC 3261 session 12.1.2. quoted below: 
 *   "The route set MUST be set to the list of URIs in the Record-Route header field from the response, 
 * taken in reverse order and preserving all URI parameters. If no Record-Route header field is present 
 * in the response, the route set MUST be set to the empty set. This route set, even if empty, overrides 
 * any pre-existing route set for future requests in this dialog."
 */
       RV_TRUE)
#else
       CallLegRouteListIsEmpty(pCallLeg))
#endif
/* SPIRENT_END */   
    {
        rv = CallLegRouteListInitialize(pCallLeg,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSubsLoadFromResponseRcvdMsg - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
        }
    }

}

/***************************************************************************
 * CallLegSubsLoadFromNotifyRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters for dialog from incoming NOTIFY request
 *          message, related to a subscription.
 *          if this NOTIFY request is received before 2xx on SUBSCRIBE was received,
 *          we load the dialog info from this request, and not from 2xx response.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hTransc  - Handle to the NOTIFY transaction.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegSubsLoadFromNotifyRequestRcvdMsg(
                                IN  CallLeg                *pCallLeg,
                                IN  RvSipTranscHandle       hTransc,
                                IN  RvSipMsgHandle          hMsg)
{
    RvSipNotifyHandle   hNotify;
    RvSipSubsState      eSubsState;
    RvStatus            rv;


    hNotify = (RvSipNotifyHandle)SipTransactionGetSubsInfo(hTransc);
    if (hNotify == NULL)
    {
        return;
    }

    /*load the contact address to the remote contact field*/
    CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);

    if(RVSIP_CALL_LEG_STATE_IDLE != pCallLeg->eState)
    {
        /* The message does not initiate a dialog, if the call-leg is not in idle state */
        return;
    }

    /* Route List from NOTIFY should be loaded into Call-Leg in two cases:
        1. NOTIFY is received before 2XX on initial SUBSCRIBE
        2. NOTIFY is received as a response to the forked SUBSCRIBE.
           In this case NOTIFY can the first message, received
           from the NOTIFY sender. That is why existing in Call-Leg Route List
           should be destructed (it is not valid anymore), and the list from
           the NOTIFY should be loaded, if exist.
    */
    eSubsState = SipSubsGetSubscriptionStateOfNotifyObj(hNotify);
    if((eSubsState == RVSIP_SUBS_STATE_NOTIFY_BEFORE_2XX_RCVD &&
        pCallLeg->eState == RVSIP_CALL_LEG_STATE_IDLE &&

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/*
 * We're going to override pre-existing route set regardless of whether it has anything because of the
 * required actions in the RFC 3261 session 12.1.2. quoted below: 
 *   "The route set MUST be set to the list of URIs in the Record-Route header field from the response, 
 * taken in reverse order and preserving all URI parameters. If no Record-Route header field is present 
 * in the response, the route set MUST be set to the empty set. This route set, even if empty, overrides 
 * any pre-existing route set for future requests in this dialog."
 */
        RV_TRUE)
#else
        CallLegRouteListIsEmpty(pCallLeg))
#endif
/* SPIRENT_END */   

       ||
       (eSubsState == RVSIP_SUBS_STATE_2XX_RCVD &&
        pCallLeg->bOriginalCall == RV_FALSE) /*bOriginalCall is RV_FALSE for forked Subscriptions only */
      )
    {
        rv = CallLegRouteListInitialize(pCallLeg,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSubsLoadFromNotifyRequestRcvdMsg - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
        }
    }
}


/***************************************************************************
 * CallLegSubsLoadFromNotifyResponseRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters from incoming NOTIFY response message,
 *          For example in a 3xx response the contact address should be
 *          loaded.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hMsg - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegSubsLoadFromNotifyResponseRcvdMsg(
                        IN  CallLeg                 *pCallLeg,
                        IN  RvSipMsgHandle          hMsg)
{
    RvUint16 responseCode = RvSipMsgGetStatusCode(hMsg);

    /*for 3xx response, load contact address*/
    if(responseCode >= 300 && responseCode < 400)
    {
        CallLegMsgLoaderLoadContactToRedirectAddress(pCallLeg,hMsg);
    }
    else if(responseCode >= 200 && responseCode < 300)/*2xx*/
    {
        CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);
    }
}

/***************************************************************************
* CallLegSubsGetSubscription
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
RvStatus RVCALLCONV CallLegSubsGetSubscription (
                                            IN  CallLeg*               pCallLeg,
                                            IN  RvSipListLocation      location,
                                            IN  RvSipSubsHandle        hRelative,
                                            OUT RvSipSubsHandle        *phSubs)
{
    RLIST_ITEM_HANDLE       listElem;

    *phSubs = NULL;

    switch(location)
    {
    case RVSIP_FIRST_ELEMENT:
        {
            RLIST_GetHead (NULL, pCallLeg->hSubsList, &listElem);
            break;
        }
    case RVSIP_LAST_ELEMENT:
        {
            RLIST_GetTail (NULL, pCallLeg->hSubsList, &listElem);
            break;
        }
    case RVSIP_NEXT_ELEMENT:
        {
            RLIST_GetNext (NULL, pCallLeg->hSubsList,SipSubsGetListItemHandle(hRelative), &listElem);
            break;

        }
    case RVSIP_PREV_ELEMENT:
        {
            RLIST_GetPrev (NULL, pCallLeg->hSubsList, SipSubsGetListItemHandle(hRelative), &listElem);
            break;
        }
    default:
        return RV_ERROR_UNKNOWN;
    }

    if(listElem != NULL)
    {
        *phSubs = *(RvSipSubsHandle*)listElem;
    }
     else
    {
        *phSubs = NULL;
    }

    return RV_OK;
}

/***************************************************************************
* CallLegSubsGetDialogHandle
* ------------------------------------------------------------------------
* General: Gets a subscription dialog handle.
* Return Value: dialog handle.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - The subscription handle.
***************************************************************************/
RvSipCallLegHandle RVCALLCONV CallLegSubsGetDialogHandle(IN  RvSipSubsHandle hSubs)
{
    RvSipCallLegHandle hDialogCallLeg = NULL;

    RvSipSubsGetDialogObj(hSubs, &hDialogCallLeg);
    return hDialogCallLeg;
}

/***************************************************************************
* CallLegSubsTerminateAllReferSubs
* ------------------------------------------------------------------------
* General: The function checks all call-leg subscriptions, and terminate all
*          refer subscriptions.
* Return Value: true or false.
* ------------------------------------------------------------------------
* Arguments:
* Input: pCallLeg - The Call-leg.
***************************************************************************/
void RVCALLCONV CallLegSubsTerminateAllReferSubs(IN  CallLeg* pCallLeg)
{
    RvSipSubsEventPackageType ePackType = RVSIP_SUBS_EVENT_PACKAGE_TYPE_UNDEFINED;
    RvSipSubsHandle hSubs = NULL;

    CallLegSubsGetSubscription(pCallLeg, RVSIP_FIRST_ELEMENT, hSubs, &hSubs);
    while(hSubs != NULL)
    {
        RvSipSubsGetEventPackageType(hSubs, &ePackType);

        if(ePackType == RVSIP_SUBS_EVENT_PACKAGE_TYPE_REFER)
        {
            if(SipSubsGetCurrState(hSubs) != RVSIP_SUBS_STATE_TERMINATED)
            {
                RvSipSubsTerminate(hSubs);
            }
        }
        CallLegSubsGetSubscription(pCallLeg, RVSIP_NEXT_ELEMENT, hSubs, &hSubs);
    }
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * IsReferWithNoIdEventHeader
 * ------------------------------------------------------------------------
 * General: Check if a given event header is the header "event:refer"
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hEvent - Handle to the event header.
 ***************************************************************************/
static RvBool IsReferWithNoIdEventHeader(RvSipEventHeaderHandle hEvent)
{
    RvChar packageBuff[7];
    RvUint32 len = 7;
    RvStatus rv;

    if(SipEventHeaderGetEventId(hEvent) != UNDEFINED ||
       SipEventHeaderGetEventTemplate(hEvent) != UNDEFINED)
    {
        /* id or template exist for this event header */
        return RV_FALSE;
    }

     rv = RvSipEventHeaderGetEventPackage(hEvent, packageBuff, len, &len);
     if(rv != RV_OK)
     {
         return RV_FALSE;
     }

     if(RV_TRUE == SipEventHeaderIsReferEventPackage(hEvent))
     {
         return RV_TRUE;
     }

     return RV_FALSE;
}

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* ifndef RV_SIP_PRIMITIVES */



