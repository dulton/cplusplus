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
 *                              <TransmitterCallbacks.c>
 *
 * This file contains all callbacks called from the Transmitter layer
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos               January 2004
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonUtils.h"
#include "TransmitterCallbacks.h"
#include "TransmitterDest.h"
#include "rvansi.h"
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER CALLBACK FUNCTIONS                    */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransmitterCallbackStateChangeEv
 * ------------------------------------------------------------------------
 * General: Calls the state change callback of the transmitter
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx      - The transmitter pointer
 *        eNewState - The new state of the transmitter.
 *        eReason   - The reason for the state change.
 *        hMsg      - A message related to this state
 *        pExtraInfo- Extra info related to this state.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TransmitterCallbackStateChangeEv(
                            IN  Transmitter                       *pTrx,
                            IN  RvSipTransmitterState             eNewState,
                            IN  RvSipTransmitterReason            eReason,
                            IN  RvSipMsgHandle                    hMsg,
                            IN  void*                             pExtraInfo)

{
    RvInt32                      nestedLock = 0;
    RvSipTransmitterEvHandlers  *pTrxEvHandlers = &pTrx->evHandlers;

    /* Call the event state changed that was set to register-clients manager */
    if (NULL != (*(pTrxEvHandlers)).pfnStateChangedEvHandler)
    {
        RvSipAppTransmitterHandle  hAppTrx = pTrx->hAppTrx;
        SipTripleLock             *pTripleLock;
        RvThreadId                 currThreadId=0; 
        RvInt32                    threadIdLockCount=0;

        pTripleLock = pTrx->pTripleLock;

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterCallbackStateChangeEv: Transmitter 0x%p - Before callback",pTrx));

        if(pTrx->bIsAppTrx == RV_TRUE)
        {
            SipCommonUnlockBeforeCallback(pTrx->pTrxMgr->pLogMgr,pTrx->pTrxMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*(pTrxEvHandlers)).pfnStateChangedEvHandler(
                                      (RvSipTransmitterHandle)pTrx,
                                      hAppTrx,
                                      eNewState,
                                      eReason,
                                      hMsg,
                                      pExtraInfo);
        if(pTrx->bIsAppTrx == RV_TRUE)
        {
            SipCommonLockAfterCallback(pTrx->pTrxMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterCallbackStateChangeEv: Transmitter 0x%p,After callback",pTrx));

    }
    else
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterCallbackStateChangeEv: Transmitter 0x%p,Application did not registered to callback",pTrx));

    }
}



/***************************************************************************
 * TransmitterCallbackOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: calls to pfnOtherURLAddressFoundEv
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTrx           - The sip stack Transmitter handle
 *        hTransc        - The transaction handle
 *        hAppTransc     - The application handle for this transaction.
 *        hMsg           - The message that includes the other URL address.
 *        hAddress       - Handle to unsupport address to be converted.
 * Output:hSipURLAddress - Handle to the SIP URL address - this is an empty
 *                         address object that the application should fill.
 *        bAddressResolved-Indication wether the SIP URL address was filled.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterCallbackOtherURLAddressFoundEv(
                     IN  Transmitter*               pTrx,
                     IN  RvSipMsgHandle             hMsg,
                     IN  RvSipAddressHandle         hAddress,
                     OUT RvSipAddressHandle         hSipURLAddress,
                     OUT RvBool*                    bAddressResolved)
{
    RvInt32                     nestedLock  = 0;
    RvStatus                    rv          = RV_OK;
    RvSipTransmitterEvHandlers *pEvHandlers = &pTrx->evHandlers;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipAppTransmitterHandle   hAppTrx = pTrx->hAppTrx;

    pTripleLock = pTrx->pTripleLock;

    if((*pEvHandlers).pfnOtherURLAddressFoundEvHandler!= NULL)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                 "TransmitterCallbackOtherURLAddressFoundEv: Transmitter 0x%p, Before callback",pTrx));
        if(pTrx->bIsAppTrx == RV_TRUE)
        {
            SipCommonUnlockBeforeCallback(pTrx->pTrxMgr->pLogMgr,pTrx->pTrxMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pEvHandlers).pfnOtherURLAddressFoundEvHandler(
                                              (RvSipTransmitterHandle)pTrx,
                                               hAppTrx,
                                               hMsg,
                                               hAddress,
                                               hSipURLAddress,
                                               bAddressResolved);
        if(pTrx->bIsAppTrx == RV_TRUE)
        {
            SipCommonLockAfterCallback(pTrx->pTrxMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                 "TransmitterCallbackOtherURLAddressFoundEv - Transmitter 0x%p After callback", pTrx));
    }
    else
    {
       *bAddressResolved = RV_FALSE;
       RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "TransmitterCallbackOtherURLAddressFoundEv: Transmitter 0x%p,Application did not registered to callback",pTrx));
    }

    return rv;
}

/************************************************************************************
 * TransmitterCallbacksResolveApplyRegExpCb
 * ---------------------------------------------------------------------------
 * General: asks the application to resolve a regular expression
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input: pTrx - resolver
 *        strRegexp      - a regular expression from NAPTR query.
 *        strString      - the string to apply the regexp on
 *        matchSize      - the size of the pMatchArray
 *        eFlags         - flags to take in account when applying the 
 *                         regular expression
 * Output:pMatches       - The eFlags should be filled with results of applying 
 *                         the regexp on the string 
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterCallbacksResolveApplyRegExpCb(
                            IN Transmitter* pTrx,
                             IN  RvChar*   strRegExp,
                             IN  RvChar*   strString,
                             IN RvInt32     matchSize,
                             IN  RvSipTransmitterRegExpFlag eFlags,
                             OUT RvSipTransmitterRegExpMatch* pMatches)
{

    RvInt32                     nestedLock   = 0;
    SipTripleLock*              pTripleLock;
    RvThreadId                  currThreadId=0; 
    RvInt32                     threadIdLockCount=0;
    RvSipTransmitterMgrEvHandlers* pEvHandlers = &(pTrx->pTrxMgr->mgrEvents);
    RvStatus                    rv          = RV_OK;

    pTripleLock = pTrx->pTripleLock;

    if((*pEvHandlers).pfnRegExpResolutionNeededEvHandler != NULL)
    {
        RvSipAppTransmitterHandle hOwner = (RV_TRUE == pTrx->bIsAppTrx) ? pTrx->hAppTrx : NULL;
        RvSipTransmitterRegExpResolutionParams    regexpParams;

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterCallbacksResolveApplyRegExpCb - pTrx=0x%p. before callback (appreslv = 0x%p,regexp=%s,string=%s)",
            pTrx,hOwner,strRegExp,strString));
        
        /* filling the regexp application struct */
        regexpParams.eFlags    = eFlags;
        regexpParams.matchSize = matchSize;
        regexpParams.strRegExp = strRegExp;
        regexpParams.strString = strString;
        regexpParams.pMatches  = pMatches;
        SipCommonUnlockBeforeCallback(pTrx->pTrxMgr->pLogMgr,pTrx->pTrxMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        
        rv = pEvHandlers->pfnRegExpResolutionNeededEvHandler((RvSipTransmitterMgrHandle)pTrx->pTrxMgr,
                                                        pTrx->pTrxMgr->pAppTrxMgr,
                                                        (RvSipTransmitterHandle)pTrx,
                                                        hOwner,
                                                        &regexpParams);
        SipCommonLockAfterCallback(pTrx->pTrxMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        if (RV_OK == rv)
        {
#define CB_CHARS_PER_MATCH 12
            RvChar  matchRes[CB_CHARS_PER_MATCH*TRANSMITTER_DNS_REG_MAX_MATCHES];
            RvChar* pMatchRes = matchRes;
            RvInt   i = 0;
            matchRes[0] = '\0';
            
            for (i=0;i<TRANSMITTER_DNS_REG_MAX_MATCHES;i++)
            {
                pMatchRes +=RvSprintf(pMatchRes,"(%d,%d)",pMatches[i].startOffSet,pMatches[i].endOffSet);
            }
            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterCallbacksResolveApplyRegExpCb - pTrx=0x%p. after callback matches=%s(rv=%d)",
                pTrx, matchRes, rv));
        }
        else
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterCallbacksResolveApplyRegExpCb - pTrx=0x%p. after callback (rv=%d)",
                pTrx, rv));
        }
