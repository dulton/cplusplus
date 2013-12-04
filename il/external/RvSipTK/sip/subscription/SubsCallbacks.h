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
 *                              <SubsCallbacks.h>
 *
 * This file defines the subscription and notification objects.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 June 2002
 *********************************************************************************/

#ifndef SUBS_CALLBACKS_H
#define SUBS_CALLBACKS_H

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
#include "RvSipSubscriptionTypes.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                            SUBS CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackSubsCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnSubsCreatedEvHandler callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pMgr         - Handle to the subs manager.
*         hSubs       - The new sip stack subs handle
* Output:(-)
***************************************************************************/
void RVCALLCONV SubsCallbackSubsCreatedEv(IN  SubsMgr             *pMgr,
                                          IN  Subscription*        pSubs);

/***************************************************************************
 * SubsCallbackChangeSubsStateEv
 * ------------------------------------------------------------------------
 * General: calls to pfnStateChangedEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackChangeSubsStateEv(
                                         IN  Subscription              *pSubs,
                                         IN  RvSipSubsState             eState,
                                         IN  RvSipSubsStateChangeReason eReason) ;

/***************************************************************************
 * SubsCallbackExpirationAlertEv
 * ------------------------------------------------------------------------
 * General: calls to pfnExpirationAlertEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackExpirationAlertEv( IN  Subscription   *pSubs) ;

/***************************************************************************
 * SubsCallbackExpirationEv
 * ------------------------------------------------------------------------
 * General: calls to pfnSubsExpiredEvHandler
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the subscription.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackExpirationEv( IN  Subscription   *pSubs) ;

/*-----------------------------------------------------------------------*/
/*                            NOTIFY CALLBACKS                           */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsCallbackNotifyTranscCreatedEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNotifyCreatedEvHandler
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr    - Pointer to the subs manager
 *            pSubs   - Pointer to the subscription object.
 *          pNotify - Pointer to the notification object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsCallbackNotifyTranscCreatedEv(
                                         IN    SubsMgr*             pMgr,
                                         IN    Subscription*        pSubs,
                                         IN    Notification*        pNotify);

/***************************************************************************
 * SubsCallbackChangeNotifyStateEv
 * ------------------------------------------------------------------------
 * General: calls to PCB_UnLockBeforeCallback
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify    - Pointer to the notification.
 *            eNotifyStatus - Status of the notification object.
 *          eNotifyReason - Reason to the indicated status.
 *          hNotifyMsg    - The received notify msg.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV SubsCallbackChangeNotifyStateEv(IN  Notification          *pNotify,
                                               IN  RvSipSubsNotifyStatus eNotifyStatus,
                                               IN  RvSipSubsNotifyReason eNotifyReason,
                                               IN  RvSipMsgHandle        hNotifyMsg) ;

/******************************************************************************
 * SubsCallbackSubsCreatedDueToForkingEv
 * ----------------------------------------------------------------------------
 * General: calls to pfnNotifyForMultipleSubsEvHandler
 *
 * Return Value: RV_OK on success, error code, defined in RV_SIP_DEF.h or
 *               rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs       - Pointer to the Subscription Object.
 * Output:  pExpires    - Pointer to the memory, where Application can set
 *                      expiration value.
 *          phAppSubs   - Pointer to the memory, where Application can store
 *                      any data of sizeof(RvSipAppSubsHandle) bytes.
 *          pRejectStatus-If Application decides to terminate this Subscription
 *                      it should set the pointer to point to positive integer,
 *                      representing Error Code.
 *                      In this case Stack will respond to the forked NOTIFY
 *                      with error code and will free Subscription object and
 *                      all the Subscription related resources.
 *****************************************************************************/
RvStatus RVCALLCONV SubsCallbackSubsCreatedDueToForkingEv(
                                IN      Subscription        *pSubs,
                                IN OUT  RvInt32             *pExpires,
                                OUT     RvSipAppSubsHandle  *phAppSubs,
                                OUT     RvUint16            *pRejectStatus);

/*-----------------------------------------------------------------------*/
/*                            MESSAGE CALLBACKS                          */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackMsgRcvdEv
* ------------------------------------------------------------------------
* General: calls to pfnMsgReceivedEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - Pointer to the subscription object.
*          hNotify - handle to the notify object.
*          hAppNotify - handle to the notify app object.
*          hMsg - msg received.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackMsgRcvdEv(
                            IN  Subscription*        pSubs,
                            IN  RvSipNotifyHandle    hNotify,
                            IN  RvSipAppNotifyHandle hAppNotify,
                            IN  RvSipMsgHandle       hMsg);
