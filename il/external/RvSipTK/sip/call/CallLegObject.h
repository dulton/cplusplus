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
 *                              <CallLegObject.h>
 *
 * This file defines the callLeg object, attributes and interface functions.
 * CallLeg represents a SIP call leg as defined in RFC 2543.
 * This means that a SIPCallLeg is defined using it's Call-Id, From and To
 * fields.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef CALL_LEG_OBJECT_H
#define CALL_LEG_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLegTypes.h"
#include "CallLegTypes.h"
#include "_SipTransaction.h"
#include "CallLegMgrObject.h"
#include "RvSipAuthenticator.h"
#include "_SipAuthenticator.h"
#include "RvSipReplacesHeader.h"
#include "_SipCommonTypes.h"

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */
    
/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

#define CALL_LEG_UPDATE_METHOD_STR			  "UPDATE"
#define CALL_LEG_STATE_ACCEPTED_STR           "Accepted"
#define CALL_LEG_STATE_CONNECTED_STR          "Connected"
#define CALL_LEG_STATE_DISCONNECTED_STR       "Disconnected"
#define CALL_LEG_STATE_DISCONNECTING_STR      "Disconnecting"
#define CALL_LEG_STATE_IDLE_STR               "Idle"
#define CALL_LEG_STATE_INVITING_STR           "Inviting"
#define CALL_LEG_STATE_PROCEEDING_STR         "Proceeding"
#define CALL_LEG_STATE_OFFERING_STR           "Offering"
#define CALL_LEG_STATE_REDIRECTED_STR         "Redirected"
#define CALL_LEG_STATE_TERMINATED_STR         "Terminated"
#define CALL_LEG_STATE_UNAUTHENTICATED_STR    "Unauthenticated"
#define CALL_LEG_STATE_CANCELLING_STR         "Cancelling"
#define CALL_LEG_STATE_CANCELLED_STR          "Cancelled"
#define CALL_LEG_STATE_MSG_SEND_FAILURE_STR   "Msg Send Failure"
#define CALL_LEG_STATE_PROCEEDING_TIMEOUT_STR "Proceeding Timeout"
#define CALL_LEG_STATE_REMOTE_ACCEPTED_STR    "RemoteAccepted"
#define CALL_LEG_STATE_UNDEFINED_STR          "Undefined"
    
#define CALL_LEG_DIRECTION_INCOMING_STR  "IN"
#define CALL_LEG_DIRECTION_OUTGOING_STR  "OUT"
#define CALL_LEG_DIRECTION_UNDEFINED_STR "UNDEFINED"
    
#define CALL_LEG_OBJECT_NAME_STR         "Call-Leg"
#define CALL_LEG_INVITE_OBJECT_NAME_STR  "Invite"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define MAX_LOOP(pcall) (pCallLeg->pMgr->maxNumOfTransc*10)

/* callLegCbName - internal enumeration of the call-leg callbacks.
   We use the enumeration value as the matching bit in the callbacks bitmap.
   the allocated bitmap is currently 32 bits */
typedef enum
{
    CB_CALL_UNDEFINED = 0,
    CB_CALL_CALL_CREATED = 1,
    CB_CALL_CALL_STATE_CHANGED,
    CB_CALL_MSG_TO_SEND,
    CB_CALL_MSG_RCVD,
    CB_CALL_MODIFY_STATE_CHANGED,
    CB_CALL_REINVITE_CREATED,
    CB_CALL_REINVITE_STATE_CHANGED,
    CB_CALL_TRANSC_CREATED,
    CB_CALL_TRANSC_STATE_CHANGED,
    CB_CALL_BYE_CREATED,
    CB_CALL_BYE_STATE_CHANGED,
    CB_CALL_REFER_STATE_CHANGED,
    CB_CALL_REFER_NOTIFY,
    CB_CALL_PRACK_STATE_CHANGED,
    CB_CALL_AUTH_CREDENTIALS_FOUND,
    CB_CALL_AUTH_COMPLETED,
    CB_CALL_SESSION_TIMER_REFRESH_ALERT,
    CB_CALL_SESSION_TIMER_NOTIFICATION,
    CB_CALL_SESSION_TIMER_NEGOTIATION_FAULT,
    CB_CALL_OTHER_URL_FOUND,
    CB_CALL_FINAL_DEST_RESOLVED,
    CB_CALL_SIGCOMP_NOT_RESPONDED,
    CB_CALL_NESTED_INITIAL_REQ,
    CB_CALL_NEW_CONN_IN_USE,
    CB_CALL_CREATED_DUE_TO_FORKING,
    CB_CALL_PROV_RESP_RCVD, /* 26 */
    /* the bitmap is 32 bits, so there is space for 6 more callbacks */
    CB_CALL_FOR_FUTUR_USE_1,
    CB_CALL_FOR_FUTUR_USE_2,
    CB_CALL_FOR_FUTUR_USE_3,
    CB_CALL_FOR_FUTUR_USE_4,
    CB_CALL_FOR_FUTUR_USE_5,
    CB_CALL_FOR_FUTUR_USE_6 /*32*/
}callLegCbName;
    

/* CallLegTranscListAction - internal enumeration. indicates the action that the function
   CallLegActOnGeneralTranscList() should perform on each transaction in the list */
typedef enum
{
    TRANSC_LIST_TERMINATE_ALL_TRANSC,
    TRANSC_LIST_REMOVE_TRANSC_FROM_LIST,
    TRANSC_LIST_FIND_PRACK_TRANSC,
    TRANSC_LIST_VERIFY_NO_ACTIVE,
    TRANSC_LIST_RESET_APP_HANDLE_ALL_TRANSC,
    TRANSC_LIST_DETACH_BYE,
    TRANSC_LIST_STOP_GENERAL_RETRANS,
    TRANSC_LIST_487_ON_GEN_PENDING
}CallLegTranscListAction;

