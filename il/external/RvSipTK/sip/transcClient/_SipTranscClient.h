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
 *                              <_SipTranscClient.h>
 *
 * This file defines the TranscClient API functions for the upper layer pub-client and reg-client.
 * The API contains two major parts:
 * 1. The register-client manager API: The transaction-client manager is incharged
 *                                     of all the transaction-clients. It is used
 *                                     to set the event handlers of the
 *                                     transaction-client module and to create new
 *                                     transaction-client.
 * 2. The transaction-client API: Using the transaction-client API the user (pub/reg clients) can
 *                             request to Register or Publish at a chosen server. It
 *                             can redirect a register or publish request and authenticate
 *                             a register or publish request when needed.
 *                             A TranscClient is a stateful object and has a set
 *                             of states associated with it.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                  Aug 2006
 *********************************************************************************/


#ifndef SIP_TRANSC_CLIENT_H
#define SIP_TRANSC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipTranscClientTypes.h"
#include "_SipTransactionTypes.h"
#include "RvSipPartyHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransportDNSTypes.h"
#include "RvSipAuthenticator.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipAuthorizationHeader.h"
#include "RvSipSecurityTypes.h"
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                TRANSC_CLIENT MANAGER  API                                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipTranscClientSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the Transc-client call-id. If the call-id is not set the Transc-client
 *          will use the TranscClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The Transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The Sip Stack handle to the Transc-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCallId (
                                      IN  SipTranscClient	  *pTranscClient,
                                      IN  RvChar              *strCallId);


/***************************************************************************
 * SipTranscClientSetCallIdFromPage
 * ------------------------------------------------------------------------
 * General: Sets the Transc-client call-id. If the call-id is not set the Transc-client
 *          will use the TranscClient manager call-Id. Calling this function is
 *          optional.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The Transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The Sip Stack handle to the Transc-client
 *            strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCallIdFromPage(
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RPOOL_Ptr				*srcCallIdPage);


/***************************************************************************
 * SipTranscClientSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set the event handlers defined by the client module to
 *          the transcClient.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 ***************************************************************************/
void RVCALLCONV SipTranscClientSetEvHandlers( IN SipTranscClient *pTranscClient,
                                         IN SipTranscClientEvHandlers  *pTranscClientEvHandler,
										 IN RvInt32						size);

/***************************************************************************
 * SipTranscClientActivate
 * ------------------------------------------------------------------------
 * General: Sends a client request to the Destination. The Request-Url,To,
 *          From , Expires and Contact headers that were set to the transaction-
 *          client are inserted to the outgoing REGISTER/PUBLISH request.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the register-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid transc-client state for this
 *                                  action, or the To and From header have
 *                                  yet been set to the transc-client object,
 *                                  or the destination Request-Url has not
 *                                  yet been set to the transc-client object.
 *               RV_ERROR_OUTOFRESOURCES - transc-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRegClient - The register-client handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientActivate (
                                         IN SipTranscClient			  *pTranscClient,
										 IN SipTransactionMethod	  eTranscMethod,
										 IN RvChar*					  strMethod);

/***************************************************************************
 * SipTranscClientGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Transc-client Call-Id.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *         contains the size of the Call-ID string in the Transc-client.
 *
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was insufficient.
 *               RV_ERROR_NOT_FOUND           - The Transc-client does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient   - The Sip Stack handle to the Transc-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCallId (
                            IN  SipTranscClient		*pTranscClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize);

/***************************************************************************
 * SipTranscClientUpdateReceivedMsg
 * ------------------------------------------------------------------------
 * General: An API for the client to notify the transcClient that a new msg received.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Header copied successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phDestContactHeader - Pointer to the destination Contact handle.
 *            hSourceHeader - Handle to the source Contact header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientUpdateReceivedMsg(RvSipTranscHandle hTransc, SipTranscClient *pTranscClient);

/***************************************************************************
 * SipTranscClientCheckValidityOfAuthObj
 * ------------------------------------------------------------------------
 * General: An API for the client to check validity of the auth object
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - Header copied successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phDestContactHeader - Pointer to the destination Contact handle.
 *            hSourceHeader - Handle to the source Contact header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientCheckValidityOfAuthObj(SipTranscClient  *pTranscClient, RvBool *bValid);

/***************************************************************************
 * SipTranscClientMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transc-client
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTarnscClientMgr   - Handle to the transc-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrGetStackInstance(
                                   IN   SipTranscClientMgrHandle   hTranscClientMgr,
                                   OUT  void*         *phStackInstance);




/***************************************************************************
 * TranscClientCreateNewTransaction
 * ------------------------------------------------------------------------
 * General: Creates a new transaction for the Transaction-client according to the
 *          To and From headers of the Transaction-client object and according to
 *          the Call-Id and CSeq-Step of the transaction-clients manager.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create a new
 *                                   transaction.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The pointer to the transaction-client object.
 ***************************************************************************/										 
