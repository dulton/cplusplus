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
 *                             <SipParserManager.h>
 *    This file contains the initialization of the sip parser moudle.
 *
 *
 *
 *    Author                         Date
 *    ------                        ------
 * Michal Mashiach               Dec 2000
 *********************************************************************************/
#include "RV_SIP_DEF.h"

#include <string.h>
#include "AdsRpool.h"
#include "_SipParserManager.h"
#include "_SipCommonUtils.h"
#include "rvmemory.h"

/***************************************************************************
 * RvSipParserMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct the parser module.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCfg  - Structre with relevant configuration parameters.
 * Output:  phParserMgr - Handle to the parser mgr.
 ***************************************************************************/
RvStatus RVCALLCONV SipParserMgrConstruct(
                           IN SipParserMgrCfg*     pCfg,
                           OUT SipParserMgrHandle* phParserMgr)
{
    ParserMgr* pMgr;
    /*------------------------------------------------------------
       allocates the parser manager structure and initialize it
    --------------------------------------------------------------*/
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(ParserMgr), pCfg->pLogMgr, (void*)&pMgr))
    {
        RvLogError(pCfg->moduleLogId,(pCfg->moduleLogId,
                  "SipParserMgrConstruct - Failed ,RvMemoryAlloc failed"));
        return RV_ERROR_UNKNOWN;
    }
    pMgr->pLogMgr = pCfg->pLogMgr;
    pMgr->pLogSrc = pCfg->moduleLogId;
    pMgr->pool = pCfg->hGeneralPool;

    *phParserMgr = (SipParserMgrHandle)pMgr;
    return RV_OK;
}

/***************************************************************************
 * RvSipParserMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the parser module.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phParserMgr - Handle to the parser mgr.
 ***************************************************************************/
void RVCALLCONV SipParserMgrDestruct(SipParserMgrHandle hParserMgr)
{
    ParserMgr* pMgr = (ParserMgr *)hParserMgr;

      /* free call-leg mgr data structure */
    RvMemoryFree((void*)pMgr, pMgr->pLogMgr);
}

/***************************************************************************
 * SipParserMgrSetMsgMgrHandle
 * ------------------------------------------------------------------------
 * General: Set msgMgrHandle in parser manager.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 ***************************************************************************/
RvStatus RVCALLCONV SipParserMgrSetMsgMgrHandle(
                           IN SipParserMgrHandle hParserMgr,
                           IN RvSipMsgMgrHandle  hMsgMgr)
{
    ((ParserMgr*)hParserMgr)->hMsgMgr = hMsgMgr;
    return RV_OK;
}

