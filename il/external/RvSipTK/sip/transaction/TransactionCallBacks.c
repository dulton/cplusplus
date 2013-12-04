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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <TransactionCallBacks.c>
 * From this c file all transaction layer callbacks are called.
 * The file also includes default callbacks for transactions with no owner.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2000
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "TransactionCallBacks.h"
#include "TransactionObject.h"
#include "_SipTransaction.h"
#include "_SipPartyHeader.h"
#include "_SipTransport.h"
#include "_SipCommonUtils.h"
#include "_SipTransmitter.h"
#include <string.h>
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
static RvUint16 GetResponseCode(IN Transaction* pTransc);

/*-----------------------------------------------------------------------*/
/*                          Application Callbacks                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionCallbackAppTranscCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new server transaction was
 *          created. The newly created transaction always assumes the Idle
 *          state. The application should decide whether to handle this transaction.
 *          If so, the application can exchange handles with the SIP Stack
 *          using this callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc     - The new SIP Stack transaction handle.
 * Output:  pbAppHandleTransc - Indicated whether the application wishes to handle
 *                           the transaction - RV_TRUE.
 *                           if set to RV_FALSE the stack will handle the
 *                           transaction by itself. The normal behavior will
 *                           be returning of 501 not implemented.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAppTranscCreatedEv(
                      IN    Transaction              *pTransc,
                      OUT   RvSipTranscOwnerHandle   *phAppTransc,
                      OUT   RvBool                   *pbAppHandleTransc)
{
    RvInt32                     nestedLock   = 0;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers *pEvHandlers = &(pTransc->pMgr->pAppEvHandlers);

    pTripleLock = pTransc->tripleLock;

    *pbAppHandleTransc = RV_FALSE;

    if((*pEvHandlers).pfnEvTransactionCreated != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAppTranscCreatedEv:Transc 0x%p, Before callback",
                  pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP ||
            pTransc->eOwnerType == SIP_TRANSACTION_OWNER_NON)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_CREATED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvTransactionCreated ((RvSipTranscHandle)pTransc,
                                                pTransc->pMgr->pAppMgr,
                                                phAppTransc,
                                                pbAppHandleTransc);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_CREATED);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAppTranscCreatedEv:Transc 0x%p, After callback, pbAppHandleTransc = %d",
                  pTransc, *pbAppHandleTransc));
        

    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAppTranscCreatedEv: Transc 0x%p, Application did not register to call back, default pbAppHandleTransc = %d",
                  pTransc, *pbAppHandleTransc));

    }
}

/***************************************************************************
 * TransactionCallbackStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies of a transaction state change and the
 *          associated state change reason.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            eState       - The new transaction state.
 *            eReason      - The reason for the state change.
 ***************************************************************************/
void RVCALLCONV  TransactionCallbackStateChangedEv(
                            IN Transaction                      *pTransc,
                              IN RvSipTransactionState             eState,
                            IN RvSipTransactionStateChangeReason eReason)
{

    RvInt32                      nestedLock   = 0;
    void*                        pOwner       = pTransc->pOwner;
    RvSipTransactionEvHandlers  *pEvHandlers  = pTransc->pEvHandlers;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if(NULL != pEvHandlers && (*pEvHandlers).pfnEvStateChanged != NULL)
    {

        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackStateChangedEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_STATE_CHANGED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvStateChanged((RvSipTranscHandle)pTransc,
                                         pOwner,
                                         eState,
                                         eReason);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_STATE_CHANGED);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackStateChangedEv: Transc 0x%p,After callback",pTransc));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackStateChangedEv: Transc 0x%p,Application did not register to call back",pTransc));

    }
}

