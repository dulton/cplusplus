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
 *                              <PubClientObject.c>
 *
 * This file defines the PubClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/

#ifndef PUB_CLIENT_OBJECT_H
#define PUB_CLIENT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipPubClientTypes.h"
#include "_SipTransaction.h"
#include "PubClientMgrObject.h"
#include "RvSipExpiresHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipAuthenticator.h"
#include "_SipCommonTypes.h"

#include "TranscClientObject.h"

#ifdef RV_SIP_AUTH_ON
#include "_SipAuthenticator.h"
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

#define SIP_ETAG_HEADER_NAME "SIP-ETag"
#define SIP_IF_MATCH_HEADER_NAME "SIP-If-Match"


/* PubClient
 * --------------------------------------------------------------------------
 * The PubClient is a PUBLISH client object. It is used to request publication
 * from a chosen server according to the rules defined in the RFC 3903.
 * It supportes redirect and un-authenticated responses.
 *
 * pAuthListInfo -- Information pubarding to the hListAuthObj.
 * hListAuthObj  -- List of Authentication Data structures,
 *                  each of which contains challenge, received with 401/407 response.
 * hInitialAuthorization     -- Handle to the empty authorization header, to be set in
 *                            the outgoing initial request.
 * -----------------------
 * version 4.5 parameters
 * -----------------------
 * hSecObj       -- Handle to the Security Object
 * hSecAgree         -- A security-agreement associated with this pub-client. The pub-client will 
 *                      obtain a security-agreement with the server and maintain it via 
 *                      this security-agreement object.
 */
typedef struct
{
    HPAGE                           hPage;				/* an inner memory page */
	HPAGE							hSipIfMatchPage;	/* an inner memory page used for the sip-if-match header stored int he pubClient*/
    RvSipAppPubClientHandle         hAppPubClient;		/* The application handle to the publish-client*/
    RvSipPubClientState             eState;				/* The inner state of the publish-client */

	RvSipOtherHeaderHandle			hSipIfMatchHeader;

	RvSipEventHeaderHandle			hEventHeader;		/*The handle of the event-header of this publish client.*/
    PubClientMgr                   *pMgr;				/* the publish-clients manager */
    SipTripleLock                  *tripleLock;			/* Processing/API & object locks */

	RvBool							bWasPublished;
    RvSipPubClientEvHandlers        *pubClientEvHandlers;
	
	RvBool							bRemoveInProgress; /*Indicate whether the publish client is in removing progress*/
	
	RvUint32						retryAfter;		   /*The value of the retry-after header found in the last reject response received.*/
	
	SipTranscClient					transcClient;


} PubClient;


/***************************************************************************
 * PubClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new publish-client object.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   PubClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - pointer to the publish-clients manager.
 *          pPubClient - pointer to the new publish-client object.
 *          hAppPubClient - The API's handle to the publish-client.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientInitiate(
                                  IN PubClientMgr           *pPubClientMgr,
                                  IN PubClient              *pPubClient,
                                  IN RvSipAppPubClientHandle hAppPubClient);


/***************************************************************************
 * PubClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a publish-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClientMgr - pointer to the publish-clients manager.
 *          pPubClient - pointer to the publish-client object.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientDestruct(IN PubClient *pPubClient);

/***************************************************************************
 * PubClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the publish-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the publish-client.
 *        eReason - The reason for the state change.
 *        pPubClient - The publish-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV PubClientChangeState(
                            IN  RvSipPubClientState             eNewState,
                            IN  RvSipPubClientStateChangeReason eReason,
                            IN  PubClient                       *pPubClient);



/***************************************************************************
 * PubClientAddContactsToMsg
 * ------------------------------------------------------------------------
 * General: Inserts to the message the list of Contact headers assosiated
 *          with this publish-client.
 * Return Value: RV_ERROR_NULLPTR - pTransaction or hMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pPubClient - The handle to the publish-client.
 *          hMsg - The message to insert the Contact headers to.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientAddContactsToMsg(
                                            IN PubClient     *pPubClient,
                                            IN RvSipMsgHandle hMsg);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * PubClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to API schema using the transcClient layer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
RvStatus RVCALLCONV PubClientLockAPI(IN  PubClient *pPubClient);


/************************************************************************************
 * PubClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
void RVCALLCONV PubClientUnLockAPI(IN  PubClient *pPubClient);

/************************************************************************************
 * PubClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks pub-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
RvStatus RVCALLCONV PubClientLockMsg(IN  PubClient *pPubClient);


/************************************************************************************
 * PubClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks pub-client according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient - pointer to the pub-client.
***********************************************************************************/
void RVCALLCONV PubClientUnLockMsg(IN  PubClient *pPubClient);