/* CallLeg
 * --------------------------------------------------------------------------
 * CallLeg represents a SIP call leg as defined in RFC 2543.
 * This means that a CallLeg is defined using it's Call-Id, From and To
 * fields. CallLeg is a stateful object    and has a set of states associated
 * with it.
 *
 *                  invalid pointers that still points to this call-leg.
 * hPage       --   memory page used for dinamic memory allocation. for example
 *                  for callId To and From headers and other strings
 * hAppCallLeg --   Application handle to this call-leg.
 * eState      --   The call-leg state parameter indicated the state of the session
 *                  set up between two SIP user-agents. This parameter allows only a get function.
 * ePrevState  --   The previous state. this state is for msg_send_failure state. using
 *                  this state, after user calls to continueDNS function, stack knows to
 *                  which state to move back!
 * eModifyState  -- A connected call-leg can be modified with a re-Invite
 *                      request. The re-Invite process is indicated in the modify state.
 * ePrevModifyState -- The previous modify state. this state is for msg_send_failure state.
 *                  using this state, after user calls to continueDNS function, stack
 *                  knows to which state to move back!
 * hFromHeader --   The call-leg From header.
 * hToHeader   --   The call-leg To header.
 * strCallId   --   The call-leg Call-Id value.
 * hActiveTransc -- The transaction currently in charge.
 * pMgr  --         pointer the the manager this call-leg belongs to.
 * hLocalContact -- The local contact address is the address by which the caller
 *                  identifies itself to the remote party. For outgoing calls the
 *                  local contact address will usually coincide with the From address
 *                  but the application can set a different address if it wishes to.
 *                  The local contact address is inserted into the initial INVITE
 *                  request as a Contact field. For incoming calls the local contact
 *                  is taken from the INVITE message request URI field.
 * hRemoteContact - The remote contact address is the address of the remote party.
 *                  This is the address with which the remote party may be contacted.
 *                  For outgoing calls the remote contact address will usually coincide
 *                  with the To address but the application can set a different address
 *                  if it wishes to. For incoming calls the remote contact address
 *                  is the Contact address received in the received INVITE message.
 * hRemoteContactPage - a page holds the remote contact address.
 * hOriginalContact - The original contact saves the remote contact address we used
 *                  when sending the initial INVITE. we need it because the regular
 *                  remote-contact parameter might be changed when receiving 1xx, and
 *                  then, on receiving of 401/407/3xx responses, we want be able to re-send
 *                  the initial INVITE request to the original target.
 * hOriginalContactPage -
 * hRedirectAddress-The address the call should be redirected to. This is the address
 *                  taken from the contact header received in the 3xx respons.
 * localCSeq   --   A counter used to determine the CSeq of outgoing requests.
 * remoteCSeq  --   Keeps track of the last incoming request CSeq
 * eDirection  --   The direction parameter indicates whether the call-leg represents
 *                  the incoming or an outgoing side of the session.
 * hTranscList --   Handle to list of transaction handled by this call-leg.
 * hMutex      --   A lock object associated with this call-leg. Used to lock the call-leg
 *                  object before referencing it to avoid threads collisions.
 * callLegUniqueIdentifier - A unique identifier for this CallLeg.
 *                  It is generated randomly on CallLeg creation.
 *                  If it is 0, the CallLeg object is vacant.
 * pAuthListInfo -  Information regarding to the hListAuthObj.
 * hListAuthObj -   List of the structures, containing Authentication Headers,
 *                  nonceCount for their usage, e.t.c. List contains handles.
 *                  The structures and headers themselves are built
 *                  on the List's page.
 * eAuthenticationOriginState -- Saves the call-leg state before receiving 401 or 407.
 * hRouteList       -- handle to the route list. The route list is a list of addresses.
 * hRecordRoutePage -- page used for building the record route list.
 * callLegEvHandlers - pointer to the callLeg event handler struct
 * bAckRcvd         -- This boolean will turn to RV_TRUE if the call is in disconnecting
 *                     state and the previous state was accepted. the purpose of this parameter
 *                     is to save the fact that an ACK message was received, so that if the
 *                     BYE message will be rejected by the destination, the call leg will
 *                     move to state connected and not accepted.
 * localAddr        -- The local IP port and UDP socket to send messages from.
 * bIsHidden        -- RV_TRUE if this call-leg is a subscription's dialog, and not an
 *                     active call-leg.
 * bIsReferCallLeg  -- RV_TRUE if this call-leg is associated to a refer request. if this is a
 *                     refer on idle call-leg, and application chose to work with the old refer
 *                     APIs, then the incoming refer request, will create a hidden call-leg.
 *                     since we want to call call-leg callbacks (e.g msgToSend, msgRcvd), we must
 *                     indicate that this is not a regular hidden call-leg.
 * subsList         -- List of subscription objects.
 * hReplacesHeader  -- Handle to the replaces header. This is the Replaces header in the
 *                       REFER message, inside the Refer-To header.
 * statusReplaces   -- The status of the Replaces header. Required/Supported/Undefined
 * terminationInfo  -- termination inof, that helps to process out of resources when
 *                       sending object termination event
 * tripleLock       -- the call-leg triple lock that consists of object lock, API lock & processing lock
 * pSessionTimer    -- The session timer params of the call.
 * pNegotiationSessionTimer -- The session timer params of of the re-INVITE before ith has been accepted.
 * bTerminateCallLeg -- If the call leg was not terminated because there are
 *                     transactions in the call leg transaction list, we set this
 *                     parameter to true and the last transaction terminated will
 *                     terminate the call leg (this is done to make sure that the
 *                     call leg will terminate only after there are no transactions
 *                     in its list).
 * hSigCompCompartment  -- Handle to the SigComp compartment that is related to the CallLeg.
 * bSigCompEnabled      -- Indication if the application chose to stop the SigComp processing in
 * bExpectComparmentID  -- Indication if the SigComp UDVM is waiting for the relation of
 *                         the CallLeg to a Compartment.
 * new refer parameters:
 * --------------------
 * hActiveReferSubs    -- When working with configuration parameter disableRefer3515=true,
 *                        this parameter 'replace' the active transaction parameter, and
 *                        holds the active refer subscription.
 * hReferSubsGenerator -- Handle of the refer subscription that created this call-leg.
 *                        (in case that this call-leg was created as a result of accepting
 *                         a refer request).
 * hReferredByHeader   -- The referred-by header, that should be set in the first INVITE
 *                        request of this call-leg. identify that this call-leg is created
 *                        as a result of a refer request.
 * hHeadersListToSetInInitialRequest -- a list of headers, that were received in a refer
 *                         request (in the refer-to url), and should be set in the first
 *                         INVITE request of this call-leg.
 * bFirstReferExists   -- Identify if this call-leg already had a refer subscription.
 *                        (helps the subscription layer to decide for a refer subscription
 *                         whether it is the first refer subscription or not).
 *
 * new forking parameters:
 * -----------------------
 * bOriginalCall -- Defines if this is an original call-leg that sent the INVITE request,
 *                  or a forked call-leg, that was created by a second 1xx/2xx response.
 * parameters relevant for the original call-leg:
 * ==============================================
 * bForkingEnabled -- Defines whether the original call-leg (that sent the invite request)
 *                    wants to enable call-leg creation for forked 1xx/2xx responses.
 * parameters relevant for the forked call-leg:
 * ==============================================
 * hOriginalForkedCall -- handle to the original call-leg that it's second 1xx/2xx
 *                        response created this forked call-leg.
 * originalForkedCallUniqueIdentifier -- identifier of the original call-leg that it's
 *                        second 1xx/2xx response created this forked call-leg.
 *
 * hAckMsg      -- The ACK message that was created for the incoming 2xx forked response.
 * hAckTrx      -- Handle to the transmitter that responsible to send the ACK request.
 * hAckTrxPage  -- The page for the ACK transmitter.
 * forkingTimer -- a timer handle that has 2 uses (the timer is set with a different
 *                 expiration value and function for each purpose):
 *                 (1) the time-to-live of the ACK transmitter.
 *                 (2) since call-leg creation (by receiving 1xx) till receiving 2xx.
 * ackTrxTimerTimeout -- The timeout value for forkingTimer in mode (1)
 * forkedCallLegTimerTimeout -- The timeout value for forkingTimer in mode (2)
 * bForceOutboundAddr -- Forces the usage of outbound address regardless of the call-leg state
 *                       or message content.
 * bAddAuthInfoToMsg -- determines whether authentication information will be added
 *                     to outgoing messages.
 *
 *
 *
 * bInviteReSentInCB -- When 3xx/401 is received on initial invite, we want to detach the
 *                      transaction in the invite object, so it will be available for the 
 *                      next invite transaction. We send the ACK, and then call to 
 *                      change-state callback. 
 *                      we don't want to detach the transaction before the callback - so
 *                      application can get the received message. On the other hand,
 *                      after the callback, we can not know for sure that we still need
 *                      to detach it (if app re-sent the invite synchronously,
 *                      then the transaction was already detached in the connect function).
 *                      The bInviteReSentInCB parameter helps us in this decision.
 * bUseFirstRouteForInitialRequest -- Application may want to use a preloaded route header when 
 *                      sending an initial request. for this purpose, it should add the route
 *                      header to the outbound message, and to set this flag to true (using
 *                      RvSipCallLegUseFirstRouteForInitialRequest). only if this flag is
 *                      true, we shall check the outbound message for a route header,
 *                      otherwise, we will send to the remote contact...
 * -----------------------
 * version 4.5 parameters
 * -----------------------
 * receivedFrom      -- Address, from which the latest messages was received
 * hInitialAuthorization     -- Handle to the empty authorization header, to be set in
 *                            the outgoing initial request.
 * hSecAgree         -- Handle to the Security Agreement object
 * hMsgRcvdTrans     -- Stores the last transaction that received a message
 * hSecObj           -- Handle to the Security Object
 * hActiveTrx        -- Saves the active transmitter. in final-dest-resolved, the application may
 *                      get this parameter directly, and get the dest address from it.
 *                      (no need to use the transaction anymore)
 * -----------------------
 * version 5.0 parameters
 * -----------------------
 * cbBitmap          -- Bitmap that saves the callback information. 
 *                      When we enter a callback, we turn the callback matching bit on.  
 *                      When we leave a callback, we turn the callback matching bit off
 *                      This information can be used to block APIs that are forbidden to be
 *                      used in specific callbacks.  
 * pRcvdFromAddr     -- The address on which the call-leg received its last message. This field
 *                      is enabled only when there is no transaction to the call-leg, for example
 *                      in case of ACK on a successful INVITE. 
 */
