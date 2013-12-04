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
 *                              <SipRegClientMgr.h>
 *
 *  The SipRegClientMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the register-client manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2001
 *********************************************************************************/


#ifndef SIP_REG_CLIENT_MGR_H
#define SIP_REG_CLIENT_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipRegClientTypes.h"
#include "_SipTranscClientTypes.h"
#include "_SipCommonUtils.h"
#include "_SipTransactionMgr.h"
#include "AdsRpool.h"
#include "RvSipAuthenticator.h"
#include "RvSipResourcesTypes.h"

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/ 
#include "rvlog.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

#define DEFAULT_MAX_NUM_OF_REG_CLIENTS 5
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
	SipTranscClientMgrHandle  hTranscClientMgr;
    RvSipMsgMgrHandle         hMsgMgr;
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    RvUint32                  maxNumOfRegClients;
    RvRandomGenerator        *seed;
    RvBool                    bIsPersistent;
	RvInt32					  alertTimeout;
	
} SipRegClientMgrCfg;

/***************************************************************************
 * SipRegClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new register-client manager. The register-client
 *          manager is encharged of all the register-clients. The manager
 *          holds a list of register-clients and is used to create new
 *          call-legs.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the register-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   register-client manager.
 *               RV_OK -        register-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each register-client for internal
 *                          memory allocations.
 * Input and Output:
 *          maxNumRegClients - Max number of concurrent register-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack - Handle to the stack manager.
 * Output:     *phRegClientMgr - Handle to the newly created register-client
 *                            manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrConstruct(
                        INOUT SipRegClientMgrCfg       *pCfg,
                        IN    void*          hStack,
                         OUT   RvSipRegClientMgrHandle  *phRegClientMgr);


/***************************************************************************
 * SipRegClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the register-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The register-client handle is invalid.
 *                  RV_OK -  register-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClientMgr - Handle to the manager to destruct.
 ***************************************************************************/

RvStatus RVCALLCONV SipRegClientMgrDestruct(
                                   IN RvSipRegClientMgrHandle hRegClientMgr);





/***************************************************************************
 * SipRegClientMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the register-client module. It includes the register-client list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrGetResourcesStatus (
                                 IN  RvSipRegClientMgrHandle   hMgr,
                                 OUT RvSipRegClientResources  *pResources);

/***************************************************************************
 * SipRegClientMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of reg-clients that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The reg-client manager.
 ***************************************************************************/
void RVCALLCONV SipRegClientMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipRegClientMgrHandle  hMgr);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipRegClientMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in reg-client mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the reg-client manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipRegClientMgrSetSecAgreeMgrHandle(
                                        IN RvSipRegClientMgrHandle   hRegClientMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_REG_CLIENT_MGR_H */