/***************************************************************************
 * TransactionCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Indicates that a transaction-related outgoing message is about
 *          to be sent.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMsgToSendEv(
                            IN Transaction        *pTransc,
                            IN RvSipMsgHandle      hMsgToSend)
{

    RvStatus                    rv           = RV_OK;
    RvInt32                     nestedLock   = 0;
    void                       *pOwner       = pTransc->pOwner;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers *pEvHandlers  = pTransc->pEvHandlers;

    pTripleLock = pTransc->tripleLock;
    if (NULL != pEvHandlers && NULL != pEvHandlers->pfnEvMsgToSend)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackMsgToSendEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_MSG_TO_SEND);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pEvHandlers).pfnEvMsgToSend(
                                       (RvSipTranscHandle)pTransc,
                                       pOwner,
                                       hMsgToSend);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_MSG_TO_SEND);

        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackMsgToSendEv: Transc 0x%p,After callback (rv=%d)",pTransc,rv));
        
        if(rv == RV_OK && pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionCallbackMsgToSendEv: Transc 0x%p, transc was terminated in cb. return failure",pTransc));
            rv = RV_ERROR_DESTRUCTED;
        }
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackMsgToSendEv: Transc 0x%p,Application did not register to call back (default rv=%d)",pTransc,rv));

    }
    return rv;
}

/***************************************************************************
 * TransactionCallBackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: Indicates that a transaction-related incoming message has been
 *          received.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction handle.
 *            hMsgReceived - Handle to the incoming message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallBackMsgReceivedEv (
                            IN Transaction            *pTransc,
                            IN RvSipMsgHandle         hMsgReceived)
{
    RvInt32                      nestedLock  = 0;
    void*                        pOwner      = pTransc->pOwner;
    RvStatus                     rv          = RV_OK;
    RvSipTransactionEvHandlers  *pEvHandlers = pTransc->pEvHandlers;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if(pEvHandlers!= NULL && (*pEvHandlers).pfnEvMsgReceived != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallBackMsgReceivedEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_MSG_RECEIVED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pEvHandlers).pfnEvMsgReceived(
                                    (RvSipTranscHandle)pTransc,
                                    pOwner,
                                    hMsgReceived);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_MSG_RECEIVED);

        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallBackMsgReceivedEv: Transc 0x%p,After callback (rv=%d)",pTransc,rv));

        if(rv == RV_OK && pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionCallBackMsgReceivedEv: Transc 0x%p, transc was terminated in cb. return failure",pTransc));
            rv = RV_ERROR_DESTRUCTED;
        }

    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallBackMsgReceivedEv: Transc 0x%p,Application did not register to call back (default rv=%d)",pTransc,rv));

    }
    return rv;

}


/***************************************************************************
 * TransactionCallbackAppInternalClientCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new
 *          client transaction has been created by the SIP Stack. The newly
 *          created transaction always assumes the Idle state.
 *          This callback is called for CANCEL and PRACK transactions that are
 *          created automatically by the SIP Stack.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc     - The new SIP Stack transaction pointer.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAppInternalClientCreatedEv(
                      IN    Transaction             *pTransc)
{
    RvInt32                      nestedLock   = 0;
    RvSipTranscOwnerHandle       hOwner       = NULL;
    void*                        pAppMgr      = pTransc->pMgr->pAppMgr;
    RvSipTransactionEvHandlers  *pEvHandlers  = pTransc->pEvHandlers;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if(pEvHandlers!= NULL && (*pEvHandlers).pfnEvInternalTranscCreated != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAppInternalClientCreatedEv: Transc 0x%p, Before callback",
                  pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_APP_INT_CREATED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvInternalTranscCreated (
                        (RvSipTranscHandle)pTransc,
                        pAppMgr,
                        &hOwner);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_APP_INT_CREATED);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionCallbackAppInternalClientCreatedEv: Transc 0x%p,After callback hOwner = 0x%p",
            pTransc,hOwner));
        
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAppInternalClientCreatedEv: Transc 0x%p,Application did not register to call back (default hOwner=0x%p)",pTransc,hOwner));

    }
    pTransc->pOwner = (void*)hOwner;
}




/***************************************************************************
 * TransactionCallbackCancelledEv
 * ------------------------------------------------------------------------
 * General: Notifies the application when a CANCEL request is received on a
 *          transaction.
 *          Note: This is not the CANCEL transaction but the cancelled
 *          transaction. For example an INVITE request that was cancelled.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackCancelledEv (
                      IN Transaction            *pTransc)
{

    RvInt32                      nestedLock  = 0;
    RvStatus                     rv          = RV_OK;
    void                        *pOwner      = pTransc->pOwner;
    RvSipTransactionEvHandlers  *pEvHandlers = pTransc->pEvHandlers;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if(NULL != pEvHandlers && NULL != (*pEvHandlers).pfnEvTranscCancelled)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCancelledEv: Transc 0x%p, Before callback",pTransc));
        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_CANCELLED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv= (*pEvHandlers).pfnEvTranscCancelled(
                                (RvSipTranscHandle)pTransc,
                                pOwner);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_CANCELLED);    
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCancelledEv: Transc 0x%p,After callback rv=%d", pTransc,rv));
        
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCancelledEv: Transc 0x%p,Application did not register to call back (default rv=%d)",pTransc,rv));

    }
    return rv;
}

/***************************************************************************
* TransactionCallbackCallTranscCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnEvTransactionCreated callback.
*          internal callback to call-leg layer.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pTranscMgr   - Handle to the transaction manager.
*         hTransc - Handle to the newly created transaction.
* In-Out pKey         - pointer to a temporary transaction key. The key is
*                       replaced with the call-leg key (on the call-leg memory
*                       page).
* Output:ppOwner      - pointer to the new transaction owner.
*        tripleLock      - pointer to the new locks structure
***************************************************************************/
void RVCALLCONV TransactionCallbackCallTranscCreatedEv(
                                     IN     TransactionMgr           *pTranscMgr,
                                     IN     RvSipTranscHandle         hTransc,
                                     INOUT  SipTransactionKey        *pKey,
                                     OUT    void                     **ppOwner,
                                     OUT    SipTripleLock             **tripleLock,
                                     OUT    RvUint16                *statusCode)
{
    RvInt32                       nestedLock = 0;
    Transaction                  *pTransc    = (Transaction *)hTransc;
    void                         *pMgr       = pTranscMgr->pCallMgrDetail.pMgr;
    SipTransactionMgrEvHandlers  *pCallMgrDetail = &(pTranscMgr->pCallMgrDetail);
    SipTripleLock                *pTripleLock;
    RvThreadId                    currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
             "TransactionCallbackCallTranscCreatedEv - Transaction 0x%p ", pTransc));

    if((*pCallMgrDetail).pfnEvTransactionCreated != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCallTranscCreatedEv: Transc 0x%p, Before callback",pTransc));
        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_CALL_CREATED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pCallMgrDetail).pfnEvTransactionCreated(
                            pMgr,
                            hTransc, pKey,
                            ppOwner, tripleLock,statusCode);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_CALL_CREATED);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCallTranscCreatedEv: Transc 0x%p, After callback",pTransc));

    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCallTranscCreatedEv: Transc 0x%p,Application did not register to call back",pTransc));

    }
}





#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * TransactionCallbackCallRelProvRespRcvdEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that a reliable provisional response has
 *          received. This callback is called only if the manualPrack flag is
 *          set to RV_TRUE.
 *          internal callback to call-leg layer.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction.
 *       rseqStep - the rseq step that was received in this 1xx message.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackCallRelProvRespRcvdEv(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint32        rseqStep)
{
    RvStatus                      rv          = RV_OK;
    void                         *pOwner      = pTransc->pOwner;
    SipTransactionMgrEvHandlers  *pCallMgrDetail = &(pTransc->pMgr->pCallMgrDetail);

    if (pTransc->eOwnerType != SIP_TRANSACTION_OWNER_CALL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
           "TransactionCallbackCallRelProvRespRcvdEv: Transc 0x%p, ignore  rel 1xx. owner!=CALL",pTransc));
        return RV_OK;
    }

    if((*pCallMgrDetail).pfnRelProvRespRcvd != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCallRelProvRespRcvdEv: Transc 0x%p, Before callback",pTransc));

        rv = (*pCallMgrDetail).pfnRelProvRespRcvd(
                                            (RvSipTranscHandle)pTransc,
                                            pOwner,
                                            rseqStep);

        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackCallRelProvRespRcvdEv: Transc 0x%p, After callback",pTransc));

    }

    return rv;
}

