
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
 *                              <CallLegSubs.h>
 *
 *  Handles SUBSCRIBE process methods.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef CALL_LEG_SUBS_H
#define CALL_LEG_SUBS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef  RV_SIP_SUBS_ON

#include "RvSipCallLegTypes.h"
#include "CallLegObject.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSubsFindSubscription
 * ------------------------------------------------------------------------
 * General: Find a subscription in the subscription list, by it's Event header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg - Handle to the call-leg holding the subscription.
 *          hEvent   - Event of subscription.
 *          eSubsType - type of subscription (RVSIP_SUBS_TYPE_SUBSCRIBER or RVSIP_SUBS_TYPE_NOTIFIER).
 * Output:     phSubs - Pointer to the Found subscription handle.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSubsFindSubscription(
                                       IN  RvSipCallLegHandle     hCallLeg,
                                       IN  RvSipEventHeaderHandle hEvent,
                                       IN  RvSipSubscriptionType  eSubsType,
                                       OUT RvSipSubsHandle        *phSubs);

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
void RVCALLCONV CallLegSubsTerminateAllSubs(IN  CallLeg*   pCallLeg);

/***************************************************************************
* CallLegSubsIsSubsListEmpty
* ------------------------------------------------------------------------
* General: indicate if the call-leg's subscription list is empty.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer the call-leg holding the subscription list.
***************************************************************************/
RvBool RVCALLCONV CallLegSubsIsSubsListEmpty(RvSipCallLegHandle hCallLeg);

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
                                       OUT RLIST_ITEM_HANDLE  *phItem);

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
                                                  IN RLIST_ITEM_HANDLE    hItem);

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
                                                 RvBool            bCheckInHash);

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
                                IN  RvSipMsgHandle          hMsg);

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
                        IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * CallLegSubsLoadFromNotifyRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load needed parameters for dialog from incoming NOTIFY request
 *          message, releated to a subscription.
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
                                IN  RvSipMsgHandle          hMsg);

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
                        IN  RvSipMsgHandle          hMsg);

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
                                            OUT RvSipSubsHandle        *phSubs);

/***************************************************************************
* CallLegSubsGetDialogHandle
* ------------------------------------------------------------------------
* General: Gets a subscription dialog handle.
* Return Value: dialog handle.
* ------------------------------------------------------------------------
* Arguments:
* Input: hSubs - The subscription handle.
***************************************************************************/
RvSipCallLegHandle RVCALLCONV CallLegSubsGetDialogHandle(IN  RvSipSubsHandle hSubs);

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
void RVCALLCONV CallLegSubsTerminateAllReferSubs(IN  CallLeg* pCallLeg);

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_SUBS_H */


