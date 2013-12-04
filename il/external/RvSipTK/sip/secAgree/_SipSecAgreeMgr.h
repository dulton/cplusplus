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
 *                              <_SipSecAgreeMgr.h>
 *
 * This file defines the internal API of the security-agreement manager object.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef _SIP_SEC_AGREE_MGR_H
#define _SIP_SEC_AGREE_MGR_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "_SipSecAgreeTypes.h"
#include "RvSipResourcesTypes.h"
#include "RvSipMsg.h"
#include "_SipTransportTypes.h"
#include "_SipCommonTypes.h"
#include "RvSipSecurityTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* SipSecAgreeMgrCfg
 * --------------------------------------------------------------------------
 * The configuration parameters that are required in order to construct a
 * security-agreement manager
 */
typedef struct
{
	void*                     hSipStack;
    HRPOOL                    hMemoryPool;
	RvLogSource*              pLogSrc;
	RvLogMgr*                 pLogMgr;
	RvSipTransportMgrHandle   hTransport;
	RvSipSecMgrHandle         hSecMgr;
	RvSipAuthenticatorHandle  hAuthenticator;
	RvSipMsgMgrHandle         hMsgMgr;
	RvRandomGenerator*        seed;
    RvInt32                   maxSecAgrees;
	RvInt32                   maxSecAgreeOwners;
	RvInt32					  maxOwnersPerSecAgree;
	
} SipSecAgreeMgrCfg;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipSecAgreeMgrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes the security-manager object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConfiguration   - A structure off initialized configuration parameters.
 * Output:  phSecAgreeMgr    - The newly created Security-Mgr
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrConstruct(
							IN   SipSecAgreeMgrCfg*           pConfiguration,
							OUT  RvSipSecAgreeMgrHandle*      phSecAgreeMgr);

/***************************************************************************
 * SipSecAgreeMgrDestruct
 * ------------------------------------------------------------------------
 * General: Deallocates all resources held by the security-manager
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr - Handle to the Security-Mgr to terminate.
 ***************************************************************************/
void RVCALLCONV SipSecAgreeMgrDestruct(
                                 IN RvSipSecAgreeMgrHandle      hSecAgreeMgr);

/***************************************************************************
 * SipSecAgreeMgrHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : modifies the state of the current 
 *          security-agreement according to the received message, calls
 *          SecAgreeRequiredEv if necessary, and loads any additional security
 *          agreement information.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr    - Handle to the Security-Mgr
 *         hSecAgree       - Handle to the current security-agreement
 *         pOwner          - Pointer to the owner that received the message
 *         eOwnerType      - The type of the supplied owner
 *         pOwnerLock      - The triple lock of the owner, to open before callback
 *         hMsg            - Handle to the message
 *         hLocalAddr      - The address that the message was received on. Required
 *                           only for requests.
 *         pRcvdFromAddr   - The address the message was received from. Required
 *                           only for requests.
 *         hRcvdOnConn     - The connection the message was received on. Required
 *                           only for requests.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrHandleMsgRcvd(
							 IN  RvSipSecAgreeMgrHandle          hSecAgreeMgr,
							 IN  RvSipSecAgreeHandle		     hSecAgree,
							 IN  void                           *pOwner,
							 IN  RvSipCommonStackObjectType      eOwnerType,
							 IN  SipTripleLock                  *pOwnerLock,
							 IN  RvSipMsgHandle                  hMsg,
							 IN  RvSipTransportLocalAddrHandle   hLocalAddr,
							 IN  SipTransportAddress            *pRcvdFromAddr,
							 IN	 RvSipTransportConnectionHandle  hRcvdOnConn);

/***************************************************************************
 * SipSecAgreeMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the securit-agreement module, i.e., the security-agreement list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr - Handle to the security-agreement manager.
 * Output:    pResources   - Pointer to a security-agreement resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrGetResourcesStatus(
                                 IN  RvSipSecAgreeMgrHandle   hSecAgreeMgr,
                                 OUT RvSipSecAgreeResources  *pResources);

/***************************************************************************
 * SipSecAgreeMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Resets the counts of max number of security-agreements used 
 *          simultaneously so far.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecAgreeMgr - The security-agreement manager.
 ***************************************************************************/
void RVCALLCONV SipSecAgreeMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipSecAgreeMgrHandle  hSecAgreeMgr);

/***************************************************************************
 * SipSecAgreeMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets an event handlers structure to the security-manager object.
 *          The security-manager stores them according to the type of owner.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgreeMgr    - Handle to the Security-Mgr
 *          pEvHandlers     - The structure of even handlers
 *          eOwnerType      - The type of owner setting the event handlers
 *                            (there is a different set of handlers for each
 *                             type of owner).
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrSetEvHandlers(
							IN  RvSipSecAgreeMgrHandle           hSecAgreeMgr,
							IN  SipSecAgreeEvHandlers           *pEvHandlers,
							IN  RvSipCommonStackObjectType       eOwnerType);

/***************************************************************************
 * SipSecAgreeMgrIsInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks whether the received request contains Require or Proxy-Require
 *          with sec-agree, but more than one Via. This message must be rejected
 *          with 502 (Bad Gateway).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr	 - Handle to the security-agreement manager
 *         hMsg          - Handle to the message
 * Output: pResponseCode - 502 if this is invalid response. 0 otherwise.
 ***************************************************************************/
RvStatus RVCALLCONV SipSecAgreeMgrIsInvalidRequest(
							 IN  RvSipSecAgreeMgrHandle   hSecAgreeMgr,
							 IN  RvSipMsgHandle           hMsg,
							 OUT RvUint16*                pResponseCode);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _SIP_SEC_AGREE_MGR_H*/