/***************************************************************************
 * TransactionCallbackCallIgnoreRelProvRespEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that a reliable provisional response has
 *            received. This callback is called only if the manualPrack flag is
 *            set to RV_TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:pTransc - The transaction.
 *       rseqStep - the rseq step that was received in this 1xx message.
 * Output:pbIgnore - ignore the 1xx or not.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackCallIgnoreRelProvRespEv(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint32        rseqStep,
                                    OUT RvBool*         pbIgnore)
{
    RvInt32                       nestedLock  = 0;
    void                         *pOwner      = pTransc->pOwner;
    SipTransactionMgrEvHandlers  *pCallMgrDetail = &(pTransc->pMgr->pCallMgrDetail);
    SipTripleLock                *pTripleLock;
    RvThreadId                    currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if((*pCallMgrDetail).pfnIgnoreRelProvRespRcvd != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackCallIgnoreRelProvRespEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_IGNORE_REL_PROV);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pCallMgrDetail).pfnIgnoreRelProvRespRcvd(
                                            (RvSipTranscHandle)pTransc,
                                            pOwner,
                                            rseqStep,
                                            pbIgnore);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_IGNORE_REL_PROV);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackCallIgnoreRelProvRespEv: Transc 0x%p, After callback",pTransc));

    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
           "TransactionCallbackCallIgnoreRelProvRespEv: Transc 0x%p,Application did not register to call back",pTransc));

    }

}

/***************************************************************************
 * TransactionCallbackCallInviteTranscMsgSentEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that the invite transaction sent a msg.
 *          1. On first 2xx sending, when working with new-invite handling. 
 *          2. On non-2xx Ack sending, when working with new-invite handling.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:pTransc - The transaction.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackCallInviteTranscMsgSentEv(
                            IN  Transaction                 *pTransc,
                            IN SipTransactionMsgSentOption   eOption)
{
    RvInt32                       nestedLock  = 0;
    void                         *pOwner      = pTransc->pOwner;
    SipTransactionMgrEvHandlers  *pCallMgrDetail = &(pTransc->pMgr->pCallMgrDetail);
    SipTripleLock                *pTripleLock;
    RvThreadId                    currThreadId=0; RvInt32 threadIdLockCount=0;
    RvStatus                      rv;

    pTripleLock = pTransc->tripleLock;

    if((*pCallMgrDetail).pfnInviteTranscMsgSent != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackCallInviteTranscMsgSentEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_INVITE_TRANSC_MSG_SENT);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pCallMgrDetail).pfnInviteTranscMsgSent(
                                            (RvSipTranscHandle)pTransc,
                                            pOwner,
                                            eOption);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_INVITE_TRANSC_MSG_SENT);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackCallInviteTranscMsgSentEv: Transc 0x%p, After callback(rv=%d)",
          pTransc, rv));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
           "TransactionCallbackCallInviteTranscMsgSentEv: Transc 0x%p, Application did not register to call back",pTransc));

    }

}

#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * TransactionCallbackSupplyTranscParamsEv
 * ------------------------------------------------------------------------
 * General: Notified the application that an internally created client
 *          transaction needs a CSeq step and a Request URI from its owner.
 *          The application should supply these parameters to the transaction.
 *          In this version, this callback is called only for PRACK client
 *          transactions.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc      - Pointer to the transaction structure
 * Output:  pCSeqStep    - CSeq step for the transaction
 *          phRequestUri - Request URI for the transaction.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackSupplyTranscParamsEv(
                            IN    Transaction              *pTransc,
                            OUT   RvInt32                  *pCSeqStep,
                            OUT   RvSipAddressHandle       *phRequestUri)
{
    RvInt32                      nestedLock = 0;
    void                        *pOwner    = pTransc->pOwner;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers  *pEvHandlers = pTransc->pEvHandlers;

    pTripleLock = pTransc->tripleLock;

    if(pEvHandlers!= NULL && (*pEvHandlers).pfnEvSupplyParams != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackSupplyTranscParamsEv: Transc 0x%p, Before callback", pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_SUPPLY_PARAMS);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvSupplyParams((RvSipTranscHandle)pTransc,
                                        pOwner,
                                        pCSeqStep,
                                        phRequestUri);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_SUPPLY_PARAMS);   
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionCallbackSupplyTranscParamsEv: Transc 0x%p,After callback cseq = %d", pTransc,*pCSeqStep));
        
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackSupplyTranscParamsEv: Transc 0x%p,Application did not registere to call back ",pTransc));

    }
}


#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * TransactionCallbackAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  Supply an authorization header, to pass it to the user,
 *           that will continue the authentication procedure, according to
 *           the realm and user-name parameters in it.
 *           in order to continue the procedure, user shall use
 *           RvSipTransactionAuthProceedExt().
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc   - Pointer to the transaction structure .
 *         hAuthorization - Handle to the authorization header that was found.
 *         isSupported    - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAuthCredentialsFoundEv(
                                    IN Transaction                      *pTransc,
                                    IN RvSipAuthorizationHeaderHandle hAuthorization,
                                    IN RvBool                        isSupported)
{
    RvInt32                      nestedLock = 0;
    void                        *pOwner    = pTransc->pOwner;
    SipTripleLock               *pTripleLock;
    RvThreadId                   currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers  *pEvHandlers = pTransc->pEvHandlers;


    pTripleLock = pTransc->tripleLock;
    if(pEvHandlers!= NULL && (*pEvHandlers).pfnEvAuthCredentialsFound != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCredentialsFoundEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_AUTH_CREDENTIALS_FOUND);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvAuthCredentialsFound(
                        (RvSipTranscHandle)pTransc,
                        pOwner,
                        hAuthorization,
                        isSupported);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_AUTH_CREDENTIALS_FOUND);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCredentialsFoundEv: Transc 0x%p,After callback",pTransc));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCredentialsFoundEv: Transc 0x%p,Application did not registere to callback",pTransc));

    }
}
/***************************************************************************
 * TransactionCallbackAuthCompletedEv
 * ------------------------------------------------------------------------
 * General:  Notify that authentication procedure is completed.
 *           If it is completed because a correct authorization was found,
 *           bAuthSucceed is RV_TRUE.
 *           If it is completed because there are no more authorization headers
 *           to check, or because user ordered to stop the searching for
 *           correct header, bAuthSucceed is RV_FALSE.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - Pointer to the transaction structure .
 *        bAuthSucceed - RV_TRUE if we found correct authorization header,
 *                       RV_FALSE if we did not.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAuthCompletedEv(
                                           IN Transaction* pTransc,
                                           IN  RvBool      bAuthSucceed)
{
    RvInt32                     nestedLock = 0;
    void                       *pOwner    = pTransc->pOwner;
    RvSipTransactionEvHandlers *pEvHandlers = pTransc->pEvHandlers;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;

    pTripleLock = pTransc->tripleLock;

    if(pEvHandlers != NULL && (*pEvHandlers).pfnEvAuthCompleted != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCompletedEv: Transc 0x%p, Before callback. bAuthSucceed=%d",pTransc, bAuthSucceed));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_AUTH_COMPLETED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        (*pEvHandlers).pfnEvAuthCompleted(
                            (RvSipTranscHandle)pTransc,
                            pOwner,
                            bAuthSucceed);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_AUTH_COMPLETED);
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCompletedEv: Transc 0x%p,After callback",pTransc));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackAuthCompletedEv: Transc 0x%p,Application did not registere to callback",pTransc));

    }
}
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionCallbackOpenCallLegEv
 * ------------------------------------------------------------------------
 * General: When a request that is suitable for opening a dialog is received
 *          (INVITE/ REFER/SUBSCRIBE-with no To tag), the Transaction layer
 *          asks the application whether to open a call-leg for this transaction.
 *          For a proxy application, the callback is called for INVITE/REFER/
 *          SUBSCRIBE methods.
 *          For UA applications, the callback is called only for initial
 *          REFER/SUBSCRIBE methods.
 *          An application that does not want the Stack implementation that
 *          opens a new dialog for REFER and SUBSCRIBE should implement
 *          this callback.
 *          This function can be used by proxies that wish to handle specific
 *          requests in a call-leg context by themselves.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc    - Pointer to the transaction structure
 * Output:  pbNotifyCallLegMgr - RV_TRUE if we should notify the call-leg manager
 *                       of the new transaction. RV_FALSE otherwise.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackOpenCallLegEv(
                                         IN  Transaction  *pTransc,
                                         OUT RvBool       *pbNotifyCallLegMgr)
{
    RvInt32               nestedLock = 0;
    RvSipTranscMgrHandle  hMgr       = (RvSipTranscMgrHandle)pTransc->pMgr;
    void*                 pAppMgr    = pTransc->pMgr->pAppMgr;
    SipTripleLock         *pTripleLock;
    RvThreadId            currThreadId=0;
    RvInt32               threadIdLockCount=0;
    RvStatus              rv;

    pTripleLock = pTransc->tripleLock;

    if(pTransc->pMgr->pAppEvHandlers.pfnEvOpenCallLeg != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOpenCallLegEv: Transc 0x%p, Before callback", pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP ||
            pTransc->eOwnerType == SIP_TRANSACTION_OWNER_NON)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_OPEN_CALL_LEG);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = pTransc->pMgr->pAppEvHandlers.pfnEvOpenCallLeg(
                                                hMgr,
                                                pAppMgr,
                                                (RvSipTranscHandle)pTransc,
                                                pbNotifyCallLegMgr);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_OPEN_CALL_LEG);   
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOpenCallLegEv: Transc 0x%p,After callback, bOpenCallLeg=%d (rv=%d)",
                  pTransc,*pbNotifyCallLegMgr,rv));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOpenCallLegEv: Transc 0x%p,Application did not registere to callback",pTransc));

    }
}


#endif /* RV_SIP_PRIMITIVES */
/***************************************************************************
 * TransactionCallbackOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the sip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hAppTransc     - The application handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackOtherURLAddressFoundEv (
                     IN  Transaction           *pTransc,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool                 *pbAddressResolved)
{
    RvInt32                     nestedLock = 0;
    void                       *pOwner    = pTransc->pOwner;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvStatus                    rv          = RV_OK;
    RvSipTransactionEvHandlers *pEvHandlers = pTransc->pEvHandlers;

    pTripleLock = pTransc->tripleLock;

    *pbAddressResolved = RV_FALSE;
    if(pEvHandlers != NULL && (*pEvHandlers).pfnEvOtherURLAddressFound != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOtherURLAddressFoundEv: Transc 0x%p, Before callback",pTransc));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_OTHER_URL_FOUND);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        /* Converting the other URL address to SIP URL address */
        rv =  (*pEvHandlers).pfnEvOtherURLAddressFound(
                                    (RvSipTranscHandle)pTransc,
                                    pOwner,
                                    hMsg,
                                    hAddress,
                                    hSipURLAddress,
                                    pbAddressResolved);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_OTHER_URL_FOUND);
        if(rv == RV_OK && pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionCallbackOtherURLAddressFoundEv: Transc 0x%p, transc was terminated in cb. return failure",pTransc));
            rv = RV_ERROR_DESTRUCTED;
        }
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOtherURLAddressFoundEv: Transc 0x%p,After callback, bAddressResolved=%d",pTransc,*pbAddressResolved));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackOtherURLAddressFoundEv: Transc 0x%p,Application did not registere to callback bAddressResolved=%d",pTransc,*pbAddressResolved));

    }
    return rv;

}

