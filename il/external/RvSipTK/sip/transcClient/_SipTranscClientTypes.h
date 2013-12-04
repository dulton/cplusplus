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
 *                              <_SipTranscClientTypes.h>
 *
 * The _SipTranscClientTypes.h file contains all type definitions and call-back
 * functions type definitions for the Transaction-Client module.
 *
 * Three types of handles are defined:
 * 1.SipTranscClientMgrHandle
 * 2.SipTranscClientHandle
 * These handles are used when calling the API function.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef SIP_TRANSC_CLIENT_TYPES_H
#define SIP_TRANSC_CLIENT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipMsg.h"
#include "RvSipTransactionTypes.h"
#include "RvSipCommonTypes.h"




/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/


/*
 * RvSipTranscClientMgrHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to a transaction-clients manager instance. The manager
 * object manages all transaction-clients and its handle is needed in
 * all manager related API such as creation of new register-client.
 */
RV_DECLARE_HANDLE(SipTranscClientMgrHandle);

/*
 * SipTranscClientHandle
 * Declaration of a Transc-Client handle. Transc-Client handle is needed
 * in all transc-Client API function and is used to reference the
 * transc-Client object.
 */
RV_DECLARE_HANDLE(SipTranscClientHandle);

/*
 * SipTranscClientOwnerHandle
 * Declaration of a Transaction-Client owner handle. 
 */
RV_DECLARE_HANDLE(SipTranscClientOwnerHandle);



/* SipTranscClientState
 * ----------------------------------------------------------------------
 * SipTranscClientState represents the state of a transaction client process. The
 * state changes are reported using the SipTranscClientStateChangedEv call
 * back function together with a state change reason that may indicate to
 * the client how the new state was reached
 *
 * SIP_TRANSC_CLIENT_STATE_IDLE:    Initial state for a newly created Transc-
 *                                 Client.
 *                                 The transc-client will stay in this state
 *                                 until the Activate() method will be called.
 * SIP_TRANSC_CLIENT_STATE_ACTIVATING: Message was sent to the
 *                                     destination.
 * SIP_TRANSC_CLIENT_STATE_REDIRECTED: A 3xx class response was received in the
 *                                    ACTIVATING state. The client (pub/reg) may
 *                                    confirm the redirection by calling
 *                                    Activate() method.
 * SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED:  A 401 or 407 response was received
 *                                          in the ACTIVATING state. The
 *                                          client may try to authenticate
 *                                          the request by calling the
 *                                          Authenticate() method.
 * SIP_TRANSC_CLIENT_STATE_ACTIVATED: The request was accepted by the
 *                                    server. The Transc-Client is not terminated although the
 *                                    activation is successful
 * SIP_TRANSC_CLIENT_STATE_FAILED: A 4xx, 5xx, or 6xx (other than 401, 407) was
 *                                received in the ACTIVATING state.
 * SIP_TRANSC_CLIENT_STATE_TERMINATED: The final transc-client state. The
 *                                    transc-client resources were freed.
 */

typedef enum
{
    SIP_TRANSC_CLIENT_STATE_UNDEFINED = -1,
    SIP_TRANSC_CLIENT_STATE_IDLE,
    SIP_TRANSC_CLIENT_STATE_TERMINATED,

    SIP_TRANSC_CLIENT_STATE_ACTIVATING,
    SIP_TRANSC_CLIENT_STATE_REDIRECTED,
    SIP_TRANSC_CLIENT_STATE_UNAUTHENTICATED,
    SIP_TRANSC_CLIENT_STATE_ACTIVATED,
    SIP_TRANSC_CLIENT_STATE_FAILED,
    SIP_TRANSC_CLIENT_STATE_MSG_SEND_FAILURE
} SipTranscClientState;