typedef struct
{
    HPAGE                           hPage;
    RvSipTransportObjEventInfo      terminationInfo;
    RvSipAppCallLegHandle           hAppCallLeg;
    void                            *hashElementPtr;
    RvSipCallLegState               eState;
    RvSipCallLegState               eLastState;
    RvSipCallLegState               ePrevState;
    RvSipCallLegState               eNextState;
    RLIST_HANDLE                    hInviteObjsList;
    RvSipCallLegPrackState          ePrackState;
    RvSipPartyHeaderHandle          hFromHeader;
    RvSipPartyHeaderHandle          hToHeader;
    RvInt32                         strCallId;
    RvSipTranscHandle               hActiveTransc;
    CallLegMgr                      *pMgr;
    RvSipAddressHandle              hLocalContact;
    HPAGE                           hRemoteContactPage;
    RvSipAddressHandle              hRemoteContact;
    HPAGE                           hOriginalContactPage;
    RvSipAddressHandle              hOriginalContact;
    RvSipAddressHandle              hRedirectAddress;
	SipCSeq							localCSeq;
	SipCSeq							remoteCSeq;
    RvSipCallLegDirection           eDirection;
    RLIST_HANDLE                    hGeneralTranscList;
    SipTripleLock                   *tripleLock;
    SipTripleLock                   callTripleLock;
    RLIST_HANDLE                    hRouteList;
    HPAGE                           hListPage;
    RvSipMsgHandle                  hOutboundMsg;
    RvSipMsgHandle                  hReceivedMsg;
    RvSipCallLegEvHandlers          *callLegEvHandlers;
    RvBool                          bAckRcvd;
#ifdef RV_SIP_AUTH_ON
    AuthObjListInfo                 *pAuthListInfo;
    RLIST_HANDLE                    hListAuthObj;
    RvSipCallLegState               eAuthenticationOriginState;

#endif /* #ifdef RV_SIP_AUTH_ON */
    RvSipTranscHandle               hActiveIncomingPrack;
	SipRSeq							incomingRSeq; 
	SipCSeq							incomingRel1xxCseq;
    RvSipOtherHeaderHandle          hTimeStamp;

#ifdef RV_SIP_SUBS_ON
    /* new refer parameters*/
    RvSipSubsHandle                 hActiveReferSubs;
    RvSipSubsHandle                 hReferSubsGenerator;
    RvSipReferredByHeaderHandle     hReferredByHeader;
    RvBool                          bFirstReferExists;

    RvBool                          bIsReferCallLeg; /* is this a hidden call-leg of a refer */
    RLIST_HANDLE                    hSubsList;
#endif /* #ifdef RV_SIP_SUBS_ON */
    RvBool                          bIsHidden;

    RvSipCommonListHandle           hHeadersListToSetInInitialRequest;

    SipTransportObjLocalAddresses   localAddr;
    SipTransportOutboundAddress     outboundAddr;
    RvInt32                         strOutboundProxyHostName;
    RvSipReplacesHeaderHandle       hReplacesHeader;
    RvSipCallLegReplacesStatus      statusReplaces;

#ifdef SIP_DEBUG
    RvChar                        *pCallId;
#endif
    RvInt32                        callLegUniqueIdentifier;
    CallLegSessionTimer             *pSessionTimer;
    CallLegNegotiationSessionTimer  *pNegotiationSessionTimer;
    RvSipTransportConnectionHandle   hTcpConn;
    RvBool                          bIsPersistent;
    RvBool                          bForceOutboundAddr;
    RvBool                          bAddAuthInfoToMsg;


    RvBool                          bTerminateCallLeg;
#ifdef RV_SIGCOMP_ON
    RvBool                          bSigCompEnabled;
#endif
    /*forking handling parameters */
    RvBool                bForkingEnabled;
    RvBool                bOriginalCall;
    RvSipCallLegHandle    hOriginalForkedCall; /* the call-leg that sent the initial INVITE */
    RvInt32               originalForkedCallUniqueIdentifier;
    SipTimer              forkingTimer;
    RvInt32               forked1xxTimerTimeout;

    /* Pointer to the transaction's timers timeouts values.
       points to the structure allocated on it's page */
    RvSipTimers*          pTimers;
    RvUint16              createdRejectStatusCode;
    RvBool                bInviteReSentInCB;
    RvBool                bUseFirstRouteForInitialRequest;
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpMsgSendingParams sctpParams;
#endif
    
    /* new parameters - version 4.5 */
	SipTransportAddress            receivedFrom;
    /* patch - enable getting the destAddr from the transmitter, for ACK on 2xx */
    RvSipTransmitterHandle  hActiveTrx;
#ifdef RV_SIP_IMS_ON
    RvSipAuthorizationHeaderHandle hInitialAuthorization;
	RvSipSecAgreeHandle            hSecAgree;
	RvSipTranscHandle              hMsgRcvdTransc;
    RvSipSecObjHandle              hSecObj;
#endif /* #ifdef RV_SIP_IMS_ON */

    /* new parameters - version 5.0 */
    RvInt32                cbBitmap;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    int                   cctContext;
    SipTimer              connectTimer;   /* used for handling connection-timeout events */
    int                   URI_Cfg_Flag;   // the way to configure Req_URI
#endif
//SPIRENT_END

} CallLeg;