RvStatus RVCALLCONV TranscClientCreateNewTransaction(
													IN  SipTranscClient *pTranscClient);

/*-----------------------------------------------------------------------*/
/*                 TRANSC_CLIENT API                                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipTranscClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a transc-client object. This function destructs the
 *          transc-client object. You cannot reference the object after
 *          calling this function.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transaction-client is
 *                                   invalid.
 *                 RV_OK - The transaction-client was successfully terminated
 *                            and distructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientTerminate (
                                         IN SipTranscClient *pTranscClient);

/***************************************************************************
 * SipTranscClientUseFirstRouteForRequest
 * ------------------------------------------------------------------------
 * General: The owner may want to use a preloaded route header when
 *          sending the register/publish message.For this purpose, it should add the
 *          Route headers to the outbound message, and call this function
 *          to indicate the stack to send the request to the address of the
 *          first Route header in the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient    - Handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientUseFirstRouteForRequest (
                                      IN  SipTranscClient *pTranscClient);

/***************************************************************************
 * SipTranscClientSetCSeqStep
 * ------------------------------------------------------------------------
 * General: Sets the CSeq-Step associated with the transaction-client.
 *          The supplied CSeq must be bigger then zero.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 *           cSeqStep -  The transc-client's CSeq-Step.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetCSeqStep (
                                      IN  SipTranscClient *pTranscClient,
                                      IN  RvInt32            cSeqStep);

/***************************************************************************
 * SipTranscClientGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Returns the CSeq-Step assosiated with the transc-client.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE - The given transaction-client handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK - The state was successfully returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 * Output:     pCSeqStep -  The transaction-client's CSeq-Step.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCSeqStep (
                                      IN  SipTranscClient	  *pTranscClient,
                                      OUT RvUint32            *pCSeqStep);

/***************************************************************************
 * SipTranscClientSetAlertTimer
 * ------------------------------------------------------------------------
 * General: Setting an alert timer for timeToSet
 *			calling this function with timeToSet equals to 0 will cancel the previous
 *			timer without starting new interval
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient		- The pointer to the transcClient.
 * Output:    TimeToSet			- The interval to set the alert timer.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetAlertTimer(SipTranscClient *pTranscClient, RvUint32 timeToSet);

/***************************************************************************
 * SipTranscClientSetActivateDetails
 * ------------------------------------------------------------------------
 * General: Set the transaction client from,to,destination and contact parameters that
 *          are given as strings to the transcClient. If a null value is supplied as one of
 *          the arguments this parameter will not be set.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the transaction-client is
 *                                   invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid transaction-client state for this
 *                                  action or there is a missing parameter that
 *                                  is needed for the process such as the request URI.
 *               RV_ERROR_OUTOFRESOURCES - transaction-client failed to create a new
 *                                   transaction, or message object.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 *               RV_OK - Invite message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - The transaction-client handle.
 *          strFrom    - the initiator of the request.
 *                       for example: "From: sip:user@home.com"
 *          strTo      - The processing user.
 *          strRegistrar - The request URI of the transc-client request.This
 *                         is the proxy address.
 *			bValues		 - Boolean indicates whether the strings are the values of the header
 *						   sip:user@home.com or the header including its prefix "From: sip:user@home.com"
 *          
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetActivateDetails (
                                         IN SipTranscClient		*pTranscClient,
                                         IN RvChar*             strFrom,
                                         IN RvChar*             strTo,
                                         IN RvChar*             strDestination,
										 IN RvBool					bValues);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * SipTranscClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks transc-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
RvStatus RVCALLCONV SipTranscClientLockAPI(IN  SipTranscClient *pTranscClient);


/************************************************************************************
 * SipTranscClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks transc-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
void RVCALLCONV SipTranscClientUnLockAPI(IN  SipTranscClient *pTranscClient);

/************************************************************************************
 * SipTranscClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks transc-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
RvStatus RVCALLCONV SipTranscClientLockMsg(IN  SipTranscClient *pTranscClient);


/************************************************************************************
 * SipTranscClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks transc-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - pointer to the transc-client.
***********************************************************************************/
void RVCALLCONV SipTranscClientUnLockMsg(IN  SipTranscClient *pTranscClient);

