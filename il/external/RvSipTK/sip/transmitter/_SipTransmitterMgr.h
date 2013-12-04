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
 *                              <_SipTransmitterMgr.h>
 *
 *  The _SipTransmitterMgr.h file contains Internal API functions of the
 *  Transmitter manager object.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                January 2004
 *********************************************************************************/


#ifndef SIP_TRANSMITTER_MGR_H
#define SIP_TRANSMITTER_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"
#include "RvSipResourcesTypes.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransportTypes.h"
#include "_SipCommonTypes.h"
#include "RvSipCommonTypes.h"
#include "RvSipResolverTypes.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecurityTypes.h"
#endif

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvLogMgr*                 pLogMgr;
    RvLogSource*              pLogSrc;
    RvSipTransportMgrHandle   hTransportMgr;
    RvSipMsgMgrHandle         hMsgMgr;
    RvSipResolverMgrHandle    hRslvMgr;
    RvInt32                   maxNumOfTrx;
    RvRandomGenerator         *seed;
    RvBool                    bIsPersistent;
    RvBool                    bUseRportParamInVia;
#ifdef RV_SIGCOMP_ON
    RvSipCompartmentMgrHandle hCompartmentMgr;
#endif
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    HRPOOL                    hElementPool;
    RvInt32                   maxMessages;
    RvBool                    bResolveTelUrls;
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle         hSecMgr;
#endif
} SipTransmitterMgrCfg;

/***************************************************************************
 * SipTransmitterMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new Transmitter manager. The Transmitter manager is
 *          responsible for all the transmitters. The manager holds a list of
 *          Transmitters and is used to create new Transmitters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg        - The module configuration.
 *          pStack      - A handle to the stack manager.
 * Output:  *phTrxMgr   - Handle to the newly created Transmitter manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMgrConstruct(
                            IN  SipTransmitterMgrCfg         *pCfg,
                            IN  void*                         pStack,
                            OUT RvSipTransmitterMgrHandle*    phTrxMgr);

/***************************************************************************
 * SipTransmitterMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the Transmitter manager freeing all resources.
 * Return Value:  RV_OK -  Transmitter manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrxMgr - Handle to the manager to destruct.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMgrDestruct(
                            IN RvSipTransmitterMgrHandle hTrxMgr);

/***************************************************************************
 * SipTransmitterMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the transmitter module.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTrxMgr       - Handle to the Transmitter manager.
 * Output:     pResources - Pointer to a Transmitter resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMgrGetResourcesStatus (
                                 IN  RvSipTransmitterMgrHandle  hTrxMgr,
                                 OUT RvSipTransmitterResources  *pResources);


/***************************************************************************
 * SipTransmitterMgrCreateTransmitter
 * ------------------------------------------------------------------------
 * General: Creates a new Transmitter and exchange handles with the
 *          application.  The new Transmitter assumes the IDLE state.
 *          Using this function the transaction can supply its memory
 *          page and triple lock to the transmitter.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr - Handle to the Transmitter manager
 *            hAppTrx - Application handle to the newly created Transmitter.
 *            pEvHanders  - Event handlers structure for this transmitter.
 *            hPool     - A pool for this transmitter
 *            hPage     - A memory page to be used by this transmitter. If NULL
 *                        is supplied the transmitter will allocate a new page.
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 * Output:    phTrx   -   SIP stack handle to the Transmitter
 ***************************************************************************/
RvStatus RVCALLCONV SipTransmitterMgrCreateTransmitter(
                   IN  RvSipTransmitterMgrHandle   hTrxMgr,
                   IN  RvSipAppTransmitterHandle   hAppTrx,
                   IN  RvSipTransmitterEvHandlers* pEvHandlers,
                   IN  HRPOOL                      hPool,
                   IN  HPAGE                       hPage,
                   IN  SipTripleLock*              pTripleLock,
                   OUT RvSipTransmitterHandle*     phTrx);


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_TRANSMITTER_MGR_H */