/* CallLegInvite
 * --------------------------------------------------------------------------
 * CallLegInvite handles invite and re-invite procedures
 *
 * hAppInvite   --   Handle to the application invite object.
 * eModifyState --   state of the re-invite state machine.
 * ePrevModifyState -- previous state of the re-invite state machine.
 * hInvitePage  --  A page for invite object parameters.
 * hToFrom2xx   --  A copy of the To header that was received in the 2xx response.
 *                  (will be set to the ACK request).
 * hTrx         --  transmitter. sends the ACK for outgoing invite.
 *                  sends the 2xx retransmissions for incoming invite.
 * timer          -- timer. define of the ack transmitter lifetime, for 
 *                   an outgoing invite. define when to retransmit the 2xx
 *                   for an incoming invite. in this case it will also define
 *                   when to terminate the call-leg if no ACK was received.
 * cseq           -- the cseq value of the invite request.
 * bFirstAck      -- outgoing invite: is it the first ACK request to be sent.
 *                   incoming invite: is it the first ACK request received or not.
 * pCallLeg       -- pointer to the call-leg, holding this invite object.
 * hInviteTransc  -- Handle of the invite transaction - when exists.
 * long2xxTimeout -- when will the call-leg be destructed, in case that ACK was
 *                   not received after 2xx retransmissions.
 * bObjSentToTerminationQueue -- Was the invite object already sent to the event queue or not.
 * hClientResponseConnBackUp -- We save the Invite connection since we might receive 
 *                   2xx retransmissions on it. it will be closed when the invite obj terminates.
 * bModifyIdleInOldInvite -- indicates if we already informed application on state Modify-Idle
 *                   or not. relevant for old invite handling only. solve this problem:
 *                   in state modify-terminate, we may need to inform on state idle (in case of
 *                   timeout, msg-send-failure), or we already informed on idle state (in
 *                   state modify-ack-sent or modify-ack-rcvd).
 * hListAuthorizationHeaders -- List of Authorization Headers  that are going
 *                   to be sent with INVITE/ACK. List contains handles.
 *                   The headers themselves are built on the List's page.
 */
typedef struct 
{
    RvSipTransportObjEventInfo      terminationInfo;
    RvSipCallLegDirection           eDirection;
    RvSipAppCallLegInviteHandle     hAppInvite;
    RvBool                          bInitialInvite;
    RvSipCallLegModifyState         eModifyState;
    RvSipCallLegModifyState         eLastModifyState;
    RvSipCallLegModifyState         ePrevModifyState;
    RvSipCallLegStateChangeReason   eTerminateReason;
    HPAGE                           hInvitePage;
    RvSipPartyHeaderHandle          hToFrom2xx;
    RvSipMsgHandle                  hAckMsg;
    RvSipTransmitterHandle          hTrx;
    SipTimer                        timer;
    SipCSeq                         cseq;
    RvBool                          bFirstAck;
    CallLeg*                        pCallLeg;
    RvSipTranscHandle               hInviteTransc;
    RvInt64                         long2xxTimeout;
    RvInt32                         numOf2xxRetransmissions;
    RvBool                          bObjSentToTerminationQueue;
    RvSipTransportConnectionHandle  hActiveConn;
    RvSipTransportConnectionHandle  hClientResponseConnBackUp;
    RvBool                          bModifyIdleInOldInvite;
#ifdef RV_SIP_AUTH_ON
    RLIST_HANDLE                    hListAuthorizationHeaders;
#endif /* #ifdef RV_SIP_AUTH_ON */
} CallLegInvite;

/*CallLegTranscMemory*/
/*--------------------------------*/
/* This structure contains the parameter we want to save in the transaction object
   for the call-leg usage 
   dontDestructRouteList - determines weather a Reject response is because the message
                           was an incoming INVITE transaction inside an INVITE transaction
                           If this is the case then the record route list is not destructed.
   pInvite - Pointer to the invite object in case of invite transaction.
*/
typedef struct 
{
    RvBool          dontDestructRouteList;
    CallLegInvite*  pInvite;
}CallLegTranscMemory;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new call in the Idle state. Sets all values to their
 *          initial values, allocates a memory page for this call.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize call-led due to a
 *                                   resources problem.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the new call-leg
 *            pMgr - pointer to the manager of this call.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInitialize(IN  CallLeg          *pCallLeg,
                                      IN  CallLegMgr       *pMgr,
                                      IN  RvSipCallLegDirection eDirection,
                                       IN  RvBool           bIsHidden);

