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
 *                              <RegClientObject.c>
 *
 * This file defines the TranscClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govring                 Aug 2006
 *********************************************************************************/

#ifndef TRANSC_CLIENT_OBJECT_H
#define TRANSC_CLIENT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipTransaction.h"
#include "TranscClientMgrObject.h"
#include "RvSipRegClientTypes.h"
#include "_SipTranscClientTypes.h"
#include "RvSipExpiresHeader.h"
#include "RvSipPartyHeader.h"

#include "RvSipAuthenticator.h"
#include "_SipCommonTypes.h"

#ifdef RV_SIP_AUTH_ON
#include "_SipAuthenticator.h"
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */


/* TranscClient
 * --------------------------------------------------------------------------
 * The TranscClient is a TRANSACTION client object. It is used as a platform for
 * both reg-client and pub-client objects. 
 * It supportes redirect and un-authenticated responses.
 *
 * pAuthListInfo -- Information regarding the hListAuthObj.
 * hListAuthObj  -- List of Authentication Data structures,
 *                  each of which contains challenge, received with 401/407 response.
 * hInitialAuthorization     -- Handle to the empty authorization header, to be set in
 *                            the outgoing initial request.
 * -----------------------
 * version 4.5 parameters
 * -----------------------
 * hSecObj           -- Handle to the Security Object
 * hSecAgree         -- A security-agreement associated with this transc-client. The transc-client will 
 *                      obtain a security-agreement with the server and maintain it via 
 *                      this security-agreement object.
 */
typedef struct
{
	RvSipTransportObjEventInfo     terminationInfo;		/*Information required for the object termination
														in case first try to send object termination
														event failed due to no enough resources */

    RvInt32							strCallId;
    HPAGE							hPage;				/*an inner memory page */
    RvBool							bWasActivated;		/*Indicates if the transcClient was once in state accepted*/
    RvSipAddressHandle              hRequestUri;		/*The Request-Uri of the ESC/registrar */
    RvSipPartyHeaderHandle          hFromHeader;		/*From header assosiated with the register-client */
    RvSipPartyHeaderHandle          hToHeader;			/*To header assosiated with the register-client */
    RvSipExpiresHeaderHandle        hExpireHeader;		/*Expires header assosiated with this transc-client */
    RvSipTranscHandle               hActiveTransc;		/*A transaction to be used to comunicate with the server */
    TranscClientMgr                 *pMgr;				/*the transc-clients manager */
	
    RvSipMsgHandle                  hReceivedMsg;		/*The last message to be received by this transaction-client
														(when relevant) */
    RvSipMsgHandle                  hOutboundMsg;		/*The next message to be sent by this transaction client
														(when relevant */
	SipTimer						alertTimer;			/*The timer to init for object expiration alert*/
	RvBool							bWasAlerted;
	
	SipTranscClientState			eState;
	SipTranscClientEvHandlers		transcClientEvHandlers;
	
	RvSipTransportConnectionEvHandlers transportEvHandler; /*Holds the transport evHandle of the client*/

	SipCSeq                         cseq;            /* The CSeq-Step associated with this transaction-client */
#ifdef RV_SIP_AUTH_ON
    AuthObjListInfo                 *pAuthListInfo;
    RLIST_HANDLE                    hListAuthObj;
#endif /* #ifdef RV_SIP_AUTH_ON */
    SipTransportObjLocalAddresses   localAddr;       /* The local IP port and UDP socket to send messages from */
    SipTransportOutboundAddress     outboundAddr;    /* */
    RvSipTransportConnectionHandle  hTcpConn;
    RvBool                          bIsPersistent;
#ifdef SIP_DEBUG
    RvChar                         *pCallId;
#endif
#ifdef RV_SIGCOMP_ON
    RvBool                            bSigCompEnabled;     /* Indication if the application chose to stop  */
                                                           /* the SigComp processing in outgoing messages  */
#endif
    /* Pointer to the transaction's timers timeouts values.
       points to the structure allocated on it's page */
    RvSipTimers*          pTimers;
    RvBool                bUseFirstRouteRequest; /*The request should use the first route*/
	
	RvSipCommonStackObjectType  eOwnerType;
	SipTranscClientOwnerHandle	hOwner;

	SipTranscClientOwnerUtils	ownerLockUtils;

	SipTripleLock               *tripleLock;     /* The lock of the owner of this transcClient */

	SipTripleLock               transcClientTripleLock; /* TranscClient object only Processing/API & object locks */

	RvInt32                     transcClientUniqueIdentifier;
                                   /* A unique identifier for this TransClient.
                                      It is generated randomly on TransClient
                                      creation. If it is 0, the TransClient
                                      object is vacant. */

#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpMsgSendingParams sctpParams;
#endif
    
#ifdef RV_SIP_IMS_ON
    RvSipAuthorizationHeaderHandle hInitialAuthorization;
#endif /*#ifdef RV_SIP_IMS_ON*/
    
	/* new parameters - version 4.5 */
#ifdef RV_SIP_IMS_ON
    RvSipSecAgreeHandle            hSecAgree;
    RvSipSecObjHandle   hSecObj;    /* Handle to the Security Object */
#endif /* #ifdef RV_SIP_IMS_ON */

#if defined(UPDATED_BY_SPIRENT)
    RvInt32              cctContext;
    RvInt32              URI_cfg;		/*The configuration flag of Request-Uri*/
#endif
} SipTranscClient;