#endif /*RV_LOGMASK != RV_LOGLEVEL_NONE*/
    }
    else
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterCallbacksResolveApplyRegExpCb - pTrx=0x%p. Application did not register to call back  returnig err (regexp=%s string=%s)",
            pTrx,strRegExp,strString));
        rv = RV_ERROR_UNKNOWN;
    }
    return rv;
}

/***************************************************************************
* TransmitterCallbackMgrResolveAddressEv
* ------------------------------------------------------------------------
* General: calls to pfnMgrResolveAddressEvHandler
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hTrx           - The sip stack Transmitter handle
*        hostName       - The hostname string that should be resolved.
*        pAddrToResolve - A structure containing all the information of
*                         the address that should be resolved.
* Output:pAddrToResolve - A structure containing all the information of
*                         the resolved address.
*        pbAddressResolved - Indication whether the pResolvedAddress was filled.
*        pbConfinueResolution - indication to the stack how to proceed (one
*                          of three options that were described above).
***************************************************************************/
RvStatus RVCALLCONV TransmitterCallbackMgrResolveAddressEv(
                                           IN  Transmitter*               pTrx,
                                           IN  RvChar*                    hostName,
                                           INOUT  RvSipTransportAddr      *pAddrToResolve,
                                           OUT RvBool                     *pbAddressResolved,
                                           OUT RvBool                     *pbConfinueResolution)
{
    RvStatus        rv   = RV_OK;
    TransmitterMgr* pMgr = pTrx->pTrxMgr;
    void*           pAppTrxMgr = pMgr->pAppTrxMgr;
    RvInt32                     nestedLock   = 0;
    SipTripleLock*              pTripleLock;
    RvThreadId                  currThreadId=0; 
    RvInt32                     threadIdLockCount=0;
    RvSipTransmitterMgrEvHandlers* pEvHandlers = &(pTrx->pTrxMgr->mgrEvents);
    
    pTripleLock = pTrx->pTripleLock;
    
    if((*pEvHandlers).pfnMgrResolveAddressEvHandler!= NULL)
    {

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransmitterCallbackMgrResolveAddressEv: Transmitter 0x%p, Before callback",pTrx));

        SipCommonUnlockBeforeCallback(pMgr->pLogMgr, pMgr->pLogSrc, pTripleLock,
                                      &nestedLock,&currThreadId,&threadIdLockCount);
        
        rv = pEvHandlers->pfnMgrResolveAddressEvHandler(
                                (RvSipTransmitterMgrHandle)pMgr,
                                pAppTrxMgr,
                                hostName,
                                pAddrToResolve,
                                pbAddressResolved,
                                pbConfinueResolution);

        SipCommonLockAfterCallback(pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransmitterCallbackMgrResolveAddressEv - Transmitter 0x%p After callback. pbAddressResolved=%d pbConfinueResolution=%d",
            pTrx, *pbAddressResolved, *pbConfinueResolution));
    }
    else
    {
        *pbAddressResolved = RV_FALSE;
        *pbConfinueResolution = RV_TRUE;
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "TransmitterCallbackMgrResolveAddressEv: Transmitter 0x%p,Application did not registered to callback",pTrx));
    }
    
    return rv;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