/***************************************************************************
 * CallLegVerifyValidity
 * ------------------------------------------------------------------------
 * General: verify that the call-leg we found is in the hash, not terminated.
 * Return Value: RV_TURE - legal. RV_FALSE - not legal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr           - The manager of the new call-leg
 *          pForkedCallLeg - The call-leg
 ***************************************************************************/
RvBool RVCALLCONV CallLegVerifyValidity( IN  CallLegMgr* pMgr,
                                         IN  CallLeg*    pCallLeg,
                                         IN  RvInt32     curIdentifier);

/***************************************************************************
 * CallLegShouldRejectInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks if call-leg should reject the transaction. 
 *          This function will be called when the CSeq is out of order /
 *          when a required option is unsupported / When a Replaces header is
 *          in a request other then INVITE.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg handle
 *          hTransc  - The transaction handle
 *          responseCode - The response code
 *          eTranscState  - The transaction state.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegShouldRejectInvalidRequest(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle      hTransc,
                            OUT RvUint16                *pRejectCode,
                            OUT RvBool                 *pbCseqTooSmall);

/***************************************************************************
 * CallLegTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object and move to the terminated state.
 *          Free the resources taken by the call-leg and
 *          remove it from the manager call-leg list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 ***************************************************************************/
void RVCALLCONV CallLegTerminate(IN  CallLeg    *pCallLeg,
                                 IN RvSipCallLegStateChangeReason eReason);

/***************************************************************************
 * CallLegTerminateIfPossible
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object if possible. If there are transactions in
 *          the call leg transaction list or subscription in the call leg then the call
 *          leg will not be terminated. The last subscription / transaction in the call
 *          leg will terminate the call leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 ***************************************************************************/
void CallLegTerminateIfPossible(IN  CallLeg    *pCallLeg,
                                IN RvSipCallLegStateChangeReason eReason);

/***************************************************************************
 * CallLegRemoveFromHash
 * ------------------------------------------------------------------------
 * General: remove the call-leg from the hash table..
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to remove from hash.
 ***************************************************************************/
void RVCALLCONV CallLegRemoveFromHash(IN CallLeg* pCallLeg);

/***************************************************************************
 * CallLegDestruct
 * ------------------------------------------------------------------------
 * General: Free the call-leg resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to free.
 ***************************************************************************/
void RVCALLCONV CallLegDestruct(IN  CallLeg    *pCallLeg);

/***************************************************************************
 * CallLegGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given CallLeg object.
 *          The call-id is kept on the Call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGenerateCallId(IN  CallLeg*           pCallLeg);


/***************************************************************************
 * CallLegGetCallLegOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets the outbound details such as address, transport, host name
 *          etc., (if they were set) that the CallLeg uses when sending a
 *          request to destintaion address.
 *          For each outbound detail, if it wasn't configured before in the
 *          CallLeg, then general SIP Stack outbound values are returned
 *          If the call-leg is not using an outbound address NULL values
 *          are returned.
 *          NOTE: You must supply a valid consecutive buffer to get the
 *                outbound strings (host name and ip address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - Handle to the call-leg.
 *          sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure, containing data that is going to be
 *                           retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetCallLegOutboundDetails(
                            IN  CallLeg                         *pCallLeg,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);


/***************************************************************************
 * CallLegGetRequestURI
 * ------------------------------------------------------------------------
 * General: Retrieves the request URI from a call-leg.
 * Return Value: RV_ERROR_UNKNOWN - Can't find a request URI.
 *               RV_OK - Request URI was retrieved successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *          hTransc  - The handle to the transaction. Could be NULL.
 * Output:  phAddress - Handle to the result request URI address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRequestURI(IN  CallLeg*           pCallLeg,
                                          IN  RvSipTranscHandle  hTransc,
                                          OUT RvSipAddressHandle *phAddress,
                                          OUT RvBool*   pbSendToFirstRouteHeader);


/***************************************************************************
 * CallLegGetRemoteTargetURI
 * ------------------------------------------------------------------------
 * General: Retrieves the remote target request URI from a call-leg.
 *          If there is a remote contact it will be the request URI. If not
 *          The From / To address will be the request URI.
 * Return Value: RV_ERROR_UNKNOWN - Can't find the remote target request URI.
 *               RV_OK - Remote target request URI was retrieved successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 * Output:  phAddress - Handle to the result request URI address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRemoteTargetURI(IN  CallLeg*           pCallLeg,
                                               OUT RvSipAddressHandle *phAddress);

/***************************************************************************
 * CallLegSaveReceivedFromAddr
 * ------------------------------------------------------------------------
 * General: saves the address from which the last message was received.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  -- Handle to the call-leg.
 *            pRecvFrom -- the address from which a message was received
 ***************************************************************************/
void RVCALLCONV CallLegSaveReceivedFromAddr(
                                      IN  CallLeg*              pCallLeg,
                                      IN  SipTransportAddress*  pRecvFrom);

/***************************************************************************
 * CallLegSendRequest
 * ------------------------------------------------------------------------
 * General: Sends a request to the remote party. First the Call-leg's request
 *          URI is determined and then a request is sent using the transaction API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The request message was not
 *                           sent.
 *               RV_OK - Request message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the request.
 *            hTransc - The transaction used to send the request.
 *            eRequestMethod - The request method to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSendRequest(
                            IN CallLeg                       *pCallLeg,
                            IN RvSipTranscHandle             hTransc,
                            IN SipTransactionMethod        eRequestMethod,
                            IN RvChar*                      strMethod);
/***************************************************************************
 * CallLegDisconnectWithNoBye
 * ------------------------------------------------------------------------
 * General: This function is used to disconnect a call-leg with out sending
 *          BYE first. Usually it will be called when the disconnection is
 *          a result of an error or a time out. The terminate function should
 *          be called after this function to terminate the call-leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to disconnect with no bye.
 *            eReason - the disconnecting reason.
 * Output:(-)
 ***************************************************************************/

void RVCALLCONV CallLegDisconnectWithNoBye(
                             IN  CallLeg*                       pCallLeg,
                             IN  RvSipCallLegStateChangeReason  eReason) ;


/***************************************************************************
 * CallLegChangeState
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
void RVCALLCONV CallLegChangeState(
                                 IN  CallLeg                      *pCallLeg,
                                 IN  RvSipCallLegState             eState,
                                 IN  RvSipCallLegStateChangeReason eReason);

/***************************************************************************
 * CallLegChangeModifyState
 * ------------------------------------------------------------------------
 * General: Changes the call-leg connect sub state.
 *          The connect sub state is used for incoming and outgoing re-Invite
 *          messages.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            pInvite  - Pointer to the invite object.
 *            eSubState - The Modify state.
 *            eReason  - The state change reason.
 *            hReceivedMsg - The received message, that caused the state change.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegChangeModifyState(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  CallLegInvite                 *pInvite,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason);


/***************************************************************************
 * CallLegAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the call-leg as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus CallLegAttachOnConnection(IN CallLeg                        *pCallLeg,
                                    IN RvSipTransportConnectionHandle  hConn);

/***************************************************************************
 * CallLegDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the call-leg from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 ***************************************************************************/
