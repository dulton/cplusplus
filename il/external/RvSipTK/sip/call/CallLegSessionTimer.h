
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
 *                              <CallLegSssionTimer.h>
 *
 *  Handles SESSION TIMER process methods which allows periodic refresh of
 *  SIP sesssion through a re-INVITE.
 *    Author                         Date
 *    ------                        ------
 *   Michal Mashiach               June 2002
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#ifndef CALL_LEG_SESSION_TIMER_H
#define CALL_LEG_SESSION_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegObject.h"

/***************************************************************************
 * CallLegSessionTimerRefresh
 * ------------------------------------------------------------------------
 * General: Causes a re-INVITE to be sent in order to refresh the session time.
 *          Can be called only in the connected
 *          state when there is no other pending re-Invite transaction.
 *          The remote-party's response to the re-INVITE will be given
 *          in the EvModifyResultRecvd callback.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Request failed due to resource problem.
 *               RV_ERROR_UNKNOWN - Failed to send re-Invite.
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg the user wishes to modify.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerRefresh (IN  CallLeg   *pCallLeg,
                                                IN  CallLegInvite* pInvite);

/***************************************************************************
* CallLegSessionTimerGeneralRefresh
* ------------------------------------------------------------------------
* General: Creates a transaction related to the call-leg and sends a
*          Request message with the given method in order to refresh the call.
*          The request will have the To, From and Call-ID of the call-leg and
*          will be sent with a corrent CSeq step. It will be record routed if needed.
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
* Output:  hTransc - The handle to the newlly created transaction
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerGeneralRefresh (
     IN  CallLeg                                        *pCallLeg,
     IN  RvChar                                        *strMethod,
     IN  RvInt32                                       sessionExpires,
     IN  RvInt32                                       minSE,
     IN  RvSipCallLegSessionTimerRefresherPreference    eRefresher,
     OUT RvSipTranscHandle                              *hTransc);

/***************************************************************************
* CallLegSessionTimerAddSessionExpiresToMsg
* ------------------------------------------------------------------------
* General: Adds a Session-Expires header to outgoing message. The header is taken
*          from the call-leg object.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     sessionExpires - the session expires that will be set in the msg.
*           eRefresherType - the refresher type that will be set in the msg.
*            hMsg           - Handle to the message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerAddSessionExpiresToMsg(
        IN  RvInt32                         sessionExpires,
        IN  RvSipSessionExpiresRefresherType eRefresherType,
        IN  RvSipMsgHandle                   hMsg);

/***************************************************************************
* CallLegSessionTimerHandleSendMsg
* ------------------------------------------------------------------------
* General:Handle session timer for INVITE and UPDATE out-going calls.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pCallLeg - Pointer to the call-leg.
*           hTransc  - The transaction that received this message.
*           hMsg     - Handle to the message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleSendMsg(
								 IN  CallLeg                *pCallLeg,
								 IN  RvSipTranscHandle	   hTransc,
								 IN  RvSipMsgHandle          hMsg);

/***************************************************************************
* CallLegSessionTimerHandleRequestRcvd
* ------------------------------------------------------------------------
* General:Handle session timer INVITE request received.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg.
*            hMsg     - Handle to the message.
*           hTransc  - The transaction that received this message.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleRequestRcvd(
                    IN  CallLeg                *pCallLeg,
                    IN  RvSipTranscHandle       hTransc,
					IN  RvSipMsgHandle			hMsg,
                    OUT RvBool                 *pbIsRejected);

/***************************************************************************
* CallLegSessionTimerHandleFinalResponseSend
* ------------------------------------------------------------------------
* General:handle the session timer in final response send
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg.
* hTransc  - handle to the transaction.
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleFinalResponseSend(
                                       IN  CallLeg           *pCallLeg,
									   IN  RvSipTranscHandle  hTransc,
                                       IN  RvSipMsgHandle     hMsg);

/***************************************************************************
* CallLegSessionTimerHandleFinalResponseRcvd
* ------------------------------------------------------------------------
* General:handle the session timer in INVITE final response received
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pCallLeg - Pointer to the call-leg.
            hMsg     - handle to the received response
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleFinalResponseRcvd(
                                          IN  CallLeg           *pCallLeg,
										  IN  RvSipTranscHandle  hTransc,
                                          IN  RvSipMsgHandle	 hMsg);

/***************************************************************************
* CallLegSessionTimerHandleTimers
* ------------------------------------------------------------------------
* General:handle the session timers
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg..
*           bMsgReceived  - is the msg received or sent
*           hTransc  - The transaction (UPDATE/INVITE).
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerHandleTimers(
                                           IN  CallLeg           *pCallLeg,
                                           IN  RvSipTranscHandle  hTransc,
										   IN  RvSipMsgHandle	  hMsg,
                                           IN  RvBool             bMsgRecieved);

/***************************************************************************
 * CallLegSessionTimerRefreshAlertTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever a the alert time to refresh the call is expires.
 *          The stack in session timer auto mode will be sent a re-INVITE
 *          which will refresh the call,in manual mode the call-back
 *          RvSipCallLegSessionTimerRefreshAlertEv will notify the application
 *          that the alert time of the session is been expires.
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The call-Leg this timer was called for.
 ***************************************************************************/
