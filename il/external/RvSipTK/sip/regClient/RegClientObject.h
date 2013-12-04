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
 * This file defines the RegClient object, attributes and interface functions.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *********************************************************************************/

#ifndef REG_CLIENT_OBJECT_H
#define REG_CLIENT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipRegClientTypes.h"
#include "_SipTransaction.h"
#include "RegClientMgrObject.h"
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


/* RegClient
 * --------------------------------------------------------------------------
 * The RegClient is a REGISTER client object. It is used to request registration
 * from a chosen server according to the rules defined in the RFC 2543.
 * It supportes redirect and un-authenticated responses.
 *
 * pAuthListInfo -- Information regarding to the hListAuthObj.
 * hListAuthObj  -- List of Authentication Data structures,
 *                  each of which contains challenge, received with 401/407 response.
 * hInitialAuthorization     -- Handle to the empty authorization header, to be set in
 *                            the outgoing initial request.
 */
typedef struct
{

    HPAGE                           hContactPage;    /* an inner memory page, holding the contact list and headers */
    RvSipAppRegClientHandle         hAppRegClient;   /* The application handle to the register-client*/
    RvSipRegClientState             eState;          /* The inner state of the register-client */

	RvBool							bIsDetach;			/*true if the transc client is independent in managing its call-id and cseq step */

	RLIST_HANDLE                    hContactList;    /* A list of all Contact headers assosiated with this
                                                        register-client */
    RegClientMgr                   *pMgr;            /* the register-clients manager */
    SipTripleLock                  *tripleLock;     /* Processing/API & object locks */
    
	RvSipRegClientEvHandlers        *regClientEvHandlers;
#ifdef SUPPORT_EV_HANDLER_PER_OBJ
	RvSipRegClientEvHandlers		objEvHandlers;
#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/

	SipTranscClient					transcClient;


} RegClient;


/***************************************************************************
 * RegClientInitiate
 * ------------------------------------------------------------------------
 * General: Initiate a new register-client object.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough resources to initiate a new
 *                                   RegClient object.
 *               RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - pointer to the register-clients manager.
 *          pRegClient - pointer to the new register-client object.
 *          hAppRegClient - The API's handle to the register-client.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientInitiate(
                                  IN RegClientMgr           *pRegClientMgr,
                                  IN RegClient              *pRegClient,
                                  IN RvSipAppRegClientHandle hAppRegClient);


/***************************************************************************
 * RegClientDestruct
 * ------------------------------------------------------------------------
 * General: destruct a register-client object. Recycle all allocated memory.
 * Return Value: RV_OK -        Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - pointer to the register-clients manager.
 *          pRegClient - pointer to the register-client object.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientDestruct(IN RegClient *pRegClient);

/***************************************************************************
 * RegClientChangeState
 * ------------------------------------------------------------------------
 * General: Change the register-client state to the state given. Call the
 *          EvStateChanged callback with the new state.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The new state of the register-client.
 *        eReason - The reason for the state change.
 *        pRegClient - The register-client in which the state has
 *                       changed.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV RegClientChangeState(
                            IN  RvSipRegClientState             eNewState,
                            IN  RvSipRegClientStateChangeReason eReason,
                            IN  RegClient                       *pRegClient);



/***************************************************************************
 * RegClientAddContactsToMsg
 * ------------------------------------------------------------------------
 * General: Inserts to the message the list of Contact headers assosiated
 *          with this register-client.
 * Return Value: RV_ERROR_NULLPTR - pTransaction or hMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 *               RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pRegClient - The handle to the register-client.
 *          hMsg - The message to insert the Contact headers to.
 ***************************************************************************/
RvStatus RVCALLCONV RegClientAddContactsToMsg(
                                            IN RegClient     *pRegClient,
                                            IN RvSipMsgHandle hMsg);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * RegClientLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
RvStatus RVCALLCONV RegClientLockAPI(IN  RegClient *pRegClient);


/************************************************************************************
 * RegClientUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
void RVCALLCONV RegClientUnLockAPI(IN  RegClient *pRegClient);

/************************************************************************************
 * RegClientLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks regclient according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
RvStatus RVCALLCONV RegClientLockMsg(IN  RegClient *pRegClient);


/************************************************************************************
 * RegClientUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks regclient according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient - pointer to the regclient.
***********************************************************************************/
void RVCALLCONV RegClientUnLockMsg(IN  RegClient *pRegClient);

#else
#define RegClientLockAPI(a) (RV_OK)
#define RegClientUnLockAPI(a)
#define RegClientLockMsg(a) (RV_OK)
#define RegClientUnLockMsg(a)
#endif

/***************************************************************************
 * RegClientMgrLock
 * ------------------------------------------------------------------------
 * General: Lock the register-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV RegClientMgrLock(IN RegClientMgr *pRegClientMgr);


/***************************************************************************
 * RegClientMgrUnLock
 * ------------------------------------------------------------------------
 * General: Lock the register-client manager.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
void RVCALLCONV RegClientMgrUnLock(IN RegClientMgr *pRegClientMgr);

/***************************************************************************
 * RegClientTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object, Free the resources taken by it and
 *          remove it from the manager call-leg list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 *          eReason - termination reason
 ***************************************************************************/
void RVCALLCONV RegClientTerminate(IN RegClient *pRegClient,
                                   RvSipRegClientStateChangeReason eReason);

/***************************************************************************
 * RegClientAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the RegClient as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient    - pointer to RegClient
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus RegClientAttachOnConnection(IN RegClient                        *pRegClient,
                                 IN RvSipTransportConnectionHandle  hConn);


/***************************************************************************
 * RegClientDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the RegClient from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClient    - pointer to RegClient
 ***************************************************************************/
void RegClientDetachFromConnection(IN RegClient        *pRegClient);


/***************************************************************************
 * RegClientStateIntoString
 * ------------------------------------------------------------------------
 * General: returns the string for the state received as enumeration.
 * Return Value: The state string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eNewState - The state enumeration.
 ***************************************************************************/
RvChar *RVCALLCONV RegClientStateIntoString(
                                            IN RvSipRegClientState eNewState);


/***************************************************************************
 * RegClientGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a unique CallId for this re-boot cycle.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The register-client manager page was full
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRegClientMgr - Pointer to the register-clients manager.
 *          hPage - The memory page to use.
 * Output:  pstrCallId - Pointer to the call-id offset
 *          ppCallId - Pointer to the call-id pointer offset (NULL when sip-debug)
 ***************************************************************************/
RvStatus RVCALLCONV RegClientGenerateCallId(
                                         IN  RegClientMgr            *pRegClientMgr,
                                         IN  HPAGE                    hPage,
                                         OUT RvInt32                *pstrCallId,
                                         OUT RvChar                **ppCallId);

#endif /* RV_SIP_PRIMITIVES */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RegClientSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Reg-Client.
 *          As a result of this operation, all messages, sent by this
 *          Reg-Client, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pRegClient - Pointer to the Reg-Client object.
 *          hSecObj    - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus RegClientSetSecObj(IN  RegClient*        pRegClient,
                            IN  RvSipSecObjHandle hSecObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

/******************************************************************************
 * RegClientSetAlertTimerIfNeeded
 * ----------------------------------------------------------------------------
 * General: This method set a timer for the transcClient to invoke when the object is expired.
 * Return Value: 
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pRegClient	- Pointer to the Reg-Client object.
 *          hMsg		- The received message, used to get the expires header.
 *****************************************************************************/
void RegClientSetAlertTimerIfNeeded(IN RegClient					*pRegClient, 
									IN  RvSipMsgHandle				hMsg);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef REG_CLIENT_OBJECT_H */










