void CallLegDetachFromConnection(IN CallLeg        *pCallLeg);

/***************************************************************************
 * CallLegConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn   -   The connection handle
 *          hOwner  -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV CallLegConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hCallLeg,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason);


/***************************************************************************
 * CallLegSetKey
 * ------------------------------------------------------------------------
 * General: Set the call leg key (to, from, call-id).
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - key was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            pKey - The given key.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetKey(
                            IN  CallLeg                       *pCallLeg,
                            IN  SipTransactionKey               *pKey);


/*------------------------------------------------------------------------
           S T A T E   C H C K I N G   F U N C T I O N S
 -------------------------------------------------------------------------*/
/***************************************************************************
 * CallLegIsStateLegalForReInviteHandling
 * ------------------------------------------------------------------------
 * General: Is the call-leg state is legal for handling an incoming re-invite,
 *          and for sending re-invite.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegIsStateLegalForReInviteHandling(CallLeg* pCallLeg);


/*------------------------------------------------------------------------
           T R A N S A C T I O N   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * CallLegCreateTransaction
 * ------------------------------------------------------------------------
 * General: creates a new transaction, attach the call as owner and insert
 *          the new transaction into the call-leg transactions list.
 * Return Value: RV_ERROR_UNKNOWN - If the call doesn't have all the information
 *                            needed to create the transaction for example
 *                            To header is missing or there is a general
 *                            failure when creating the transaction.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create transaction because of
 *                                   resource problem.
 *               RV_OK        - Transaction was created successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer the call-leg creating the new transaction.
 *          bAddToTranscList - Add the transaction to call-leg transactions list
 *                      or not. (subscription and notify transaction will be saved
 *                      in their objects, and not in the call-leg's list)
 * Output:     *phTransc - Handle to the newly created transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCreateTransaction(
                                       IN  CallLeg*           pCallLeg,
                                       IN  RvBool            bAddToTranscList,
                                       OUT RvSipTranscHandle  *phTransc);


/***************************************************************************
 * CallLegAddTranscToList
 * ------------------------------------------------------------------------
 * General: Adds a given transaction to the call-leg transaction list.
 * Return Value: RV_ERROR_OUTOFRESOURCES - List is full, failed to insert.
 *               RV_OK - Transaction was inserted to list successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hTransc - Handle to the transaction that should be inserted
 *                    into the list.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAddTranscToList(IN  CallLeg*           pCallLeg,
                                            IN  RvSipTranscHandle  hTransc);


/***************************************************************************
 * CallLegRemoveTranscFromList
 * ------------------------------------------------------------------------
 * General: Removes a given transaction from the call-leg transaction list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given transaction was not found
 *                                  in the call-leg transaction list.
 *               RV_ERROR_UNKNOWN - There was an infinite  loop while looking
 *                            for the transaction.
 *               RV_OK - Transaction was removed from the list
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hTransc - Handle to the transaction that should be removed
 *                    from the list.
 ***************************************************************************/
RvStatus RVCALLCONV  CallLegRemoveTranscFromList(IN  CallLeg*           pCallLeg,
                                                  IN  RvSipTranscHandle  hTransc);

/***************************************************************************
 * CallLegAllocTransactionCallMemory
 * ------------------------------------------------------------------------
 * General: Allocates memory in the transaction for call-leg usage.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The transaction handle
 * Output:   ppMemory - Pointer to the allocated memory.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAllocTransactionCallMemory(IN RvSipTranscHandle hTransc,
                                                      OUT CallLegTranscMemory** ppMemory);

/***************************************************************************
 * CallLegTerminateAllGeneralTransc
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, Terminate the transcation
 *          using the transaction API and remove it from the list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
void RVCALLCONV CallLegTerminateAllGeneralTransc(IN  CallLeg*      pCallLeg);

/***************************************************************************
 * CallLegTerminateAllInvites
 * ------------------------------------------------------------------------
 * General: Go over the call-leg invite list, Terminate the objects
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        bInformOfIdleState - Indicates if we want to inform of Idle state
 *                   (for old invite) when the invite is out of termination queue.
 ***************************************************************************/
void RVCALLCONV CallLegTerminateAllInvites(IN  CallLeg*   pCallLeg,
                                           IN  RvBool     bInformOfIdleState);


#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * CallLegLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks calleg according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
RvStatus RVCALLCONV CallLegLockAPI(IN  CallLeg*   pCallLeg);