/* SipTranscClientStateChangeReason
 * ------------------------------------------------------------------
 * A transc-client object has an inner state. The state changes according
 * to the state machine defined for the transc-client. Each state change
 * is explained by one of the reasons enumerated bellow.
 *
 * SIP_TRANSC_CLIENT_REASON_UNDEFINED         - The reason is undefined.
 * SIP_TRANSC_CLIENT_REASON_USER_REQUEST      - The change in state was in
 *                                             reaction to a user's request.
 * SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECEIVED  - 2xx was received.
 * SIP_TRANSC_CLIENT_REASON_RESPONSE_REDIRECTION_RECEIVED - 3XX was received
 * SIP_TRANSC_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECEIVED - 401 or 407
 *                                                             was received.
 * SIP_TRANSC_CLIENT_REASON_RESPONSE_REJECT_RECEIVED - 4xx, 5xx or 6xx other
 *                                                    than 401, 407 received.
 * SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT - The transaction had
 *                                               a timeout. This means that
 *                                               no final response was received
 */
typedef enum
{

    SIP_TRANSC_CLIENT_REASON_UNDEFINED = -1,
    SIP_TRANSC_CLIENT_REASON_USER_REQUEST,
    SIP_TRANSC_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD,
    SIP_TRANSC_CLIENT_REASON_RESPONSE_REQUEST_FAILURE_RECVD,
    SIP_TRANSC_CLIENT_REASON_RESPONSE_SERVER_FAILURE_RECVD,
    SIP_TRANSC_CLIENT_REASON_RESPONSE_GLOBAL_FAILURE_RECVD,
    SIP_TRANSC_CLIENT_REASON_UNAUTHENTICATE,
	SIP_TRANSC_CLIENT_REASON_REDIRECT_RESPONSE_RECVD,
	SIP_TRANSC_CLIENT_REASON_LOCAL_FAILURE,
    SIP_TRANSC_CLIENT_REASON_TRANSACTION_TIMEOUT,
    SIP_TRANSC_CLIENT_REASON_GIVE_UP_DNS,
    SIP_TRANSC_CLIENT_REASON_NETWORK_ERROR,
    SIP_TRANSC_CLIENT_REASON_503_RECEIVED,
    SIP_TRANSC_CLIENT_REASON_CONTINUE_DNS
} SipTranscClientStateChangeReason;
/*-----------------------------------------------------------------------
       T R A N S C  C L I E N T:  C A L L - B A C K  F U N C T I O N S
 ------------------------------------------------------------------------*/

/************************************************************************************
 * OwnerLockFunc
 * ----------------------------------------------------------------------------------
 * General: function that is used to lock the owner (usually XXXLockAPI())
 *
 * Return Value:(-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj     - A pointer to stack object
 ***********************************************************************************/
typedef RvStatus (RVCALLCONV * OwnerObjLockFunc)(IN  void* pObj);

/************************************************************************************
 * OwnerUnLockFunc
 * ----------------------------------------------------------------------------------
 * General: function that is used to unlock the owner (usually XXXUnLockAPI())
 *
 * Return Value:(-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj     - A pointer to stack object
 ***********************************************************************************/
typedef void (RVCALLCONV * OwnerObjUnLockFunc)(IN  void* pObj);

/***************************************************************************
 * SipTranscClientStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies the client of a transc-client state change.
 *          For each state change the new state is supplied and the
 *          reason for the state change is also given.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient - The SIP stack transc-client handle.
 *          hClientTranscClient - The client handle for this transc-client.
 *          eNewState - The new state of the transc-client object.
 *          eReason - The reason for the change in state.
 ***************************************************************************/
typedef void
        (RVCALLCONV * SipTranscClientStateChangedEv)(
                            IN  SipTranscClientOwnerHandle       hTranscClientOwner,
                            IN  SipTranscClientState			 eNewState,
                            IN  SipTranscClientStateChangeReason eReason,
							IN  RvInt16							 responseCode);