/***************************************************************************
 * TransactionCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *          When this callback is called the application can query the
 *          transaction about the destination address using the
 *          RvSipTransactionGetCurrentDestAddr() API function.
 *          If the application wishes it can update the sent-by part of
 *          the top most via. The application must not update the branch parameter.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackFinalDestResolvedEv(
                            IN Transaction        *pTransc,
                            IN RvSipMsgHandle      hMsgToSend)
{

    RvStatus                    rv           = RV_OK;
    RvInt32                     nestedLock   = 0;
    void*                       pOwner       = pTransc->pOwner;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers *pEvHandlers = pTransc->pEvHandlers;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SipTransportAddress        destAddr;
    RvChar                     strDestAddress[RVSIP_TRANSPORT_LEN_STRING_IP];
    SipTransmitterGetDestAddr(pTransc->hTrx,&destAddr);

    RvAddressGetString(&destAddr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,strDestAddress);
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
              "TransactionCallbackFinalDestResolvedEv - Transc 0x%p - Destination address is: ip=%s, port=%d, transport=%s, address type=%s",
               pTransc,strDestAddress,
               RvAddressGetIpPort(&destAddr.addr),
               SipCommonConvertEnumToStrTransportType(destAddr.eTransport),
               SipCommonConvertEnumToStrTransportAddressType(SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&destAddr.addr)))));
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    pTripleLock = pTransc->tripleLock;
    if (NULL != pEvHandlers && NULL != pEvHandlers->pfnEvFinalDestResolved)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackFinalDestResolvedEv - Transc=0x%p, hMsg=0x%p, Before callback",pTransc,hMsgToSend));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_FINAL_DEST_RESOLVED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pEvHandlers).pfnEvFinalDestResolved(
                                       (RvSipTranscHandle)pTransc,
                                       pOwner,
                                       hMsgToSend);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_FINAL_DEST_RESOLVED);
        if(rv == RV_OK && pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionCallbackFinalDestResolvedEv: Transc 0x%p, transc was terminated in cb. return failure",pTransc));
            rv = RV_ERROR_DESTRUCTED;
        }
    
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackFinalDestResolvedEv - Transc 0x%p,After callback (rv=%d)",pTransc,rv));
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackFinalDestResolvedEv - Transc=0x%p, hMsg=0x%p, Application did not register to call back",
                  pTransc,hMsgToSend));

    }
    return rv;
}


/***************************************************************************
 * TransactionCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnNewConnInUseEvHandler callback
 * Return Value: RV_OK (the returned status is ignored in the current
 *                      stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc  - The transaction pointer.
 *            hConn    - The handle to the connection.
 *            bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackNewConnInUseEv(
                     IN  Transaction*                   pTransc,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated)
{

    RvStatus                    rv           = RV_OK;
    RvInt32                     nestedLock   = 0;
    void                       *pOwner       = pTransc->pOwner;
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvSipTransactionEvHandlers *pEvHandlers  = pTransc->pEvHandlers;

    pTripleLock = pTransc->tripleLock;
    if (NULL != pEvHandlers && NULL != pEvHandlers->pfnEvNewConnInUse)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackNewConnInUseEv - Transc=0x%p, hConn=0x%p, Before callback",
          pTransc, hConn));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_NEW_CONN_IN_USE);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        rv = (*pEvHandlers).pfnEvNewConnInUse(
                                       (RvSipTranscHandle)pTransc,
                                       pOwner,
                                       hConn,
                                       bNewConnCreated);
        
        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_NEW_CONN_IN_USE);    
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackNewConnInUseEv - Transc 0x%p, After callback (rv=%d)", pTransc,rv));
       
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionCallbackNewConnInUseEv - Transc=0x%p, hConn=0x%p, Application did not register to call back",
                   pTransc,hConn));

    }
    return rv;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that no response to the last sent
 *          SigComp message was received.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc       - The transaction handle
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackSigCompMsgNotRespondedEv (
                     IN  Transaction                 *pTransc,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp)
{
    SipTripleLock              *pTripleLock;
    RvThreadId                  currThreadId=0; RvInt32 threadIdLockCount=0;
    RvInt32                     nestedLock  = 0;
    void                       *pOwner      = pTransc->pOwner;
    RvStatus                    rv          = RV_OK;
    RvSipTransactionEvHandlers *pEvHandlers = pTransc->pEvHandlers;

    pTripleLock = pTransc->tripleLock;

    if(pEvHandlers != NULL && (*pEvHandlers).pfnEvSigCompMsgNotResponded != NULL)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackSigCompMsgNotRespondedEv: Transc=0x%p, retrans=%d, prev=%s, Before callback",
          pTransc,retransNo,TransactionGetMsgCompTypeName(ePrevMsgComp)));

        if (pTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
        {
            OBJ_ENTER_CB(pTransc, CB_TRANSC_SIGCOMP_NOT_RESPONDED);
            SipCommonUnlockBeforeCallback(pTransc->pMgr->pLogMgr,pTransc->pMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        /* notifying on non responded message */
        rv =  (*pEvHandlers).pfnEvSigCompMsgNotResponded(
                                    (RvSipTranscHandle)pTransc,
                                    pOwner,
                                    retransNo,
                                    ePrevMsgComp,
                                    peNextMsgComp);

        SipCommonLockAfterCallback(pTransc->pMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        OBJ_LEAVE_CB(pTransc, CB_TRANSC_SIGCOMP_NOT_RESPONDED);    
        
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackSigCompMsgNotRespondedEv: Transc=0x%p,After callback, next=%s",
          pTransc,TransactionGetMsgCompTypeName(*peNextMsgComp)));
        
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
          "TransactionCallbackSigCompMsgNotRespondedEv: Transc=0x%p, retrans=%d, prev=%s, Application did not registere to callback",
           pTransc,retransNo,TransactionGetMsgCompTypeName(ePrevMsgComp)));

    }
    return rv;

}
#endif /* RV_SIGCOMP_ON */