/************************************************************************************
 * CallLegUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks call-leg according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV CallLegUnLockAPI(IN  CallLeg*   pCallLeg);


/************************************************************************************
 * CallLegLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks calleg according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
RvStatus RVCALLCONV CallLegLockMsg(IN  CallLeg*   pCallLeg);


/************************************************************************************
 * CallLegUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks calleg according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV CallLegUnLockMsg(IN  CallLeg*   pCallLeg);

#else
#define CallLegLockAPI(a) (RV_OK)
#define CallLegUnLockAPI(a)
#define CallLegLockMsg(a) (RV_OK)
#define CallLegUnLockMsg(a)
#endif

/*-----------------------------------------------------------------------
       C A L L  - L E G:  G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * CallLegSetDefaultLocalAddresses
 * ------------------------------------------------------------------------
 * General: Set the local address structure to default local addresses used
 *          by the stack.
 * Return Value: RvStatus
 *           The function will return RV_Sucssess if there will be at least
 *           one local address.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetDefaultLocalAddresses(
                                           IN  CallLeg      *pCallLeg);



/***************************************************************************
 * CallLegGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Call-Id of the call-leg.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was isdufficient.
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
RvStatus RVCALLCONV CallLegGetCallId (
                            IN  CallLeg              *pCallLeg,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize);

/***************************************************************************
 * CallLegSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the call-leg's call-id. The given call-id is copied into the
 *          call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg -  Pointer to the call-leg.
 *            strCallId - Null terminating string with the new call id.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetCallId (
                                      IN  CallLeg *     pCallLeg,
                                      IN  RPOOL_Ptr    *strCallId);



/***************************************************************************
 * CallLegSetFromHeader
 * ------------------------------------------------------------------------
 * General: Set the From header associated with the call-leg.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hFrom - Handle to an application constructed from header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetFromHeader (
                                      IN  CallLeg *               pCallLeg,
                                      IN  RvSipPartyHeaderHandle  hFrom);




/***************************************************************************
 * CallLegGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from address associated with the call. Note
 *          that the address is not copied. Instead the call-leg's internal from
 *          header is referenced. Attempting to alter the from address after
 *          call has been initiated might cause unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phFrom - Pointer to the call-leg from header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetFromHeader (
                                      IN  CallLeg *               pCallLeg,
                                      OUT RvSipPartyHeaderHandle  *phFrom);



/***************************************************************************
 * CallLegSetToHeader
 * ------------------------------------------------------------------------
 * General: Set the To address associated with the call-leg. Note
 *          that attempting to altert the To address after call has
 *             been initiated might cause unexpected results.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hTo - Handle to an application constructed to header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetToHeader (
                                      IN  CallLeg *               pCallLeg,
                                      IN  RvSipPartyHeaderHandle  hTo);



/***************************************************************************
 * CallLegGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the to address associated with the call. Note
 *          that the header is not copied. Instead the call-leg's
 *          internal to header is referenced. Attempting to altert
 *          the to address after call has been initiated might cause
 *          unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phTo - Pointer to the call-leg to header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetToHeader (
                                      IN  CallLeg *               pCallLeg,
                                      OUT RvSipPartyHeaderHandle  *phTo);



/***************************************************************************
 * CallLegGetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Get the remote party's contact address. This is the
 *          address supplied by the remote party by which it can be
 *          directly contacted in future requests. Note
 *          that the address is not copied. Instead the call-leg's
 *          internal handle is referenced. Attempting to altert
 *          this information might cause unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phContactAddress - Pointer to the call-leg Remote Contact Address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRemoteContactAddress (
                                IN  CallLeg *             pCallLeg,
                                OUT RvSipAddressHandle *  phContactAddress);



/***************************************************************************
 * CallLegSetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Set the contact address of the remote party. This is the address
 *          with which the remote party may be contacted with. This method
 *          may be used for outgoing calls where the user wishes to use a
 *          Request-URI that is different from the call-leg's To value.
 * Return Value: RV_ERROR_UNKNOWN - If the supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hContactAddress - Handle to an application constructed address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetRemoteContactAddress (
                                IN  CallLeg *       pCallLeg,
                                IN  RvSipAddressHandle hContactAddress);


/***************************************************************************
 * CallLegSetRedirectAddress
 * ------------------------------------------------------------------------
 * General: Set the redirect address. This is the address taken from a 3xx
 *          and will be used to reconnect the call-leg to a different party.
 *          This address will replace the remote contact in state redirected.
 * Return Value: RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hRedirectAddress - Handle to the redirect address
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetRedirectAddress (
                                      IN  CallLeg *          pCallLeg,
                                      IN  RvSipAddressHandle hRedirectAddress);
/***************************************************************************
 * CallLegGetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Gets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. If no value is
 *          supplied the From header's address part is taken. The
 *          local contact address is used by the remote party to
 *          directly contact the local party. Note
 *          that the address is not copied. Instead the call-leg's
 *          internal address is referenced. Attempting to altert
 *          this information might cause unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments: RV_OK
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phContactAddress - Pointer to the call-leg local Contact Address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetLocalContactAddress (
                               IN  CallLeg *             pCallLeg,
                               OUT RvSipAddressHandle *  phContactAddress);



/***************************************************************************
 * CallLegSetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Sets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. If no value is
 *          supplied the From header's address part is taken. The
 *          local contact address is used by the remote party to
 *          directly contact the local party.
 * Return Value: RV_ERROR_UNKNOWN - If the supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hContactAddress - Handle to an application constructed address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetLocalContactAddress (
                                 IN  CallLeg *             pCallLeg,
                                 IN  RvSipAddressHandle    hContactAddress);


/***************************************************************************
 * CallLegGetDirection
 * ------------------------------------------------------------------------
 * General: Returns wether the call-leg is incoming or outgoing.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     peDirection - The call-leg direction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetDirection(
                                 IN  CallLeg *             pCallLeg,
                                 OUT RvSipCallLegDirection *peDirection);



/***************************************************************************
 * CallLegGetCurrentState
 * ------------------------------------------------------------------------
 * General: Returns the current state of the call-leg.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     peState - The call-leg state.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetCurrentState (
                                      IN  CallLeg *          pCallLeg,
                                      OUT RvSipCallLegState* pState);




/***************************************************************************
 * CallLegGetTranscByMsg
 * ------------------------------------------------------------------------
 * General: Returns the transaction that this message belongs to.
 *          This function looks for the transaction in the transaction list
 *          of the call-leg and compares each transaction by its isUac,method
 *          and cseq.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hMsg     - The given message.
 *          bIsMsgRcvd - RV_TRUE if this is a received message. RV_FALSE otherwise.
 * Output:     phTransc - The transaction handle or NULL if no such transaction
 *                     exists.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetTranscByMsg (
                                 IN  CallLeg                *pCallLeg,
                                 IN  RvSipMsgHandle         hMsg,
                                  IN  RvBool                bIsMsgRcvd,
                                 OUT RvSipTranscHandle      *phTransc);



/*------------------------------------------------------------------------
          N A M E    F U N C T I O N S
 -------------------------------------------------------------------------*/

 /***************************************************************************
 * CallLegGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state. (This function is used in
 *          the High Availability module in addition to printing ability.)
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetStateName(
                                      IN  RvSipCallLegState  eState);

/***************************************************************************
 * CallLegGetStateEnum
 * ------------------------------------------------------------------------
 * General: Returns the state enum according to given state name string
 * Return Value: The the state enum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strState - The state name string.
 ***************************************************************************/
RvSipCallLegState RVCALLCONV CallLegGetStateEnum(IN RvChar *strState);

/***************************************************************************
 * CallLegGetDirectionName
 * ------------------------------------------------------------------------
 * General: The function converts a Direction type enum to string.
 * Return Value: The suitable string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eDirection - The direction type.
 ***************************************************************************/
RvChar *CallLegGetDirectionName(RvSipCallLegDirection eDirection);

/***************************************************************************
 * CallLegGetDirectionEnum
 * ------------------------------------------------------------------------
 * General: The function converts a Direction string to enum type.
 * Return Value: The suitable enum value.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strDirection - The direction string.
 ***************************************************************************/
RvSipCallLegDirection CallLegGetDirectionEnum(RvChar *strDirection);


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * CallLegGetStateChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state change reason.
 * Return Value: The string with the state change reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The state change reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetStateChangeReasonName (
                                IN  RvSipCallLegStateChangeReason  eReason);



