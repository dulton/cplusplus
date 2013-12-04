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
 *                              <RvSipPubClientTypes.h>
 *
 * The RvSipPubClientTypes.h file contains all type definitions and call-back
 * functions type definitions for the Publish-Client module.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef RV_SIP_PUB_CLIENT_TYPES_H
#define RV_SIP_PUB_CLIENT_TYPES_H

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
 * RvSipPubClientMgrHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to a publish-clients manager instance. The manager
 * object manages all publish-clients and its handle is needed in
 * all manager related API such as creation of new publish-client.
 */
RV_DECLARE_HANDLE(RvSipPubClientMgrHandle);


/*
 * RvSipPubClientHandle
 * Declaration of a publish-Client handle. publish-Client handle is needed
 * in all publish-Client API function and is used to reference the
 * publish-Client object.
 */
RV_DECLARE_HANDLE(RvSipPubClientHandle);


/*
 * RvSipAppPubClientHandle
 * ----------------------------------------------------------------------
 * Declaration  of application handle to a Publish-Client. This handle is
 * used by the application in order to associate sip stack Publish-Client
 * objects with application Publish-Client object. The application gives
 * the application handle when a new Publish-Client is created. The sip
 * stack will give this handle back to the application in every callback
 * function.
 */
RV_DECLARE_HANDLE(RvSipAppPubClientHandle);



/*-----------------------------------------------------------------------*/
/*                    PUB CLIENT TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/


/* RvSipPubClientState
 * ----------------------------------------------------------------------
 * RvSipPubClientState represents the state of a Publish process. The
 * state changes are reported using the RvSipPubClientStateChangedEv call
 * back function together with a state change reason that may indicate to
 * the application how the new state was reached
 */
typedef enum
{
    RVSIP_PUB_CLIENT_STATE_UNDEFINED = -1,
    RVSIP_PUB_CLIENT_STATE_IDLE,
		/*Initial state for a newly created publish-
		Client. The publish-client will stay in this state
		until the Publish() method will be called.*/
    RVSIP_PUB_CLIENT_STATE_TERMINATED,
		/*The final publish-client state. The
		  publish-client resources were freed and
		  the application must no longer reference
		  this publish-client object.*/
    RVSIP_PUB_CLIENT_STATE_PUBLISHING,
		/*Publish message was sent to the destination.*/
    RVSIP_PUB_CLIENT_STATE_REDIRECTED,
		/*A 3xx class response was received in the
			Publishing state. The application may
			confirm the redirection by calling
			Publish() method.*/
    RVSIP_PUB_CLIENT_STATE_UNAUTHENTICATED,
		/*A 401 or 407 response was received in the Publishing state. The
		  application may try to authenticate the request by calling the
		  Authenticate() method.*/	
    RVSIP_PUB_CLIENT_STATE_PUBLISHED,
		/*The Publish request was accepted by the
		  server. The Pub-Client is not terminated although the
		  Publish is successful*/
	RVSIP_PUB_CLIENT_STATE_REMOVING,
		/*When the application invoke the remove API the object assumes the REMOVING 
		  state until the PUBLISH requst responded*/
	RVSIP_PUB_CLIENT_STATE_REMOVED,
		/*After the PUBLISH request with expires=0 is responded the object change its state
		  to REMOVED*/
    RVSIP_PUB_CLIENT_STATE_FAILED,
		/*A 4xx, 5xx, or 6xx (other than 401, 407) was
		  received in the Publishing state.*/
    RVSIP_PUB_CLIENT_STATE_MSG_SEND_FAILURE
		/*The publish-client object assumes the MSG_SEND_FAILURE only for 503 response when DNS is on*/
} RvSipPubClientState;



/* RvSipPubClientStateChangeReason
 * ------------------------------------------------------------------
 * A publish-client object has an inner state. The state changes according
 * to the state machine defined for the publish-client. Each state change
 * is explained by one of the reasons enumerated bellow.
 */