/* ----------- T R A N S A C T I O N   M G R   C A L L B A C K S  -------------*/

/***************************************************************************
 * TransactionCallbackMgrOutOfContextMsgEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a message the does
 *          not match any existing stack object.
 *          The callback is called in the following cases:
 *          1. ACK is received and does not match any existing transaction or dialog.
 *             (User agant applications has to configure the stack to work with dynamic
 *             invite handling to receive this callback for such ACK requests.)
 *          2. A response message is received and there is no matching client transaction.
 *          3. CANCEL is received and there is no matching INVITE transaction to cancel.
 *          4. The application returned RV_FALSE in the RvSipTranscMgrNewRequestRcvdEv
 *             indicating that it does not wish the stack to create
 *             a new transaction for an incoming request.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr - The transaction manager which received the message
 *            hMsg       - The received Out of Context message.
 *            hLocalAddr - indicates the local address on which the message
 *                         was received.
 *            pRcvdFromAddr - The address from where the message
 *                         was received.
 *            pOptions -   Other identifiers of the message such as compression type.
 *            hConn      - If the message was received on a specific connection,
 *                         the connection handle is supplied. Otherwise this parameter
 *                         is set to NULL.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrOutOfContextMsgEv (
                     IN TransactionMgr*                pTranscMgr,
                     IN RvSipMsgHandle                 hReceivedMsg,
                     IN RvSipTransportLocalAddrHandle  hLocalAddr,
                     IN SipTransportAddress*           pRcvdFromAddr,
                     IN RvSipTransportAddrOptions*     pOptions,
                     IN RvSipTransportConnectionHandle hConn)
{
    RvStatus rv = RV_OK;
    RvSipTransportAddr  localAddress;
    RvSipTransportAddr  rcvdFromAddress;

    memset(&localAddress,0,sizeof(RvSipTransportAddr));
    memset(&rcvdFromAddress,0,sizeof(RvSipTransportAddr));

    rv = RvSipTransportMgrLocalAddressGetDetails(hLocalAddr,
                                                 sizeof(RvSipTransportAddr),
                                                 &localAddress);
    if(RV_OK != rv)
    {
        RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
            "TransactionCallbackMgrOutOfContextMsgEv - Failed to get address info for local address 0x%p (rv=%d)",
            hLocalAddr,rv));
        return rv;
    }

    SipTransportConvertSipTransportAddressToApiTransportAddr(pRcvdFromAddr, &rcvdFromAddress);

    /*calling the new callback*/
    if(NULL != pTranscMgr->pMgrEvHandlers.pfnEvOutOfContextMsg)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOutOfContextMsgEv: Before callback hMsg = 0x%p. port=%d, transport=%d",
                  hReceivedMsg,localAddress.port,localAddress.eTransportType));

        rv = pTranscMgr->pMgrEvHandlers.pfnEvOutOfContextMsg(
                                     (RvSipTranscMgrHandle)pTranscMgr,
                                      pTranscMgr->pAppMgr,
                                      hReceivedMsg,
                                      hLocalAddr,
                                      &rcvdFromAddress,
                                      pOptions,
                                      hConn);
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOutOfContextMsgEv: after callback, hMsg = 0x%p. port=%d, transport=%d (rv=%d)",
                  hReceivedMsg,localAddress.port,localAddress.eTransportType,rv));
    }
    /*calling the deprecated callback*/
    else if(NULL != pTranscMgr->proxyEvHandlers.pfnEvOutOfContextMsg)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOutOfContextMsgEv: Before callback hMsg = 0x%p. port=%d, transport=%d",
                  hReceivedMsg,localAddress.port,localAddress.eTransportType));

        rv = pTranscMgr->proxyEvHandlers.pfnEvOutOfContextMsg(
                                     (RvSipTranscMgrHandle)pTranscMgr,
                                      pTranscMgr->pAppMgr,
                                      hReceivedMsg,
                                      localAddress.port,
                                      localAddress.eTransportType,
                                      localAddress.eAddrType,
                                      localAddress.strIP);

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOutOfContextMsgEv: after callback, hMsg = 0x%p. port=%d, transport=%d",
                  hReceivedMsg,localAddress.port,localAddress.eTransportType));
    }
    else
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOutOfContextMsgEv: Application did not registere to callback hMsg=0x%p",hReceivedMsg));

    }

    return rv;

}