#else
#define SipTranscClientLockAPI(a) (RV_OK)
#define SipTranscClientUnLockAPI(a)
#define SipTranscClientLockMsg(a) (RV_OK)
#define SipTranscClientUnLockMsg(a)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/*-----------------------------------------------------------------------
       TRANSC  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * SipTranscClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: Stops retries to send a Register/publish request after send failure.
 *          When a Register/publish request fails due to timeout, network error or
 *          503 response the Transc-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state the owner can do one of the following:
 *          1. Send the request to the next DNS resolved IP.
 *          2. Give up on this request.
 *          Calling SipTranscClientDNSGiveUp() indicates that the application
 *          wishes to give up on this request. Retries to send the request
 *          will stop and the Transc-client will change its state back to
 *          the previous state.
 *          If this is the initial request of the Transc-Client, calling
 *          DNSGiveUp() will terminate the transc-Client object. If this is a
 *          refresh, the Transc-Client will move back to the ACCEPTED state.
 * Return Value: RvStatus;
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSGiveUp (
                                   IN  SipTranscClient	   *pTranscClient);
/***************************************************************************
 * SipTranscClientDNSContinue
 * ------------------------------------------------------------------------
 * General: Prepares the Transc-Client object to a retry to send a request after
 *          the previous attempt failed.
 *          When a request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSContinue (
                                      IN  SipTranscClient		*pTranscClient);

/***************************************************************************
 * SipTranscClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: Re-sends a request after the previous attempt failed.
 *          When a Register/publish request fails due to timeout, network error or
 *          503 response the Transc-Client Object moves to the MSG_SEND_FAILURE state.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client that sent the request.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSReSendRequest (
                                            IN	  SipTranscClient		*pTranscClient,
											IN    SipTransactionMethod	eMethod,
											IN	  RvChar*				strMethod);

/***************************************************************************
 * SipTranscClientDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves DNS list object from the Transc-Client current active
 *          transaction.
 *          When a Register request fails due to timeout, network error or
 *          503 response the Reg-Client Object moves to the MSG_SEND_FAILURE state.
 *          At this state you can use SipTranscClientDNSGetList() to get the
 *          DNS list if you wish to view or change it.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client that sent the request.
 * Output   phDnsList - The DNS list handle
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDNSGetList(
                              IN  SipTranscClient			  *pTranscClient,
                              OUT RvSipTransportDNSListHandle *phDnsList);

/*-----------------------------------------------------------------------
       TRANSC - C L I E N T:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * SipTranscClientGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the transc-client page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information
 *         and set it back to the transc-client using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         Party   - you should set these headers back with the SipTranscClientSetToHeader()
 *                   or SipTranscClientSetFromHeader() API functions.
 *         Contact - you should set this header back with the SipTranscClientSetContactHeader()
 *                   API function.
 *         Expires - you should set this header back with the SipTranscClientSetExpiresHeader()
 *                   API function.
 *         Authorization - you should set this header back with the header to the SipTranscClientSetInitialAuthorization() 
 *                   API function that is available in the IMS add-on only.
 *         address - you should supply the address to the SipTranscClientSetDestination() API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetNewMsgElementHandle (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader);


/***************************************************************************
 * SipTranscClientGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the transc client. You can
 *          call this function from the SipTranscClientStateChangedEv call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phMsg           - pointer to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetReceivedMsg(
                                            IN  SipTranscClient			*pTranscClient,
                                             OUT RvSipMsgHandle          *phMsg);


/***************************************************************************
 * SipTranscClientGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the transc-client.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transc-client.
 * Output:     phMsg           -  pointer to the message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundMsg(
                                     IN  SipTranscClient		  *pTranscClient,
                                     OUT RvSipMsgHandle           *phMsg);

/***************************************************************************
 * SipTranscClientResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets the outbound message of the transaction-client to NULL. If the
 *          transaction-client is about to send a message it will create its own
 *          message to send.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientResetOutboundMsg(
                                     IN  SipTranscClient		*pTranscClient);

/***************************************************************************
 * SipTranscClientSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the from header associated with the transaction-client. If the
 *          From header was constructed by the
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid
 *                                   or the From header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hFrom      - Handle to an application constructed an initialized
 *                       from header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetFromHeader (
                                      IN  SipTranscClient		*pTranscClient,
                                      IN  RvSipPartyHeaderHandle hFrom);


/***************************************************************************
 * SipTranscClientGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the transaction-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 * Output:     phFrom     - Pointer to the transaction-client From header handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetFromHeader (
                                      IN  SipTranscClient		 *pTranscClient,
                                      OUT RvSipPartyHeaderHandle *phFrom);


/***************************************************************************
 * SipTranscClientSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the To header associated with the transaction-client. If the To
 *          header was constructed by the SipTranscClientGetNewPartyHeaderHandle
 *          function, the To header will be attached to the transaction-client
 *          object.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle or the To
 *                                   header handle is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - To header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hTo        - Handle to an application constructed and initialized
 *                       To header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetToHeader (
                                      IN  SipTranscClient       *pTranscClient,
                                      IN  RvSipPartyHeaderHandle hTo);



/***************************************************************************
 * SipTranscClientGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To header associated with the transc-client.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The register-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 * Output:     phTo       - Pointer to the transaction-client To header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV SipTranscClientGetToHeader (
                                      IN    SipTranscClient         *pTranscClient,
                                      OUT   RvSipPartyHeaderHandle  *phTo);


/***************************************************************************
 * SipTranscClientSetContact
 * ------------------------------------------------------------------------
 * General: Set the contact header in the transc-client object.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_BADPARAM - The supplied address handle is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hRequestUri - Handle to the contactc to be set to the
 *                        transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetContactHeader (
                                 IN  SipTranscClient				*pTranscClient,
                                 IN  RvSipContactHeaderHandle		hContact);

/***************************************************************************
 * SipTranscClientSetDestination
 * ------------------------------------------------------------------------
 * General: Set the SIP-Url of the destination in the transc-client object.
 *          The transaction request will be sent to this SIP-Url.
 *          This ability can be used for updating the destination's SIP-Url
 *          in case of redirections and refreshes.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_BADPARAM - The supplied address handle is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hRequestUri - Handle to the registrar SIP-Url to be set to the
 *                        register-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetDestination (
                                 IN  SipTranscClient		*pTranscClient,
                                 IN  RvSipAddressHandle		hRequestUri);


/***************************************************************************
 * SipTranscClientGetDestination
 * ------------------------------------------------------------------------
 * General: Gets the SIP-Url of the destination associated with the
 *          transc-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transc-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the owner.
 *               RV_OK        - The address object was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phRequestUri    - Handle to the destination SIP-Url.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetDestination (
                               IN  SipTranscClient          *pTranscClient,
                               OUT RvSipAddressHandle       *phRequestUri);


/***************************************************************************
 * SipTranscClientSetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Set an Expires header in the transaction-client object. 
 *
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Expires header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Expires header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transaction-client.
 *            hExpiresHeader - Handle to an Expires header to be set to the
 *                           transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetExpiresHeader (
                                 IN  SipTranscClient         *pTranscClient,
                                 IN  RvSipExpiresHeaderHandle hExpiresHeader);


/***************************************************************************
 * SipTranscClientGetExpiresHeader
 * ------------------------------------------------------------------------
 * General: Gets the Expires header associated with the transaction-client.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction-client handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - The Expires header was successfully
 *                                   returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     phExpiresHeader - Handle to the register-client's Expires header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetExpiresHeader (
                               IN  SipTranscClient			*pTranscClient,
                               OUT RvSipExpiresHeaderHandle *phExpiresHeader);

/***************************************************************************
 * SipTranscClientGetExpiresValue
 * ------------------------------------------------------------------------
 * General: Gets the Expires value associated with the transaction-client expires header.
 * Return Value: RV_ERROR_NOT_FOUND   - The transc-client expires header was set yet.
 *               RV_OK				  - The expires value was found.
 *				 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transaction-client.
 * Output:     pExpVal			 - The value of the transc-client expires header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetExpiresValue (
                               IN  SipTranscClient			*pTranscClient,
                               OUT RvUint32					*pExpVal);


/***************************************************************************
 * SipTranscClientGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transc-client
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - Handle to the transc-client object.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetStackInstance(
                                   IN   SipTranscClient		*pTranscClient,
                                   OUT  void*				*phStackInstance);

/***************************************************************************
 * SipTranscClientSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the Transc-Client will send outgoing
 *          requests.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The transc-client handle.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType   - The address type(IPv4 or IPv6).
 *            strLocalIPAddress   - A string with the local address to be set to this reg-client.
 *          localPort      - The local port to be set to this reg-client. If you set
 *                           the local port to 0, you will get a default value of 5060.
 *          if_name        - The local interface name
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetLocalAddress(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *strLocalIPAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    );

/***************************************************************************
 * SipTranscClientGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address the Transc-Client will use to send outgoing
 *          requests to a destination with the supplied
 *          transport type and address type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient      - The transaction-client handle
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:    strLocalIPAddress    - The local address the Transc-Client will use
 *                            for the supplied transport and address types.
 *                            (The buffer must be longer than 48 bytes).
 *          pLocalPort       - The local port the Reg-Client will use.
 *          if_name - the local interface name
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetLocalAddress(
                            IN  SipTranscClient			 *pTranscClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *strLocalIPAddress,
                            OUT  RvUint16                *pLocalPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    );

/***************************************************************************
 * SipTranscClientSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the transc-client object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameters such as the IP address or host name,
 *          transport, port and compression type.
 *          Requests sent by this object will use the outbound details
 *          specifications as a remote address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transaction-client.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundDetails(
                            IN  SipTranscClient				   *pTranscClient,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg);

/***************************************************************************
 * SipTranscClientGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the transaction-client object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type. If the outbound details were not set to the specific
 *          register-client object, but the outbound proxy was defined to the SIP
 *          Stack on initialization, the SIP Stack parameters will be returned.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - Handle to the transaction-client.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to outbound proxy configuration structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundDetails(
                            IN  SipTranscClient					*pTranscClient,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);

/***************************************************************************
 * SipTranscClientSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy IP address of the Transc-Client object. All clients
 *          requests sent by this Transc-Client will be sent directly to the Transc-Client
 *          outbound IP address.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transaction-client handle
 *            strOutboundAddrIp   - The outbound IP to be set to this
 *                              transc-client.
 *          outboundProxyPort - The outbound port to be set to this
 *                              Reg-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundAddress(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvChar               *strOutboundAddrIp,
                            IN  RvUint16              outboundProxyPort);


/***************************************************************************
 * SipTranscClientGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Gets the address of outbound proxy the Transc-Client is using. If an
 *          outbound address was set to the Transc-Client this address will be
 *          returned. If no outbound address was set to the transc-client
 *          but an outbound proxy was configured for the stack, the configured
 *          address is returned. '\0' is returned if the transc-client is not using
 *          an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transction-client handle
 * Output:  outboundProxyIp   - A buffer with the outbound proxy IP the transc-client.
 *                              is using(must be longer than 48).
 *          pOutboundProxyPort - The outbound proxy port the transc-client is using.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundAddress(
                            IN   SipTranscClient	 *pTranscClient,
                            OUT  RvChar              *outboundProxyIp,
                            OUT  RvUint16            *pOutboundProxyPort);

/***************************************************************************
 * SipTranscClientSetOutboundHostName
 * ------------------------------------------------------------------------
 * General:  Sets the outbound proxy host name of the Transc-Client object.
 *           The outbound host name will be resolved each time a Register
 *           request is sent and the clients requests will be sent to the
 *           resolved IP address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient           - Handle to the TanscClient
 *            strOutboundHost    - The outbound proxy host to be set to this
 *                               TranscClient.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               RegClient. If you set the port to zero it
 *                               will be determined using the DNS procedures.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundHostName(
                            IN  SipTranscClient		  *pTranscClient,
                            IN  RvChar                *strOutboundHost,
                            IN  RvUint16              outboundPort);


/***************************************************************************
 * SipTranscClientGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy the TranscClient is using. If an
 *          outbound host was set to the TranscClient this host will be
 *          returned. If no outbound host was set to the RegClient
 *          but an outbound host was configured for the stack, the configured
 *          address is returned. '\0' is returned if the TranscClient is not using
 *          an outbound host.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTranscClient      - Handle to the TranscClient
 * Output:
 *            srtOutboundHostName   -  A buffer with the outbound proxy host name
 *          pOutboundPort - The outbound proxy port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundHostName(
                            IN   SipTranscClient	 *pTranscClient,
                            OUT  RvChar              *strOutboundHostName,
                            OUT  RvUint16            *pOutboundPort);


/***************************************************************************
 * SipTranscClientSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the Transc-Client outbound proxy.
 *          This transport will be used for the outbound proxy that you set
 *          using the SipTranscClientSetOutboundAddress() function or the
 *          SipTranscClientSetOutboundHostName() function.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The Transc-Client handle
 *            eTransportType    - The outbound proxy transport to be set to this
 *                              Transc-Client
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetOutboundTransport(
                            IN  SipTranscClient		  *pTranscClient,
                            IN  RvSipTransport         eTransportType);


/***************************************************************************
 * SipTranscClientGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of outbound proxy the Transc-Client is using. If an
 *          outbound transport was set to the transc-client this transport will be
 *          returned. If no outbound transport was set to the transc-client
 *          but an outbound proxy was configured for the stack, the configured
 *          transport is returned. RVSIP_TRANSPORT_UNDEFINED is returned if the
 *          transc-client is not using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - The transaction-client handle
 * Output:  eOutboundProxyTransport    - The outbound proxy transport type
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetOutboundTransport(
                            IN   SipTranscClient		*pTranscClient,
                            OUT  RvSipTransport			*eOutboundProxyTransport);

#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SipTranscClientSetCct(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvInt32    			cctContext);
/***************************************************************************
 * SipTranscClientSetURIFlag
 * ------------------------------------------------------------------------
 * General: Sets the Req-URI configuration flag of the Transc-Client 
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The Transc-Client handle
 *            uri_flag          - URI configuration flag to be set to this
 *                              Transc-Client
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetURIFlag(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvUint32    			uri_flag);
#endif

/***************************************************************************
 * SipTranscClientResetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Reset the received message in the transaction-client.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient - The transaction handle
 *          eTranscState  - The transaction state.
 ***************************************************************************/
