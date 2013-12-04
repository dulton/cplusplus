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
 *                              <SipPubClientMgr.h>
 *
 *  The SipPubClientMgr.h file contains Internal Api functions Including the
 *  construct and destruct of the publish-client manager module and getting the
 *  module resource status.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govrin                 Aug 2006
 *********************************************************************************/


#ifndef SIP_PUB_CLIENT_MGR_H
#define SIP_PUB_CLIENT_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipPubClientTypes.h"
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

#define DEFAULT_MAX_NUM_OF_PUB_CLIENTS 5
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
    RvUint32                  maxNumOfPubClients;
    RvRandomGenerator        *seed;
    RvBool                    bIsPersistent;
	RvInt32					  alertTimeout;
	
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle         hSecMgr;
#endif
} SipPubClientMgrCfg;

/***************************************************************************
 * SipPubClientMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new publish-client manager. The publish-client
 *          manager is encharged of all the publish-clients. The manager
 *          holds a list of publish-clients and is used to create new
 *          call-legs.
 * Return Value: RV_ERROR_NULLPTR -     The pointer to the publish-client mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                   publish-client manager.
 *               RV_OK -        publish-client manager was created
 *                                   Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hLog -          Handle to the log module.
 *            moduleLogId -   The module log id used for printing.
 *            hTranscMgr -    The transaction manager handle.
 *          hGeneralPool  - Pool used by each publish-client for internal
 *                          memory allocations.
 * Input and Output:
 *          maxNumPubClients - Max number of concurrent publish-clients.
 *          maxNumOfContactHeaders - Max number of Contact headers in all
 *                                   transactions.
 *          hStack - Handle to the stack manager.
 * Output:     *phPubClientMgr - Handle to the newly created publish-client
 *                            manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrConstruct(
                        INOUT SipPubClientMgrCfg       *pCfg,
                        IN    void*          hStack,
                         OUT   RvSipPubClientMgrHandle  *phPubClientMgr);


/***************************************************************************
 * SipPubClientMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the publish-client manager freeing all resources.
 * Return Value:    RV_ERROR_INVALID_HANDLE - The publish-client handle is invalid.
 *                  RV_OK -  publish-client manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPubClientMgr - Handle to the manager to destruct.
 ***************************************************************************/

RvStatus RVCALLCONV SipPubClientMgrDestruct(
                                   IN RvSipPubClientMgrHandle hPubClientMgr);





/***************************************************************************
 * SipPubClientMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the publish-client module. It includes the publish-client list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrGetResourcesStatus (
                                 IN  RvSipPubClientMgrHandle   hMgr,
                                 OUT RvSipPubClientResources  *pResources);

/***************************************************************************
 * SipPubClientMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of pub-clients that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The pub-client manager.
 ***************************************************************************/
void RVCALLCONV SipPubClientMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipPubClientMgrHandle  hMgr);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipPubClientMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in pub-client mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the pub-client manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipPubClientMgrSetSecAgreeMgrHandle(
                                        IN RvSipPubClientMgrHandle   hPubClientMgr,
                                        IN RvSipSecAgreeMgrHandle    hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_PUB_CLIENT_MGR_H */