/***************************************************************************
 * SipTranscClientMsgToSendEv
 * ------------------------------------------------------------------------
 * General: An event indicating that a new Transc-client related outgoing
 *          message is about to be sent (not a retransmission).
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClientOwner- The handle for this transc-client owner.
 *            hMsg				- Handle to the outgoing message.
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * SipTranscClientMsgToSendEv)(
                                   IN  SipTranscClientOwnerHandle	hTranscClientOwner,
                                   IN  RvSipMsgHandle				hMsg);


/***************************************************************************
 * SipTranscClientMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: An event indicating that a new transc-client related incoming
 *          message was received (not a retransmission).
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClientOwner- The handle for this transc-client owner.
 *            hMsg				- Handle to the incoming message.
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * SipTranscClientMsgReceivedEv)(
                                    IN  SipTranscClientOwnerHandle	hTranscClientOwner,
                                    IN  RvSipMsgHandle				hMsg);


/***************************************************************************
 * SipTranscClientOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the owner client that a message needs to be sent and
 *          the destination address is a URL type that is currently not
 *          supported by the stack. The URL has to be converted to a SIP URL
 *          for the message to be sent.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient		- The sip stack register-client handle
 *            hTranscClientOwner- The  handle for this transc-client owner.
 *            hMsg				- The message that includes the other URL address.
 *            hAddress			- Handle to unsupport address to be converted.
 * Output:    hSipURLAddress	- HHandle to the SIP URL address - this is an empty
 *                               address object that the application should fill.
 *            bAddressResolved	-RV_TRUE if the SIP URL address was filled. RV_FALSE otherwise.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTranscClientOtherURLAddressFoundEv)(
                     IN  RvSipTranscHandle			hTransc,
                     IN  SipTranscClientOwnerHandle hTranscClientOwner,
                     IN  RvSipMsgHandle				hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool                *bAddressResolved);

/***************************************************************************
 * SipTranscClientFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: Notifies the client that the transc-client is about to send a 
 *          message after the destination address was resolved.
 *          This callback supplies the final message object and the transaction
 *          that is responsible for sending this message. Changes in the message 
 *          at this stage will not effect the destination address. 
 *
 * Return Value: RvStatus. If the client returns a value other then
 *               RV_OK the message will not be sent. The transaction will
 *               terminate with error and the transc-client will move to the
 *               Failed state.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transc-client.
 *            hTranscClientOwner- The handle for this transc client Owner.
 *            hTransc			- The transaction handle
 *            hMsgToSend		- The handle to the outgoing message.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTranscClientFinalDestResolvedEv)(
                      IN  RvSipTranscHandle				hTransc,
                      IN  SipTranscClientOwnerHandle	hTranscClientOwner,
                      IN  RvSipMsgHandle				hMsgToSend);

/***************************************************************************
 * SipTranscClientNewConnInUseEv
 * ------------------------------------------------------------------------
 * General:Notifies the client that the transaction is now using a new 
 *         connection. The connection can be a totally new one or a suitable 
 *         one that was found in the hash.
 *
 * Return Value: RV_OK (the returned status is ignored in the current
 *                      stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transc-client.
 *            hAppRegClient		- The handle for this transc client owner.
 *            hConn				- The connection handle
 *            bNewConnCreated	- RV_TRUE if the connection is a newly 
 *								created connection. RV_FALSE if the connection 
 *                              was found in the connection hash.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTranscClientNewConnInUseEv) (
                     IN  SipTranscClientOwnerHandle      hTranscClientOwner,
                     IN  RvSipTransportConnectionHandle	 hConn,
                     IN  RvBool                          bNewConnCreated);

/***************************************************************************
 * SipTranscClientObjExpiredAlert
 * ------------------------------------------------------------------------
 * General:Notifies the client that the transcClient is about to expire in the server side.
 *
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transc-client.
 *            hAppRegClient		- The handle for this transc client owner.
 *            hConn				- The connection handle
 *            bNewConnCreated	- RV_TRUE if the connection is a newly 
 *								created connection. RV_FALSE if the connection 
 *                              was found in the connection hash.
 ***************************************************************************/
typedef void (RVCALLCONV *SipTranscClientObjExpiredAlert) (
                     IN  SipTranscClientOwnerHandle      hTranscClientOwner);


/***************************************************************************
 * SipTranscClientObjTerminatedEv
 * ------------------------------------------------------------------------
 * General:Notifies the owner about termination of the transcClient.
 *
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClientOwner     - The Owner handle.
 ***************************************************************************/