/***************************************************************************
 *  CallLegGetModifyStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given modify state.
 * Return Value: The string with the modify state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eModifyState - The modify state.
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetModifyStateName(
                                        IN RvSipCallLegModifyState eModifyState);

/***************************************************************************
 * CallLegGetCallLegTranscStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state that belongs to a call-leg
 *          transaction.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetCallLegTranscStateName (
                                      IN  RvSipCallLegTranscState  eState);


/***************************************************************************
 * CallLegGetPrackStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given prack state.
 * Return Value: The string with the prack state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     ePrackState - The prack state.
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetPrackStateName(
                                   IN  RvSipCallLegPrackState  ePrackState);

/***************************************************************************
 * CallLegGetCallLegByeTranscStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state that belongs to a call-leg
 *          bye transaction.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetCallLegByeTranscStateName (
                                      IN  RvSipCallLegByeState  eState);

#endif  /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */



/***************************************************************************
 * CallLegUpdateReplacesStatus
 * ------------------------------------------------------------------------
 * General: Update the replaces status of a Call-Leg from a received
 *          INVITE message or REFER message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegUpdateReplacesStatus(
                                            IN CallLeg       *pCallLeg,
                                            IN RvSipMsgHandle hMsg);
/***************************************************************************
 * CallLegResetAllInviteObj
 * ------------------------------------------------------------------------
 * General: Go over the call-leg's invite list and nullify its active transaction.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
void RVCALLCONV CallLegResetAllInviteObj(IN CallLeg *pCallLeg);

/***************************************************************************
* CallLegLockMgr
* ------------------------------------------------------------------------
* General: Locks the call-leg mgr. This function is used when changing parameters
*          inside the call-leg that influence the hash key of the call.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg    - Pointer to the call-leg structure
***************************************************************************/
void CallLegLockMgr(CallLeg *pCallLeg);

/***************************************************************************
* CallLegUnlockMgr
* ------------------------------------------------------------------------
* General: Unlocks the call-leg mgr. This function is used when changing parameters
*          inside the call-leg that influence the hash key of the call.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg    - Pointer to the call-leg structure
***************************************************************************/
void CallLegUnlockMgr(CallLeg *pCallLeg);

/***************************************************************************
* CallLegIsUpdateTransc
* ------------------------------------------------------------------------
* General: Check if the CallLeg transaction method is UPDATE
* Return Value: RV_TRUE/RV_FALSE
* ------------------------------------------------------------------------
* Arguments:
* Input:  hTransc          - Handle to the Transaction.
* Output: -
***************************************************************************/
RvBool RVCALLCONV CallLegIsUpdateTransc(IN RvSipTranscHandle hCallTransc);

void RVCALLCONV CallLegToLower(IN RvChar *str, IN RvInt32 length);


/***************************************************************************
 * CallLegSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the call-leg's transactions timers.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pCallLeg - Handle to the call-leg object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus CallLegSetTimers(IN  CallLeg*      pCallLeg,
                          IN  RvSipTimers*  pNewTimers);

/***************************************************************************
 * CallLegResetIncomingRSeq
 * ------------------------------------------------------------------------
 * General: Resets the CallLeg incoming RSeq variable
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Handle to the call-leg object.
 ***************************************************************************/
void CallLegResetIncomingRSeq(IN  CallLeg      *pCallLeg);

/***************************************************************************
 * CallLegSetIncomingRSeq
 * ------------------------------------------------------------------------
 * General: Sets the CallLeg incoming RSeq variable a new value
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Handle to the call-leg object.
 ***************************************************************************/
void CallLegSetIncomingRSeq(IN  CallLeg      *pCallLeg,
							IN  RvUint32	  rseqVal);
/***************************************************************************
 * CallLegResetActiveTransc
 * ------------------------------------------------------------------------
 * General: Reset the call-leg active transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegResetActiveTransc (IN  CallLeg  *pCallLeg);

/***************************************************************************
 * CallLegSetActiveTransc
 * ------------------------------------------------------------------------
 * General: Set the call-leg active transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetActiveTransc (IN  CallLeg  *pCallLeg,
                                            IN  RvSipTranscHandle hTransc);


/***************************************************************************
 * CallLegGetIsHidden
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE is the call-leg is hidden otherwise RV_FALSE
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegGetIsHidden(IN CallLeg  *pCallLeg); 

/***************************************************************************
 * CallLegGetIsRefer
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE is the call-leg is refer otherwise RV_FALSE
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegGetIsRefer(IN CallLeg *pCallLeg); 

/***************************************************************************
 * CallLegGetActiveReferSubs
 * ------------------------------------------------------------------------
 * General: Returns the call-leg's active refer subscription handle
 * Return Value: The active refer subs
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvSipSubsHandle RVCALLCONV CallLegGetActiveReferSubs(IN CallLeg *pCallLeg);

/***************************************************************************
 * CallLegIsInitialRequest
 * ------------------------------------------------------------------------
 * General: returns RV_TURE if this is an initial request - request with
 *          an empty to-tag.
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvBool RVCALLCONV CallLegIsInitialRequest(CallLeg* pCallLeg);

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * CallLegSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Call-Leg.
 *          As a result of this operation, all messages, sent by this Call-Leg,
 *          will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg - Pointer to the Call-Leg object.
 *          hSecObj  - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus CallLegSetSecObj(IN  CallLeg*          pCallLeg,
                          IN  RvSipSecObjHandle hSecObj);
#endif /*#ifndef RV_SIP_IMS_ON*/

/******************************************************************************
 * CallLegGetCbName
 * ----------------------------------------------------------------------------
 * General: Print the name of callback represented by the bit turned on in the 
 *          mask results.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  MaskResults - the bitmap holding the callback representing bit.
 *****************************************************************************/
RvChar* CallLegGetCbName(RvInt32 MaskResults);

/***************************************************************************
 * CallLegActOnGeneralTranscList
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, and act on the transactions
 *          as required by the eAction parameter.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTranscForCompare - Handle to a transaction. used by functions that 
 *                   need to compare the transc in the list to a specific transc.
 *        eAction  - The required action on the transactions in the list.
 * Output: phFoundTransc - The transaction handle from the list, for actions 
 *                   that require to find a transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegActOnGeneralTranscList(IN  CallLeg*   pCallLeg,
                                                  IN  RvSipTranscHandle hTranscForCompare,
                                                  IN  CallLegTranscListAction eAction,
                                                  OUT RvSipTranscHandle      *phFoundTransc);

#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef CALL_LEG_OBJECT_H*/