RvBool CallLegSessionTimerRefreshAlertTimeout(
                                              IN void    *pContext,
                                              IN RvTimer *timerInfo);

/***************************************************************************
 * CallLegSessionTimerSessionExpiresTimeout
 * ------------------------------------------------------------------------
 * General: Called when ever a the session time timer is expires.
 *          The stack in session timer auto mode will be sent a BYE
 *          if no other re-INVITE was received ,in manual mode the call-back
 *          pfnSessionTimerNotificationEvHandler with reason
 *          RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES
 *          will notify the application that the session time of the session
 *          is been expires.
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: timerHandle - The timer that has expired.
 *          pContext - The call-Leg this timer was called for.
 ***************************************************************************/
RvBool CallLegSessionTimerSessionExpiresTimeout(
                                                IN void    *pContext,
                                                IN RvTimer *timerInfo);
/***************************************************************************
 * CallLegSessionTimerReleaseTimer
 * ------------------------------------------------------------------------
 * General: Release the call-Leg's timer, and set it to NULL.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-Leg to release the timer from.
 ***************************************************************************/
void RVCALLCONV CallLegSessionTimerReleaseTimer(IN CallLeg *pCallLeg);
/***************************************************************************
 * CallLegSessionTimerAllocate
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the session timer object attach to
 *          the  call leg.
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg   - Handle to the call-leg.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV CallLegSessionTimerAllocate(
                                IN  CallLeg      *pCallLeg,
                                IN  RvInt32     size);

/***************************************************************************
 * CallLegSessionTimerAllocateNegotiationSessionTimer
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the temp session timer object attach to
 *          the  call leg.
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg   - Handle to the call-leg.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV CallLegSessionTimerAllocateNegotiationSessionTimer(
                                IN  CallLeg      *pCallLeg,
                                IN  RvInt32     size);
/***************************************************************************
 * CallLegSessionTimerInitialize
 * ------------------------------------------------------------------------
 * General: Initialize the session timer object in the call leg
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize due to a
 *                                   resources problem.
 *               RV_OK - initialize generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the new call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerInitialize(IN  CallLeg          *pCallLeg);
/***************************************************************************
* CallLegSessionTimerAddParamsToMsg
* ------------------------------------------------------------------------
* General: Copy the session timer parameters from the call to the msg
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - pointer to the new call-leg
*           hMsg - handle to the msg
***************************************************************************/
RvStatus RVCALLCONV CallLegSessionTimerAddParamsToMsg(
               IN  CallLeg                          *pCallLeg,
               IN  RvInt32                         sessionExpires,
               IN  RvInt32                         minSE,
               IN  RvSipSessionExpiresRefresherType eRefresherType,
               IN  RvSipMsgHandle                   hMsg);

/***************************************************************************
* CallLegSessionTimerSetMgrDefaults
* ------------------------------------------------------------------------
* General: Assigns the default values, which are taken from the CallLeg Mgr
*          structure into the session parameters.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input :  pCallLegMgr   - Pointer to the CallLeg Mgr.
*          bUpdateMinSE  - Indicates if the MinSE is updated as well.
* Output:  pSessionTimer - Pointer to the set session timer structure.
***************************************************************************/
void RVCALLCONV CallLegSessionTimerSetMgrDefaults(
                       IN  CallLegMgr                     *pCallLegMgr,
                       IN  RvBool                          bUpdateMinSE,
                       OUT CallLegNegotiationSessionTimer *pSessionTimer);

#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_SESSION_TIMER_H */