typedef void (RVCALLCONV *SipTranscClientObjTerminatedEv) (
                     IN  SipTranscClientOwnerHandle      hTranscClientOwner);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTranscClientSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: Notifies the client that the timer of the transaction 
 *          responsible for sending the request has expired and that the
 *          SIP Stack has not yet received a response to the SigComp compressed 
 *          request.
 *          This callback is called in the following cases:
 *              - Before there is a need to retransmit a compressed message. 
 *                This happens when the transaction timer expires and, 
 *                according to RFC 3261, the SIP Stack needs to retransmit the message.
 *              - When the sigCompTcpTimer expires. The SIP Stack added a
 *                timer for SigComp TCP transmissions. When this timer expires
 *                the client can send the message again in a plain format.
 *                (According to RFC 3261, there are no retransmission in TCP).
 *          This callback is part of a recovery mechanism. The callback supplies
 *          information about compression type that was used in the last transmission. 
 *          The callback also gives the client the ability to determine if 
 *          there will be additional retransmissions, and the compression type of 
 *          the next retransmission.
 *          For example, the client can decide that the next retransmission 
 *          will not be compressed.
 *          Note: If you set the peNextMsgComp to
 *                RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED, the SIP Stack
 *                will stop the message retransmissions.
 *
 * Return Value: RvStatus. If the client returns a value other then
 *               RV_OK no further messages will be sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient			- Handle to the transc client.
 *            hTranscClientOwner	- The handle for this transc-client owner.
 *            hTransc				- The transaction handle.
 *            retransNo				- The number of retransmissions of the request
 *									until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            client wishes to stop sending requests).
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTranscClientSigCompMsgNotRespondedEv) (
                     IN  RvSipTranscHandle				hTransc,
					 IN  SipTranscClientOwnerHandle		hTranscClientOwner,
                     IN  RvInt32						retransNo,
                     IN  RvSipTransmitterMsgCompType	ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType	*peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/* SipTranscClientEvHandlers
 * ------------------------------------------------------------------------
 * Structure with function pointers to the module's call-backs.
 * This structure is used to set the client call-backs in the function
 * SipTranscClientSetEvHandler();
 *
 * pfnStateChangedEvHandler         - notify of transc-client state change.
 * pfnMsgToSendEvHandler            - notify that a message is about to be sent.
 * pfnMsgReceivedEvHandler          - notify that a message was received.
 * pfnOtherURLAddressFoundEvHandler - notify that abs url address was found and
 *                                      has to be converted to SIP URL address.
 * pfnFinalDestResolvedEvHandler    - notifies that the final destination address
 *                                    of the request was resolved.
 * pfnFinalDestResolvedEvHandler    - Notifies that the transc-client is about 
 *                                    to send a message after the destination
 *                                    address was resolved. This callback supplies 
 *                                    the final message object and the transaction
 *                                    that is responsible for sending this message. 
 *                                    The client can use the transaction API 
 *                                    to get the destination address.
 * pfnNewConnInUseEvHandler         - Notifies that the transc-client is now 
 *                                    using a new connection. The connection can
 *                                    be a totally new connection or a suitable 
 *                                    connection that was found in the hash.
 * pfnSigCompMsgNotRespondedEvHandler - Notifies that a timer expired before the 
 *                                      SIP Stack received a response to a 
 *                                      SigComp compressed request. This callback 
 *                                      is part of a recovery mechanism
 *                                      and it supplies information about the sent 
 *                                      request. The client can decide if
 *                                      it wishes to continue with compressed 
 *                                      retransmissions. 
 */
typedef struct
{
    SipTranscClientStateChangedEv           pfnStateChangedEvHandler;
    SipTranscClientMsgToSendEv              pfnMsgToSendEvHandler;
    SipTranscClientMsgReceivedEv            pfnMsgReceivedEvHandler;
    SipTranscClientOtherURLAddressFoundEv   pfnOtherURLAddressFoundEvHandler;
    SipTranscClientFinalDestResolvedEv      pfnFinalDestResolvedEvHandler;
    SipTranscClientNewConnInUseEv           pfnNewConnInUseEvHandler;
	SipTranscClientObjExpiredAlert			pfnObjExpiredEvHandler;
	SipTranscClientObjTerminatedEv			pfnObjTerminatedEvHandler;
#ifdef RV_SIGCOMP_ON
    SipTranscClientSigCompMsgNotRespondedEv pfnSigCompMsgNotRespondedEvHandler;
#endif
} SipTranscClientEvHandlers;

typedef struct 
{
    OwnerObjLockFunc    pfnOwnerLock;
    OwnerObjUnLockFunc  pfnOwnerUnlock;
} SipTranscClientOwnerUtils;

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_REG_CLIENT_TYPES_H */