/***************************************************************************
 * TransactionCallbackMgrNewRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new request that
 *          is not a retransmission and not an ACK request.
 *          The application should instruct the stack whether it should
 *          create a new transaction for the request or not.
 *          If you do not implement this callback, a new transaction
 *          will be created by default. Stateless proxies will usually want
 *          to prevent the stack from creating a new transaction.
 * Return Value: RvStatus - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr - The transaction manager that received the request.
 *          hMsg       - The handle to the new request message.
 * Output:  pbCreateTransc - Indicates whether the stack should handle the
 *                       new request by creating a new transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrNewRequestRcvdEv (
                        IN TransactionMgr*         pTranscMgr,
                        IN  RvSipMsgHandle         hMsg,
                        OUT RvBool               *pbCreateTransc)
{
    RvStatus rv = RV_OK;
    *pbCreateTransc = RV_TRUE;
	if (NULL != pTranscMgr->pMgrEvHandlers.pfnEvNewRequestRcvd)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			"TransactionCallbackMgrNewRequestRcvdEv: Before callback hMsg = 0x%p",hMsg));
		
        rv = pTranscMgr->pMgrEvHandlers.pfnEvNewRequestRcvd(
											(RvSipTranscMgrHandle)pTranscMgr,
											pTranscMgr->pAppMgr,
											hMsg,
											pbCreateTransc);
		
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			"TransactionCallbackMgrNewRequestRcvdEv: after callback, hMsg = 0x%p. bCreateTransc=%d",
			hMsg,*pbCreateTransc));
		
		
    }
	else if(NULL != pTranscMgr->proxyEvHandlers.pfnEvNewRequestRcvd)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			"TransactionCallbackMgrNewRequestRcvdEv: Before callback hMsg = 0x%p",hMsg));
		
        rv = pTranscMgr->proxyEvHandlers.pfnEvNewRequestRcvd(
			(RvSipTranscMgrHandle)pTranscMgr,
			pTranscMgr->pAppMgr,
			hMsg,
			pbCreateTransc);
		
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
			"TransactionCallbackMgrNewRequestRcvdEv: after callback, hMsg = 0x%p. bCreateTransc=%d",
			hMsg,*pbCreateTransc));
		
    }
    else
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrNewRequestRcvdEv: Application did not registere to callback hMsg=0x%p",hMsg));

    }
    return rv;

}
/***************************************************************************
 * TransactionCallbackMgrOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager sends an out of context message.
 *            Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr     - The transaction manager that received the request.
 *            hMsg           - The handle to the new request message.
 *            hAddress       - Handle to the unsupported address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrOtherURLAddressFoundEv(
                            IN TransactionMgr          *pTranscMgr,
                            IN  RvSipMsgHandle          hMsg,
                            IN  RvSipAddressHandle      hAddress,
                            OUT RvSipAddressHandle      hSipURLAddress,
                            OUT RvBool                 *bAddressResolved)
{
    RvStatus rv = RV_OK;
    if(NULL != pTranscMgr->proxyEvHandlers.pfnEvOtherURLAddressFound)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOtherURLAddressFoundEv: Before callback hMsg = 0x%p",hMsg));

        /* Converting the other URL address to SIP URL address */
        rv = (pTranscMgr->proxyEvHandlers).pfnEvOtherURLAddressFound(
                                    (RvSipTranscMgrHandle)pTranscMgr,
                                    pTranscMgr->pAppMgr,
                                    hMsg,
                                    hAddress,
                                    hSipURLAddress,
                                    bAddressResolved);

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOtherURLAddressFoundEv: after callback, hMsg = 0x%p. bAddressResolved=%d",
                  hMsg,*bAddressResolved));
    }
    else
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrOtherURLAddressFoundEv: Application did not registere to callback hMsg=0x%p",hMsg));
    }
    return rv;
}