typedef enum
{

    RVSIP_PUB_CLIENT_REASON_UNDEFINED = -1,
		/*The reason is undefined.*/
    RVSIP_PUB_CLIENT_REASON_USER_REQUEST,
		/*The change in state was in reaction to a user's request.*/
    RVSIP_PUB_CLIENT_REASON_RESPONSE_SUCCESSFUL_RECVD,
		/*2xx was received.*/
    RVSIP_PUB_CLIENT_REASON_RESPONSE_REDIRECTION_RECVD,
		/*3XX was received*/
    RVSIP_PUB_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD,	
		/*401 or 407 was received.*/
    RVSIP_PUB_CLIENT_REASON_RESPONSE_REMOTE_REJECT_RECVD,
		/*4xx, 5xx or 6xx other than 401, 407 received.*/
    RVSIP_PUB_CLIENT_REASON_TRANSACTION_TIMEOUT,
		/*The publish transaction had a timeout. This means that
		  no final response was received*/
    RVSIP_PUB_CLIENT_REASON_NETWORK_ERROR,
		/*A network error caused the state to change*/
	RVSIP_PUB_CLIENT_REASON_RESPONSE_COND_REQ_FAILED,
		/*A 412 response received indicate the the server does not recognized the 
		  SipIfMatch that was sent on the PUBLISH request.*/
    RVSIP_PUB_CLIENT_REASON_503_RECEIVED,
		/*503 response caused the state to change*/
	RVSIP_PUB_CLIENT_REASON_MISSING_SIP_ETAG,
		/*Indicates that the state was changed but the SIP-ETAG was missing on the response.*/
	RVSIP_PUB_CLIENT_REASON_MISSING_EXPIRES
		/*Indicates that the state was changed but the expires header was missing on the response.*/
} RvSipPubClientStateChangeReason;



/*-----------------------------------------------------------------------
       P U B  C L I E N T:  C A L L - B A C K  F U N C T I O N S
 ------------------------------------------------------------------------*/



/***************************************************************************
 * RvSipPubClientStateChangedEv
 * ------------------------------------------------------------------------
 * General: 
 *  Notifies the application of a publish-client state change.
 *  For each state change the new state is supplied and the
 *  reason for the state change is also given.
 *
 * Arguments:
 * Input:   hPubClient - The SIP stack publish-client handle.
 *          hAppPubClient - The application handle for this publish-client.
 *          eNewState - The new state of the publish-client object.
 *          eReason - The reason for the change in state.
 * Output: None
 * Return Value: void
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipPubClientStateChangedEv)(
                            IN  RvSipPubClientHandle            hPubClient,
                            IN  RvSipAppPubClientHandle         hAppPubClient,
                            IN  RvSipPubClientState             eNewState,
                            IN  RvSipPubClientStateChangeReason eReason);


/***************************************************************************
 * RvSipPubClientMsgToSendEv
 * ------------------------------------------------------------------------
 * General: 
 *  An event indicating that a new publish-client related outgoing
 *  message is about to be sent (not a retransmission).
 *
 * Arguments:
 * Input:     hPubClient    - The sip stack publish-client handle
 *            hAppPubClient - The application handle for this publish-client.
 *            hMsg          - Handle to the outgoing message.
 * Output: None.
 * Return Value: RV_OK 
 *               Otherwise - will cease the sending of the message.
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * RvSipPubClientMsgToSendEv)(
                                   IN  RvSipPubClientHandle    hPubClient,
                                   IN  RvSipAppPubClientHandle hAppPubClient,
                                   IN  RvSipMsgHandle          hMsg);


/***************************************************************************
 * RvSipPubClientMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: 
 *  An event indicating that a new publish-client related incoming
 *  message was received (not a retransmission).
 * 
 * Arguments:
 * Input:     hPubClient    - The sip stack publish-client handle
 *            hAppPubClient - The application handle for this publish-client.
 *            hMsg          - Handle to the incoming message.
 * Output: None
 * Return Value: RV_OK 
 *               Otherwise - will cease the handling of the message.
 * ------------------------------------------------------------------------
 ***************************************************************************/
