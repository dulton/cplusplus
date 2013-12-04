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
 *                              <SubsRefer.h>
 *
 * This file contains implementation for the refer subscription object.
 * .
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 October 2003
 *********************************************************************************/

#ifndef SUBS_REFER_H
#define SUBS_REFER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON

#include "SubsMgrObject.h"
#include "AdsRlist.h"
#include "_SipCommonTypes.h"
#include "_SipTransportTypes.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsReferInit
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with mandatory parameters:
 *          refer-to and referred-by headers of the refer subscription.
 *          If the subscription was not created inside of a call-leg you must
 *          call RvSipSubsDialogInitStr() before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request.
 *          You should call RvSipSubsRefer() in order to send a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - Handle to the subscription the user wishes to initialize.
 *          strReferTo    - String of the Refer-To header value.
 *                          for Example: "A<sip:176.56.23.4:4444;method=INVITE>"
 *          strReferredBy - String of the Referred-By header value.
 *                          for Example: "<sip:176.56.23.4:4444>"
 *          strReplaces  - The Replaces header to be set in the Refer-To header of
 *                         the REFER request. The Replaces header string doesn't
 *                           contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                           the subscription object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferInit(
                         IN  RvSipSubsHandle             hSubs,
                         IN  RvSipReferToHeaderHandle    hReferToHeader,
                         IN  RvSipReferredByHeaderHandle hReferredByHeader,
                         IN  RvChar*                    strReferTo,
                         IN  RvChar*                    strReferredBy,
                         IN  RvChar*                    strReplaces);
/***************************************************************************
 * SubsReferSetInitialReferParams
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with its mandatory parameters:
 *          refer-to and referred-by headers.
 *          If the subscription was not created inside of a call-leg you must
 *          initiate its dialog parameters first, by calling RvSipSubsDialogInit()
 *          before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request. Call RvSipSubsRefer() for sending a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES   - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR       - Bad pointer was given by the application.
 *               RV_OK          - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs       - Handle to the subscription the user wishes to initialize.
 *          hReferTo    - Handle to a refer-to header to set in the subscription.
 *          hReferredBy - Handle to a referred-by header to set in the subscription.
 ***************************************************************************/
RvStatus SubsReferSetInitialReferParams(
                                 IN  Subscription*               pSubs,
                                 IN  RvSipReferToHeaderHandle    hReferTo,
                                 IN  RvSipReferredByHeaderHandle hReferredBy);

/***************************************************************************
* SubsIncomingReferInitialize
* ------------------------------------------------------------------------
* General: The function init a refer subscription, that was created as a result
*          of an incoming REFER request.
*          The function allocate a ReferInfo on a subscription page, and initialize a
*          it's parameters.
* Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize subscription due to a
*                                   resources problem.
*               RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr - pointer the the subscription manager.
*           pSubs - pointer to the subscription.
*           hTransc - Handle to the refer transaction.
*           cseq - the cseq step of the transaction.
*            hMsg  - handle to the received message.
***************************************************************************/
RvStatus RVCALLCONV SubsReferIncomingReferInitialize(
                                         IN SubsMgr             *pMgr,
                                         IN Subscription        *pSubs,
                                         IN RvSipTranscHandle   hTransc,
                                         IN RvSipMsgHandle      hMsg);

/***************************************************************************
 * SubsReferSetReferToHeader
 * ------------------------------------------------------------------------
 * General: Sets the Refer-To header of the subscription.
 * Return Value: RV_ERROR_BADPARAM - If the supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Refer-To Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription.
 *          hReferTo   - Handle to an application constructed header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReferToHeader(IN Subscription*            pSubs,
                                               IN RvSipReferToHeaderHandle hReferTo);

/***************************************************************************
 * SubsReferSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Sets the Referred-By header of the subscription.
 * Return Value: RV_ERROR_BADPARAM - If the supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription.
 *          hReferredBy - Handle to an application constructed header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReferredByHeader(IN Subscription*               pSubs,
                                                  IN RvSipReferredByHeaderHandle hReferredBy);

/***************************************************************************
 * SubsReferSetReplacesInReferToHeader
 * ------------------------------------------------------------------------
 * General: Sets the Replaces header in the Refer-To header of the call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg       - Pointer to the call-leg.
 *            strReplaces    - The Replaces header value as string to set in the Refer-
 *                             To header of the subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReplacesInReferToHeader(IN Subscription *pSubs,
                                                         IN RvChar      *strReplaces);

/***************************************************************************
 * SubsReferDecideOnReferRcvdReason
 * ------------------------------------------------------------------------
 * General: decide whether use RVSIP_SUBS_REASON_REFER_RCVD reason or
 *          RVSIP_SUBS_REASON_REFER_RCVD_WITH_REPLACES when telling application
 *          that a new refer message was received.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the subscription.
 *          hMsg  - Handle to incoming refer message.
 ***************************************************************************/
RvSipSubsStateChangeReason RVCALLCONV SubsReferDecideOnReferRcvdReason(
                                                  IN Subscription*     pSubs,
                                                  IN RvSipMsgHandle    hMsg);