/***************************************************************************
 * TransactionCallbackMgrFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnEvFinalDestResolved callback.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr     - The transaction manager pointer.
 *            hMsg           - The handle to the message that is about to be sent.
 *            hTrx           - The transmitter that is going to send the message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrFinalDestResolvedEv(
                          IN TransactionMgr          *pTranscMgr,
                          IN RvSipMsgHandle           hMsg,
                          IN RvSipTransmitterHandle   hTrx)
{
    RvStatus rv = RV_OK;
    RvChar                    strLocalAddress[RVSIP_TRANSPORT_LEN_STRING_IP];
#if defined(UPDATED_BY_SPIRENT)
    RvChar                    if_name[RVSIP_TRANSPORT_IFNAME_SIZE];
    RvUint16                  vdevblock;
#endif
    RvSipTransport            eTransportType;
    RvSipTransportAddressType eAddressType;
    RvUint16                  localPort;
    RvSipTransportAddr        destAddr;

    rv = RvSipTransmitterGetDestAddress(hTrx,sizeof(RvSipTransportAddr),
                                               0,&destAddr,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionCallbackMgrFinalDestResolvedEv - TransactionMgr 0x%p:Failed to get dest address from transmitter 0x%p (rv=%d)",
                   pTranscMgr,hTrx,rv));
        return rv;
    }

    rv = RvSipTransmitterGetCurrentLocalAddress(hTrx,&eTransportType,&eAddressType,
						strLocalAddress,&localPort
#if defined(UPDATED_BY_SPIRENT)
						,if_name
						,&vdevblock
#endif
						);

    if(rv != RV_OK)
    {
        RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                   "TransactionCallbackMgrFinalDestResolvedEv - TransactionMgr 0x%p:Failed to get local address from transmitter 0x%p (rv=%d)",
                   pTranscMgr,hTrx,rv));
        return rv;
    }
    RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
              "TransactionCallbackMgrFinalDestResolvedEv: hMsg = 0x%p, transport=%s, addressType=%s, local ip=%s, local port=%d, dest ip=%s, dest port=%d",
              hMsg,SipCommonConvertEnumToStrTransportType(eTransportType),
              SipCommonConvertEnumToStrTransportAddressType(eAddressType),
              strLocalAddress,localPort, destAddr.strIP, destAddr.port));

    if(NULL != pTranscMgr->proxyEvHandlers.pfnEvFinalDestResolved)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrFinalDestResolvedEv(hMsg=0x%p): Before callback",
                  hMsg));

        /* Converting the other URL address to SIP URL address */
        rv = (pTranscMgr->proxyEvHandlers).pfnEvFinalDestResolved(
                                    (RvSipTranscMgrHandle)pTranscMgr,
                                    pTranscMgr->pAppMgr,
                                    hMsg,
                                    eTransportType,
                                    eAddressType,
                                    destAddr.strIP,
                                    destAddr.port,
                                    strLocalAddress,
                                    &localPort
#if defined(UPDATED_BY_SPIRENT)
				    ,if_name
				    ,&vdevblock
#endif
				    );

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrFinalDestResolvedEv: after callback, hMsg = 0x%p,local ip=%s, local port=%d, (rv=%d)",
                  hMsg,strLocalAddress,localPort,rv));

        /*set the local address back to the transmitter*/
        if(rv == RV_OK)
        {
            rv = RvSipTransmitterSetLocalAddress(hTrx,eTransportType,eAddressType,strLocalAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
						 ,if_name
						 ,vdevblock
#endif
						 );
            if(rv != RV_OK)
            {
                RvLogError(pTranscMgr->pLogSrc ,(pTranscMgr->pLogSrc ,
                    "TransactionCallbackMgrFinalDestResolvedEv - TransactionMgr 0x%p:Failed to set local address to transmitter 0x%p (rv=%d)",
                    pTranscMgr,hTrx,rv));
                return rv;
            }
        }
    }
    else
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrFinalDestResolvedEv: Application did not register to callback hMsg=0x%p",hMsg));
    }
    return rv;
}