typedef RvStatus
        (RVCALLCONV * RvSipPubClientMsgReceivedEv)(
                                    IN  RvSipPubClientHandle    hPubClient,
                                    IN  RvSipAppPubClientHandle hAppPubClient,
                                    IN  RvSipMsgHandle          hMsg);


/***************************************************************************
 * RvSipPubClientOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: 
 *  Notifies the application that a message needs to be sent and
 *  the destination address is a URL type that is currently not
 *  supported by the stack. The URL has to be converted to a SIP URL
 *  for the message to be sent.
 * Arguments:
 * Input:     hPubClient    - The sip stack publish-client handle
 *            hAppPubClient - The application handle for this publish-client.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - HHandle to the SIP URL address - this is an empty
 *                               address object that the application should fill.
 *            bAddressResolved-RV_TRUE if the SIP URL address was filled. RV_FALSE otherwise.
 * 
 * Return Value: RvStatus.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipPubClientOtherURLAddressFoundEv)(
                     IN  RvSipPubClientHandle    hPubClient,
                     IN  RvSipAppPubClientHandle hAppPubClient,
                     IN  RvSipMsgHandle          hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool                *bAddressResolved);


/***************************************************************************
 * RvSipPubClientFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: 
 *  Notifies the application that the publish-client is about to send a 
 *  message after the destination address was resolved.
 *  This callback supplies the final message object and the transaction
 *  that is responsible for sending this message. Changes in the message 
 *  at this stage will not effect the destination address. 
 *  When this callback is called, the application can query the transaction
 *  about the destination address using the 
 *  RvSipTransactionGetCurrentDestAddress() function. If the application 
 *  wishes, it can update the sent-by part of the top-most Via header. 
 *  The application must not update the branch parameter.
 *
 * Arguments:
 * Input:     hPubClient     - Handle to the publish client.
 *            hAppPubClient  - The application handle for this publish client.
 *            hTransc        - The transaction handle
 *            hMsgToSend     - The handle to the outgoing message.
 * Output: None
 * 
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK the message will not be sent. The transaction will
 *               terminate with error and the publish-client will move to the
 *               Failed state.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV * RvSipPubClientFinalDestResolvedEv)(
                      IN  RvSipPubClientHandle    hPubClient,
                      IN  RvSipAppPubClientHandle hAppPubClient,
                      IN  RvSipTranscHandle       hTransc,
                      IN  RvSipMsgHandle          hMsgToSend);

/***************************************************************************
 * RvSipPubClientNewConnInUseEv
 * ------------------------------------------------------------------------
 * General:
 *  Notifies the application that the transaction is now using a new 
 *  connection. The connection can be a totally new one or a suitable 
 *  one that was found in the hash.
 * Arguments:
 * Input:     hPubClient     - Handle to the publish client.
 *            hAppPubClient  - The application handle for this publish client.
 *            hConn          - The connection handle
 *            bNewConnCreated - RV_TRUE if the connection is a newly 
 *                              created connection. RV_FALSE if the connection 
 *                              was found in the connection hash.
 * Output: None.
 * 
 * Return Value: RV_OK (the returned status is ignored in the current
 *                      stack version)
 * ------------------------------------------------------------------------
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipPubClientNewConnInUseEv) (
                     IN  RvSipPubClientHandle           hPubClient,
                     IN  RvSipAppPubClientHandle        hAppPubClient,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);

/***************************************************************************
 * RvSipPubClientObjExpiredEv
 * ------------------------------------------------------------------------
 * General:
 *  Notifies the application that the object is about to expire.
 *	An expiration timer is triggered for successful responses for PUBLISH requests.
 *
 * Arguments:
 * Input:     hPubClient     - Handle to the Publish-client.
 *			  hAppPubClient  - Handle to the application Publish-client object.
 * Output: None
 *            
 * Return Value: 
 ***************************************************************************/