void RVCALLCONV SipTranscClientResetReceivedMsg(
                            IN  SipTranscClient				*pTranscClient);

/***************************************************************************
 * SipTranscClientUpdateContactToRedirect
 * ------------------------------------------------------------------------
 * General: Update the request uri of the transaction-client according to the contact
 *			received in the received msg.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            hMsgReceived  - Handle to the received msg.
 ***************************************************************************/
RvStatus SipTranscClientUpdateContactToRedirect(SipTranscClient	*pTranscClient, 
												RvSipMsgHandle hMsgReceived);

/***************************************************************************
 * SipTranscClientHandleSigCompTransc
 * ------------------------------------------------------------------------
 * General: Handle SigComp for the incoming transaction.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            hMsgReceived  - Handle to the received msg.
 ***************************************************************************/
RvStatus SipTranscClientHandleSigCompTransc(RvSipTranscHandle hTransc, 
											SipTranscClient	*pTranscClient);


/***************************************************************************
 * SipTranscClientSetTransportEvHandle
 * ------------------------------------------------------------------------
 * General: Sets the transport ev handler that will be in use for this client.
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            evHandler		- The transport ev handlere of this client.
 ***************************************************************************/
void SipTranscClientSetTransportEvHandle(SipTranscClient					*pTranscClient, 
										 RvSipTransportConnectionEvHandlers evHandler);

