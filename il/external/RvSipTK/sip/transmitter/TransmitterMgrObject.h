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
 *                              <TransmitterMgrObject.h>
 *     The Transmitter manager object holds all the Transmitter module resources
 *   Including memory pool, pools of lists, locks and more.
 *   Its main functionality is to manage the Transmitter list, to create new
 *   Transmitters and to associate new transaction to a specific Transmitter according
 *   to the transaction key.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

#ifndef TRANSMITTER_MGR_OBJECT_H
#define TRANSMITTER_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvmutex.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"
#include "rvares.h"

#include "RvSipTransmitterTypes.h"
#include "RvSipTransportTypes.h"
#include "_SipTransmitterMgr.h"
#include "_SipCommonTypes.h"
#include "AdsRlist.h"
#include "AdsRpool.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* TransmitterMgr
 * ---------------------------------------------------------------------------
 * The Transmitter manager object holds the Transmitter module configuration and
 * resource. The manager manages that Transmitters list.
 *
 * pLogMgr          -- The stack log manager.
 * pLogSrc          -- The module log-source.
 * hTrxListPool     -- Handle to a pool of Transmitter lists. only one list is
 *                     allocated from this pool.
 * hTrxList         -- Handle to the Transmitters list.
 * hMutex           -- A lock used to lock the manager object to avoid threads
 *                      collisions.
 * maxNumOfTrx      -- Max number of transmitters in the Transmitter list.
 * hTransportMgr    -- Handle to the transport module manager.
 * hMsgMgr          -- Handle to the message module manager.
 * pAppTrxMgr       -- Pointer to the application's Transmitter manager object.
 * seed             -- Used to generate unique identifier to the transmitters
 * bIsPersistent    -- Indicates whether the transmitters should look for connections
 *                     in the hash.
 * hRslvMgr         -- handle to resolver manager (for DNS functionality)
 * hCompartmentMgr  -- The compartment manager handle.
 * maxNumOfTrx      -- Number of transmitter objects.
 * hGeneralPool     -- Handle to the stack general pool
 * bOtherUrlCB4Tel  -- Call the Other URL found callbacks 
 *                     RV_TRUE -  do not try and resolve tel URL by NAPTR, 
 *                                use the otherURLFound CBs
 *                     RV_FALSE - Do NAPTR for tel URL
 * bReuseServerConnection - Indicates if the stack is configured to re-use server
 *                      connections or not. (if TRUE, 'alias' parameter will be set to the
 *                      message top via header).
 */
typedef struct
{
    RvLogMgr*                 pLogMgr;
    RvLogSource*              pLogSrc;
    RvInt32                   maxNumOfTrx;
    RLIST_POOL_HANDLE         hTrxListPool;
    RLIST_HANDLE              hTrxList;
    RvMutex*                  pMutex;
    RvSipTransportMgrHandle   hTransportMgr;
    RvSipMsgMgrHandle         hMsgMgr;
    void                      *pAppTrxMgr;
    RvRandomGenerator         *seed;
    RvBool                    bIsPersistent;
    void*                     pStack;
    RvSipResolverMgrHandle    hRslvMgr;
    RvSipTransmitterMgrEvHandlers mgrEvents;

#ifdef RV_SIGCOMP_ON
    RvSipCompartmentMgrHandle hCompartmentMgr;
#endif
    HRPOOL                    hGeneralPool;
    HRPOOL                    hMessagePool;
    HRPOOL                    hElementPool;
    RvInt32                   maxMessages;
    RvBool                    bUseRportParamInVia;
    RvBool                    bResolveTelUrls;
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle         hSecMgr;
#endif
    
} TransmitterMgr;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransmitterMgrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs the Transmitter manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCfg        - The module configuration.
 *          pStack      - A handle to the stack manager.
 * Output:  *ppTrxMgr   - Pointer to the newly created Transmitter manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrConstruct(
                            IN  SipTransmitterMgrCfg*         pCfg,
                            IN  void*                         pStack,
                            OUT TransmitterMgr**              ppTrxMgr);

/***************************************************************************
 * TransmitterMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the Transmitter manager and free all its resources.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrxMgr     - The Transmitter manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrDestruct(
                            IN TransmitterMgr*   pTrxMgr);

/***************************************************************************
 * TransmitterMgrCreateTransmitter
 * ------------------------------------------------------------------------
 * General: Creates a new transmitter
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr   - Handle to the Transmitter manager
 *            bIsAppTrx - indicates whether the transmitter was created
 *                        by the application or by the stack.
 *            hAppTrx   - Application handle to the transmitter.
 *            hPool     - A pool for this transmitter
 *            hPage     - A memory page to be used by this transmitter. If NULL
 *                        is supplied the transmitter will allocate a new page.
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 * Output:     phTrx    - sip stack handle to the new Transmitter
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMgrCreateTransmitter(
                   IN     TransmitterMgr            *pTrxMgr,
                   IN     RvBool                    bIsAppTrx,
                   IN     RvSipAppTransmitterHandle hAppTrx,
                   IN     HRPOOL                    hPool,
                   IN     HPAGE                     hPage,
                   IN     SipTripleLock*            pTripleLock,
                   OUT    RvSipTransmitterHandle    *phTrx);

/***************************************************************************
 * TransmitterMgrRemoveTransmitter
 * ------------------------------------------------------------------------
 * General: removes a transmitter object from the transmitters list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr   - Handle to the Transmitter manager
 *            hTrx      - Handle to the Transmitter
 ***************************************************************************/
void RVCALLCONV TransmitterMgrRemoveTransmitter(
                                IN TransmitterMgr*        pTrxMgr,
                                IN RvSipTransmitterHandle hTrx);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSMITTER_MGR_OBJECT_H */