#else
#define PubClientLockAPI(a) (RV_OK)
#define PubClientUnLockAPI(a)
#define PubClientLockMsg(a) (RV_OK)
#define PubClientUnLockMsg(a)
#endif

/***************************************************************************
 * PubClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the publish-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV PubClientMgrLock(IN PubClientMgr *pPubClientMgr);


/***************************************************************************
 * PubClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the publish-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV PubClientMgrUnLock(IN PubClientMgr *pPubClientMgr);

/***************************************************************************
 * SipPubClientAddSipIfMatchHeaderIfNeeded
 * ------------------------------------------------------------------------
 * General: This function responsible to add the sip-if-match header known by this pub client.
 *			The sip-if-match is used by the publisher to identify the incoming publish-client.
 * Return Value: - RV_OK - when the sip-if match header was succesfully inserted into the message or
 *				   the sip-if-match is unknown (i.e. pub-client didn't published yet), otherwise rv_error.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: ppubClient - The pub-client that is going to be publishing
 *			 hMsgToSend - The message to be update.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientAddSipIfMatchHeaderIfNeeded(IN PubClient *pPubClient, 
															IN RvSipMsgHandle hMsgToSend);

/***************************************************************************
 * SipPubClientAddEventHeaderIfNeeded
 * ------------------------------------------------------------------------
 * General: This function responsible to add the Event header known by this pub client.
 *			Adding the event header will be only if such header doesn't exist in the message.
 * Return Value: - RV_OK - when the Event header was succesfully inserted into the message or
 *				   the event header already exists , otherwise rv_error.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: ppubClient - The pub-client that is going to be publishing
 *			 hMsgToSend - The message to be update.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientAddEventHeaderIfNeeded(IN PubClient *pPubClient, 
													   IN RvSipMsgHandle hMsgToSend);

/***************************************************************************
 * PubClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object, Free the resources taken by it and
 *          remove it from the manager call-leg list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 ***************************************************************************/
void RVCALLCONV PubClientTerminate(IN PubClient *pPubClient);

/***************************************************************************
 * PubClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the PubClient as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient    - pointer to PubClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus PubClientAttachOnConnection(IN PubClient                        *pPubClient,
                                 IN RvSipTransportConnectionHandle  hConn);


/***************************************************************************
 * PubClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the PubClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pPubClient    - pointer to PubClient
 ***************************************************************************/
void PubClientDetachFromConnection(IN PubClient        *pPubClient);

/***************************************************************************
 * PubClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV PubClientStateIntoString(
                                            IN RvSipPubClientState eNewState);

/***************************************************************************
 * PubClientUpdateSipIfMatchHeader
 * ------------------------------------------------------------------------
 * General: This function will invoked upon response received and will update the 
 *			SIP-If-Match header of the pub client from the SipEtag header exist in this response.
 * Return Value: pPubClient - the publish client to be update.
 *				 hRecievedMsg - the response received
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvStatus PubClientUpdateSipIfMatchHeader(IN PubClient *pPubClient,RvSipMsgHandle hRecievedMsg);

/***************************************************************************
 * PubClientSetEventHeader
 * ------------------------------------------------------------------------
 * General: Set an Event header in the publish-client object. Before
 *          calling Publish(), the application can use this function to
 *          supply the required Event header to use in the Publish
 *          request. 
 * Return Value: RV_ERROR_INVALID_HANDLE  - The publish-client handle is invalid.
 *                 RV_ERROR_BADPARAM - The supplied Event header is not
 *                                     supported or illegal.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK        - New Event header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPubClient - Handle to the publish-client.
 *            hEventHeader - Handle to an Event header to be set to the
 *                           publish-client.
 ***************************************************************************/
RvStatus RVCALLCONV PubClientSetEventHeader (
                                 IN  PubClient				*pPubClient,
                                 IN  RvSipEventHeaderHandle hEventHeader);


#endif /* RV_SIP_PRIMITIVES */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * PubClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Pub-Client.
 *          As a result of this operation, all messages, sent by this
 *          Pub-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pPubClient - Pointer to the Pub-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus PubClientSetSecObj(IN  PubClient*        pPubClient,
                            IN  RvSipSecObjHandle hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

/******************************************************************************
 * PubClientSetAlertTimerIfNeeded
 * ----------------------------------------------------------------------------
 * General: This method set a timer for the transcClient to invoke when the object is expired.
 * Return Value: 
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pPubClient	- Pointer to the Pub-Client object.
 *          hMsg		- The received message, used to get the expires header.
 *****************************************************************************/
RvStatus PubClientSetAlertTimerIfNeeded(IN PubClient					*pPubClient, 
									IN  RvSipMsgHandle				hMsg);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef PUB_CLIENT_OBJECT_H */