/***************************************************************************
 * SipTranscClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the TranscClient as the connection owner
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 *            hConn         - The connection handle
 ***************************************************************************/
void SipTranscClientAttachOnConnection(IN SipTranscClient				*pTranscClient,
                                 IN RvSipTransportConnectionHandle		  hConn);

/***************************************************************************
 * SipTranscClientSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the transaction-client:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient       - Pointer to the Transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSecAgreeMsgSendFailure(
											IN  SipTranscClient			*pTranscClient);

/***************************************************************************
 * SipTranscClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the TranscClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscClient    - pointer to TranscClient
 ***************************************************************************/
void SipTranscClientDetachFromConnection(IN SipTranscClient			*pTranscClient);


/***************************************************************************
 * SipTranscClientSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the Transc-Client persistency definition on run time.
 *          The function receives a boolean value that indicates whether the
 *          application wishes this Transc-Client to be persistent or not.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hRegClient - The Transc-Client handle
 *          bIsPersistent - Determines the Transc-Client persistency definition.
 *                          RV_TRUE = persistent, RV_FALSE otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetPersistency(
                           IN SipTranscClient		 *pTranscClient,
                           IN RvBool                  bIsPersistent);

/***************************************************************************
 * SipTranscClientGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the Transc-Client persistency definition.
 *          RV_TRUE indicates that the Transc-Client is persistent, RV_FALSE
 *          otherwise.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscClient - The Transc-Client handle
 * Output:  pbIsPersistent - The Transc-Client persistency definition.
 *                           RV_TRUE indicates that the Transc-Client is persistent,
 *                           RV_FALSE otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetPersistency(
        IN  SipTranscClient						  *pTranscClient,
        OUT RvBool                                *pbIsPersistent);

/***************************************************************************
 * SipTranscClientSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the Transc-Client transactions.
 *          The Transc-Client object holds this connection in its internal database.
 *          Whenever the Transc-Client creates a new transaction it supplies
 *          the transaction with given connection.
 *          The transaction will use the connection only if it fits the transaction's
 *          local and remote addresses.
 *          Otherwise the transaction will either locate a suitable connection in the hash
 *          or create a new connection. The Transc-Client object will be informed that
 *          the supplied connection did not fit and that a different connection was used and
 *          will update its database.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the transc-client
 *          hConn - Handle to the connection.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetConnection(
                   IN  SipTranscClient						*pTranscClient,
                   IN  RvSipTransportConnectionHandle		hConn);


/***************************************************************************
 * SipTranscClientGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Transc-Client.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle to the Transc-Client.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetConnection(
                            IN  SipTranscClient					*pTranscClient,
                           OUT  RvSipTransportConnectionHandle  *phConn);


/***************************************************************************
 * SipTranscClientDisableCompression
 * ------------------------------------------------------------------------
 * General:Disables the compression mechanism in a transaction-client.
 *         This means that even if the message indicates compression,
 *         it will not be compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTranscClient    - The handle to the transaction-client.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientDisableCompression(
                                      IN  SipTranscClient	*pTranscClient);


/***************************************************************************
* SipTranscClientSetTranscTimers
* ------------------------------------------------------------------------
* General: Sets timeout values for the TranscClient's transactions timers.
*          If some of the fields in pTimers are not set (UNDEFINED), this
*          function will calculate it, or take the values from configuration.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    Input: hTranscClient - Handle to the TranscClient object.
*           pTimers - Pointer to the struct contains all the timeout values.
*           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetTranscTimers(
                            IN  SipTranscClient			*pTranscClient,
                            IN  RvSipTimers				*pTimers,
                            IN  RvInt32					sizeOfTimersStruct);

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * SipTranscClientSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message,
 *          belonging to the TranscClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTranscClient - Handle to the TranscClient object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams  - Pointer to the struct that contains all the parameters.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSctpMsgSendingParams(
                    IN  SipTranscClient						*pTranscClient,
                    IN  RvUint32							sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams	*pParams);

/******************************************************************************
 * SipTranscClientGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message, belonging
 *          to the TranscClient, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTranscClient - Handle to the TranscClient object.
 *           sizeOfStruct - Size of the SipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSctpMsgSendingParams(
                    IN  SipTranscClient						*pTranscClient,
                    IN  RvUint32							sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams	*pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON

/***************************************************************************
 * SipTranscClientSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: update the secAgree about the dest resolved.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            hMsgReceived  - Handle to the received msg.
 ***************************************************************************/