/***************************************************************************
 * SipTranscClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new transc-client object.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   TranscClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClientMgr - pointer to the transaction-clients manager.
 *			eOwnerType		 - The type of the client (owner)
 *          pTranscClient	 - pointer to the new transaction-client object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientInitiate(
                                  IN SipTranscClientMgrHandle           hTranscClientMgr,
								  IN SipTranscClientOwnerHandle			hOwner,
								  IN RvSipCommonStackObjectType			eOwnerType,
								  IN SipTranscClient					*pTranscClient);


/***************************************************************************
 * SipTranscClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a transc-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClientMgr - pointer to the transaction-clients manager.
 *          pTranscClient - pointer to the transaction-client object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDestruct(IN SipTranscClient *pTranscClient);

/***************************************************************************
 * TranscClientGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The transc-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClientMgr - Pointer to the transaction-clients manager.
 *          hPage - The memory page to use.
 * Output:  pstrCallId - Pointer to the call-id offset
 *          ppCallId - Pointer to the call-id pointer offset (NULL when sip-debug)
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientGenerateCallId(
                                         IN  TranscClientMgr       *pTranscClientMgr,
                                         IN  HPAGE                  hPage,
                                         OUT RvInt32               *pstrCallId,
                                         OUT RvChar                **ppCallId);

/***************************************************************************
 * TranscClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Call-Id of the transc-client object.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was isdufficient.
 *               RV_ERROR_NOT_FOUND           - The transc-client does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient   - The Sip Stack handle to the transc-client.
 *			  bufSize    - The size of the application buffer for the call-id.
 * Output:    strCallId  - An application allocated buffer.
 *			  actualSize - The call-id actual size.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientGetCallId (
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvInt32					bufSize,
                            OUT RvChar					*pstrCallId,
                            OUT RvInt32					*actualSize);

/***************************************************************************
 * TranscClientSendRequest
 * ------------------------------------------------------------------------
 * General: Request with REGISTER or PUBLISH method on the active transaction of the
 *          transc-client object.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources request.
 *               RV_ERROR_UNKNOWN - failure sending the request.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The pointer to the transaction-client object.
 *            hRequestUri - The Request-Uri to send the request to.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSendRequest(
                                       IN  SipTranscClient		*pTranscClient,
                                       IN SipTransactionMethod	eMethod,
									   IN RvChar*					strMethod);

/***************************************************************************
 * TranscClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the transaction-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV TranscClientMgrLock(IN TranscClientMgr *pTranscClientMgr);


/***************************************************************************
 * TranscClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the transaction-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV TranscClientMgrUnLock(IN TranscClientMgr *pTranscClientMgr);

/***************************************************************************
 * TranscClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a transaction client object, Free the resources taken by it and
 *          remove it from the manager transc-client list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - Pointer to the transc-client to terminate.
 ***************************************************************************/
void RVCALLCONV TranscClientTerminate(IN SipTranscClient *pTranscClient,
									  IN SipTranscClientStateChangeReason eReason);

/***************************************************************************
 * TranscClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the TranscClient as the connection owner
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient    - handle to TranscClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus TranscClientAttachOnConnection(IN SipTranscClient				*pTranscClient,
                                 IN RvSipTransportConnectionHandle		hConn);


/***************************************************************************
 * TranscClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the TranscClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 ***************************************************************************/
void TranscClientDetachFromConnection(IN SipTranscClient        *pTranscClient);


/***************************************************************************
 * TranscClientConnStateChangedEv
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
RvStatus RVCALLCONV TranscClientConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hTranscClient,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason);

/***************************************************************************
 * TranscClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the transc-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the transc-client.
 *        eReason - The reason for the state change.
 *        pTranscClient - The transc-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TranscClientChangeState(
                            IN  SipTranscClientState             eNewState,
                            IN  SipTranscClientStateChangeReason eReason,
                            IN  SipTranscClient                  *pTranscClient,
							IN  RvInt16							 responseCode);


/***************************************************************************
 * TranscClientSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the Transc-client's transactions timers.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTranscClient - Handle to the Transc-client object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus TranscClientSetTimers(IN  SipTranscClient*      pTranscClient,
                            IN  RvSipTimers*  pNewTimers);

#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
* TranscClientAlertTimerHandleTimeout
* ------------------------------------------------------------------------
* General: Called when ever the alert timer expires.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:	timerHandle - The timer that has expired.
*			pContext - The transcClient this timer was called for.
***************************************************************************/
RvBool TranscClientAlertTimerHandleTimeout(IN void  *pContext,
											IN RvTimer *timerInfo);

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TranscClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transc-Client.
 *          As a result of this operation, all messages, sent by this
 *          Transc-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTranscClient - Pointer to the Transc-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TranscClientSetSecObj(IN  SipTranscClient*        pTranscClient,
                            IN  RvSipSecObjHandle hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * TranscClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV TranscClientStateIntoString(
                                            IN SipTranscClientState eNewState);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef TRANSC_CLIENT_OBJECT_H */










































