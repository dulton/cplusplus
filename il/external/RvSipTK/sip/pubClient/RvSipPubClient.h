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
 *                              <RvSipPubClient.h>
 *
 * This file defines the PubClient API functions.
 * The API contains two major parts:
 * 1. The publish-client manager API: The publish-client manager is incharged
 *                                     of all the publish-clients. It is used
 *                                     to set the event handlers of the
 *                                     publish-client module and to create new
 *                                     publish-clients.
 * 2. The publish-client API: Using the publish-client API the user can
 *                             request to Publish at a chosen server. It
 *                             can redirect a publish request and authenticate
 *                             a publish request when needed.
 *                             A PubClient is a stateful object and has a set
 *                             of states associated with it.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef RV_SIP_PUB_CLIENT_H
#define RV_SIP_PUB_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipPubClientTypes.h"
#include "RvSipPartyHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipTransportTypes.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransportDNSTypes.h"
#include "RvSipAuthenticator.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipAuthorizationHeader.h"
#include "RvSipSecurityTypes.h"
#include "RvSipSecAgree.h"
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                PUB_CLIENT MANAGER  API                                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPubClientMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: 
 *	Sets event handlers for all publish-client events.
 *
 * Arguments:
 * Input:     hMgr - Handle to the publish-client manager.
 *            pEvHandlers - Pointer to the application event handler structure
 *            structSize - The size of the event handler structure.
 * 
 * Output: None
 *
 * Return Value:RV_OK in case of success otherwise relevant error will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrSetEvHandlers(
                                   IN  RvSipPubClientMgrHandle   hMgr,
                                   IN  RvSipPubClientEvHandlers *pEvHandlers,
                                   IN  RvInt32                  structSize);




