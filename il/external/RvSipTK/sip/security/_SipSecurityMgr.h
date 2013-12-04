/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <_SipSecurityMgr.h>
 *
 * The _SipSecurityMgr.h file contains Internal API functions of the Security
 * Module, including the module construct and destruct and resource status API.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/


#ifndef SIP_SECURITY_MGR_H
#define SIP_SECURITY_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipResourcesTypes.h"
#include "RvSipSecurityTypes.h"
#include "_SipCommonUtils.h"
#include "AdsRlist.h"
#include "rvlog.h"


#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/* SipSecurityMgrCfg
 * ----------------------------------------------------------------------------
 * SipSecurityMgrCfg unites parameters of Security Manager configuration
 *
 * maxNumOfSecObj - Maximal number of coexisting Security Objects
 * maxNumOfIpsecSessions - Maximal number of coexisting IPsec sessions.
 *                  Each Security Object handles no more than one IPsec session
 * spiRangeStart  - lowest SPI, that can be used by Security Manager
 * spiRangeEnd - highest SPI, that can be used by Security Manager
 * pLogMgr        - Handle to the Log Manager
 * pLogSrc        - Handle to the Log Source
 * hMsgMgr        - Handle to the Message Manager
 * hTransportMgr  - Handle to the Transport Manager
 * hStack         - Handle to the SIP Toolkit Stack instance
 * pSeed          - Pointer to seed, used for random number generating
 */
typedef struct
{
    RvInt32                 maxNumOfSecObj;
    RvInt32                 maxNumOfIpsecSessions;
    RvUint32                spiRangeStart;
    RvUint32                spiRangeEnd;
    RvLogMgr*               pLogMgr;
    RvLogSource*            pLogSrc;
    RvSipMsgMgrHandle       hMsgMgr;
    RvSipTransportMgrHandle hTransportMgr;
    void*                   hStack;
    RvBool                  bTcpEnabled;
    RvRandomGenerator*      pSeed;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    void*                   (*getAlternativeSecObjFunc)(void* so,int isResp);
#endif
//SPIRENT_END
} SipSecurityMgrCfg;

/******************************************************************************
 * SipSecurityMgrConstruct
 * ----------------------------------------------------------------------------
 * General: Construct a new Security manager.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg          - Handle to the configuration
 * Output:  phSecurityMgr - Handle to the created Security Manager.
 *****************************************************************************/
RvStatus SipSecurityMgrConstruct(
                                IN  SipSecurityMgrCfg* pCfg,
                                OUT RvSipSecMgrHandle* phSecMgr);

/******************************************************************************
 * SipSecurityMgrDestruct
 * ----------------------------------------------------------------------------
 * General: Destructs the Security manager freeing all resources.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr - Handle to the manager to be destructed
 *****************************************************************************/

RvStatus SipSecurityMgrDestruct(
                                IN RvSipSecMgrHandle hSecMgr);

/******************************************************************************
 * SipSecurityMgrGetResourcesStatus
 * ----------------------------------------------------------------------------
 * General: Fills the provided resources structure with the status of
 *          resources used by the Security module.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - Handle to the Security manager.
 * Output:  pResources - Security resources.
 *****************************************************************************/
RvStatus SipSecurityMgrGetResourcesStatus (
                                IN  RvSipSecMgrHandle       hSecMgr,
                                OUT RvSipSecurityResources* pResources);

/******************************************************************************
 * SipSecurityMgrResetMaxUsageResourcesStatus
 * ----------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of call-legs that
 *          were used ( at one time ) until the call to this routine.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecurityMgr - The Security manager.
 *****************************************************************************/
void SipSecurityMgrResetMaxUsageResourcesStatus (
                                IN  RvSipSecMgrHandle  hSecMgr);

/******************************************************************************
 * SipSecMgrSetEvHandlers
 * ----------------------------------------------------------------------------
 * General: Set owner event handlers for all Security events.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr     - Handle to the Security Manager
 *            pEvHandlers - Structure with owner event handlers
 *****************************************************************************/
RvStatus SipSecMgrSetEvHandlers(
                                   IN  RvSipSecMgrHandle   hSecMgr,
                                   IN  RvSipSecEvHandlers* pEvHandlers);

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_SECURITY_MGR_H */