/***************************************************************************
 * SubsReferCheckReferRequestValidity
 * ------------------------------------------------------------------------
 * General: check the validity of a refer request
 *          a message is valid if it has exactly 1 refer-to header,
 *          and 0-1 referred-by header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the subscription.
 *          hMsg  - Handle to incoming refer message.
 * Output:  pResponseCode - response code for rejecting the message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferCheckReferRequestValidity(
                                                  IN Subscription*     pSubs,
                                                  IN RvSipMsgHandle    hMsg,
                                                  OUT RvUint16*       pResponseCode);

/***************************************************************************
 * SubsReferSetRequestParamsInMsg
 * ------------------------------------------------------------------------
 * General: The function sets the refer-to and referred-by headers in the
 *          refer request.
 *          The function also updates the event id of the refer subscription.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the subscription.
 *          hMsg  - Handle to incoming refer message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetRequestParamsInMsg(
                                              IN Subscription*        pSubs,
                                              IN RvSipTranscHandle    hTransc,
                                              IN RvSipMsgHandle       hMsg);

/***************************************************************************
 * SubsReferAttachNewReferResultObject
 * ------------------------------------------------------------------------
 * General: Accepting of the REFER request, creates a new object associated
 *          with the refer subscription.
 *          The type of the new object is according to the method parameter
 *          in the URL of the Refer-To header.
 *          if method=SUBSCRIBE or method=REFER - a new subscription will be created.
 *          if method=INVITE or no mehtod parameter exists in the Refer-To URL -
 *          a new call-leg will be created.
 *          for all other methods - a new transaction object will be created.
 *          This function associate only dialog-creating objects (call-leg or
 *          subscription), to the refer subscription. (meaning that the notifyReadyEv
 *          will be called only for new call-leg and subscriptions).
 *          application should check the method parameter, in order to know which
 *          application object to allocate, and which handle type to supply
 *          this function as an output parameter.
 *          The function sets the following paramateres to the dialog of the new
 *          object :
 *          Call-Id: The Call-Id of the REFER request,
 *          To:      The Refer-To header of the REFER request,
 *          From:    The local contact,
 *          Referred-By: The Referred-By header of the RFEER request.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription the user wishes to accept.
 *          hAppNewObj - Handle to the new application object. (RvSipAppCallLegHandle
 *                       in case of method=INVITE or no method parameter,
 *                       RvSipAppSubsHandle in case of method=SUBSCRIBE /REFER)
 *          eForceObjType - In case application wants to force the stack to create
 *                       a specific object type (e.g. create a transaction and
 *                       not call-leg for method=INVITE in the refer-to url) it
 *                       can set the type in this argument. otherwise it should be NULL.
 * Output:  pCreatedObjType - The type of object that was created by stack.
 *                       if application gave eForceObjType parameter, it will be
 *                       equal to eForceObjType. Otherwise it will be according
 *                       to the method parameter in the refer-to url.
 *          phNewObj   - Handle to the new object that was created by SIP Stack.
 *                       (RvSipCallLegHandle in case of method=INVITE or no
 *                       method parameter,
 *                       RvSipSubsHandle in case of method=SUBSCRIBE/REFER)
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAttachNewReferResultObject (
                            IN    Subscription*              pSubs,
                            IN    void*                       hAppNewObj,
                            IN    RvSipCommonStackObjectType  eForceObjType,
                            OUT   RvSipCommonStackObjectType *pCreatedObjType,
                            OUT   void*                      *phNewObj);

/***************************************************************************
 * RvSipNotifySetReferInfoInMsg
 * ------------------------------------------------------------------------
 * General: build the body for a notify request of a refer subscription.
 *          The function sets the notify outbound message, with correct
 *          body and content-type header.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_UNKNOWN       -  Failed to set the notify body.
 *               RV_OK       -  set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify    - Handle to the notification object.
 *          statusCode - status code to be set in NOTIFY message body.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetNotifyBody(
                                            IN  Notification*   pNotify,
                                            IN  RvInt16        statusCode);

/***************************************************************************
 * SubsReferAttachResultObjToReferSubsGenerator
 * ------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAttachResultObjToReferSubsGenerator (
                            IN    Subscription*              pSubs,
                            IN    RvSipCommonStackObjectType eObjType,
                            IN    void*                      pObj);

/***************************************************************************
 * SubsReferDetachReferResultObjAndReferSubsGenerator
 * ------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferDetachReferResultObjAndReferSubsGenerator (
                            IN    Subscription*              pSubs);

/***************************************************************************
* SubsReferGeneratorCallNotifyReady
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - reason for the notify ready.
*          hResponseMsg - Handle to the received response message.
****************************************************************************/
RvStatus RVCALLCONV SubsReferGeneratorCallNotifyReady(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipSubsReferNotifyReadyReason eReason,
                                   IN  RvSipMsgHandle     hResponseMsg);

/***************************************************************************
 * SubsReferLoadFromNotifyRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load the refer final status from incoming NOTIFY request message.
 *          This parameter is the response code that was received in the
 *          message body.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify - Pointer to the Notification Object;
 *            hMsg    - Handle to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferLoadFromNotifyRequestRcvdMsg(
                                IN  Notification           *pNotify,
                                IN  RvSipMsgHandle          hMsg);

/***************************************************************************
 * SubsReferAnalyseSubsReferToHeader
 * ------------------------------------------------------------------------
 * General: The function analyse the refer-to header, of a REFER request.
 *          It takes the headers string from the header's url address object,
 *          and if exists, search for replaces header in it.
 *          if replaces header was found, remove it from headers list, and
 *          save it directly in the refer-to header.
 *          at the end of the function, the headers list is saved in the
 *          subscription object for further usage. (setting the headers in
 *          the new object that will be created when accepting the refer request).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAnalyseSubsReferToHeader(Subscription* pSubs);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SubsReferGetNotifyReadyReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SubsReferGetNotifyReadyReasonName(
                               IN  RvSipSubsReferNotifyReadyReason  eReason);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_REFER_H*/