/***************************************************************************
 * RvSipPubClientMgrCreatePubClient
 * ------------------------------------------------------------------------
 * General: 
 *  Creates a new publish-client and replace handles with the
 *  application.  The new publish-client assumes the idle state.
 *  In order to publish to a publisher you should:
 *  1. Create a new PubClient with this function.
 *  2. Set at least the Request-uri, To, From and Event headers.
 *  3. Call the Publish() function.
 *
 * Arguments:
 * Input:     hPubClientMgr - Handle to the publish-client manager
 *            hAppPubClient - Application handle to the newly created publish-
 *                          client.
 * Output:     hPubClient -    sip stack handle to the publish-client
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrCreatePubClient(
                                   IN  RvSipPubClientMgrHandle hPubClientMgr,
                                   IN  RvSipAppPubClientHandle hAppPubClient,
                                   OUT RvSipPubClientHandle   *phPubClient);


/***************************************************************************
 * RvSipPubClientMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the handle to the stack instance to which this pub-client
 *  manager belongs to.
 *
 * Arguments:
 * Input:     hPubClientMgr   - Handle to the publish-client manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientMgrGetStackInstance(
                                   IN   RvSipPubClientMgrHandle   hPubClientMgr,
                                   OUT  void*         *phStackInstance);


/*-----------------------------------------------------------------------*/
/*                 PUB_CLIENT API                                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipPubClientPublish
 * ------------------------------------------------------------------------
 * General: 
 *  Sends a Publish request to the publisher. The Request-Url,To,
 *  From , Expires and Contact headers that were set to the publish
 *  client are inserted to the outgoing PUBLISH request.
 *  The publish-client will assumes the Publishing state and wait for
 *  a response from the server.
 * Arguments:
 *
 * Input:  hPubClient - The publish-client handle.
 * Output: None
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientPublish (
                                         IN RvSipPubClientHandle hPubClient);

/***************************************************************************
 * RvSipPubClientRemove
 * ------------------------------------------------------------------------
 * General: 
 *  Sends a PUBLISH request to the publisher. The Request-Url,To,
 *  From , Expires and Contact headers that were set to the publish
 *  client are inserted to the outgoing PUBLISH request.
 *  The publish-client will assumes the Removing state and wait for
 *  a response from the server.
 *
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 * Output: None.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientRemove (
                                         IN RvSipPubClientHandle hPubClient);


/***************************************************************************
 * RvSipPubClientInit
 * ------------------------------------------------------------------------
 * General: Set the publish client from,to,request-uri parameters that
 *          are given as strings to the pubClient. 
 *			If a null value is supplied as one of the arguments this parameter 
 *			will not be set. 
 *          The publish-client will stay in the IDLE state.
 * Return Value: RV_OK - Success otherwise an error status will return.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClient - The publish-client handle.
 *          strFrom    - the initiator of the publication request.
 *                       for example: "sip:user@home.com"
 *          strTo      - The publishing user.
 *                       "sip:bob@proxy.com"
 *          strPublisher - The request URI of the publication request.This
 *                         is the proxy address.
 *                         for example: "sip:proxy.com"
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientInit (
                                         IN RvSipPubClientHandle hPubClient,
                                         IN RvChar*             strFrom,
                                         IN RvChar*             strTo,
                                         IN RvChar*             strPublisher);


/***************************************************************************
 * RvSipPubClientTerminate
 * ------------------------------------------------------------------------
 * General: 
 *  Terminates a publish-client object. This function destructs the
 *  publish-client object. You cannot reference the object after
 *  calling this function.
 * Arguments:
 * Input:  hPubClient - The publish-client handle.
 * Output: None.
 * 
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientTerminate (
                                         IN RvSipPubClientHandle hPubClient);

/***************************************************************************
 * RvSipPubClientDetachOwner
 * ------------------------------------------------------------------------
 * General: 
 *  Detach the Publish-Client owner. After calling this function the user will
 *  stop receiving events for this Publish-Client. This function can be called
 *  only when the object is in terminated state.
 * Arguments:
 *  Input:     hPubClient - The publish-client handle.
 * 
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDetachOwner (
                                         IN RvSipPubClientHandle hPubClient);

/***************************************************************************
 * RvSipPubClientUseFirstRouteForPublishRequest
 * ------------------------------------------------------------------------
 * General: 
 *  The Application may want to use a preloaded route header when
 *  sending the publish message.For this purpose, it should add the
 *  Route headers to the outbound message, and call this function
 *  to indicate the stack to send the request to the address of the
 *  first Route header in the outbound message.
 * Arguments:
 *  Input:     hPubClient    - Handle to the publish client.
 *  Output: None.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientUseFirstRouteForPublishRequest (
                                      IN  RvSipPubClientHandle hPubClient);

/*-----------------------------------------------------------------------
       P U B  - C L I E N T:  D N S   A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPubClientDNSGiveUp
 * ------------------------------------------------------------------------
 * General: 
 *  Stops retries to send a Publish request after send failure.
 *  When a Publish request fails due to timeout, network error or
 *  503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *  At this state the application can do one of the following:
 *  1. Send the request to the next DNS resolved IP.
 *  2. Give up on this request.
 *  Calling RvSipPubClientDNSGiveUp() indicates that the application
 *  wishes to give up on this request. The Publish client will change its state back to
 *  the PUBLISHED state.
 *  If this is the initial Publish request of the Pub-Client, calling
 *  DNSGiveUp() will move the Pub-Client object to the FAILED state.
 *
 * Arguments:
 * Input:  hPubClient - Handle to the Pub-Client that sent the request.
 * Output: None.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSGiveUp (
                                   IN  RvSipPubClientHandle   hPubClient);
/***************************************************************************
 * RvSipPubClientDNSContinue
 * ------------------------------------------------------------------------
 * General: 
 *  Prepares the Pub-Client object to a retry to send a request after
 *  the previous attempt failed.
 *  When a Publish request fails due to timeout, network error or
 *  503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *  At this state the application can do one of the following:
 *  1. Send the request to the next DNS resolved IP.
 *  2. Give up on this request.
 *  In order to retry to send the message the application must first
 *  call the RvSipPubClientDNSContinue() function.
 *  Calling this function, clones the failure transaction and
 *  updates the DNS list. (In order to actually re-send the request
 *  to the next IP address use the function RvSipPubClientDNSReSendRequest()).
 *
 * Arguments:
 * Input:     hPubClient - Handle to the Pub-Client that sent the request.
 * Output: None
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSContinue (
                                      IN  RvSipPubClientHandle   hPubClient);

/***************************************************************************
 * RvSipPubClientDNSReSendRequest
 * ------------------------------------------------------------------------
 * General: 
 *  Re-sends a publish request after the previous attempt failed.
 *  When a Publish request fails due to timeout, network error or
 *  503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *  At this state the application can do one of the following:
 *  1. Send the request to the next DNS resolved IP.
 *  2. Give up on this request.
 *  In order to re-send the request first call RvSipPubClientDNSContinue().
 *  You should then call RvSipPubClientDNSReSendRequest().
 *  The request will automatically be sent to the next resolved IP address
 *  in the DNS list.
 * Arguments:
 * Input:  hPubClient - Handle to the pub client that sent the request.
 * Output: None.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSReSendRequest (
                                            IN  RvSipPubClientHandle   hPubClient);

/***************************************************************************
 * RvSipPubClientDNSGetList
 * ------------------------------------------------------------------------
 * General: 
 *  Retrieves DNS list object from the Pub-Client current active
 *  transaction.
 *  When a Publish request fails due to timeout, network error or
 *  503 response the Pub-Client Object moves to the MSG_SEND_FAILURE state.
 *  At this state you can use RvSipPubClientDNSGetList() to get the
 *  DNS list if you wish to view or change it.
 * Arguments:
 * Input:   hPubClient - Handle to the pub client that sent the request.
 * Output	phDnsList - The DNS list handle
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDNSGetList(
                              IN  RvSipPubClientHandle         hPubClient,
                              OUT RvSipTransportDNSListHandle *phDnsList);

/*-----------------------------------------------------------------------
       P U B - C L I E N T:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipPubClientGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:
 *  Allocates a new message element on the pub-client page, and returns the new
 *  element handle.
 *  The application may use this function to allocate a message header or a message
 *  address. It should then fill the element information
 *  and set it back to the pub-client using the relevant 'set' function.
 *
 *  The function supports the following elements:
 *  Party   - you should set these headers back with the RvSipPubClientSetToHeader()
 *			  or RvSipPubClientSetFromHeader() API functions.
 *  Expires - you should set this header back with the RvSipPubClientSetExpiresHeader()
 *			  API function.
 *  Authorization - you should set this header back with the header to the RvSipPubClientSetInitialAuthorization() 
 *            API function that is available in the IMS add-on only.
 *  address - you should supply the address to the RvSipPubClientSetRequestURI() API function.
 *
 *
 * Arguments:
 * Input:     hPubClient - Handle to the pub-client.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phHeader - Handle to the newly created header or address.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetNewMsgElementHandle (
                                      IN   RvSipPubClientHandle   hPubClient,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phHeader);


/***************************************************************************
 * RvSipPubClientGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the message that was received by the publish client. You can
 *  call this function from the RvSipPubClientStateChangedEv call back function
 *  when the new state indicates that a message was received.
 *  If there is no valid received message, NULL will be returned.
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phMsg           - pointer to the received message.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetReceivedMsg(
                                            IN  RvSipPubClientHandle  hPubClient,
                                             OUT RvSipMsgHandle          *phMsg);


/***************************************************************************
 * RvSipPubClientGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the message that is going to be sent by the publish-client.
 *  You can call this function before you call API functions such as
 *  Publish(), Make() and Authenticate().
 *  Note: The message you receive from this function is not complete.
 *  In some cases it might even be empty.
 *  You should use this function to add headers to the message before
 *  it is sent. To view the complete message use event message to
 *  send.
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phMsg           -  pointer to the message.
 *
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientGetOutboundMsg(
                                     IN  RvSipPubClientHandle  hPubClient,
                                     OUT RvSipMsgHandle           *phMsg);

/***************************************************************************
 * RvSipPubClientResetOutboundMsg
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the outbound message of the publish-client to NULL. If the
 *  publish-client is about to send a message it will create its own
 *  message to send.
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * 
 * Return Value: RV_OK - Success otherwise an error status will return.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientResetOutboundMsg(
                                     IN  RvSipPubClientHandle  hPubClient);

/***************************************************************************
 * RvSipPubClientSetCallId
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the Pub-Client Call-Id. Calling this function is
 *  optional. If the Call-Id is not set
 *  the Pub-Client will take the Call-Id from the Pub-Client
 *  Manager. Detached Pub-Client will generate it's own Call-Id.
 * Arguments:
 * Input:     hPubClient - The Sip Stack handle to the Pub-Client
 *            strCallId - Null terminating string with the new call Id.
 * Output: None.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetCallId (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvChar              *strCallId);

/***************************************************************************
 * RvSipPubClientGetCallId
 * ------------------------------------------------------------------------
 * General:
 *  Returns the Call-Id used by the Pub-Client.
 *  If the buffer allocated by the application is insufficient
 *  an RV_ERROR_INSUFFICIENT_BUFFER status is returned and actualSize
 *  contains the size of the Call-ID string in the Pub-Client.
 *
 * Arguments:
 * Input:     hPubClient   - The Sip Stack handle to the pub-client.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The Call-Id actual size.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCallId (
                            IN  RvSipPubClientHandle   hPubClient,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize);

/***************************************************************************
 * RvSipPubClientSetFromHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the from header associated with the publish-client. If the
 *  From header was constructed by the
 *  RvSipPubClientGetNewMsgElementHandle function, the From header
 *  is attached to the publish-client object.
 *  Otherwise it will be copied by this function. Note
 *  that attempting to alter the from address after Publish has
 *  been requested might cause unexpected results.
 *
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hFrom      - Handle to an application constructed an initialized
 *                       from header.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetFromHeader (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvSipPartyHeaderHandle hFrom);

/***************************************************************************
 * RvSipPubClientGetFromHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the from header associated with the publish-client.
 *  Attempting to alter the from address after Pubitration has
 *  been requested might cause unexpected results.
 * 
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 * Output:    phFrom     - Pointer to the publish-client From header handle.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetFromHeader (
                                      IN  RvSipPubClientHandle    hPubClient,
                                      OUT RvSipPartyHeaderHandle *phFrom);

/***************************************************************************
 * RvSipPubClientSetToHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the To header associated with the publish-client. If the To
 *  header was constructed by the RvSipPubClientGetNewMsgElementHandle
 *  function, the To header will be attached to the publish-client
 *  object.
 *  Otherwise the To header is copied by this function. Note
 *  that attempting to alter the To header after publication has
 *  been requested might cause unexpected results.
 *
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hTo        - Handle to an application constructed and initialized
 *                       To header.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetToHeader (
                                      IN  RvSipPubClientHandle   hPubClient,
                                      IN  RvSipPartyHeaderHandle hTo);

/***************************************************************************
 * RvSipPubClientGetToHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the To header associated with the publish-client.
 *  Attempting to alter the To header after publication has
 *  been requested might cause unexpected results.
 *
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 * Output:    phTo       - Pointer to the publish-client To header handle.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetToHeader (
                                      IN    RvSipPubClientHandle    hPubClient,
                                      OUT   RvSipPartyHeaderHandle *phTo);


/***************************************************************************
 * RvSipPubClientGetRetryAfterInterval
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the Retry-after interval found in the last rejected response.
 *	If the retry-after is not valid, meaning that it was not appear in the last response,
 *	the return value is 0.
 *
 * Arguments:
 * Input:     hPubClient			- Handle to the publish-client.
 * Output:    pRetryAfterInterval	- The publishClient retryAfter interval.
 * 
 * Return Value: RV_OK  - If the interval value returned successfully.
 *               else error code is return.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetRetryAfterInterval (
                                      IN    RvSipPubClientHandle    hPubClient,
                                      OUT   RvUint32				*pRetryAfterInterval);

/***************************************************************************
 * SipPubClientGetSipIfMatchHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the SIP-IF-MATCH header associated with the pub-client.
 *
 * Arguments:
 * Input:     hPubClient  - Handle to the pub-client.
 * Output:    phSipIfMatch- Pointer to the transaction-client To header handle.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSipIfMatchHeader (
                                      IN    RvSipPubClientHandle	hPubClient,
                                      OUT   RvSipOtherHeaderHandle  *phSipIfMatch);


/***************************************************************************
 * RvSipPubClientSetRequestURI
 * ------------------------------------------------------------------------
 * General: 
 *  Set the SIP-Url of the Publisher in the publish-client object.
 *  Before calling Publish(), your application should use
 *  this function to supply the SIP-Url of the Publisher.
 *  The publish request will be sent to this SIP-Url.
 *  You can change the Publisher's SIP-Url each time you call
 *  Publish().
 *  This ability can be used for updating the Publisher's SIP-Url
 *  in case of redirections and refreshes.
 *  The Publisher address must be a SIP-Url with no user name.
 *
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hRequestUri - Handle to the Publisher SIP-Url to be set to the
 *                        publish-client.
 * Output: None
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetRequestURI (
                                 IN  RvSipPubClientHandle  hPubClient,
                                 IN  RvSipAddressHandle    hRequestUri);

/***************************************************************************
 * RvSipPubClientGetRequestURI
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the SIP-Url of the Publisher associated with the
 *  publish-client.
 *
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:    phRequestUri    - Handle to the Publisher SIP-Url.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetRequestURI (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipAddressHandle       *phRequestUri);

/***************************************************************************
 * RvSipPubClientSetExpiresHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Set an Expires header in the publish-client object. Before
 *  calling Publish(), the application can use this function to
 *  supply the required Expires header to use in the Publish
 *  request. This Expires header is inserted to the Publish
 *  request message before it is sent to the Publish.
 *
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hExpiresHeader - Handle to an Expires header to be set to the
 *                           publish-client.
 * Output: None
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetExpiresHeader (
                                 IN  RvSipPubClientHandle     hPubClient,
                                 IN  RvSipExpiresHeaderHandle hExpiresHeader);

/***************************************************************************
 * RvSipPubClientGetExpiresHeader
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the Expires header assosiated with the publish-client.
 *
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phExpiresHeader - Handle to the publish-client's Expires header.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetExpiresHeader (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipExpiresHeaderHandle *phExpiresHeader);

/***************************************************************************
 * RvSipPubClientGetEventHeader
 * ------------------------------------------------------------------------
 * General: Gets the Event header associated with the publish-client.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client.
 * Output:     phEventHeader - Handle to the publish-client's Event header.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetEventHeader (
                               IN  RvSipPubClientHandle      hPubClient,
                               OUT RvSipEventHeaderHandle *phEventHeader);

/***************************************************************************
 * RvSipPubClientSetEventHeader
 * ------------------------------------------------------------------------
 * General: Set an Event header in the publish-client object. Before
 *          calling Publish(), the application can use this function to
 *          supply the required Event header to use in the Publish
 *          request. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hEventHeader - Handle to an Event header to be set to the
 *                           publish-client.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetEventHeader (
                                 IN  RvSipPubClientHandle     hPubClient,
                                 IN  RvSipEventHeaderHandle hEventHeader);

/***************************************************************************
 * RvSipPubClientGetCurrentState
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the current state of the publish-client.
 *
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 * Output:    peState -  The publish-client's current state.
 * 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCurrentState (
                                      IN  RvSipPubClientHandle hPubClient,
                                      OUT RvSipPubClientState *peState);

/***************************************************************************
 * RvSipPubClientSetCSeqStep
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the CSeq-Step associated with the publish-client.
 *  The supplied CSeq must be bigger then zero.
 *  If you don't set the CSeq step the publish client will send the
 *  first publish request with CSeq=1 and will increase the CSeq for
 *  subsequent requests.
 *  Note1: Most applications do not need to use this function.
 *  The publish client manages the CSeq step automatically.
 *  Note2: The CSeq supplied using this function will be used only if
 *  the publish client detached from its manager.
 *
 * Arguments:
 * Input:     hPubClient -  The publish-client handle.
 *            cSeqStep   -  The CSeq-Step.
 * Output: None
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetCSeqStep (
                                      IN  RvSipPubClientHandle hPubClient,
                                      IN  RvUint32            cSeqStep);

/***************************************************************************
 * RvSipPubClientGetCSeqStep
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the CSeq-Step assosiated with the publish-client.
 *  Note: The CSeq-Step is valid only after a publishation request
 *  was successfully executed. Otherwise the CSeq-Step is 0.
 *
 * Arguments:
 * Input:     hPubClient - The publish-client handle.
 * Output:     pCSeqStep -  The publish-client's CSeq-Step.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCSeqStep (
                                      IN  RvSipPubClientHandle hPubClient,
                                      OUT RvUint32           *pCSeqStep);


/***************************************************************************
 * RvSipPubClientGetStackInstance
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the handle to the stack instance to which this pub-client
 *  belongs to.
 * 
 * Arguments:
 * Input:     hPubClient      - Handle to the publish-client object.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetStackInstance(
                                   IN   RvSipPubClientHandle   hPubClient,
                                   OUT  void*       *phStackInstance);

/***************************************************************************
 * RvSipPubClientSetLocalAddress
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the local address from which the Pub-Client will send outgoing
 *  requests.
 *  The stack can be configured to listen to many local addresses.
 *  Each local address has a transport type (UDP/TCP/TLS) and an address type
 *  (IPv4/IPv6). When the stack sends an outgoing request, the local address
 *  (from where the request is sent) is chosen according to the characteristics
 *  of the remote address. Both the local and remote addresses must have
 *  the same characteristics i.e. the same transport type and address type.
 *  If several configured local addresses
 *  match the remote address characteristics, the first configured address is taken.
 *  You can use RvSipPubClientSetLocalAddress() to force the Pub-Client to choose
 *  a specific local address for a specific transport and address type.
 *  For example, you can force the Pub-Client to use the second configured UDP/IPv4
 *  local address. If the Pub-Client will send a request to an
 *  UDP/IPv4 remote address it will use the local address that you set instead
 *  of the default first local address.
 *
 *  Note: The localAddress string you provide this function must match exactly
 *  to the local address that was inserted in the configuration structure in the
 *  initialization of the sip stack.
 *  If you configured the stack to listen to a 0.0.0.0 local address you must use
 *  the same notation here.
 * 
 * Arguments:
 * Input:   hPubClient     - The publish-client handle.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType   - The address type(IPv4 or IPv6).
 *          strLocalIPAddress   - A string with the local address to be set to this pub-client.
 *          localPort      - The local port to be set to this pub-client. If you set
 *                           the local port to 0, you will get a default value of 5060.
 *          if_name - local interface name
 *          hwport         - the local hardware port
 * Output: None.
 *
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetLocalAddress(
                            IN  RvSipPubClientHandle      hPubClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *strLocalIPAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                *if_name
			    , IN RvUint16                hwport
#endif
			    );

/***************************************************************************
 * RvSipPubClientGetLocalAddress
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the local address the Pub-Client will use to send outgoing
 *  requests to a destination with the supplied
 *  transport type and address type.
 *  This is the address the user set using the
 *  RvSipPubClientSetLocalAddress() function. If no address was set the
 *  function will return the default first configured local address
 *  of the requested transport and address type.
 *
 * Arguments:
 * Input:   hPubClient      - The publish-client handle
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:  strLocalIPAddress    - The local address the Pub-Client will use
 *                            for the supplied transport and address types.
 *                            (The buffer must be longer than 48 bytes).
 *          pLocalPort       - The local port the Pub-Client will use.
 *          if_name - local interface name
 *          hwport         - the local hardware port
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetLocalAddress(
                            IN  RvSipPubClientHandle      hPubClient,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *strLocalIPAddress,
                            OUT  RvUint16                *pLocalPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *hwport
#endif
			    );

/***************************************************************************
 * RvSipPubClientSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: 
 *  Sets all outbound proxy details to the publish-client object.
 *  All details are supplied in the RvSipTransportOutboundProxyCfg
 *  structure that includes parameters such as the IP address or host name,
 *  transport, port and compression type.
 *  Requests sent by this object will use the outbound details
 *  specifications as a remote address.
 *  Note: If you specify both the IP address and a host name in the
 *  configuration structure, either of them will be set but the IP
 *  address will be used. If you do not specify port or transport,
 *  both will be determined according to the DNS
 *  procedures specified in RFC 3263.
 *
 * Arguments:
 * Input:     hPubClient     - Handle to the publish-client.
 *            pOutboundCfg   - A pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 * Output: None
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetOutboundDetails(
                            IN  RvSipPubClientHandle            hPubClient,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg);

/***************************************************************************
 * RvSipPubClientGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: 
 *  Gets all the outbound proxy details that the publish-client object uses.
 *  The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *  includes parameters such as the IP address or host name, transport, port and
 *  compression type. If the outbound details were not set to the specific
 *  publish-client object, but the outbound proxy was defined to the SIP
 *  Stack on initialization, the SIP Stack parameters will be returned.
 *  If the publish-client is not using an outbound address, NULL/UNDEFINED
 *  values are returned.
 *  Note: You must supply a valid consecutive buffer in the
 *  RvSipTransportOutboundProxyCfg structure to get the outbound strings
 *  (host name and IP address).
 *
 * Arguments:
 * Input:     hPubClient     - Handle to the publish-client.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - A pointer to outbound proxy configuration structure.
 * 
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetOutboundDetails(
                            IN  RvSipPubClientHandle            hPubClient,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);


/***************************************************************************
 * RvSipPubClientSetPersistency
 * ------------------------------------------------------------------------
 * General: 
 *  Changes the Pub-Client persistency definition on run time.
 *  The function receives a boolean value that indicates whether the
 *  application wishes this Pub-Client to be persistent or not.
 *  A persistent Pub-Client object will apply its persistency to the
 *  transactions it creates. A persistent transaction will try
 *  to locate a suitable connection in the connection hash before opening
 *  a new connection.A persistent Pub-Client will also try to use the same connection
 *  for all outgoing requests by appling the last used connection object
 *  to the next created transaction.
 * 
 * Arguments:
 * Input:    hPubClient - The Pub-Client handle
 *          bIsPersistent - Determines the Pub-Client persistency definition.
 *                          RV_TRUE = persistent, RV_FALSE otherwise.
 * Output: None
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetPersistency(
                           IN RvSipPubClientHandle       hPubClient,
                           IN RvBool                  bIsPersistent);

/***************************************************************************
 * RvSipPubClientGetPersistency
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the Pub-Client persistency definition.
 *  RV_TRUE indicates that the Pub-Client is persistent, RV_FALSE
 *  otherwise.
 *
 * Arguments:
 * Input:    hPubClient - The Pub-Client handle
 * Output:  pbIsPersistent - The Pub-Client persistency definition.
 *                           RV_TRUE indicates that the Pub-Client is persistent,
 *                           RV_FALSE otherwise.
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipPubClientGetPersistency(
        IN  RvSipPubClientHandle                   hPubClient,
        OUT RvBool                                *pbIsPersistent);

/***************************************************************************
 * RvSipPubClientSetConnection
 * ------------------------------------------------------------------------
 * General: 
 *  Sets a connection object to be used by the Pub-Client transactions.
 *  The Pub-Client object holds this connection in its internal database.
 *  Whenever the Pub-Client creates a new transaction it supplies
 *  the transaction with given connection.
 *  The transaction will use the connection only if it fits the transaction's
 *  local and remote addresses.
 *  Otherwise the transaction will either locate a suitable connection in the hash
 *  or create a new connection. The Pub-Client object will be informed that
 *  the supplied connection did not fit and that a different connection was used and
 *  will update its database.
 *  Note: You can set a connection only to a persistent Pub-Client.
 * 
 * Arguments:
 * Input:  hPubClient - Handle to the pub-client
 *         hConn - Handle to the connection.
 * Output: None
 * 
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetConnection(
                   IN  RvSipPubClientHandle                hPubClient,
                   IN  RvSipTransportConnectionHandle   hConn);

/***************************************************************************
 * RvSipPubClientGetConnection
 * ------------------------------------------------------------------------
 * General: 
 *  Returns the connection that is currently beeing used by the
 *  Pub-Client.
 *  Note: only persistent Pub-Clients keep the currently used connection.
 *  NULL will be returned if the Pub-Client is not persistent.
 * 
 * Arguments:
 * Input:     hPubClient - Handle to the Pub-Client.
 * Output:    phConn - Handle to the currently used connection
 * 
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetConnection(
                            IN  RvSipPubClientHandle             hPubClient,
                           OUT  RvSipTransportConnectionHandle  *phConn);

/***************************************************************************
 * RvSipPubClientSetAppHandle
 * ------------------------------------------------------------------------
 * General: 
 *  Sets the Pub-Client application handle. Usually the application
 *  replaces handles with the stack in the
 *  RvSipPubClientMgrCreatePubClient() API function.
 *  This function is used if the application wishes to set a new
 *  application handle to the Pub-Client object
 * 
 * Arguments:
 * Input:  hPubClient    - Handle to the Pub-Client.
 *         hAppPubClient - A new application handle to set in the Pub-Client.
 * Output: None
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetAppHandle (
                                      IN  RvSipPubClientHandle     hPubClient,
                                      IN  RvSipAppPubClientHandle  hAppPubClient);

/***************************************************************************
 * RvSipPubClientGetAppHandle
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the Pub-Client application handle.
 *
 * Arguments:
 * Input:     hPubClient    - Handle to the Pub-Client.
 *            hAppPubClient - The Pub-Client application handle
 *
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetAppHandle (
                                      IN  RvSipPubClientHandle     hPubClient,
                                      IN  RvSipAppPubClientHandle *phAppPubClient);

/***************************************************************************
 * RvSipPubClientSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the publish-client to a SigComp compartment.
 *          The publish-client will use this compartment in the message
 *          compression process.
 *          Note The message will be compressed only if the remote URI includes the
 *          comp=sigcomp parameter.
 *
 * Arguments:
 * Input:     hPubClient   - The handle to the publish client.
 *            hCompartment - Handle to the SigComp Compartment.
 * Output: None
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetCompartment(
                            IN RvSipPubClientHandle   hPubClient,
                            IN RvSipCompartmentHandle hCompartment);

/***************************************************************************
 * RvSipPubClientGetCompartment
 * ------------------------------------------------------------------------
 * General: 
 *  Retrieves the SigComp compartment associated with the publish-client.
 *
 * Arguments:
 * Input:  hPubClient    - The handle to the publish client.
 * Output: phCompartment - The handle to the SigComp compartment.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCompartment(
                            IN  RvSipPubClientHandle    hPubClient,
                            OUT RvSipCompartmentHandle *phCompartment);

/***************************************************************************
 * RvSipPubClientDisableCompression
 * ------------------------------------------------------------------------
 * General:
 *  Disables the compression mechanism in a publish-client.
 *  This means that even if the message indicates compression,
 *  it will not be compressed.
 *
 * Arguments:
 * Input:  hPubClient    - The handle to the publish client.
 * Output: None
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientDisableCompression(
                                      IN  RvSipPubClientHandle hPubClient);

/***************************************************************************
* RvSipPubClientSetTranscTimers
* ------------------------------------------------------------------------
* General: 
*  Sets timeout values for the PubClient's transactions timers.
*  If some of the fields in pTimers are not set (UNDEFINED), this
*  function will calculate it, or take the values from configuration.
* 
* Arguments:
* Input: hPubClient - Handle to the PubClient object.
*        pTimers - Pointer to the struct contains all the timeout values.
*        sizeOfTimersStruct - Size of the RvSipTimers structure.
* Output: None
*
* Return Value: RvStatus.
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetTranscTimers(
                            IN  RvSipPubClientHandle  hPubClient,
                            IN  RvSipTimers           *pTimers,
                            IN  RvInt32               sizeOfTimersStruct);

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipPubClientSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: 
 *  Sets parameters that will be used while sending message,
 *  belonging to the PubClient, over SCTP.
 * 
 * Arguments:
 * Input: hPubClient - Handle to the PubClient object.
 * sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                structure.
 * pParams  - Pointer to the struct that contains all the parameters.
 * 
 * Return Value: RvStatus.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSctpMsgSendingParams(
                    IN  RvSipPubClientHandle               hPubClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/******************************************************************************
 * RvSipPubClientGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: 
 *  Gets parameters that are used while sending message, belonging
 *  to the PubClient, over SCTP.
 * 
 * Arguments:
 * Input: hPubClient - Handle to the PubClient object.
 * sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                structure.
 * pParams    - Pointer to the struct contains all the parameters.
 * 
 * Return Value: RvStatus.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSctpMsgSendingParams(
                    IN  RvSipPubClientHandle               hPubClient,
                    IN  RvUint32                           sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipPubClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: 
 *  Sets Security Object into the Pub-Client.
 *  As a result of this operation, all messages, sent by this
 *  Pub-Client, will be protected with the security mechanism,
 *  provided by the Security Object.
 * 
 * Arguments:
 * Input:  hPubClient - Handle to the Pub-Client object.
 *         hSecObj    - Handle to the Security Object. Can be NULL.
 * Output: None
 * 
 * Return Value: RvStatus
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSecObj(
                                        IN  RvSipPubClientHandle    hPubClient,
                                        IN  RvSipSecObjHandle       hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipPubClientGetSecObj
 * ----------------------------------------------------------------------------
 * General: 
 *  Gets Security Object set into the Pub-Client.
 * 
 * Arguments:
 * Input:  hPubClient - Handle to the Pub-Client object.
 * Output: phSecObj   - Handle to the Security Object.
 * 
 * Return Value: RvStatus
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSecObj(
                                        IN  RvSipPubClientHandle    hPubClient,
                                        OUT RvSipSecObjHandle*      phSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipPubClientGetCurrProcessedAuthObj
 * ------------------------------------------------------------------------
 * General: 
 *  The function retrieve the auth-object that is currently being
 *  processed by the authenticator.
 *  (for application usage in the RvSipAuthenticatorGetSharedSecretEv
 *  callback).
 * 
 * Arguments:
 * Input:     hPubClient        - Handle to the call-leg.
 * Output:    phAuthObj         - The Auth-Obj handle.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetCurrProcessedAuthObj (
                                      IN   RvSipPubClientHandle  hPubClient,
                                      OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * RvSipPubClientAuthObjGet
 * ------------------------------------------------------------------------
 * General: 
 *  The function retrieve auth-objects from the list in the Pub-Client.
 *  you may get the first/last/next object.
 *  in case of getting the next object, please supply the current
 *  object in the relative parameter.
 * 
 * Arguments:
 * Input:     hPubClient        - Handle to the Pub-Client.
 *            eLocation         - Location in the list (FIRST/NEXT/LAST)
 *            hRelativeAuthObj  - Relative object in the list (relevant for NEXT location)
 * Output:    phAuthObj         - The Auth-Obj handle.
 * 
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientAuthObjGet (
                                      IN   RvSipPubClientHandle  hPubClient,
                                      IN   RvSipListLocation     eLocation,
			                          IN   RvSipAuthObjHandle    hRelativeAuthObj,
			                          OUT  RvSipAuthObjHandle*   phAuthObj);

/***************************************************************************
 * RvSipPubClientAuthObjRemove
 * ------------------------------------------------------------------------
 * General: 
 *  The function removes an auth-obj from the list in the Pub-Client.
 *
 * Arguments:
 * Input:     hPubClient  - Handle to the Pub-Client.
 *            hAuthObj - The Auth-Obj handle.
 * Output: None.
 * Return Value: RvStatus
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientAuthObjRemove (
                                      IN   RvSipPubClientHandle hPubClient,
                                      IN   RvSipAuthObjHandle   hAuthObj);
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipPubClientSetInitialAuthorization
 * ------------------------------------------------------------------------
 * General: 
 *  Sets an initial Authorization header in the outgoing request.
 *  An initial authorization header is a header that contains
 *  only the client private identity, and not real credentials.
 *  for example:
 *  "Authorization: Digest username="user1_private@home1.net",
 *  realm="123", nonce="", uri="sip:...", response="" "
 *  The pub-client will set the initial header to the message, only if
 *  it has no other Authorization header to set.
 *
 * Arguments:
 * Input:     hPubClient     - The publish-client handle.
 *            hAuthorization - The Authorization header.
 * Output: None.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetInitialAuthorization (
                                         IN RvSipPubClientHandle hPubClient,
                                         IN RvSipAuthorizationHeaderHandle hAuthorization);

/***************************************************************************
 * RvSipPubClientSetSecAgree
 * ------------------------------------------------------------------------
 * General: 
 *  Sets a security-agreement object to this pub-client. If this
 *  security-agreement object maintains an existing agreement with the
 *  remote party, the pub-client will exploit this agreement and the data
 *  it brings. If not, the pub-client will use this security-agreement
 *  object to negotiate an agreement with the remote party.
 *
 * Arguments:
 * Input:   hPubClient   - Handle to the pub-client.
 *          hSecAgree    - Handle to the security-agreement.
 * Output: None
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientSetSecAgree(
							IN  RvSipPubClientHandle         hPubClient,
							IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * RvSipPubClientGetSecAgree
 * ------------------------------------------------------------------------
 * General: 
 *  Gets the security-agreement object associated with the pub-client.
 *  The security-agreement object captures the security-agreement with
 *  the remote party as defined in RFC3329.
 * Arguments:
 * Input:   hPubClient    - Handle to the pub-client.
 *          hSecAgree   - Handle to the security-agreement.
 * Output: None
 *
 * Return Value: RvStatus.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipPubClientGetSecAgree(
									IN   RvSipPubClientHandle    hPubClient,
									OUT  RvSipSecAgreeHandle    *phSecAgree);

#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef __cplusplus
}
#endif

#endif /*RV_SIP_PRIMITIVES*/
#endif /* END OF: #ifndef RV_SIP_PUB_CLIENT_H */