void SipTranscClientSecAgreeDestResolved(SipTranscClient	*pTranscClient, 
											 RvSipMsgHandle hMsgToSend, 
											 RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTranscClientSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: update the secAgree about the received msg.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient - Handle of the transaction-client.
 *            hMsgReceived  - Handle to the received msg.
 ***************************************************************************/
void SipTranscClientSecAgreeMsgRcvd(SipTranscClient	*pTranscClient, 
									RvSipMsgHandle	hMsgReceived);

/***************************************************************************
 * SipTranscClientSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Handle detach secAgree from this transcClient.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeDetachSecAgree(SipTranscClient *pTranscClient);

/***************************************************************************
 * SipTranscClientSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Handle detach secAgree event from secAgree.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeDetachSecAgreeEv(SipTranscClient *pTranscClient, 
												 RvSipSecAgreeHandle hSecAgree);


/***************************************************************************
 * SipTranscClientSecAgreeAttachSecObj
 * ------------------------------------------------------------------------
 * General: Attaches the secObj to the secAgree if needed.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus SipTranscClientSecAgreeAttachSecObj(SipTranscClient *pTranscClient, RvSipSecAgreeHandle hSecAgree, RvSipSecObjHandle hSecObj);

/******************************************************************************
 * SipTranscClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the TRansc-Client.
 *          As a result of this operation, all messages, sent by this
 *          Transc-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTranscClient - Handle to the Transc-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSecObj(
                                        IN  SipTranscClient			*pTranscClient,
                                        IN  RvSipSecObjHandle		hSecObj);
/******************************************************************************
 * SipTranscClientGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Transc-Client.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTranscClient - Handle to the Transc-Client object.
 *  Output: phSecObj   - Handle to the Security Object.
 *****************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSecObj(
                                        IN  SipTranscClient			*pTranscClient,
                                        OUT RvSipSecObjHandle*      phSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * SipTranscClientGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: The function retrieve the auth-object that is currently being
 *          processed by the authenticator.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetCurrProcessedAuthObj (
                                      IN   SipTranscClient		*pTranscClient,
                                      OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * SipTranscClientAuthObjGet
 * ------------------------------------------------------------------------
 * General: The function retrieve auth-objects from the list in the Transc-Client.
 *          you may get the first/last/next object.
 *          in case of getting the next object, please supply the current
 *          object in the relative parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient        - Handle to the Transc-Client.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientAuthObjGet (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipListLocation     eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * SipTranscClientAuthObjRemove
 * ------------------------------------------------------------------------
 * General: The function removes an auth-obj from the list in the Transc-Client.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient  - Handle to the Transc-Client.
 *            hAuthObj - The Auth-Obj handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientAuthObjRemove (
                                      IN   SipTranscClient		*pTranscClient,
                                      IN   RvSipAuthObjHandle   hAuthObj);
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTranscClientSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: Sets an initial Authorization header in the outgoing request.
 *          An initial authorization header is a header that contains
 *          only the client private identity, and not real credentials.
 *          for example:
 *          "Authorization: Digest username="user1_private@home1.net",
 *                         realm="123", nonce="", uri="sip:...", response="" "
 *          The transc-client will set the initial header to the message, only if
 *          it has no other Authorization header to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscClient     - The transaction-client handle.
 *            hAuthorization - The Authorization header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetInitialAuthorization (
                                         IN SipTranscClient					*pTranscClient,
                                         IN RvSipAuthorizationHeaderHandle	hAuthorization);

/***************************************************************************
 * SipTranscClientSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this transc-client. If this
 *          security-agreement object maintains an existing agreement with the
 *          remote party, the transc-client will exploit this agreement and the data
 *          it brings. If not, the transc-client will use this security-agreement
 *          object to negotiate an agreement with the remote party.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient   - Handle to the transc-client.
 *          hSecAgree    - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientSetSecAgree(
							IN  SipTranscClient				*pTranscClient,
							IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * SipTranscClientGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the transc-client.
 *          The security-agreement object captures the security-agreement with
 *          the remote party as defined in RFC3329.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 *          hSecAgree   - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetSecAgree(
									IN   SipTranscClient		*pTranscClient,
									OUT  RvSipSecAgreeHandle    *phSecAgree);

/***************************************************************************
 * SipTranscClientGetActiveTransc
 * ------------------------------------------------------------------------
 * General: Gets the active transaction of the transc-client.
 *          The active transaction is the transaction in progress of
 *          the transc-client.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClient    - Handle to the transc-client.
 * Output:  phActiveTransc   - Handle to the active transaction
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientGetActiveTransc(
									IN   SipTranscClient	  *pTranscClient,
									OUT  RvSipTranscHandle    *phActiveTransc);

#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef __cplusplus
}
#endif

#endif /*RV_SIP_PRIMITIVES*/
#endif /* END OF: #ifndef SIP_TRANSC_CLIENT_H */