/***************************************************************************
* SubsCallbackMsgToSendEv
* ------------------------------------------------------------------------
* General: calls to pfnMsgToSendEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs - Pointer to the subscription object.
*          hNotify - handle to the notify object.
*          hAppNotify - handle to the notify app object.
*          hMsg - msg to send.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackMsgToSendEv(
                            IN  Subscription*        pSubs,
                            IN  RvSipNotifyHandle    hNotify,
                            IN  RvSipAppNotifyHandle hAppNotify,
                            IN  RvSipMsgHandle       hMsg);

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                            AUTH CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackAuthCredentialsFoundEv
* ------------------------------------------------------------------------
* General: Calls to pfnAuthCredentialsFoundEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          hNotify      - handle to the notify object.
*          hAppNotify   - handle to the notify app object.
*          hAuthorization - Authorization header with credentials.
*          bCredentialsSupported - Are the credentials supported by the SIP stack or not.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackAuthCredentialsFoundEv(
                              IN  Subscription*                   pSubs,
                              IN  RvSipNotifyHandle               hNotify,
                              IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                              IN  RvBool                         bCredentialsSupported);

/***************************************************************************
* SubsCallbackAuthCompletedEv
* ------------------------------------------------------------------------
* General: calls to
* Return Value: RV_OK only pfnAuthCompletedEvHandler
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          hNotify      - handle to the notify object.
*          hAppNotify   - handle to the notify app object.
*          bAuthSucceed - RV_TRUE if we found correct authorization header,
*                         RV_FALSE if we did not.
* Output:  (-)
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackAuthCompletedEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvBool              bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
* SubsCallbackOtherURLAddressFoundEv
* ------------------------------------------------------------------------
* General: calls to pfnOtherURLAddressFoundEvHandler
* Return Value: RV_OK only.RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs        - Pointer to the subscription object.
*           hNotify      - handle to the notify object.
*           hMsg           - The message that includes the other URL address.
*            hAddress       - Handle to unsupport address to be converted.
* Output:    hSipURLAddress - Handle to the known SIP URL address.
*            bAddressResolved-Indication of a successful/failed address
*                             resolving.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackOtherURLAddressFoundEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipMsgHandle       hMsg,
                              IN  RvSipAddressHandle   hAddress,
                              OUT RvSipAddressHandle   hSipURLAddress,
                              OUT RvBool               *bAddressResolved);


/***************************************************************************
* SubsCallbackFinalDestResolvedEv
* ------------------------------------------------------------------------
* General: calls to pfnFinalDestResolvedEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs          - Pointer to the subscription object.
*           hNotify        - handle to the notify object.
*           hAppNotify     - the notify application handle.
*           hTrasnc        - the transaction handle
*           hMsg           - The message that includes the other URL address.
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackFinalDestResolvedEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipTranscHandle    hTransc,
                              IN  RvSipMsgHandle       hMsg);

/***************************************************************************
 * SubsCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: calls to pfnNewConnInUseEvHandler
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pSubs          - Pointer to the subscription object.
 *           hNotify        - handle to the notify object.
 *           hAppNotify     - the notify application handle.
 *           hConn          - The connection handle
 *           bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV SubsCallbackNewConnInUseEv(
                              IN  Subscription*        pSubs,
                              IN  RvSipNotifyHandle    hNotify,
                              IN  RvSipAppNotifyHandle hAppNotify,
                              IN  RvSipTransportConnectionHandle hConn,
                              IN  RvBool               bNewConnCreated);


#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SubsCallbackSigCompMsgNotRespondedEv
* ------------------------------------------------------------------------
* General: calls to pfnSigCompMsgNotRespondedEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSubs          - Pointer to the subscription object.
*           hNotify        - handle to the notify object.
*           hAppNotify     - the notify application handle.
*           hTrasnc        - the transaction handle
*           hAppTransc     - The transaction application handle.
*           retransNo      - The number of retransmissions of the request
*                            until now.
*           ePrevMsgComp   - The type of the previous not responded request
* Output:   peNextMsgComp  - The type of the next request to retransmit (
*                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
*                            application wishes to stop sending requests).
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackSigCompMsgNotRespondedEv(
                              IN  Subscription*                pSubs,
                              IN  RvSipNotifyHandle            hNotify,
                              IN  RvSipAppNotifyHandle         hAppNotify,
                              IN  RvSipTranscHandle            hTransc,
                              IN  RvSipAppTranscHandle         hAppTransc,
                              IN  RvInt32                      retransNo,
                              IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                              OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           REFER CALLBACKS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SubsCallbackReferNotifyReadyEv
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - what caused the decision, that a NOTIFY request
*                         should be sent
*          responseCode - .
*          hResponseMsg - .
***************************************************************************/
RvStatus RVCALLCONV SubsCallbackReferNotifyReadyEv(
                                   IN  Subscription*                    pSubs,
                                   IN  RvSipSubsReferNotifyReadyReason  eReason,
                                   IN  RvInt16                         responseCode,
                                   IN  RvSipMsgHandle                   hResponseMsg);


#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_CALLBACKS_H*/