/***************************************************************************
 * TransactionCallbackMgrAckNoTranscEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnAckNoTranscRcvd callback.
 *
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hAckMsg       - The handle to the received ACK message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrAckNoTranscEv(
                     IN    TransactionMgr*			pTranscMgr,
                     IN    RvSipMsgHandle			hAckMsg,
                     IN    SipTransportAddress*     pRcvdFromAddr,
                     OUT   RvBool*					pbWasHandled)
{
    RvStatus  rv = RV_OK;
#ifndef RV_SIP_PRIMITIVES
    void     *pMgr = pTranscMgr->pCallMgrDetail.pMgr;

    RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
              "TransactionCallbackMgrAckNoTranscEv: hAckMsg = 0x%p",hAckMsg));

    if(NULL != pTranscMgr->pCallMgrDetail.pfnAckNoTranscRcvd)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAckNoTranscEv: Before callback hAckMsg = 0x%p",hAckMsg));

        rv = (pTranscMgr->pCallMgrDetail).pfnAckNoTranscRcvd(pMgr, hAckMsg, pRcvdFromAddr, pbWasHandled);

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAckNoTranscEv: after callback, hAckMsg = 0x%p",hAckMsg));
    }
    else
    {
        *pbWasHandled = RV_FALSE;
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAckNoTranscEv: Application did not register to callback hAckMsg=0x%p",hAckMsg));
    }
#else
    RV_UNUSED_ARG(pbWasHandled)
    RV_UNUSED_ARG(hAckMsg)
    RV_UNUSED_ARG(pTranscMgr || pRcvdFromAddr)
#endif
    return rv;
}


#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * TransactionCallbackMgrInviteResponseNoTranscEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnInviteResponseNoTranscRcvd callback.
 *
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hResponseMsg  - The handle to the received response message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrInviteResponseNoTranscEv(
                     IN    TransactionMgr*			pTranscMgr,
                     IN    RvSipMsgHandle			hMsg,
                     IN    SipTransportAddress*     pRcvdFromAddr,
                     OUT   RvBool*					pbWasHandled)
{
    RvStatus  rv = RV_OK;
    void     *pMgr = pTranscMgr->pCallMgrDetail.pMgr;

    RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
              "TransactionCallbackMgrInviteResponseNoTranscEv: hMsg = 0x%p",hMsg));

    if(NULL != pTranscMgr->pCallMgrDetail.pfnInviteResponseNoTranscRcvd)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrInviteResponseNoTranscEv: Before callback, hMsg = 0x%p",hMsg));

        rv = (pTranscMgr->pCallMgrDetail).pfnInviteResponseNoTranscRcvd(pMgr, hMsg, pRcvdFromAddr, pbWasHandled);

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrInviteResponseNoTranscEv: after callback, hMsg = 0x%p",hMsg));
    }
    else
    {
        *pbWasHandled = RV_FALSE;
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrInviteResponseNoTranscEv: Application did not register to callback hMsg=0x%p",hMsg));
    }
    return rv;
}


#else /*#ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
 * TransactionCallbackMgrAppForkedInviteRespEv
 * ------------------------------------------------------------------------
 * General: Notifies of a transaction state change and the
 *          associated state change reason.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr     - The transaction manager pointer.
 *          hMsg           - The handle to the message that is about to be sent.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackMgrAppForkedInviteRespEv(
                            IN TransactionMgr  *pTranscMgr,
                            IN RvSipMsgHandle  hMsg)
{

    RvSipTransactionEvHandlers  *pEvHandlers = &pTranscMgr->pAppEvHandlers;

    if(NULL != pEvHandlers && (*pEvHandlers).pfnEvForkedInviteRespRcvd != NULL)
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAppForkedInviteRespEv: TranscMgr 0x%p, Before callback",
                  pTranscMgr));

        (*pEvHandlers).pfnEvForkedInviteRespRcvd(pTranscMgr, hMsg);

        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAppForkedInviteRespEv: TranscMgr 0x%p,After callback",
                  pTranscMgr));
    }
    else
    {
        RvLogDebug(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                  "TransactionCallbackMgrAppForkedInviteRespEv: TranscMgr 0x%p,Application did not register to call back",
                  pTranscMgr));

    }
}
#endif /*#ifdef RV_SIP_PRIMITIVES*/


/* ----------- T R A N S A C T I O N   D E F A U L T   C A L L B A C K S  -------------*/

/***************************************************************************
 * TransactionCallBacksNoOwnerEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in one of the transactions.
 *          This is a default implementation that will be set to the callbacks
 *          in the construct function, and after detaching the transaction's owner.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occured.
 *            hTransc - The transaction in which the "event state changed"
 *                         occured.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV TransactionCallBacksNoOwnerEvStateChanged(
                            IN RvSipTranscHandle                   hTransc,
                            IN RvSipTranscOwnerHandle              hOwner,
                            IN RvSipTransactionState               eNewState,
                            IN RvSipTransactionStateChangeReason   eReason)
{
    Transaction *pTransc = (Transaction*)hTransc;
    RvStatus rv;
                
    RV_UNUSED_ARG(hOwner);
    RV_UNUSED_ARG(eReason);

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "TransactionCallBacksNoOwnerEvStateChanged: pTransc 0x%p, handle state %s",
        pTransc, TransactionGetStateName(eNewState)));

    switch (eNewState)
    {
        case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
        case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
        case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
            {
                RvUint16 responseCode;

                if (pTransc->responseCode != 0)
                {
                    responseCode = pTransc->responseCode;
                }
                else
                {
                    responseCode = GetResponseCode(pTransc);
                }
                rv = RvSipTransactionRespond(hTransc, responseCode, NULL);
                if (RV_OK != rv)
                {
                    RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                        "TransactionCallBacksNoOwnerEvStateChanged: pTransc 0x%p: failed to send response(rv=%d:%s). Terminate transaction",
                        pTransc, rv, SipCommonStatus2Str(rv)));
                    TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
                }
            }
            break;

        case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
            TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
            break;
            
        case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionCallBacksNoOwnerEvStateChanged: pTransc 0x%p, send ACK on final response", pTransc));
            rv = SipTransactionAck(hTransc, NULL);
            if (RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                    "TransactionCallBacksNoOwnerEvStateChanged: pTransc 0x%p: failed to send ACK(rv=%d:%s). Terminate transaction",
                    pTransc, rv, SipCommonStatus2Str(rv)));
                TransactionTerminate(RVSIP_TRANSC_REASON_ERROR, pTransc);
            }
            break;

        default:
            break;
    }
}

/******************************************************************************
 * GetResponseCode
 * ----------------------------------------------------------------------------
 * General: based on the presence of TO and FROM tags finds the response code
 *          (either 486 or 501).
 * Return Value: either 486 or 501.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - transaction.
 *****************************************************************************/
static RvUint16 GetResponseCode(IN Transaction* pTransc)
{
    RvUint16  responseCode;
    RvInt32   toTag   = UNDEFINED;
    RvInt32   fromTag = UNDEFINED;

    if(pTransc->hToHeader != NULL)
    {
        toTag = SipPartyHeaderGetTag(pTransc->hToHeader);
    }
    if(pTransc->hFromHeader != NULL)
    {
        fromTag = SipPartyHeaderGetTag(pTransc->hFromHeader);
    }
    if (toTag != UNDEFINED  && fromTag != UNDEFINED)
    {
        responseCode = 481;
    }
    else
    {
        if(pTransc->eMethod == SIP_TRANSACTION_METHOD_INVITE)
        {
            responseCode = 486; /*Busy Here*/
        }
        else
        {
            responseCode = 501;
        }
    }
    return responseCode;
}

#endif /*#ifndef RV_SIP_LIGHT*/

