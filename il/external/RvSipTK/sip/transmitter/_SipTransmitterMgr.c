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
 *                              <_SipTransmitterMgr.c>
 *
 *  The _SipTransmitterMgr.h file contains Internal API functions of the
 *  Transmitter manager object.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                January 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "RvSipTransmitterTypes.h"
#include "_SipTransmitterMgr.h"
#include "_SipCommonUtils.h"
#include "TransmitterObject.h"
#include "_SipTransport.h"
#include "_SipCommonCore.h"

/*-----------------------------------------------------------------------*/
/*                        MACRO DEFINITIONS                              */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER INTERNAL API FUNCTIONS                */
/*-----------------------------------------------------------------------*/
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
                            OUT RvSipTransmitterMgrHandle*    phTrxMgr)
{
    RvStatus           rv        = RV_OK;
    TransmitterMgr*    pTrxMgr   = NULL;
    *phTrxMgr = NULL;
    RvLogInfo(pCfg->pLogSrc,(pCfg->pLogSrc,
              "SipTransmitterMgrConstruct - About to construct the Transmitter manager"))
    rv = TransmitterMgrConstruct(pCfg, pStack, &pTrxMgr);
    if(rv != RV_OK)
    {
         RvLogError(pCfg->pLogSrc,(pCfg->pLogSrc,
                   "SipTransmitterMgrConstruct - Failed, destructing the module (rv = %s)",RV2STR(rv)));
         TransmitterMgrDestruct(pTrxMgr);
         return rv;
    }
    *phTrxMgr = (RvSipTransmitterMgrHandle)pTrxMgr;
    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "SipTransmitterMgrConstruct - The Transmitter manager 0x%p was constructed successfully",pTrxMgr))
    return RV_OK;
}

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
                            IN RvSipTransmitterMgrHandle hTrxMgr)
{
    TransmitterMgr*    pTrxMgr   = NULL;
    RvStatus           rv        = RV_OK;
    if(hTrxMgr == NULL)
    {
        return RV_OK;
    }
    pTrxMgr = (TransmitterMgr*)hTrxMgr;

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "SipTransmitterMgrDestruct - About to destruct the Transmitter manager 0x%p",pTrxMgr))

    rv = TransmitterMgrDestruct(pTrxMgr);
    if(rv != RV_OK)
    {
         RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                   "SipTransmitterMgrDestruct - Failed, destructing the module (rv = %s)",RV2STR(rv)));
         return rv;
    }
    return RV_OK;
}


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
                                 OUT RvSipTransmitterResources  *pResources)
{
    TransmitterMgr*  pTrxMgr                 = NULL;
    RvUint32         allocNumOfLists         = 0;
    RvUint32         allocMaxNumOfUserElement= 0;
    RvUint32         currNumOfUsedLists      = 0;
    RvUint32         currNumOfUsedUserElement= 0;
    RvUint32         maxUsageInLists         = 0;
    RvUint32         maxUsageInUserElement   = 0;

    pTrxMgr = (TransmitterMgr*)hTrxMgr;
    /* getting transmitters */
    RLIST_GetResourcesStatus(pTrxMgr->hTrxListPool,
                             &allocNumOfLists,
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);

    pResources->transmitters.currNumOfUsedElements  = currNumOfUsedUserElement;
    pResources->transmitters.numOfAllocatedElements = allocMaxNumOfUserElement;
    pResources->transmitters.maxUsageOfElements     = maxUsageInUserElement;
    return RV_OK;
}




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
                   OUT RvSipTransmitterHandle*     phTrx)
{
    RvStatus          rv      = RV_OK;
    Transmitter*      pTrx    = NULL;
    TransmitterMgr*   pTrxMgr = (TransmitterMgr*)hTrxMgr;

    *phTrx = NULL;
    
    if(NULL == pEvHandlers  ||  NULL == pEvHandlers->pfnStateChangedEvHandler)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "SipTransmitterMgrCreateTransmitter - Illegal action: EvHandler were not supplied"));
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "SipTransmitterMgrCreateTransmitter - Creating a new Transmitter"));

    /*create a new Transmitter. This will lock the Transmitter manager*/
    rv = TransmitterMgrCreateTransmitter(pTrxMgr,RV_FALSE,hAppTrx,hPool,hPage,pTripleLock,phTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "SipTransmitterMgrCreateTransmitter - Failed, TransmitterMgr failed to create a new transmitter (rv = %d)",rv));
        return rv;
    }
    pTrx = (Transmitter*)*phTrx;

    /*set the transmitter event handler*/
    memcpy(&pTrx->evHandlers,pEvHandlers,sizeof(pTrx->evHandlers));

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "SipTransmitterMgrCreateTransmitter - New Transmitter 0x%p was created successfully",*phTrx));
    return RV_OK;

}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