typedef void (RVCALLCONV *RvSipPubClientObjExpiredEv) (
                     IN  RvSipPubClientHandle           hPubClient,
					 IN  RvSipAppPubClientHandle        hAppPubClient);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RvSipPubClientSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: 
 *  Notifies the application that the timer of the transaction 
 *  responsible for sending the PUBLISH request has expired and that the
 *  SIP Stack has not yet received a response to the SigComp compressed 
 *  PUBLISH request.
 *  This callback is called in the following cases:
 *     - Before there is a need to retransmit a compressed message. 
 *       This happens when the transaction timer expires and, 
 *       according to RFC 3261, the SIP Stack needs to retransmit the message.
 *     - When the sigCompTcpTimer expires. The SIP Stack added a
 *       timer for SigComp TCP transmissions. When this timer expires
 *       the application can send the message again in a plain format.
 *       (According to RFC 3261, there are no retransmission in TCP).
 *  This callback is part of a recovery mechanism. The callback supplies
 *  information about compression type that was used in the last transmission. 
 *  The callback also gives the application the ability to determine if 
 *  there will be additional retransmissions, and the compression type of 
 *  the next retransmission.
 *  For example, the application can decide that the next retransmission 
 *  will not be compressed.
 *  Note: If the peNextMsgComp is set to RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED, 
 *	the SIP Stack will stop the message retransmissions.
 *
 * Arguments:
 * Input:     hPubClient    - Handle to the publish client.
 *            hAppPubClient - The application handle for this pub-client.
 *            hTransc       - The transaction handle.
 *            retransNo     - The number of retransmissions of the request
 *                            until now.
 *            ePrevMsgComp  - The type of the previous not responded request
 * Output:    peNextMsgComp - The type of the next request to retransmit (
 *                            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                            application wishes to stop sending requests).
 * 
 * Return Value: RvStatus. If the application returns a value other then
 *               RV_OK no further messages will be sent.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSipPubClientSigCompMsgNotRespondedEv) (
                     IN  RvSipPubClientHandle         hPubClient,
                     IN  RvSipAppPubClientHandle      hAppPubClient,
                     IN  RvSipTranscHandle            hTransc,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */

/* RvSipPubClientEvHandlers
 * ------------------------------------------------------------------------
 * Structure with function pointers to the module's call-backs.
 * This structure is used to set the application call-backs in the function
 * RvSipPubClientMgrSetEvHandlers()
 */
typedef struct
{
    RvSipPubClientStateChangedEv           pfnStateChangedEvHandler;
		/*notify of publish-client state change.*/
    RvSipPubClientMsgToSendEv              pfnMsgToSendEvHandler;
		/*notify that a message is about to be sent.*/
    RvSipPubClientMsgReceivedEv            pfnMsgReceivedEvHandler;
		/*notify that a message was received.*/
    RvSipPubClientOtherURLAddressFoundEv   pfnOtherURLAddressFoundEvHandler;
		/*notify that abs url address was found and has to be converted to SIP URL address.*/
    RvSipPubClientFinalDestResolvedEv      pfnFinalDestResolvedEvHandler;
		/*Notifies that the publish-client is about to send a message after the destination
		  address was resolved. This callback supplies the final message object and the transaction
		  that is responsible for sending this message. The application can use the transaction API 
		  to get the destination address.*/
    RvSipPubClientNewConnInUseEv           pfnNewConnInUseEvHandler;
		/*Notifies that the publish-client is now using a new connection. The connection can
		  be a totally new connection or a suitable connection that was found in the hash.*/
	RvSipPubClientObjExpiredEv			   pfnObjExpiredEvHandler;
		/*Notifies the application that the object is about to expires in pubAlertTimer the 
	      was set in the configuration file.*/
#ifdef RV_SIGCOMP_ON
    RvSipPubClientSigCompMsgNotRespondedEv pfnSigCompMsgNotRespondedEvHandler;
		/*Notifies that a timer expired before the SIP Stack received a response to a PUBLISH
		  SigComp compressed request. This callback is part of a recovery mechanism
		  and it supplies information about the sent request. The application can decide if
		  it wishes to continue with compressed retransmissions. */	
#endif
} RvSipPubClientEvHandlers;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_PUB_CLIENT_TYPES_H */
