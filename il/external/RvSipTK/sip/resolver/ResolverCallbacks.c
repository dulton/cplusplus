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
 *                              <ResolverCallbacks.c>
 *
 * Resolver layer callback function wrappers
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    Feb 2005
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipResolverTypes.h"
#include "ResolverObject.h"
#include "_SipCommonUtils.h"
#include "ResolverDns.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                                 FUNCTIONS                             */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ResolverReportData
 * ------------------------------------------------------------------------
 * General: reports data to upper layer 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRslv     - pointer to the Resolver
 *            bIsError  - do we have an error in the data
 **************************************************************************/
void RVCALLCONV ResolverCallbacksReportData(IN Resolver*  pRslv,
                                   IN RvBool    bIsError)
{

    /*going back to undefined to allow new queries */
    ResolverChangeState(pRslv,RESOLVER_STATE_UNDEFINED);
    if (NULL != pRslv->pfnReport)
    {
        RvInt32                    nestedLock = 0;
        SipTripleLock*             pTripleLock;
        RvThreadId                 currThreadId=0; 
        RvInt32                    threadIdLockCount=0;
        RvSipAppResolverHandle     hOwner   = NULL;
        RvSipResolverReportDataEv      pfnReport;
        RvStatus rv;

        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverCallbacksReportData - pRslv=0x%p, bIsError=%d eMode=%s befor callback",
            pRslv,bIsError,ResolverGetModeName(pRslv->eMode)));
        pfnReport   = pRslv->pfnReport;
        pTripleLock = pRslv->pTripleLock;
        hOwner      = pRslv->hOwner;

        if(pRslv->bIsAppRslv == RV_TRUE)
        {
            SipCommonUnlockBeforeCallback(pRslv->pRslvMgr->pLogMgr,pRslv->pRslvMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = pfnReport((RvSipResolverHandle)pRslv,hOwner,bIsError,pRslv->eMode);
        if(pRslv->bIsAppRslv == RV_TRUE)
        {
            SipCommonLockAfterCallback(pRslv->pRslvMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }

        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverCallbacksReportData - pRslv=0x%p, after callback(rv=%d)",
            pRslv, rv));
    }
    else
    {
        RvLogDebug(pRslv->pRslvMgr->pLogSrc,(pRslv->pRslvMgr->pLogSrc,
            "ResolverCallbacksReportData - pRslv = 0x%p, not implemented",pRslv));
    }
}

