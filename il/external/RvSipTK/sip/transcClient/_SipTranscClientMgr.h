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
 *                              <SipTranscClientMgr.h>
 *
 *  The SipTranscClientMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the transaction-clients manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef SIP_TRANSC_CLIENTS_MGR_H
#define SIP_TRANSC_CLIENTS_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "_SipTranscClientTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "AdsRpool.h"
#include "RvSipAuthenticator.h"
#include "RvSipResourcesTypes.h"
#include "rvlog.h"
#include "TranscClientObject.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

#define DEFAULT_MAX_NUM_OF_TRANSC_CLIENTS 5
#define DEFAULT_MAX_NUM_OF_CONTACT_HEADERS 10

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvLogMgr                 *pLogMgr;
    RvLogSource              *moduleLogId;
    RvSipTranscMgrHandle      hTranscMgr;
    RvSipTransportMgrHandle   hTransport;
    RvSipMsgMgrHandle         hMsgMgr;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
   RvRandomGenerator        *seed;
#ifdef RV_SIP_AUTH_ON
    RvSipAuthenticatorHandle  hAuthModule;
#endif /* #ifdef RV_SIP_AUTH_ON */
    RvBool                    bIsPersistent;

#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle         hSecMgr;
	RvSipSecAgreeMgrHandle	  hSecAgreeMgr;
#endif
} SipTranscClientMgrCfg;

/***************************************************************************
 * SipTranscClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new transaction-client manager. The transaction-client
 *          manager is encharged of all the transaction-clients. The manager
 *          holds a list of transaction-clients and is used to create new
 *          call-legs.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the transaction-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   transaction-client manager.
 *               RV_OK -        transaction-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each transaction-client for internal
 *                          memory allocations.
 *          hAuthModule   - A handle to the authentication module
 * Input and Output:
 *          maxNumTranscClients - Max number of concurrent transaction-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack -				Handle to the stack manager.
 * Output:  *phTranscClientMgr -	Handle to the newly created transaction-client
 *										manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrConstruct(
                        INOUT SipTranscClientMgrCfg       *pCfg,
                        IN    void*          hStack,
                         OUT   SipTranscClientMgrHandle  *phTranscClientMgr);


/***************************************************************************
 * SipTranscClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the transaction-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The transaction-client handle is invalid.
 *                  RV_OK -  transaction-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClientMgr - Handle to the manager to destruct.
 *			eOwnerType		 - The type of the owner that asked for destruction.
 ***************************************************************************/

RvStatus RVCALLCONV SipTranscClientMgrDestruct(
                                   IN SipTranscClientMgrHandle hTranscClientMgr);

/***************************************************************************
 * SipTranscClientConstruct
 * ------------------------------------------------------------------------
 * General: Populate all relevalt memebers of this transaction-client object.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The transaction-client handle is invalid.
 *                  RV_OK -  transaction-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscClientMgr - Handle to the manager to destruct.
 *			eOwnerType		 - The owner that asked to destruct.
 *			pOwnerLockUtils  - This struct holds the lock, unlock and vacancy check of the owner.
 ***************************************************************************/
RV_Status SipTranscClientConstruct(SipTranscClientMgrHandle	 hTranscClientMgr, 
								  SipTranscClient			 *pTranscClient, 
								  SipTranscClientOwnerHandle hOwner,
								  RvSipCommonStackObjectType eOwnerType,
								  SipTranscClientOwnerUtils	 *pOwnerLockUtils,
								  SipTripleLock				 **tripleLock);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTranscClientsMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in transc-client mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the transc-client manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscClientMgrSetSecAgreeMgrHandle(
                                        IN SipTranscClientMgrHandle   hTranscClientsMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr,
										IN RvSipCommonStackObjectType eOwnerType);
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_TRANSC_CLIENTS_MGR_H */






















