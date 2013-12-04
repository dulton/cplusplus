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
 *                              <TransactionObject.c>
 *
 *  This file organizes and uses the transaction fields that are used in receiving
 *  and sending messages. These fields are To, From, Call-Id, CSeq-Step, Via list.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2000
 *********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include <string.h>
#include "_SipCommonConversions.h"

#include "TransactionObject.h"
#include "TransactionMgrObject.h"
#include "RvSipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipAddress.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipTransport.h"
#include "TransactionTimer.h"
#include "_SipPartyHeader.h"
#include "_SipAddress.h"
#include "_SipViaHeader.h"
#include "AdsRlist.h"
#include "_SipMsg.h"
#include "RvSipOtherHeader.h"
#include "TransactionTransport.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
#include "RvSipRSeqHeader.h"
#include "_SipOtherHeader.h"
#include "TransactionAuth.h"
#include "_SipTransactionMgr.h"
#include "RvSipReferredByHeader.h"
#include "TransactionCallBacks.h"
#ifdef RV_SIGCOMP_ON
#include "_SipCompartment.h"
#endif
#include "_SipCommonUtils.h"
#include "_SipTransmitterMgr.h"
#include "_SipTransmitter.h"
#include "rpool_API.h"
#include "AdsCopybits.h"
#include "_SipCommonCSeq.h"
#ifdef RV_SIP_IMS_ON
#include "TransactionSecAgree.h"
#include "_SipSecuritySecObj.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define TRANSC_STATE_CLIENT_MSG_SEND_FAILURE_STR "Client Message Send failure"
#define TRANSC_STATE_TERMINATED_STR              "Terminated"
#define TRANSC_OBJECT_NAME_STR                   "Transaction"

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
static void RVCALLCONV UpdateCancelTranscPairLockingOnDestruction(
                                                Transaction *pTransc);

static RvStatus RVCALLCONV AddMaxForwardsToMsg(
                                      IN RvSipMsgHandle hMsg);

static RvStatus  RVCALLCONV TerminateTransactionEventHandler(IN void *trans,
                                                             IN RvInt32  reason);

static RvStatus RVCALLCONV UpdateFromHeader(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV UpdateToHeader(
                                           IN Transaction   *pTransc,
                                           IN RvSipMsgHandle hMsg,
                                           IN RvUint16 responseCode);
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus  RVCALLCONV MsgSendFailureEventHandler( IN void   *trans,
                                                        IN RvInt32 reason,
                                                        IN RvInt32 notUsed,
                                                        IN RvInt32 objUniqueIdentifier);

#endif
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus TransactionLock(IN Transaction    *pTransc,
                                IN TransactionMgr *pMgr,
                                IN SipTripleLock  *tripleLock,
                                IN RvInt32         identifier);

static void RVCALLCONV TransactionUnLock(IN  RvLogMgr   *pLogMgr,
                                         IN  RvMutex    *hLock);


static void RVCALLCONV TransactionTerminateUnlock(IN RvLogMgr      *pLogMgr,
                                                  IN RvLogSource   *pLogSrc,
                                                  IN RvMutex       *hObjLock,
                                                  IN RvMutex       *hProcessingLock);
#else
#define TransactionLock(a,b,c,d) (RV_OK)
#define TransactionUnLock(a,b)
#define TransactionTerminateUnlock(a,b,c,d)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/



#ifndef RV_SIP_PRIMITIVES
static RvStatus RVCALLCONV AddReliableInfoToMsg(
                                    IN Transaction *pTransc,
                                    IN RvSipMsgHandle hMsg);

static RvStatus RVCALLCONV AddSupportedListToMsg(IN Transaction*   pTransc,
                                    IN RvSipMsgHandle hMsg);

static RvBool RVCALLCONV IsRequireInSupportedList(
                                    IN Transaction             *pTransc,
                                    IN RvSipOtherHeaderHandle  hRequire);

static RvStatus RVCALLCONV AddUnsupportedListToMsg(
                                    IN Transaction*   pTransc,
                                    IN RvSipMsgHandle hMsg);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar* TransactionOwner2Str(SipTransactionOwner eOwner);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


#ifdef RV_SIGCOMP_ON

static RvStatus LookForSigCompUrnInRcvdMsg(IN   Transaction      *pTransc,
                                           IN   RvSipMsgHandle    hMsg, 
                                           OUT  RvBool           *pbUrnFound,
                                           OUT  RPOOL_Ptr        *pUrnRpoolPtr); 

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus DetachFromInternalSigcompCompartment(Transaction *pTransc);
#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT */
#endif /* #ifdef RV_SIGCOMP_ON */ 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TransactionInitialize
 * ------------------------------------------------------------------------
 * General: Initialize the transaction object. allocate all needed resources
 *          and set parameters to initial value.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * Return Value: RV_OK - The transaction was successfully initialized.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key
 *                                     values received as a parameter is not
 *                                     legal.
 *                                     All key fields must have legal values.
 *               RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The transaction manager to which this transaction
 *                       will belong.
 *          pTransc    - The new transaction.
 *          b_isUac    - RV_TRUE if the transaction is UAC. RV_FALSE for UAS.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionInitialize(
                                    IN  RvSipTranscMgrHandle   hTranscMgr,
                                    IN  Transaction           *pTransc,
                                    IN  RvBool                 b_isUac)
{
    RvStatus          rv;

    RV_UNUSED_ARG(hTranscMgr);
    /* Note that when initializing transaction, it's memory should never be reset to '0'
    (memset) because of transactionTripleLock that must be kept without change during
    SIP stack live */
    memset(&(pTransc->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    pTransc->cctContext        = -1;
    pTransc->URI_Cfg_Flag      = 0xff;
    pTransc->localAddr.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
    pTransc->localAddr.eTransportType = RVSIP_TRANSPORT_UNDEFINED;
#endif
//SPIRENT_END
    pTransc->pContext          = NULL;
    pTransc->pEvHandlers       = NULL;
    pTransc->bTagInTo          = RV_FALSE;
    pTransc->pOwner            = NULL;
    pTransc->eOwnerType        = SIP_TRANSACTION_OWNER_NON;
    pTransc->hAppHandle        = NULL;
    pTransc->hFromHeader       = NULL;
    pTransc->hToHeader         = NULL;
    pTransc->hRequestUri       = NULL;
    pTransc->hReceivedMsg      = NULL;
    pTransc->strCallId.hPage   = NULL_PAGE;
    pTransc->strCallId.hPool   = NULL;
    pTransc->strCallId.offset  = UNDEFINED;
    pTransc->bAckSent          = RV_FALSE;
    pTransc->bInitialRequest   = RV_FALSE;
    pTransc->bSending          = RV_FALSE;
    pTransc->hTrx              = NULL;
#ifdef SIP_DEBUG
    pTransc->pCallId           = NULL;
#endif
	SipCommonCSeqReset(&pTransc->cseq); 
    pTransc->eMethod           = SIP_TRANSACTION_METHOD_UNDEFINED;
    pTransc->strMethod         = NULL;
    pTransc->eState            = RVSIP_TRANSC_STATE_IDLE;
    /* Create a page for the first encoded message to be sent by the
       transaction */
    SipCommonCoreRvTimerInit(&pTransc->hMainTimer);
    SipCommonCoreRvTimerInit(&pTransc->hRequestLongTimer);
    pTransc->retransmissionsCount = 0;
    pTransc->pViaHeadersList      = NULL;
    pTransc->responseCode         = 0;
    pTransc->tripleLock           = NULL;
    pTransc->hashElementPtr       = NULL;
    pTransc->bAppInitiate         = RV_FALSE;
    pTransc->bIsUac               = b_isUac;
    pTransc->hCancelTranscPair    = NULL;
    pTransc->hTopViaHeader        = NULL;
    pTransc->msgType              = SIP_TRANSPORT_MSG_UNDEFINED;
    pTransc->pbIsTransactionTerminated = NULL;
    pTransc->hTimeStamp           = NULL;
    pTransc->hActiveConn               = NULL;
    pTransc->hClientResponseConnBackUp = NULL;

    pTransc->eTranspType			= RVSIP_TRANSPORT_UNDEFINED;
    pTransc->hOutboundMsg			= NULL;
#ifndef RV_SIP_PRIMITIVES
    pTransc->pSessionTimer			= NULL;
    pTransc->localRSeq.val			= 0;
	pTransc->localRSeq.bInitialized = RV_FALSE;
    pTransc->hPrackParent			= NULL;
    pTransc->relStatus				= RVSIP_TRANSC_100_REL_UNDEFINED;
    pTransc->unsupportedList		= UNDEFINED;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    pTransc->bIsProxy				= RV_FALSE;
    pTransc->bIsProxySend2xxTimer	= RV_FALSE;
    pTransc->eLastState				= RVSIP_TRANSC_STATE_IDLE;
#ifdef RV_SIP_AUTH_ON
    pTransc->pAuthorizationHeadersList = NULL;
    pTransc->lastAuthorizGotFromList   = NULL;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    pTransc->pSubsInfo            = NULL;
    pTransc->hReferredBy          = NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    pTransc->postConnectionAssertionNameOffset = -1;
#ifdef SIP_DEBUG
    pTransc->strUnsupportedList   = NULL;
#endif
    memset(&pTransc->receivedFrom,0,sizeof(pTransc->receivedFrom));
    pTransc->receivedFrom.eTransport = RVSIP_TRANSPORT_UNDEFINED;

    pTransc->bAllowCancellation   = RV_FALSE;
    pTransc->bAllowAckHandling    = RV_TRUE;
    pTransc->reqMsgFromHeader = NULL;
    pTransc->reqMsgToHeader   = NULL;
    pTransc->non2xxRespMsgToHeader   = NULL;
    pTransc->hOrigToForCancel       = NULL;
#ifdef RV_SIGCOMP_ON
    pTransc->bSigCompEnabled       = RV_TRUE; 
    pTransc->hSigCompCompartment   = NULL;
    pTransc->eIncomingMsgComp      = RVSIP_COMP_UNDEFINED;
#endif /* #ifdef RV_SIGCOMP_ON */ 
#ifndef RV_SIP_PRIMITIVES
    pTransc->hHeadersListToSetInInitialRequest = NULL;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	pTransc->hInitialRequestForReprocessing    = NULL; 
	memset(&(pTransc->reprocessingInfo),0,sizeof(RvSipTransportObjEventInfo));
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */
#ifdef RV_SIP_IMS_ON    
	pTransc->hSecAgree = NULL;
	pTransc->hSecObj = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/
    pTransc->cbBitmap = 0;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    pTransc->bMsgSentFailureInQueue = RV_FALSE;
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    pTransc->pTimers = pTransc->pMgr->pTimers;

    /* Allocate the memory page for the transaction. The Via headers
       list will be created on this page, and also the To, From,
       and Call-Id, if the transaction is a stand-alone transaction */
    rv = RPOOL_GetPage(pTransc->pMgr->hGeneralPool, 0, &(pTransc->memoryPage));
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionInitialize - Failed to get page for the transaction"));
         return rv;
    }

    /* Construct the transport transmitter for the transaction messages */
    rv = SipTransmitterMgrCreateTransmitter(
            pTransc->pMgr->hTrxMgr,
            (RvSipAppTransmitterHandle)pTransc,
            &pTransc->pMgr->transcTrxEvHandlers,
            pTransc->pMgr->hGeneralPool,
            pTransc->memoryPage,
            pTransc->tripleLock,
            &pTransc->hTrx);

    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionInitialize - Failed to Construct transmitter object"));
        RPOOL_FreePage(pTransc->pMgr->hGeneralPool,pTransc->memoryPage);
        return rv;
    }

    /* Set unique identifier only on initialization success */
    RvRandomGeneratorGetInRange(pTransc->pMgr->seed, RV_INT32_MAX,
        (RvRandom*)&pTransc->transactionUniqueIdentifier);

    return RV_OK;
}


/***************************************************************************
 * TransactionInitiateOwner
 * ------------------------------------------------------------------------
 * General: Attaches the owner and it's lock to the transaction.
 *          The transaction's callbacks will be set with a set of callbacks
 *          that are defined by this owner.
 * Return Value: RV_OK - The initialization have succeeded
 *                 RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The new transaction created.
 *         pOwner - The owner of the transaction.
 *         eOwnerType - The owner's type.
 *         tripleLock - The owner's triple lock.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionInitiateOwner(
                                    IN  Transaction           *pTransc,
                                    IN  void                  *pOwner,
                                    IN  SipTransactionOwner    eOwnerType,
                                    IN  SipTripleLock            *tripleLock)
{
    RvLogSource*  logSrc;


    logSrc = (pTransc->pMgr)->pLogSrc;
    /* Set the transaction's owner */
    pTransc->pOwner = pOwner;
    pTransc->eOwnerType = eOwnerType;


    /*------------------------------------------------------------
        Initiate application owner or a stand alone transaction
      ------------------------------------------------------------*/
    if(SIP_TRANSACTION_OWNER_APP == eOwnerType)
    {
        /*set the application event handlers*/
        pTransc->pEvHandlers = &((pTransc->pMgr)->pAppEvHandlers);
        /*initiate the transaction lock*/
        TransactionSetTripleLock(&(pTransc->transactionTripleLock),pTransc);

        return RV_OK;
    }

    /*-------------------------------------
        Initiate a stand alone transaction
      -------------------------------------*/
    if(pOwner == NULL)
    {
       /*The transaction stands alone for now. An owner if exists will be
        attached later. This should be a server transaction*/
        pTransc->pEvHandlers = &((pTransc->pMgr)->pDefaultEvHandlers);

        /*initiate the transaction lock*/
        TransactionSetTripleLock(&(pTransc->transactionTripleLock),pTransc);

        return RV_OK;
    }

    /*-----------------------------------------
             Other owner
     ------------------------------------------*/
    /*in all other cases the owner should supply its lock*/
    if (tripleLock == NULL)
    {
        RvLogError(logSrc ,(logSrc ,
                 "TransactionInitiateOwner - Error - owner did not supply a lock"));
        return RV_ERROR_BADPARAM;
    }
    TransactionSetTripleLock(tripleLock,pTransc);

    switch (eOwnerType)
    {
#ifndef RV_SIP_PRIMITIVES
    case (SIP_TRANSACTION_OWNER_CALL):
        /* The owner is a call */
        pTransc->pEvHandlers =
            &((pTransc->pMgr)->pCallLegEvHandlers);
        break;
    case (SIP_TRANSACTION_OWNER_TRANSC_CLIENT):
        /* The owner is a register client */
        pTransc->pEvHandlers =
            &((pTransc->pMgr)->pTranscClientEvHandlers);
        break;
#endif /* RV_SIP_PRIMITIVES */
    default:
        /* The owner type is not supported */
        RvLogError(logSrc ,(logSrc ,
            "TransactionInitiateOwner - Error in trying to create a new transaction"));
        return RV_ERROR_BADPARAM;
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionNoOwner
 * ------------------------------------------------------------------------
 * General: Called when attaching NULL owner. When there is no owner
 *          the To,From and Call-Id should be allocated on the transaction
 *          page. The transaction will use the lock it already has even if
 *          it does not belong to it.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 ****************************************************************************/
RvStatus RVCALLCONV TransactionNoOwner(IN Transaction *pTransc)
{
    RvStatus           rv;

    /* Set the transaction's owner to NULL */
    pTransc->pOwner = NULL;
    pTransc->eOwnerType = SIP_TRANSACTION_OWNER_NON;
    /* set transaction's user context to NULL */
    pTransc->pContext = NULL;

    /* Set the transaction's set of callbacks to default callbacks */
    pTransc->pEvHandlers = &(pTransc->pMgr->pDefaultEvHandlers);

     
    /*copy the key values to the transaction page - only if the transaction is still in the hash*/
    if(NULL != pTransc->hashElementPtr)
    {
        rv = TransactionCopyKeyToInternalPage(pTransc);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                      "TransactionNoOwner - Transaction 0x%p - Failed to copy key to transaction page",
                      pTransc));
            return rv;
        }
    }

    if((pTransc->eState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD)    ||
       (pTransc->eState == RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD) ||
       (pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE)    ||
       (pTransc->eState == RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD))
    {
        TransactionTerminate(RVSIP_TRANSC_REASON_DETACH_OWNER, pTransc);
    }
    return RV_OK;
}


/***************************************************************************
 * TransactionAttachAppOwner
 * ------------------------------------------------------------------------
 * General:
 * Return Value: RV_OK - The new owner is attached to the transaction.
 *                              (Attaching NULL is equivalent to detach. The
 *                               transaction will no longer has an owner, and the
 *                               callbacks use a default implementation).
 *                 RV_ERROR_NULLPTR - The transaction handle parameter is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - The transaction to attach the owner to.
 *            pOwner -       The new owner of the transaction.
 *          eOwnerType -   The owner's type: Call, Registrar ...
 *          bIsLast -      Is this the last owner we will try to attach this
 *                         transaction. If yes than in case this owner is NULL
 *                         the transaction is a stand alone transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAttachApplicationOwner(
                                    IN RvSipTranscHandle     hTransc,
                                    IN SipTransactionKey    *pKey,
                                    IN void                 *pOwner)
{
    RvStatus    rv;
    Transaction  *pTransc = (Transaction *)hTransc;

    if(pKey == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    pTransc->pOwner = pOwner;
    pTransc->eOwnerType = SIP_TRANSACTION_OWNER_APP;

    /*if the pTransc->tripleLock is NULL point it to the transaction lock. It will be
      different from NULL for cancel transaction when the pointer points to the cancelled transaction*/

    /*update the triple lock pointer only if it was not updated yet*/
    if(pTransc->tripleLock == NULL)
    {
        TransactionSetTripleLock(&(pTransc->transactionTripleLock),pTransc);
    }

    /* Set the callbacks defined for the application to the transaction.*/
    pTransc->pEvHandlers = &((pTransc->pMgr)->pAppEvHandlers);


    /*set the given key inside the transaction data and on its page*/
	rv = SipCommonCSeqSetStep(pKey->cseqStep,&pTransc->cseq); 
	if (rv != RV_OK)
	{
		RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionAttachApplicationOwner - Transaction 0x%p - failed to set CSeq=%d (rv=%d)",
			pTransc,pKey->cseqStep,rv));
        return rv; 
	}

    rv = TransactionCopyKeyToInternalPage(pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionAttachApplicationOwner - Transaction 0x%p - Failed to copy key",pTransc));
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * TransactionAttachOwner
 * ------------------------------------------------------------------------
 * General: Attach a new owner to the transaction. The owner should attach it's
 * callback implementations to the transaction as well, so that the
 * callbacks will be addressed to this owner (the handle to the
 * owner will be sent as a parameter to the callback). The owner should also
 * attach its lock to the transaction, so that the transaction could lock it
 * when synchronization is needed.
 * Return Value: RV_OK - The new owner is attached to the transaction.
 *                              (Attaching NULL is equivalent to detach. The
 *                               transaction will no longer has an owner, and the
 *                               callbacks use a default implementation).
 *                 RV_ERROR_NULLPTR - The transaction handle parameter is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc        - The transaction to attach the owner to.
 *          pOwner         - The new owner of the transaction.
 *          eOwnerType     - The owner's type: Call, Registrar ...
 *          bIsLast        - Is this the last owner we will try to attach this
 *                         transaction. If yes than in case this owner is NULL
 *                         the transaction is a stand alone transaction.
 *          tripleLock     - locks of the transaction
 *          bCopyTripleLock- Indication if to copy the input tripleLock to
 *                           the local pTransc->tripleLock, since there
 *                           might be a situation that the lock has already
 *                           been copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAttachOwner(
                                    IN RvSipTranscHandle       hTransc,
                                    IN void                   *pOwner,
                                    IN SipTransactionKey      *pKey,
                                    IN SipTripleLock          *tripleLock,
                                    IN SipTransactionOwner     eOwnerType,
                                    IN RvBool                  bCopyTripleLock)
{
    Transaction *pTransc;

    pTransc = (Transaction *)hTransc;

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "TransactionAttachOwner: transaction 0x%p - old triple-lock:0x%p, new triple-lock:0x%p pOwner:0x%p eOwnerType: %s",
        pTransc, pTransc->tripleLock, tripleLock, pOwner, TransactionOwner2Str(eOwnerType)));

    pTransc->pOwner = pOwner;
    pTransc->eOwnerType = eOwnerType;

    if (RV_TRUE == bCopyTripleLock)
    {
        /* Set the transaction's lock to the owner's lock */
        TransactionSetTripleLock(tripleLock,pTransc);
    }

    switch (eOwnerType)
    {
#ifndef RV_SIP_PRIMITIVES
    case (SIP_TRANSACTION_OWNER_CALL):
    /* The owner is a Call object.
        Set the callbacks defined for the Call to the transaction. */
        pTransc->pEvHandlers = &((pTransc->pMgr)->pCallLegEvHandlers);
        break;
#endif /* RV_SIP_PRIMITIVES */
    default:
        return RV_ERROR_BADPARAM;
    }
    /* The owner supplies it's key values in the key parameter, and the transaction
       copies them. */
    TransactionAttachKey(pTransc,pKey);
    return RV_OK;
}

/***************************************************************************
 * TransactionAttachKey
 * ------------------------------------------------------------------------
 * General: Attach the values of a given key to the transaction object.
 *          The key can belong to the transaction owner or the a message.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            pKey - The transaction's key.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAttachKey(
                                    IN  Transaction           *pTransc,
                                    IN  SipTransactionKey     *pKey)
{
	RvStatus rv; 
	
    if(pKey->bIgnoreLocalAddr == RV_FALSE)
    {
        SipTransmitterSetLocalAddressesStructure(pTransc->hTrx,&pKey->localAddr);
    }
    TransactionLockTranscMgr(pTransc);
    pTransc->hFromHeader = pKey->hFromHeader;
    pTransc->hToHeader   = pKey->hToHeader;
    pTransc->strCallId.hPage   = pKey->strCallId.hPage;
    pTransc->strCallId.hPool   = pKey->strCallId.hPool;
    pTransc->strCallId.offset   = pKey->strCallId.offset;
#ifdef SIP_DEBUG
    pTransc->pCallId = RPOOL_GetPtr(pTransc->strCallId.hPool,
                                    pTransc->strCallId.hPage,
                                    pTransc->strCallId.offset);
#endif
    TransactionUnlockTranscMgr(pTransc);

	rv = SipCommonCSeqSetStep(pKey->cseqStep,&pTransc->cseq);
	if (rv != RV_OK)
	{
		RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionAttachKey - Transaction 0x%p - failed to set CSeq=%d (rv=%d)",
			pTransc,pKey->cseqStep,rv));
        return rv; 
	}

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionAttachKey - Transaction 0x%p - Key was attached successfully",pTransc));

    return RV_OK;
}

/***************************************************************************
 * TransactionCopyKeyToInternalPage
 * ------------------------------------------------------------------------
 * General: In many cases the transaction key span on the transaction owner
 *          page. This function takes the transaction key on copy it to
 *          the transaction page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc    - The transaction handle
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyKeyToInternalPage(
                                    IN  Transaction      *pTransc)

{
    RvStatus rv;

    /* we are changing the key of the transaction, so we must lock the manager */
    TransactionLockTranscMgr(pTransc);

    rv = TransactionCopyFromHeader(pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionCopyKeyToInternalPage - Transaction 0x%p - Failed to copy From header",
                  pTransc));
        /* failure - remove transaction from the hash table, and unlock the manager */
        if(pTransc->hashElementPtr != NULL)
        {
            HASH_RemoveElement(pTransc->pMgr->hHashTable, pTransc->hashElementPtr);
            pTransc->hashElementPtr = NULL;
        }
        TransactionUnlockTranscMgr(pTransc);
        return rv;
    }
    /* Create an independent To header and copy the old To value to this
       header */
    rv = TransactionCopyToHeader(pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                   "TransactionCopyKeyToInternalPage - Transaction 0x%p - Failed to copy To header",
                     pTransc));
         /* failure - remove transaction from the hash table, and unlock the manager */
        if(pTransc->hashElementPtr != NULL)
        {
            HASH_RemoveElement(pTransc->pMgr->hHashTable, pTransc->hashElementPtr);
            pTransc->hashElementPtr = NULL;
        }
        TransactionUnlockTranscMgr(pTransc);
        return rv;
    }

    /* Create an independent Call-Id string and copy the old Call-Id string to
    this header */
    rv = TransactionCopyCallId(pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                   "TransactionCopyKeyToInternalPage - Transaction 0x%p - Failed to copy Call-Id",
                     pTransc));
         /* failure - remove transaction from the hash table, and unlock the manager */
        if(pTransc->hashElementPtr != NULL)
        {
            HASH_RemoveElement(pTransc->pMgr->hHashTable, pTransc->hashElementPtr);
            pTransc->hashElementPtr = NULL;
        }
        TransactionUnlockTranscMgr(pTransc);
        return rv;
    }

    TransactionUnlockTranscMgr(pTransc);
    return RV_OK;

}


/***************************************************************************
 * TransactionCreate
 * ------------------------------------------------------------------------
 * General: Called when a server transaction is created by the transaction
 * manager. The transaction key values will be received as parameters
 * and will be set to the transaction's inner parameters.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * The Transaction's callbacks will be set with a set of callbacks that
 * are defined by it's owner and the owner and it's lock will be attached to \
 * the transaction.
 * The transaction will sign itself to the hash manager.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key values
 *                                       received as a parameter is not legal.
 *                                     All key fields must have legal values.
 *               RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 *                 RV_ERROR_NULLPTR - At least one of the pointers to the
 *                                 transaction key values is a NULL pointer,
 *                                 or the pointer to the transaction pointer is a
 *                                 NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction object.
 *                                     Another possibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager to which this transaction will
 *                          belong.
 *          pKey          - The transaction's key.
 *          pOwner        - The transaction's owner.
 *          eOwnerType    - The transaction's owner type.
 *          tripleLock    - The transaction's locks.
 *          b_isUac       - is client flag
 * Output:  phTransaction - The new transaction created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreate(
                                    IN  RvSipTranscMgrHandle    hTranscMgr,
                                    IN  SipTransactionKey        *pKey,
                                    IN  void                    *pOwner,
                                    IN  SipTransactionOwner        eOwnerType,
                                    IN  SipTripleLock            *tripleLock,
                                    IN  RvBool                    b_isUac,
                                    OUT RvSipTranscHandle        *phTransaction)
{
    Transaction       *pTransc;
    RvStatus          rv;
    TransactionMgr    *pMgr = (TransactionMgr*)hTranscMgr;

    *phTransaction = NULL;

    /*create and initialize an empty transaction object*/
    rv = TransactionMgrCreateTransaction(pMgr,b_isUac,(RvSipTranscHandle *)&pTransc);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransactionCreate - Error in trying to create a new transaction"));
        return rv;
    }

    /* Initiate the transaction with key values */
    TransactionAttachKey(pTransc, pKey);


    /* Initiate the transaction's owner relevant parameters */
    rv = TransactionInitiateOwner(pTransc, pOwner, eOwnerType, tripleLock);
    if (RV_OK != rv)
    {
        TransactionDestruct(pTransc);
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransactionCreate - Failed to initiate owner"));
        return rv;
    }

    RvLogInfo(pMgr->pLogSrc ,(pMgr->pLogSrc ,
              "TransactionCreate - Transaction 0x%p: Was successfully created", pTransc));

    *phTransaction = (RvSipTranscHandle)pTransc;
    return RV_OK;
}


/***************************************************************************
 * TransactionTerminate
 * ------------------------------------------------------------------------
 * General: Sends INTERNAL_OBJECT_EVENT of terminated to the processing queue event.
 * Return Value: RV_OK        - The transaction was terminated successfully.
 *                 RV_ERROR_NULLPTR    - The pointer to the transaction is a NULL
 *                                  pointer.
 *                 RV_ERROR_OUTOFRESOURCES
 *                                - The function failed to allocate event structure
 *                                  due to no enough resources.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 *            eReason -      The reason for terminating the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTerminate(
                        IN RvSipTransactionStateChangeReason    eReason,
                        IN Transaction                            *pTransc)
{

    /*transaction was already terminated*/
    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        return RV_OK;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
             "TransactionTerminate - Transaction 0x%p: Termination called", pTransc));

   /* change the transaction state to terminate with no application
      notification. The state change callback will be called from the
      event queue*/
    /*save the last state*/
    pTransc->eLastState = pTransc->eState;
    pTransc->eState = RVSIP_TRANSC_STATE_TERMINATED;

    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    /* we don't want other threads to give us connection events from this point on */
    TransactionTransportDetachAllConnections(pTransc);

    /*the transaction may be destructed before creating a transmitter
      so we need to check for null*/
    if(pTransc->hTrx != NULL)
    {
        SipTransmitterDetachFromConnection(pTransc->hTrx);
        SipTransmitterDetachFromAres(pTransc->hTrx);
    }

#ifdef RV_SIP_IMS_ON 
	/* notify the security-agreement on transaction's termination */
    TransactionSecAgreeDetachSecAgree(pTransc);
    /* Detach from Security Object */
    if (NULL != pTransc->hSecObj)
    {
        RvStatus rv;
        rv = TransactionSetSecObj(pTransc, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionTerminate - Transaction 0x%p: Failed to unset Security Object %p",
                pTransc, pTransc->hSecObj));
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    /*remove transaction from hash*/
    TransactionMgrHashRemove((RvSipTranscHandle)pTransc);

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
             "TransactionTerminate - Transaction 0x%p: Inserting Transaction to the event queue", pTransc));

    /*insert the transaction to the event queue*/
    return SipTransportSendTerminationObjectEvent(pTransc->pMgr->hTransport,
                                        (void *)pTransc,
                                        &(pTransc->terminationInfo),
                                        (RvInt32)eReason,
                                        TerminateTransactionEventHandler,
                                        RV_TRUE,
                                        TRANSC_STATE_TERMINATED_STR,
                                        TRANSC_OBJECT_NAME_STR);
}


/***************************************************************************
 * TransactionDestruct
 * ------------------------------------------------------------------------
 * General: Destruct a transaction object.
 * Recycle personal used memory, including the transaction object itself
 * Return Value: RV_OK - The transaction has been destructed successfully
 *               RV_ERROR_NULLPTR - The pointer to the transaction to be deleted
 *                                 (received as a parameter) is a NULL pointer.
 *                 RV_ERROR_UNKNOWN - The UnManageTransaction call has returned the
 *                              RV_ERROR_NOT_FOUND return value. This is an error since
 *                              the transaction object is the only entity that
 *                              can call the UnManageTransaction function of the
 *                              transaction manager.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  trans - A pointer to the transaction that will be destructed.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionDestruct(Transaction *pTransc)
{
    RvLogSource*       pLogSrc;

    pLogSrc = pTransc->pMgr->pLogSrc;

    RvLogDebug(pLogSrc ,(pLogSrc ,
              "TransactionDestruct - Transaction 0x%p: start to destruct transaction", pTransc));

    pTransc->transactionUniqueIdentifier = 0;

    UpdateCancelTranscPairLockingOnDestruction(pTransc);
    /*release transaction timers*/
    TransactionTimerReleaseAllTimers(pTransc);

    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
    }
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	if (pTransc->hInitialRequestForReprocessing != NULL)
	{
		/* If the message was stored in the transaction for possible reprocessing
		we deallocate it now */
		RvSipMsgDestruct(pTransc->hInitialRequestForReprocessing);
	}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

    TransactionTransportDetachAllConnections(pTransc);

#ifdef RV_SIP_IMS_ON
    /* Detach from Security Object */
    if (NULL != pTransc->hSecObj)
    {
        RvStatus rv;
        rv = TransactionSetSecObj(pTransc, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionDestruct - Transaction 0x%p: Failed to unset Security Object %p",
                pTransc, pTransc->hSecObj));
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIGCOMP_ON
    /* Detach from the Transaction's related Compartment */
    if (pTransc->hSigCompCompartment != NULL) 
    {
        SipCompartmentDetach(pTransc->hSigCompCompartment, (void*)pTransc);
        pTransc->hSigCompCompartment = NULL;
    }
#endif /* #ifdef RV_SIGCOMP_ON */ 
    
    if(pTransc->hTrx != NULL)
    {
        SipTransmitterDestruct(pTransc->hTrx);
		pTransc->hTrx = NULL;
    }

#ifndef RV_SIP_PRIMITIVES
    if(pTransc->hHeadersListToSetInInitialRequest != NULL)
    {
        RvSipCommonListDestruct(pTransc->hHeadersListToSetInInitialRequest);
    }
#endif /*#ifndef RV_SIP_PRIMITIVES*/

    /* Remove transaction from Manager databases: */
    RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
    /* 1. Remove transaction from hash */
    if(pTransc->hashElementPtr != NULL)
    {
        HASH_RemoveElement(pTransc->pMgr->hHashTable,
                                       pTransc->hashElementPtr);
    }
    pTransc->hashElementPtr = NULL;
    /* 2. Remove transaction's page */
    RPOOL_FreePage(pTransc->pMgr->hGeneralPool,pTransc->memoryPage);
    /* 3. Remove transaction from list of transactions */
    RLIST_Remove(pTransc->pMgr->hTranasactionsListPool,
                 pTransc->pMgr->hTranasactionsList,
                 (RLIST_ITEM_HANDLE)pTransc);
    RvLogInfo(pLogSrc ,(pLogSrc ,
        "TransactionDestruct - Transaction 0x%p: was destructed", pTransc));
    RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);

    return RV_OK;
}



#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * TransactionLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
RvStatus RVCALLCONV TransactionLockAPI(IN  Transaction *pTransc)
{
    RvStatus          retCode;
    SipTripleLock    *tripleLock;
    TransactionMgr   *pMgr;
    RvInt32           identifier;

    if (pTransc == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {
        RvMutexLock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr);
        pMgr       = pTransc->pMgr;
        tripleLock = pTransc->tripleLock;
        identifier = pTransc->transactionUniqueIdentifier;
        RvMutexUnlock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr);

        if (tripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionLockAPI - Locking Transaction 0x%p: Triple Lock 0x%p", pTransc,
            tripleLock));

        retCode = TransactionLock(pTransc,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }


        /*make sure the triple lock was not changed after just before the locking*/
        if (pTransc->tripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                     "TransactionLockAPI - Locking Transaction 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
                     pTransc, tripleLock));
        TransactionUnLock(pMgr->pLogMgr,&tripleLock->hLock);

    } 

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        TransactionUnLock(pTransc->pMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }
    tripleLock->apiCnt++;

    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock,pTransc->pMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }


    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransactionLockAPI - Locking Transaction 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pTransc, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * TransactionUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
void RVCALLCONV TransactionUnLockAPI(IN  Transaction *pTransc)
{
    SipTripleLock        *tripleLock;
    TransactionMgr        *pMgr;
    RvInt32               lockCnt = 0;

    if (pTransc == NULL)
    {
        return;
    }

    pMgr = pTransc->pMgr;
    tripleLock = pTransc->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock,NULL,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransactionUnLockAPI - UnLocking Transaction 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pTransc, tripleLock, tripleLock->apiCnt,lockCnt));

    if (tripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransactionUnLockAPI - UnLocking Transaction 0x%p: Triple Lock 0x%p apiCnt=%d",
             pTransc, tripleLock, tripleLock->apiCnt));
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,NULL);
    }

    tripleLock->apiCnt--;
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }
    TransactionUnLock(pTransc->pMgr->pLogMgr, &tripleLock->hLock);
}


/************************************************************************************
 * TransactionLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
RvStatus RVCALLCONV TransactionLockMsg(IN  Transaction *pTransc)
{
    RvStatus          retCode      = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *tripleLock;
    TransactionMgr   *pMgr;
    RvInt32           identifier;

    if (pTransc == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    for(;;)
    {
        RvMutexLock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr);
        pMgr = pTransc->pMgr;
        tripleLock = pTransc->tripleLock;
        identifier = pTransc->transactionUniqueIdentifier;
        RvMutexUnlock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr);
        /*note: at this point the object locks can be replaced so after locking we need
          to check if we used the correct locks*/

        if (tripleLock == NULL)
        {
            return RV_ERROR_BADPARAM;
        }
        /* In case that the current thread has already gained the API lock of */
        /* the object, locking again will be useless and will block the stack */
        if (tripleLock->objLockThreadId == currThreadId)
        {
            RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransactionLockMsg - Exiting without locking Transc 0x%p: Triple Lock 0x%p apiCnt=%d already locked",
                   pTransc, tripleLock, tripleLock->apiCnt));

            return RV_OK;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransactionLockMsg - Locking Transaction 0x%p: Triple Lock 0x%p apiCnt=%d",
                  pTransc, tripleLock, tripleLock->apiCnt));

        RvMutexLock(&tripleLock->hProcessLock,pTransc->pMgr->pLogMgr);

        for(;;)
        {
            retCode = TransactionLock(pTransc,pMgr,tripleLock,identifier);
            if (retCode != RV_OK)
            {
                RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
                if (didILockAPI)
                {
                    RvSemaphorePost(&tripleLock->hApiLock,NULL);
                }
                return retCode;
            }
            if (tripleLock->apiCnt == 0)
            {
                if (didILockAPI)
                {
                    RvSemaphorePost(&tripleLock->hApiLock,NULL);
                }
                break;
            }

            TransactionUnLock(pTransc->pMgr->pLogMgr, &tripleLock->hLock);

            retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
            if (retCode != RV_OK)
            {
                RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
                return retCode;
            }

            didILockAPI = RV_TRUE;
        } 

        /*check if the transaction lock pointer was changed. If so release the locks and lock
          the transaction again with the new locks*/
        if (pTransc->tripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionLockMsg - Locking Transaction 0x%p: Tripple lock %p was changed, reentering lockMsg",
            pTransc, tripleLock));
        TransactionUnLock(pTransc->pMgr->pLogMgr, &tripleLock->hLock);
        RvMutexUnlock(&tripleLock->hProcessLock,pTransc->pMgr->pLogMgr);
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
              "TransactionLockMsg - Locking Transaction 0x%p: Triple Lock 0x%p exiting function", pTransc,
               tripleLock));

    return retCode;
}


/************************************************************************************
 * TransactionUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks Transaction according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
void RVCALLCONV TransactionUnLockMsg(IN  Transaction *pTransc)
{
    SipTripleLock   *tripleLock;
    TransactionMgr  *pMgr;
    RvThreadId       currThreadId = RvThreadCurrentId();

    if (pTransc == NULL)
    {
        return;
    }

    pMgr = pTransc->pMgr;
    tripleLock = pTransc->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransactionUnLockMsg - Exiting without unlocking Transc 0x%p: Triple Lock 0x%p wasn't locked",
                  pTransc, tripleLock));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransactionUnLockMsg - UnLocking Transaction 0x%p: Triple Lock 0x%p", pTransc,
             tripleLock));

    TransactionUnLock(pTransc->pMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock,pMgr->pLogMgr);
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
/***************************************************************************
 * TransactionSetMethodStr
 * ------------------------------------------------------------------------
 * General: Copy the method sting on to the transaction page.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            strMethod - The transaction string method.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetMethodStr(
                                    IN  Transaction           *pTransc,
                                    IN  RvChar               *strMethod)
{
    RvStatus rv;
    RvInt32                  offset;
    RPOOL_ITEM_HANDLE         ptr;

    rv = RPOOL_AppendFromExternal(pTransc->pMgr->hGeneralPool,
                                  pTransc->memoryPage,
                                  strMethod,
                                  (RvInt)strlen(strMethod)+1,
                                  RV_TRUE, &offset, &ptr);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStr - Transaction 0x%p: Failed to set string method",
                  pTransc));
        return rv;
    }
    pTransc->strMethod = RPOOL_GetPtr(pTransc->pMgr->hGeneralPool,
                                      pTransc->memoryPage, offset);
    return RV_OK;

}


/***************************************************************************
 * TransactionSetMethod
 * ------------------------------------------------------------------------
 * General: Called when a new server transaction is created. Sets the
 *          transaction's method type according to the type that was received
 *          in the message.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction to set the method to.
 *          eMethodType - The transaction's method.
 ***************************************************************************/
void RVCALLCONV TransactionSetMethod(IN RvSipTranscHandle hTransc,
                                     IN RvSipMethodType eMethodType)
{
    Transaction        *pTransc;
    pTransc = (Transaction *)hTransc;
    switch (eMethodType)
    {
    case RVSIP_METHOD_INVITE:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_INVITE;
        break;
    case RVSIP_METHOD_BYE:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_BYE;
        break;
    case RVSIP_METHOD_OTHER:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_OTHER;
        break;
    case RVSIP_METHOD_REGISTER:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_REGISTER;
        break;
		
#ifdef RV_SIP_SUBS_ON
    case RVSIP_METHOD_REFER:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_REFER;
        break;
    case RVSIP_METHOD_SUBSCRIBE:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_SUBSCRIBE;
        break;
    case RVSIP_METHOD_NOTIFY:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_NOTIFY;
        break;
#endif /* #ifdef RV_SIP_SUBS_ON */ 
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_METHOD_PRACK:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_PRACK;
        break;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    case RVSIP_METHOD_CANCEL:
        pTransc->eMethod = SIP_TRANSACTION_METHOD_CANCEL;
        break;
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethod - Undefined method received in message"));
    }
}


/***************************************************************************
 * TransactionSetMethodStrFromMsg
 * ------------------------------------------------------------------------
 * General: Called when a new server transaction is created. Sets the
 *          transaction's method string according to the string that was
 *          received in the message.
 * Return Value: RV_OK - The method was successfully set.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate method.
 *               RV_ERROR_NULLPTR - Bad pointer was received as parameter.
 *               RV_ERROR_BADPARAM - The message does not contain a
 *                                     method string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction to set the method to.
 *          hMsg - The message to take the method from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetMethodStrFromMsg(IN RvSipTranscHandle hTransc,
                                             IN RvSipMsgHandle  hMsg)
{
    Transaction          *pTransc;
    RvUint                size;
    RvSipCSeqHeaderHandle hCSeqHeader;
    RvStatus              retStatus;
    RvInt32               allocated;

    pTransc = (Transaction *)hTransc;

    /* if the message is a request - take the method from start-line.
       if respond - take it from the cseq header */

    if(RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST)
    {
        size = RvSipMsgGetStringLength(hMsg, RVSIP_MSG_REQUEST_METHOD);
        if(0 == size)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: got illegal method string length %d (request)",
                  pTransc,size));
            return RV_ERROR_BADPARAM;
        }
        /* Append memory for the Method */
        retStatus = RPOOL_Append((pTransc->pMgr)->hGeneralPool,
                                 pTransc->memoryPage,
                                 size+1, RV_TRUE, &allocated);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: Failed to append %d bytes (request)",
                  pTransc,size+1));
            return retStatus;
        }
        /* Get the pointer to the memory allocated */
        pTransc->strMethod = RPOOL_GetPtr(
                                            (pTransc->pMgr)->hGeneralPool,
                                            pTransc->memoryPage,
                                            allocated);
        if (NULL == pTransc->strMethod)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: Failed in RPOOL_GetPtr (request)",
                  pTransc));
            return RV_ERROR_OUTOFRESOURCES;
        }
        /* Copy the method to the new memory allocated for it */
        retStatus = RvSipMsgGetStrRequestMethod(hMsg, pTransc->strMethod,
                                                size+1, &size);
    }
    else /* respond */
    {
        /* Get CSeq header of the message */
        hCSeqHeader = RvSipMsgGetCSeqHeader(hMsg);
        if (NULL == hCSeqHeader)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: no cseq header in response msg",
                  pTransc));
            return RV_ERROR_BADPARAM;
        }
        /* Get the size of the method string */
        size = RvSipCSeqHeaderGetStringLength(hCSeqHeader, RVSIP_CSEQ_METHOD);
        if (0 == size)
        {
            RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: got illegal method string length %d (response)",
                  pTransc,size));
            return RV_ERROR_BADPARAM;
        }
        /* Append memory for the Method */
        retStatus = RPOOL_Append((pTransc->pMgr)->hGeneralPool,
                                 pTransc->memoryPage,
                                 size+1, RV_TRUE, &allocated);
        if (RV_OK != retStatus)
        {
             RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: Failed to append %d bytes (response)",
                  pTransc,size+1));
            return retStatus;
        }
        /* Get the pointer to the memory allocated */
        pTransc->strMethod = RPOOL_GetPtr(
                                            (pTransc->pMgr)->hGeneralPool,
                                            pTransc->memoryPage,
                                            allocated);
        if (NULL == pTransc->strMethod)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionSetMethodStrFromMsg - Transaction 0x%p: Failed in RPOOL_GetPtr (response)",
                  pTransc));
            return RV_ERROR_OUTOFRESOURCES;
        }
        /* Copy the method to the new memory allocated for it */
        retStatus = RvSipCSeqHeaderGetStrMethodType(
                                            hCSeqHeader, pTransc->strMethod,
                                            size+1, &size);
    }

    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
          "TransactionSetMethodStrFromMsg - Transaction 0x%p: Failed to get nethod str (rv=%d)",
          pTransc,retStatus));
        return retStatus;
    }
    return RV_OK;
}
/***************************************************************************
 * TransactionUpdatePartyHeaders
 * ------------------------------------------------------------------------
 * General: A user agent should strip invalid parameters from the party headers.
 *          In this case the party headers of a response can differ from the
 *          ones in the transaction. In this case we need to update the transaction.
 *          This can happen only for initial requests of a dialog: e.g. - one
 *          of the tags is missing. If this is the case we
 *          Delete the existing Party headers, and create a new ones similar to
 *            the ones received in the message.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_ERROR_BADPARAM - The message received has no To header
 *                                       in it.
 *                 RV_OutOfResource - Couldn't allocate memory for the new To
 *                                    header in the transaction object.
 *                 RV_OK - The To header was updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to set To header.
 *           pMsg - The message from which the new To header is copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdatePartyHeaders(
                                           IN Transaction   *pTransc,
                                           IN RvSipMsgHandle hMsg,
                                           IN RvUint16 responseCode)
{
    RvInt32 toTag   = UNDEFINED;
    RvInt32 fromTag = UNDEFINED;
    RvStatus rv;

    if(pTransc->hToHeader != NULL)
    {
        toTag = SipPartyHeaderGetTag(pTransc->hToHeader);
    }
    if(pTransc->hFromHeader != NULL)
    {
        fromTag = SipPartyHeaderGetTag(pTransc->hFromHeader);
    }
    /*if one of the tags is undefined do the update*/
    if (toTag == UNDEFINED )
    {
        rv = UpdateToHeader(pTransc, hMsg, responseCode);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionUpdatePartyHeaders - Transaction 0x%p: - Failed to update To header",
                pTransc));
            return rv;
        }
    }
    else  /* already has a To-tag from 1xx */
    {
        /* always update the to header of 200 response, because it might have changed,
           so further requests will have the same to header */
        if(responseCode >= 200 /*final response*/
            && pTransc->bInitialRequest == RV_TRUE) 
        {
            RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionUpdatePartyHeaders - Transaction 0x%p: - update To header",
                pTransc));
            rv = UpdateToHeader(pTransc, hMsg, responseCode);
            if (RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "TransactionUpdatePartyHeaders - Transaction 0x%p: - Failed to update To header",
                    pTransc));
                return rv;
            }
        }
    }


    if(fromTag == UNDEFINED)
    {
        rv = UpdateFromHeader(pTransc, hMsg);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionUpdatePartyHeaders - Transaction 0x%p: - Failed to update From header",
                pTransc));
            return rv;
        }
    }


    return RV_OK;
}



/***************************************************************************
 * TransactionCorrectToHeaderUrl
 * ------------------------------------------------------------------------
 * General: The SIP-URL of the To header MUST NOT contain the "transport-param",
 *          "maddr-param", "ttl-param", or "headers" elements. A server that
 *          receives a SIP-URL with these elements removes them before further
 *          processing. This function removes those parameters.
 * Return Value: RV_ERROR_NULLPTR - hMsg is a NULL pointers.
 *                 RV_ERROR_BADPARAM - The To header of hMsg or the Url
 *                                     of the To header are NULL.
 *                 RV_OK - The parameters were successfully removed.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hMsg - The message to correct the To Sip-Url in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCorrectToHeaderUrl(IN RvSipMsgHandle hMsg)
{
    RvSipPartyHeaderHandle hToHeader;
    RvSipAddressHandle     sipUrl;

    hToHeader = RvSipMsgGetToHeader(hMsg);
    if (NULL == hToHeader)
    {
        return RV_ERROR_BADPARAM;
    }
    /* Get the Sip-Url of the transaction's To header */
    sipUrl = RvSipPartyHeaderGetAddrSpec(hToHeader);
    if (NULL == sipUrl)
    {
        return RV_ERROR_BADPARAM;
    }
    /*correct the address if it is a url address*/
    if(RvSipAddrGetAddrType(sipUrl) == RVSIP_ADDRTYPE_URL)
    {
        /* Delete the Maddr parameter of the Sip-Url */
        RvSipAddrUrlSetMaddrParam(sipUrl, NULL);
        /* Delete the Transport parameter of the Sip-Url */
        RvSipAddrUrlSetTransport(sipUrl, RVSIP_TRANSPORT_UNDEFINED, NULL);
        /* Delete the Headers parameter of the Sip-Url */
        RvSipAddrUrlSetHeaders(sipUrl, NULL);
        /* Delete the Ttl parameter of the Sip-Url */
        RvSipAddrUrlSetTtlNum(sipUrl, UNDEFINED);
    }
    return RV_OK;

}


/***************************************************************************
 * TransactionAddTag
 * ------------------------------------------------------------------------
 * General: Generate and add a Tag to the transaction's To header.
 * Return Value: RV_ERROR_NULLPTR - pTransc is a NULL pointers.
 *               RV_ERROR_BADPARAM - The To header of pTransc is a NULL
 *                                     pointer.
 *                 RV_OutOfResource - Couldn't allocate memory for the new Tag
 *                                    string, in the transaction object.
 *                 RV_OK - The To tag was created and set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction in which new Tag will be set.
 *        bIsTo - RV_TRUE if the tag should be added to the To header.
 *                RV_FALSE if it should be added to the From header.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddTag(IN Transaction *pTransc,
                                         IN RvBool      bIsTo)
{
    RvSipPartyHeaderHandle *hAddTagHeader;

    if (RV_TRUE == bIsTo)
    {
        hAddTagHeader = &(pTransc->hToHeader);
    }
    else
    {
        hAddTagHeader = &(pTransc->hFromHeader);
    }
    if (NULL == *hAddTagHeader)
    {
        return RV_ERROR_BADPARAM;
    }
    if(pTransc->responseCode == 100)
    {
        if ((RV_TRUE == bIsTo) &&
            (UNDEFINED == SipPartyHeaderGetTag(*hAddTagHeader)))
        {
            /* There didn't exists tag in the To. Mark it in the transaction. */
            pTransc->bTagInTo = RV_FALSE;
        }
        else
        {
            /* There already exists tag in the To. Mark it in the transaction. */
            pTransc->bTagInTo = RV_TRUE;
        }
        /* Do not add To tag to 100 response in case of 100 */
        return RV_OK;
    }


    if (UNDEFINED == SipPartyHeaderGetTag(*hAddTagHeader))
    {
        /* The To header doesn't contain a Tag */
        RvChar   strTag[SIP_COMMON_ID_SIZE];
        RvStatus retStatus;

        retStatus = TransactionGenerateTag(pTransc->pMgr, pTransc, (RvChar*)&strTag);

        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        /* Set the new To-Tag to the transaction's To header */
        retStatus = TransactionSetTag(pTransc, strTag, bIsTo);

        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        if (RV_TRUE == bIsTo)
        {
            /* There didn't exists tag in the To. Mark it in the transaction. */
            pTransc->bTagInTo = RV_FALSE;
        }
    }
    else
    {
        if (RV_TRUE == bIsTo)
        {
            /* There already exists tag in the To. Mark it in the transaction. */
            pTransc->bTagInTo = RV_TRUE;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionGenerateTag
 * ------------------------------------------------------------------------
 * General: Generate and add a Tag to the transaction's To header.
 * Return Value: RV_ERROR_NULLPTR - pTransc is a NULL pointers.
 *               RV_ERROR_BADPARAM - The To header of pTransc is a NULL
 *                                     pointer.
 *                 RV_OutOfResource - Couldn't allocate memory for the new Tag
 *                                    string, in the transaction object.
 *                 RV_OK - The To tag was created and set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction in which new Tag will be set.
 *        bIsTo - RV_TRUE if the tag should be added to the To header.
 *                RV_FALSE if it should be added to the From header.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGenerateTag(IN TransactionMgr         *pMgr,
                                           IN Transaction            *pTransc,
                                           OUT RvChar*               strTag)
{    
    RvStatus retStatus;
    void*    pObj = (NULL!=pTransc) ? (void*)pTransc : (void*)pMgr;

    retStatus = SipTransactionGenerateIDStr(pMgr->hTransport,
                                    pMgr->seed,
                                    (void*)pObj,
                                    pMgr->pLogSrc,
                                    strTag);

    if (RV_OK != retStatus)
    {
        return retStatus;
    }
            
    return RV_OK;
}

/***************************************************************************
 * TransactionCopyToTag
 * ------------------------------------------------------------------------
 * General: Copy the To Tag from the message to the To header of the
 *          transaction. The tag is copied only if there is no tag found
 *          in the transaction object.
 * Return Value: RV_OutOfResource - Couldn't allocate memory for the new To
 *                                  header in the transaction object.
 *               RV_OK - The To header was updated successfully.
 *               RV_ERROR_UNKNOWN - The message is illegal.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to update To header.
 *           pMsg - The message from which the To tag is copied.
 *           bIgnoreTranscTag - true: copy to-tag even if transaction to-tag exists.
 *                              false: do not copy if transaction to-tag exists.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyToTag(
                                           IN Transaction   *pTransc,
                                           IN RvSipMsgHandle hMsg,
                                           IN RvBool         bIgnoreTranscTag)
{
    RvSipPartyHeaderHandle hToHeader;
    RPOOL_Ptr              strToTag;
    RvStatus              retStatus;
    RvInt32               transactionTag;

    if(bIgnoreTranscTag == RV_FALSE)
    {
        transactionTag = SipPartyHeaderGetTag(pTransc->hToHeader);
        if (UNDEFINED != transactionTag)
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionCopyToTag - Transaction 0x%p: did not copy tag - transaction already has a tag...",
                   pTransc));
            return RV_OK;
        }
    }
    hToHeader = RvSipMsgGetToHeader(hMsg);
    if (NULL == hToHeader)
    {
        return RV_ERROR_UNKNOWN;
    }
    strToTag.offset = SipPartyHeaderGetTag(hToHeader);
    if (UNDEFINED == strToTag.offset)
    {
        /* No Tag was received in the response */
        return RV_OK;
    }
    strToTag.hPage = SipPartyHeaderGetPage(hToHeader);
    strToTag.hPool = SipPartyHeaderGetPool(hToHeader);
    TransactionLockTranscMgr(pTransc);
    retStatus = SipPartyHeaderSetTag(pTransc->hToHeader, NULL,
                                     strToTag.hPool, strToTag.hPage,
                                     strToTag.offset);
    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionCopyToTag - Transaction 0x%p: Done",
               pTransc));

    TransactionUnlockTranscMgr(pTransc);
    return retStatus;
}
/***************************************************************************
 * TransactionRand
 * ------------------------------------------------------------------------
 * General: returns a random number smaller then the size of RvInt32.
 * Return Value: random RvInt32.
 * ------------------------------------------------------------------------
 * Arguments: (-)
 ***************************************************************************/
RvInt32 RVCALLCONV TransactionRand(IN Transaction         *pTransc)
{
    RvUint32 randomNum;



    RvRandomGeneratorGetInRange(pTransc->pMgr->seed,RV_INT32_MAX,(RvRandom*)&randomNum);

    /* Cseq must be positive */
    if (0 == randomNum)
    {
        randomNum = 1;
    }

    return (RvInt32)randomNum;
}

/***************************************************************************
 * TransactionUpDateViaList
 * ------------------------------------------------------------------------
 * General: Copies the list of Via headers that were received in the
 *            hMsg message, which is a request message. These headers
 *            will be set, by their order, to the responses sent by this
 *            transaction. This function will copy the Via headers and save
 *            them within the transaction object.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate the place needed for
 *                                    the Via headers list in the transaction
 *                                    object.
 *                 RV_OK - The Via headers list was created successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to which the Via header list belongs
 *                         to.
 *          pMsg - The message from which the Via headers should be copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpDateViaList(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)
{
    void                     *pHeader;
    RvSipHeaderListElemHandle hPos;
    RvSipViaHeaderHandle     *phViaHeader;
    RvStatus                 retStatus;
    RvUint32                 safeCounter;
    RvUint32                 maxLoops;
    RvBool                   b_topVia = RV_TRUE;

    pTransc->pViaHeadersList =
                        RLIST_RPoolListConstruct(
                            pTransc->pMgr->hGeneralPool,
                            pTransc->memoryPage, sizeof(void *),
                            pTransc->pMgr->pLogSrc);
    if (NULL == pTransc->pViaHeadersList)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    /* Save all via headers, except for the first one, in the transaction */
    phViaHeader = NULL;
    hPos        = NULL;
    safeCounter = 0;
    maxLoops    = 1000;
    retStatus = RV_OK;
    /* Get the rest of the Via's from the message, and insert them to the
       transaction's list */
    pHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA, RVSIP_FIRST_HEADER,
                                         RVSIP_MSG_HEADERS_OPTION_ALL, &hPos);

    while (NULL != pHeader)
    {
        retStatus = RLIST_InsertTail(
                            NULL,
                            pTransc->pViaHeadersList,
                            (RLIST_ITEM_HANDLE *)&phViaHeader);
        if (RV_OK != retStatus)
        {
            break;
        }
        retStatus = RvSipViaHeaderConstruct((pTransc->pMgr)->hMsgMgr,
                                            (pTransc->pMgr)->hGeneralPool,
                                            pTransc->memoryPage,
                                            phViaHeader);
        if (RV_OK != retStatus)
        {
            break;
        }
        retStatus = RvSipViaHeaderCopy(*phViaHeader,
                                       (RvSipViaHeaderHandle)pHeader);
        if (RV_OK != retStatus)
        {
            break;
        }

        /*the top via is kept separately for cancel transaction mutch*/
        if(b_topVia == RV_TRUE)
        {
            TransactionLockTranscMgr(pTransc);
            pTransc->hTopViaHeader = *phViaHeader;
            TransactionUnlockTranscMgr(pTransc);
            b_topVia = RV_FALSE;
        }

        pHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA, RVSIP_NEXT_HEADER,
                                         RVSIP_MSG_HEADERS_OPTION_ALL, &hPos);
        if (safeCounter > maxLoops)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                      "TransactionUpDateViaList - Error in loop - number of rounds is larger than number of headers"));
            break;
        }
        safeCounter++;
    }
    if (RV_OK == retStatus)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionUpDateViaList - Transaction 0x%p: The list of Via headers was successfully updated.",
                  pTransc));
    }
    return retStatus;
}

/***************************************************************************
 * TransactionAddViaToMsg
 * ------------------------------------------------------------------------
 * General: Adds the local IP address as a Via header to the received message.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OutOfResource - No memory for the allocations required.
 *               RV_ERROR_UNKNOWN - An error occurred.
 *                 RV_OK - The Via header was successfully created and
 *                            inserted.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to which the Via header list belongs
 *                         to.
 *          pMsg - The message from which the Via headers should be copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddViaToMsg(
                                      IN Transaction   *pTransc,
                                      IN RvSipMsgHandle hMsg)
{

   RvStatus              rv          = RV_OK;

    /* if the transaction does not have a top via at all or if this is ack on 2xx
      where we need a new top via*/
   if(pTransc->hTopViaHeader == NULL)
   {
      rv = SipTransmitterAddNewTopViaToMsg(pTransc->hTrx,hMsg, NULL,RV_FALSE);
      if (RV_OK != rv)
      {
          RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                     "TransactionAddViaToMsg - Transaction 0x%p: Failed to add new to via header (rv=%d)",
                      pTransc, rv));
          return rv;
      }
      return RV_OK;
   }

   /*ack on 2xx*/
   if((pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD) &&
      ((pTransc->responseCode >= 200) && (pTransc->responseCode < 300)))
    {
        rv = SipTransmitterAddNewTopViaToMsg(pTransc->hTrx,hMsg, NULL,RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                       "TransactionAddViaToMsg - Transaction 0x%p: Failed to add new to via header (rv=%d)",
                       pTransc, rv));
            return rv;
        }
        return RV_OK;
    }

    /* copy the via header we have in the transaction. This is good for:
    1. cancel,
    2. responses
    3. ACK on non 2xx
   */
   rv = SipTransmitterAddNewTopViaToMsg(pTransc->hTrx,hMsg, pTransc->hTopViaHeader,RV_FALSE);
   if (RV_OK != rv)
   {
       RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAddViaToMsg - Transaction 0x%p: Failed to add new to via header (rv=%d)",
                   pTransc, rv));
       return rv;
   }

    return RV_OK;
}

/***************************************************************************
 * TransactionRemoveTopViaFromMsg
 * ------------------------------------------------------------------------
 * General: Adds the local IP address as a Via header to the received message.
 * Return Value: RV_ERROR_NOT_FOUND - no via header was found in the message.
 *                 RV_OK - The Via header was successfully removed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr - The transaction manager for log printing to.
 *          hMsg - The message from which the Via headers should be removed.
 ***************************************************************************/
RvStatus TransactionRemoveTopViaFromMsg(IN TransactionMgr *pMgr,
                                         IN RvSipMsgHandle hMsg)
{
    RvStatus rv = RV_OK;
    RvSipHeaderListElemHandle   hListElem;
    RvSipViaHeaderHandle hVia;
    hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(
                                        hMsg,RVSIP_HEADERTYPE_VIA,
                                        RVSIP_FIRST_HEADER,&hListElem);
    if(hVia == NULL || hListElem == NULL)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionRemoveTopViaFromMsg - Failed: No top via header in message 0x%p", hMsg));

        return (RV_ERROR_NOT_FOUND);

    }
    rv = RvSipMsgRemoveHeaderAt(hMsg,hListElem);
    if(rv != RV_OK)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionRemoveTopViaFromMsg - Failed to remove top Via from message 0x%p (rv=%d)",
             hMsg, rv));
        return(rv);
    }
    else
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionRemoveTopViaFromMsg - Successfully removed Top Via from message 0x%p",hMsg));
        return(RV_OK);
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif

}
/***************************************************************************
 * TransactionGetMsgMethodFromTranscMethod
 * ------------------------------------------------------------------------
 * General: Returns the equivalent message method from the transaction method.
 * Return Value: the equivalent message method from the transaction method.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 ***************************************************************************/
RvSipMethodType TransactionGetMsgMethodFromTranscMethod(
                                    IN  Transaction    *pTransc)
{

    if(pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD)
    {
        return RVSIP_METHOD_ACK;
    }
    else if (pTransc->eState == RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE &&
             503 == pTransc->responseCode)
    {
        return RVSIP_METHOD_ACK;
    }
    switch (pTransc->eMethod)
    {
    case SIP_TRANSACTION_METHOD_INVITE:
        return RVSIP_METHOD_INVITE;
    case SIP_TRANSACTION_METHOD_BYE:
        return RVSIP_METHOD_BYE;
    case SIP_TRANSACTION_METHOD_REGISTER:
        return RVSIP_METHOD_REGISTER;
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER:
        return RVSIP_METHOD_REFER;
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
        return RVSIP_METHOD_SUBSCRIBE;
    case SIP_TRANSACTION_METHOD_NOTIFY:
        return RVSIP_METHOD_NOTIFY;
#endif /* #ifdef RV_SIP_SUBS_ON */
#ifndef RV_SIP_PRIMITIVES
    case SIP_TRANSACTION_METHOD_PRACK:
        return RVSIP_METHOD_PRACK;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    case SIP_TRANSACTION_METHOD_CANCEL:
        return RVSIP_METHOD_CANCEL;
    default:
        return RVSIP_METHOD_OTHER;
    }
}




/***************************************************************************
 * TransactionCreateRequestMessage
 * ------------------------------------------------------------------------
 * General: Create a request message. The request message will contain the
 *            To, From, CSeq, Call-Id values associated with the transaction
 *            and the Method and Request-Uri that were received as parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 *                 RV_OK - The Request message was successfully created.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction for which the request message is
 *                   created.
 *         eMethod - The request method.
 *         pRequestUri - The Request-Uri of the request message.
 * Output: phMsg - The new message created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreateRequestMessage(
                                        IN  Transaction       *pTransc,
                                        IN  RvSipAddressHandle hRequestUri,
                                        INOUT RvSipMsgHandle   hMsg)
{
    RvStatus              retStatus;
    RvBool                bAddMaxForwards = RV_TRUE;

    RvSipMethodType    eMethod = TransactionGetMsgMethodFromTranscMethod(pTransc);
    if(pTransc->bIsProxy == RV_TRUE)
    {
        bAddMaxForwards = RV_FALSE;
    }
    retStatus = TransactionFillBasicRequestMessage(hRequestUri, pTransc->hFromHeader,
                            pTransc->hToHeader, &(pTransc->strCallId), eMethod,
                            pTransc->strMethod, &pTransc->cseq, bAddMaxForwards, hMsg);
    if (RV_OK != retStatus)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "TransactionCreateRequestMessage - Transaction 0x%p: Failed to build basic msg",
            pTransc));
        return retStatus;
    }
    /* Insert Via header containing the local IP address and port */
    retStatus = TransactionAddViaToMsg(pTransc, hMsg);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    
#ifndef RV_SIP_PRIMITIVES
    /*if 100 rel is supported add it to the message*/
    if(eMethod != RVSIP_METHOD_ACK &&
       pTransc->pMgr->addSupportedListToMsg == RV_TRUE)
    {
        retStatus = AddSupportedListToMsg(pTransc,hMsg);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                      "TransactionCreateRequestMessage - Transaction 0x%p: Failed to add supported list to msg",
                      pTransc));
            return retStatus;
        }
    }
    if(eMethod == RVSIP_METHOD_CANCEL)
    {
       if(pTransc->bTagInTo == RV_FALSE)
       {
            RvSipPartyHeaderHandle hTo;
            hTo = RvSipMsgGetToHeader(pTransc->hOutboundMsg);
            RvSipPartyHeaderSetTag(hTo,NULL);
       }
    }
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef RV_SIP_IMS_ON
	/* Add security information to outgoing messages */
	retStatus = TransactionSecAgreeMsgToSend(pTransc, hMsg);
	if (RV_OK != retStatus)
	{
		RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
					"TransactionCreateRequestMessage - Transaction 0x%p - Failed to add security information to message - message will not be sent"
					,pTransc));
        return retStatus;
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionCreateRequestMessage - Transaction 0x%p: A request message was successfully created in a transaction",
              pTransc));
    return RV_OK;
}

/***************************************************************************
 * TransactionFillBasicRequestMessage
 * ------------------------------------------------------------------------
 * General: Fill a request message with basic parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 *                 RV_OK - The Request message was successfully created.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hRequestUri - The request Uri
 *         hFrom       - The From header
 *         hTo         - The To header.
 *         pStrCallId  - Pointer to the call-Id rpool string
 *         eMethod     - The request method.
 *         strMethod   - An other method string.
 *         pCseq       - Pointer to the cseq value.
 *         bAddMaxForwards - Add Max-Forward header to the message or not.
 * Output: phMsg      - The filled message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionFillBasicRequestMessage(
                                        IN  RvSipAddressHandle hRequestUri,
                                        IN  RvSipPartyHeaderHandle hFrom,
                                        IN  RvSipPartyHeaderHandle hTo,
                                        IN  RPOOL_Ptr         *pStrCallId,
                                        IN  RvSipMethodType    eMethod,
                                        IN  RvChar            *strMethod,
                                        IN  SipCSeq           *pCseq,
                                        IN  RvBool             bAddMaxForwards,
                                        INOUT RvSipMsgHandle   hMsg)
{
    RvSipCSeqHeaderHandle hCseqHeader;
    RvStatus              retStatus;
	RvInt32				  cseqStep;

    /*Set Method*/
    retStatus = RvSipMsgSetMethodInRequestLine(hMsg, eMethod, strMethod);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    /*Set Request-Uri*/
    retStatus = RvSipMsgSetRequestUri(hMsg, hRequestUri);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    /*create CSeq header*/
    retStatus = RvSipCSeqHeaderConstructInMsg(hMsg, &hCseqHeader);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

	retStatus = SipCommonCSeqGetStep(pCseq,&cseqStep); 
	if (RV_OK != retStatus)
	{
		return retStatus;
	}
    retStatus = RvSipCSeqHeaderSetStep(hCseqHeader, cseqStep);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipCSeqHeaderSetMethodType(hCseqHeader, eMethod, strMethod);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
   
    /* Insert To, From, CSeq, Call-ID headers to the message */
    retStatus = RvSipMsgSetFromHeader(hMsg, hFrom);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipMsgSetToHeader(hMsg, hTo);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = SipMsgSetCallIdHeader(hMsg, NULL,
                                      pStrCallId->hPool,
                                      pStrCallId->hPage,
                                      pStrCallId->offset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    if(bAddMaxForwards == RV_TRUE)
    {
        retStatus = AddMaxForwardsToMsg(hMsg);
        if (RV_OK != retStatus)
        {
             return retStatus;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionAddViaToResponseMessage
 * ------------------------------------------------------------------------
 * General: Inserts to the message the list of Via headers associated with
 *          this transaction (a server transaction).
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction for which the response message is
 *                          created.
 *          hMsg - The message to insert the Via headers to.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddViaToResponseMessage(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)
{
    RvStatus           retStatus;
    RvUint32            safeCounter;
    RvUint32            maxLoops;

    retStatus = RV_OK;
    if (NULL != pTransc->pViaHeadersList)
    {
        /* The Vial list is not empty */
        RvSipHeaderListElemHandle hPos;
        RLIST_ITEM_HANDLE hElement;
        void *hMsgListElem;;

        /* Go over the list of Via headers in the transaction, and insert each
           of this headers to the response message. The list is inserted
           according to its order in the transaction (the order it was received
           in) */
        hPos = NULL;
        safeCounter = 0;
        maxLoops = 10000;
        RLIST_GetHead(NULL, pTransc->pViaHeadersList, &hElement);
        while (NULL != hElement)
        {
            /* set via headers to the new message */
            retStatus = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER,
                                           (void *)(*((RvSipViaHeaderHandle *)hElement)),
                                           RVSIP_HEADERTYPE_VIA, &hPos,
                                           (void **)&hMsgListElem);
            if (RV_OK != retStatus)
            {
                break;
            }
            RLIST_GetNext(NULL, pTransc->pViaHeadersList, hElement, &hElement);
            if (safeCounter > maxLoops)
            {
                RvLogExcep(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                          "TransactionAddViaToResponseMessage - Error in loop - number of rounds is larger than number of Via headers"));
                break;
            }
            safeCounter++;
        }
    }
    return retStatus;
}



/***************************************************************************
 * TransactionCreateResponseMessage
 * ------------------------------------------------------------------------
 * General: Create a response message. The response message will contain the
 *            To, From, CSeq, Call-Id values associated with the transaction.
 *            The list of Via headers associated with the transaction (was
 *            received in the request message) will also be inserted to the
 *            message. The Method received as a parameters will be used in the
 *            CSeq header. The Response-Code and Reason-Phrase received as
 *            parameters will be the used in this response. The Reason-Prase is
 *            optional - if the parameter is NULL no Response-code will be set
 *            to the response.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction for which the response message is
 *                          created.
 *          responseCode - The Response-Code of this message.
 *          strReasonPhrase - The Reason-Phrase of this message (optional).
 *        b_isReliable    - Is this a reliable provisional response.
 *          pMsg - The new message created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreateResponseMessage(
                                         IN  Transaction     *pTransc,
                                         IN  RvUint16        responseCode,
                                         IN  RvChar         *strReasonPhrase,
                                         IN RvBool           b_isReliable,
                                         INOUT RvSipMsgHandle hMsg)
{
    RvSipCSeqHeaderHandle hCseqHeader;
    RvStatus              retStatus;
	RvInt32				  cseqStep;

    RvSipMethodType eMethod = TransactionGetMsgMethodFromTranscMethod(pTransc);
    /*Set Response-Code and Reason-Phrase*/
    if (NULL != strReasonPhrase)
    {
        retStatus = RvSipMsgSetStatusCode(hMsg, responseCode, RV_FALSE);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        retStatus = RvSipMsgSetReasonPhrase(hMsg, strReasonPhrase);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
    }
    else
    {
        if(SipMsgGetReasonPhrase(hMsg) != UNDEFINED)
        {
            retStatus = RvSipMsgSetStatusCode(hMsg, responseCode, RV_FALSE);
            if (RV_OK != retStatus)
            {
                 return retStatus;
            }
        }
        else
        {
            retStatus = RvSipMsgSetStatusCode(hMsg, responseCode, RV_TRUE);
            if (RV_OK != retStatus)
            {
                 return retStatus;
            }
        }
    }
    /*create CSeq header*/
    retStatus = RvSipCSeqHeaderConstructInMsg(hMsg, &hCseqHeader);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
	retStatus = SipCommonCSeqGetStep(&pTransc->cseq,&cseqStep); 
	if (RV_OK != retStatus)
	{
		return retStatus;
	}
    retStatus = RvSipCSeqHeaderSetStep(hCseqHeader, cseqStep);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipCSeqHeaderSetMethodType(hCseqHeader, eMethod,
                                             pTransc->strMethod);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    /*insert To, From, CSeq, Call-ID headers to the message*/
    retStatus = RvSipMsgSetFromHeader(hMsg, pTransc->hFromHeader);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipMsgSetToHeader(hMsg, pTransc->hToHeader);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = SipMsgSetCallIdHeader(hMsg, NULL,
                                      pTransc->strCallId.hPool,
                                      pTransc->strCallId.hPage,
                                      pTransc->strCallId.offset);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    /*Add the trasnaction's list of Via headers to the message*/
    retStatus = TransactionAddViaToResponseMessage(pTransc, hMsg);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    /*add time stamp to 100 responses*/
    if(responseCode == 100)
    {
        retStatus = TransactionCopyTimestampToMsg(pTransc,hMsg);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
    }

#ifndef RV_SIP_PRIMITIVES
    if(pTransc->pMgr->addSupportedListToMsg == RV_TRUE)
    {
        retStatus = AddSupportedListToMsg(pTransc,hMsg);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionCreateResponseMessage - Transaction 0x%p: Failed to add supported list to msg",
                pTransc));

            return retStatus;
        }
    }

    if(pTransc->unsupportedList != UNDEFINED && responseCode == 420)
    {
        retStatus = AddUnsupportedListToMsg(pTransc,hMsg);
        if(retStatus != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                      "TransactionCreateResponseMessage - Transaction 0x%p: Failed to add unsupported list to msg",
                      pTransc));
            return retStatus;
        }
    }

    /*if this is a reliable provisional response, add the Require and the RSeq to
      the message*/
    if(b_isReliable)
    {
        retStatus = AddReliableInfoToMsg(pTransc,hMsg);
        if (RV_OK != retStatus)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                      "TransactionCreateResponseMessage - Transaction 0x%p: Failed to add reliable info to msg",
                      pTransc));

            return retStatus;
        }
    }
#else
    RV_UNUSED_ARG(b_isReliable);
#endif /* #ifndef RV_SIP_PRIMITIVES  */

#ifdef RV_SIP_IMS_ON
	/* Add security information to outgoing messages */
	retStatus = TransactionSecAgreeMsgToSend(pTransc, hMsg);
	if (RV_OK != retStatus)
	{
		RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
					"TransactionCreateResponseMessage - Transaction 0x%p - Failed to add security information to message - message will not be sent"
					,pTransc));
        return retStatus;
	}
#endif /* #ifdef RV_SIP_IMS_ON */

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "TransactionCreateResponseMessage - Transaction 0x%p: A response message was successfully created in a transaction",
              pTransc));
    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

/***************************************************************************
* TransactionClone
* ------------------------------------------------------------------------
* General: Creates new transaction and copies there major transaction
*          parameters.
*          It is important to note: we do not copy the via, as new via
*          will have to generated. the reason for that is that new dest
*          protocol or local address might be in use for this transaction
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   pOrigTrans        - The source transaction structure pointer.
*          owner            - pointer to the new transaction owner info.
*          bRemoveToTag     - Remove the to tag from transaction or not.
*                             (This parameter should be TRUE for a first
*                              INVITE transaction, or for a non-callLeg
*                              transaction.
* Output:  phCloneTrans        - Pointer to the new transaction handler.
***************************************************************************/
RvStatus RVCALLCONV TransactionClone(IN  Transaction            *pOrigTrans,
                                     IN  RvSipTranscOwnerHandle  hOwner,
                                     IN  RvBool                  bRemoveToTag,
                                     OUT RvSipTranscHandle      *phCloneTrans)
{
    RvStatus           retCode      = RV_OK;
    Transaction       *pCloneTrans = NULL;
    SipTransactionKey  key;

    if(pOrigTrans == NULL)
        return RV_ERROR_UNKNOWN;
    
    RvLogDebug(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
        "TransactionClone - starting to clone transaction 0x%p",pOrigTrans));

    TransactionGetKeyFromTransc(pOrigTrans, &key);
    key.bIgnoreLocalAddr = RV_TRUE; /*the local address will be copied with the transmitter*/

    retCode = SipTransactionCreate((RvSipTranscMgrHandle)pOrigTrans->pMgr,
                                   &key, (void*)hOwner, pOrigTrans->eOwnerType,
                                   pOrigTrans->tripleLock, phCloneTrans);

    if(retCode != RV_OK)
    {
        return (retCode);
    }

    pCloneTrans = (Transaction *)*phCloneTrans;

    pCloneTrans->bIsUac         = pOrigTrans->bIsUac;
    pCloneTrans->eMethod        = pOrigTrans->eMethod;
    pCloneTrans->bIsProxy       = pOrigTrans->bIsProxy;
    pCloneTrans->bAppInitiate   = pOrigTrans->bAppInitiate;
#ifdef RV_SIP_IMS_ON
    if (pOrigTrans->hSecObj != NULL)
    {
        retCode = TransactionSetSecObj(pCloneTrans, pOrigTrans->hSecObj);
        if (RV_OK != retCode)
        {
            RvLogError(pOrigTrans->pMgr->pLogSrc,(pOrigTrans->pMgr->pLogSrc,
                "TransactionClone - Orig Transaction 0x%p - failed to set Security Object %p(rv=%d:%s)",
                pOrigTrans, pOrigTrans->hSecObj, retCode,
                SipCommonStatus2Str(retCode)));
            TransactionDestruct(pCloneTrans);
            return retCode;
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/
    

    /* Remove TAG from the TO header */
    if(bRemoveToTag == RV_TRUE)
    {
        RvSipPartyHeaderSetTag(pCloneTrans->hToHeader, NULL);
    }

    /* This copies FROM, TO and CallId from original to the
    clone transaction */
    if(pCloneTrans->eOwnerType == SIP_TRANSACTION_OWNER_APP ||
       pCloneTrans->eOwnerType == SIP_TRANSACTION_OWNER_NON)
    {
        retCode = TransactionCopyKeyToInternalPage(pCloneTrans);
        if(retCode != RV_OK)
        {
            RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
                "TransactionClone - Transaction 0x%p - Failed to copy key",pOrigTrans));
            TransactionDestruct(pCloneTrans);
            return retCode;
        }
    }
    retCode = TransactionSetRequestUri(pCloneTrans,pOrigTrans->hRequestUri);
    if(retCode != RV_OK)
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
            "TransactionClone - Transaction 0x%p - Failed to copy Request URI",pOrigTrans));
        TransactionDestruct(pCloneTrans);
        return retCode;
    }

    pCloneTrans->strMethod = NULL;
    if (pOrigTrans->strMethod != NULL)
    {
        retCode = TransactionSetMethodStr(pCloneTrans,pOrigTrans->strMethod);
    }
    if(retCode != RV_OK)
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
            "TransactionClone - Transaction 0x%p - Failed to copy Method String",pOrigTrans));
        TransactionDestruct(pCloneTrans);
        return retCode;
    }


#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    pCloneTrans->pSubsInfo = pOrigTrans->pSubsInfo;
    if(pOrigTrans->hReferredBy != NULL)
    {
        retCode = SipTransactionConstructAndSetReferredByHeader(
                        (RvSipTranscHandle)pCloneTrans, pOrigTrans->hReferredBy);
        if(retCode != RV_OK)
        {
            RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
                "TransactionClone - Transaction 0x%p - Failed to copy referred-by header",pOrigTrans));
            TransactionDestruct(pCloneTrans);
            return retCode;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */

    if(pOrigTrans->hHeadersListToSetInInitialRequest != NULL)
    {
        /* move the list with it's page from original transaction to cloned transaction.
           then reset the handle in original transaction, so the page won't be free twice... */
        pCloneTrans->hHeadersListToSetInInitialRequest = pOrigTrans->hHeadersListToSetInInitialRequest;
        pOrigTrans->hHeadersListToSetInInitialRequest = NULL;
    }
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIGCOMP_ON
    pCloneTrans->bSigCompEnabled     = pOrigTrans->bSigCompEnabled;
    pCloneTrans->hSigCompCompartment = NULL; /* The two Transactions MUST not have the same compartment */
    pCloneTrans->eIncomingMsgComp    = RVSIP_COMP_UNDEFINED;
#endif /* #ifdef RV_SIGCOMP_ON */

    retCode = SipTransmitterCopy(pCloneTrans->hTrx, pOrigTrans->hTrx,RV_TRUE);
    if(retCode != RV_OK)
    {
        RvLogError(pOrigTrans->pMgr->pLogSrc ,(pOrigTrans->pMgr->pLogSrc ,
                  "TransactionClone - Transaction 0x%p - Failed to copy dns list",pOrigTrans));
        TransactionDestruct(pCloneTrans);
    }
    return retCode;
}

#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/


/***************************************************************************
 * TransactionCopyToHeader
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's To header, and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the To header in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyToHeader(IN Transaction *pTransc)
{
    RvSipPartyHeaderHandle hToHeader;
    RvStatus              retStatus;

    hToHeader = pTransc->hToHeader;
    if (NULL != hToHeader)
    {
        TransactionLockTranscMgr(pTransc);
        /* The transaction has an old To header to copy */
        retStatus = RvSipPartyHeaderConstruct(
                                        (pTransc->pMgr)->hMsgMgr,
                                        (pTransc->pMgr)->hGeneralPool,
                                        pTransc->memoryPage,
                                        &(pTransc->hToHeader));
        if (RV_OK != retStatus)
        {
            TransactionUnlockTranscMgr(pTransc);
            return retStatus;
        }
        retStatus = RvSipPartyHeaderCopy(pTransc->hToHeader,
                                         hToHeader);
        TransactionUnlockTranscMgr(pTransc);

        if (RV_OK != retStatus)
        {
            return retStatus;
        }
    }
    return RV_OK;
}


/***************************************************************************
 * TransactionCopyFromHeader
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's From header, and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the From header in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyFromHeader(IN Transaction *pTransc)
{
    RvSipPartyHeaderHandle hFromHeader;
    RvStatus              retStatus;

    hFromHeader = pTransc->hFromHeader;
    if (NULL != hFromHeader)
    {
        TransactionLockTranscMgr(pTransc);
        /* The transaction has an old To header to copy */
        retStatus = RvSipPartyHeaderConstruct(
                                        (pTransc->pMgr)->hMsgMgr,
                                        (pTransc->pMgr)->hGeneralPool,
                                        pTransc->memoryPage,
                                        &(pTransc->hFromHeader));
        if (RV_OK != retStatus)
        {
            TransactionUnlockTranscMgr(pTransc);
            return retStatus;
        }
        retStatus = RvSipPartyHeaderCopy(pTransc->hFromHeader,
                                         hFromHeader);

        TransactionUnlockTranscMgr(pTransc);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
    }
    return RV_OK;
}


/***************************************************************************
 * TransactionCopyCallId
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's Call-Id , and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the Call-Id in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyCallId(
                                IN Transaction *pTransc)
{
    RvStatus           retStatus;
    RPOOL_Ptr           strCallId;

    strCallId.hPage = pTransc->strCallId.hPage;
    strCallId.hPool = pTransc->strCallId.hPool;
    strCallId.offset = pTransc->strCallId.offset;
    if ((NULL != strCallId.hPool) &&
        (NULL_PAGE != strCallId.hPage) &&
        (UNDEFINED != strCallId.offset))
    {
        /* The transaction has an old Call-Id to copy */
        RvInt32 allocated;
        RvUint32 length;
        /* Append memory for the Call-Id */
        length = RPOOL_Strlen(strCallId.hPool, strCallId.hPage,
                              strCallId.offset) + 1;
        retStatus = RPOOL_Append((pTransc->pMgr)->hGeneralPool,
                                 pTransc->memoryPage,
                                 length, RV_FALSE, &allocated);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        retStatus = RPOOL_CopyFrom(
                       (pTransc->pMgr)->hGeneralPool,
                       pTransc->memoryPage, allocated,
                       strCallId.hPool, strCallId.hPage, strCallId.offset,
                       length);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        TransactionSetCallID(pTransc, (pTransc->pMgr)->hGeneralPool,
            pTransc->memoryPage, allocated);
#ifdef SIP_DEBUG
        pTransc->pCallId = RPOOL_GetPtr(pTransc->strCallId.hPool,
                                             pTransc->strCallId.hPage,
                                             pTransc->strCallId.offset);
#endif
    }
    return RV_OK;
}



/***************************************************************************
 * TransactionSetRequestUri
 * ------------------------------------------------------------------------
 * General: Set the request URI on the transaction page.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the request uri in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetRequestUri(
                                   IN Transaction             *pTransc,
                                   IN RvSipAddressHandle      hRequestUri)
{
    RvStatus rv;
    RvSipAddressType eAddrCurType = RVSIP_ADDRTYPE_UNDEFINED;
    RvSipAddressType eAddrNewType = RVSIP_ADDRTYPE_UNDEFINED;

    /*no request uri given*/
    if(hRequestUri == NULL)
    {
        pTransc->hRequestUri = NULL;
        return RV_OK;
    }

    if (pTransc->hRequestUri != NULL)
    {
        /* getting the address type of the current requestUri */
        eAddrCurType = RvSipAddrGetAddrType(pTransc->hRequestUri);
    }

    /* getting the address type of the new requestUri */
    eAddrNewType = RvSipAddrGetAddrType(hRequestUri);
    /*first check if the given header was constructed on the transaction page
      and if so attach it*/
    if( SipAddrGetPool(hRequestUri) == pTransc->pMgr->hGeneralPool &&
        SipAddrGetPage(hRequestUri) == pTransc->memoryPage)
    {
        pTransc->hRequestUri = hRequestUri;
        return RV_OK;
    }

    /*the given header is not on the transc page - construct and copy*/
    /*if there is no header, construct it*/
    if(pTransc->hRequestUri == NULL || eAddrNewType != eAddrCurType)
    {
        if (eAddrNewType != RVSIP_ADDRTYPE_UNDEFINED)
        {
            rv = RvSipAddrConstruct(pTransc->pMgr->hMsgMgr,
                                    pTransc->pMgr->hGeneralPool,
                                    pTransc->memoryPage,
                                    eAddrNewType,
                                    &(pTransc->hRequestUri));
        }
        else
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
               "TransactionSetRequestUri - transc 0x%p - Unknown address type of request uri (rv=-12)",
               pTransc));
            return RV_ERROR_INVALID_HANDLE;
        }

        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                   "TransactionSetRequestUri - transc 0x%p - Failed to construct request uri (rv=%d)",
                   pTransc,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    /*copy information from the given address the the transc address*/
    rv = RvSipAddrCopy(pTransc->hRequestUri, hRequestUri);
    if(rv != RV_OK)
    {
         RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionSetRequestUri - transc 0x%p - Failed to set request uri - copy failed (rv=%d)",
                   pTransc,rv));
         return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;

}

/***************************************************************************
 * TransactionGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given transaction object.
 *          The call-id is kept on the transaction memory page.
 *          We use this function if the request was called and no
 *          call-id was supplied.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction object
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGenerateCallId(IN  Transaction*  pTransc)
{
    RvStatus           rv;
    HRPOOL             pool      = pTransc->pMgr->hGeneralPool;
    RvInt32            offset;
    RvChar             strCallId[SIP_COMMON_ID_SIZE];
    RvInt32            callIdSize;
    RvLogSource*       hLog;

    hLog = pTransc->pMgr->pLogSrc;

    rv = SipTransactionGenerateIDStr(pTransc->pMgr->hTransport,
                             pTransc->pMgr->seed,
                             (void*)pTransc,
                             hLog,
                             (RvChar*)strCallId);

    if(rv != RV_OK)
    {
        RvLogError(hLog,(hLog,
             "TransactionGenerateCallId - Failed for transc 0x%p,",pTransc));
    }

    callIdSize = (RvInt32)strlen(strCallId);

    /*allocate a buffer on the pTransc page that will be used for the
      first part of the call-id.*/
    rv = RPOOL_AppendFromExternal(pool,pTransc->memoryPage,strCallId, callIdSize+1,RV_FALSE,&offset,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
             "TransactionGenerateCallId - Transc-leg 0x%p failed in RPOOL_AppendFromExternal (rv=%d)",pTransc,rv));
        return rv;
    }
    
    TransactionSetCallID(pTransc, pool, pTransc->memoryPage, offset);

#ifdef SIP_DEBUG
    pTransc->pCallId = RPOOL_GetPtr(pool, pTransc->memoryPage, pTransc->strCallId.offset);
#endif

    return RV_OK;
}


/***************************************************************************
 * TransactionInitializeCancelTransc
 * ------------------------------------------------------------------------
 * General: Initialize a cancel transaction according to the transaction
 *          that is about to be canceled.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCancelTransc - The cancel transaction
 *        pCancelledTransc - The transaction to be cancelled
 ***************************************************************************/
RvStatus RVCALLCONV  TransactionInitializeCancelTransc(
                                            IN  Transaction*  pCancelTransc,
                                            IN  Transaction*  pCancelledTransc)
{

    RvStatus                    rv;
    SipTransactionKey           key;
    
    
    /*set the to and from headers*/
    TransactionGetKeyFromTransc(pCancelledTransc, &key);
    if(pCancelledTransc->hOrigToForCancel != NULL)
    {
        /* cancel should be sent with the exact To header as in the INVITE */
        key.hToHeader = pCancelledTransc->hOrigToForCancel;
    }
    key.bIgnoreLocalAddr = RV_TRUE;

    if (pCancelledTransc->hTopViaHeader != NULL)
    {
        rv = RvSipViaHeaderConstruct((pCancelTransc->pMgr)->hMsgMgr,
            (pCancelTransc->pMgr)->hGeneralPool,
            pCancelTransc->memoryPage,
            &pCancelTransc->hTopViaHeader);
        if (RV_OK != rv)
        {
            return rv;
        }

        rv = RvSipViaHeaderCopy(pCancelTransc->hTopViaHeader,
            pCancelledTransc->hTopViaHeader);
        if (RV_OK != rv)
        {
            return rv;
        }
    }

    /*attach the key to the transaction*/
    TransactionAttachKey(pCancelTransc,&key);

    rv = SipTransmitterCopy(pCancelTransc->hTrx, pCancelledTransc->hTrx,RV_FALSE);
    if(rv != RV_OK)
    {
        RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                  "TransactionInitializeCancelTransc - Failed for transc 0x%p, failed to clone transmitter",
                  pCancelledTransc));
        return rv;
    }

    /*attach owner to the transaction*/
    if(pCancelledTransc->eOwnerType == SIP_TRANSACTION_OWNER_APP)
    {
        /*give the cancel transaction the lock of the cancelled transaction*/
        /*update the triple lock pointer only if it was not updated yet*/
        TransactionSetTripleLock(&(pCancelledTransc->transactionTripleLock),pCancelTransc);

        rv = TransactionAttachApplicationOwner((RvSipTranscHandle)pCancelTransc,
                                               &key,
                                               pCancelledTransc->pOwner);
        if(rv != RV_OK)
        {
             RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                  "TransactionInitializeCancelTransc - Failed for transc 0x%p, failed to attach app owner (rv=%d)",
                  pCancelledTransc,rv));
             return rv;
        }
        if(pCancelTransc->pEvHandlers->pfnEvInternalTranscCreated == NULL)
        {
            pCancelTransc->pOwner = NULL;
            pCancelTransc->eOwnerType = SIP_TRANSACTION_OWNER_NON;
            pCancelTransc->pEvHandlers = &(pCancelTransc->pMgr->pDefaultEvHandlers);

        }
    }
    else
    {
        rv = TransactionAttachOwner((RvSipTranscHandle)pCancelTransc,
                                    pCancelledTransc->pOwner,
                                    &key,pCancelledTransc->tripleLock,
                                    pCancelledTransc->eOwnerType,
                                    RV_TRUE);
    }
    if(rv != RV_OK)
    {
        RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                  "TransactionInitializeCancelTransc - Failed for transc 0x%p, failed to attach owner",
                  pCancelledTransc));
        return rv;
    }
    /*set outbound address*/
    rv = SipTransmitterSetOutboundAddsStructure(pCancelTransc->hTrx,
                                           SipTransmitterGetOutboundAddsStructure(pCancelledTransc->hTrx));
    if(rv != RV_OK)
    {
        RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
            "TransactionInitializeCancelTransc - Failed in SipTransmitterSetOutboundAddsStructure pCancelledTransc 0x%p,  pCancelTransc 0x%p, rv=%d",
             pCancelledTransc,pCancelTransc,rv));
        return rv;
    }
    /*set persistency definition*/
    SipTransmitterSetPersistency(pCancelTransc->hTrx,
                        SipTransmitterGetPersistency(pCancelledTransc->hTrx));



    if(pCancelledTransc->hActiveConn != NULL &&
       SipTransmitterGetPersistency(pCancelTransc->hTrx) == RV_TRUE)
    {

        rv = TransactionAttachToConnection(pCancelTransc,pCancelledTransc->hActiveConn,RV_TRUE);
        if(rv != RV_OK)
        {
            RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                "TransactionInitializeCancelTransc - Failed to attach transaction 0x%p to connection 0x%p",pCancelTransc,pCancelledTransc->hActiveConn));
            return rv;
        }
        /*attach the connection to the cancelled transmitter as well*/
        rv = SipTransmitterSetConnection(pCancelTransc->hTrx,pCancelTransc->hActiveConn);
        if(rv != RV_OK)
        {
            RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                "TransactionInitializeCancelTransc - Failed to attach transaction 0x%p, connection 0x%p to transmitter 0x%p",
                 pCancelTransc,pCancelTransc->hActiveConn,pCancelTransc->hTrx));
            return rv;
        }
    }
    pCancelTransc->bTagInTo = pCancelledTransc->bTagInTo;

#if defined(UPDATED_BY_SPIRENT)
    pCancelTransc->cctContext = pCancelledTransc->cctContext;
    pCancelTransc->URI_Cfg_Flag = pCancelledTransc->URI_Cfg_Flag;
#endif

    if(pCancelledTransc->pTimers != pCancelTransc->pTimers && 
        pCancelTransc->eOwnerType == SIP_TRANSACTION_OWNER_CALL)
    {
        /* If the call-leg has a timers structure, the cancel transaction should use it too */
         pCancelTransc->pTimers = pCancelledTransc->pTimers;
    }

#ifdef RV_SIP_IMS_ON
    /* Set Security Object to be used */
    if (NULL != pCancelledTransc->hSecObj)
    {
        rv = TransactionSetSecObj(pCancelTransc, pCancelledTransc->hSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pCancelledTransc->pMgr->pLogSrc,(pCancelledTransc->pMgr->pLogSrc,
                "TransactionInitializeCancelTransc - Transaction 0x%p, Cancelled Transaction %p - failed to set Security Object %p(rv=%d:%s)",
                pCancelTransc, pCancelledTransc, pCancelledTransc->hSecObj, rv,
                SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    pCancelTransc->eMethod = SIP_TRANSACTION_METHOD_CANCEL;
    return RV_OK;
}

/***************************************************************************
 * TransactionGetKeyFromTransc
 * ------------------------------------------------------------------------
 * General: Fill the Key structure with transaction/s parameters.
 * Return Value: RV_ERROR_BADPARAM - no cseq in transc.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction.
 * Output:  key     - A structure of the key to fill.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGetKeyFromTransc(
                                    IN  Transaction       *pTransc,
                                    OUT SipTransactionKey *pKey)
{
    RvStatus rv;
    memset(pKey,0,sizeof(SipTransactionKey));
    rv = SipCommonCSeqGetStep(&pTransc->cseq,&pKey->cseqStep);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionGetKeyFromTransc - Transaction 0x%p, failed to retrieve CSeq",
            pTransc));
        return rv;
    }
    pKey->hFromHeader      = pTransc->hFromHeader;
    pKey->hToHeader        = pTransc->hToHeader;
    pKey->strCallId.hPage  = pTransc->strCallId.hPage;
    pKey->strCallId.hPool  = pTransc->strCallId.hPool;
    pKey->strCallId.offset = pTransc->strCallId.offset;

    return RV_OK;
}
/*-----------------------------------------------------------------
                     DNS SUPPORT
  -----------------------------------------------------------------*/

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * TransactionHandleMsgSentFailureEvent
 * ------------------------------------------------------------------------
 * General: Sends INTERNAL_OBJECT_EVENT of msg sent failure to the processing queue event.
 * Return Value: RV_OK        - The event was sent successfully.
 *                 RV_ERROR_NULLPTR    - The pointer to the transaction is a NULL
 *                                  pointer.
 *                 RV_ERROR_OUTOFRESOURCES
 *                                - The function failed to allocate event structure
 *                                  due to no enough resources.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction handle
 *            eReason -      The reason for the event
 ***************************************************************************/
RvStatus RVCALLCONV TransactionHandleMsgSentFailureEvent(
                         IN Transaction*                         pTransc,
                         IN RvSipTransactionStateChangeReason    eReason)

{
    RvStatus rv;

    /* firstly we kill all timers, we don't want the transaction to terminate */
    TransactionTimerReleaseAllTimers(pTransc);

    /*if the reason is RVSIP_TRANSC_REASON_503_RECEIVED do not use the queue since it will
      cause the response message to be lost*/
    if(eReason == RVSIP_TRANSC_REASON_503_RECEIVED)
    {
        return MsgSendFailureEventHandler((void*)pTransc,eReason, UNDEFINED, pTransc->transactionUniqueIdentifier);
    }

    /* Detach the transaction and its transmitter from the connection.
       O.W the detach action comes to solve this problem:
       if a transaction reaches the msg-sent-failure state because of a timeout, its transmitter
       is still attached to the connection, and is still in state msg-sent.
       now if the connection receives a network error, it notifies all its owners, including this trx.
       the trx state is changes from msg-sent --> msg-sent-failure and the transaction indicates
       its owner of msg-sent-failure state twice... */
    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
         "TransactionHandleMsgSentFailureEvent - Transaction 0x%p: detach transc and trx 0x%p from connection 0x%p",
         pTransc, pTransc->hTrx, pTransc->hActiveConn));
    SipTransmitterDetachFromConnection(pTransc->hTrx);
    TransactionDetachFromConnection(pTransc, pTransc->hActiveConn, RV_TRUE);
      
    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
         "TransactionHandleMsgSentFailureEvent - Transaction 0x%p: Inserting msg send failure (reason=%d) to the event queue",
         pTransc, eReason));

    pTransc->bMsgSentFailureInQueue = RV_TRUE;

    /*insert the transaction to the event queue*/
    rv =  SipTransportSendStateObjectEvent(pTransc->pMgr->hTransport,
                                          (void *)pTransc,
                                          pTransc->transactionUniqueIdentifier,
                                          (RvInt32)eReason,
                                          UNDEFINED,
                                          MsgSendFailureEventHandler,
                                          RV_TRUE,
                                          TRANSC_STATE_CLIENT_MSG_SEND_FAILURE_STR,
                                          TRANSC_OBJECT_NAME_STR);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "TransactionHandleMsgSentFailureEvent - Transaction 0x%p: Failed to insert msg send failure to the event queue, terminating the transaction", pTransc));
		TransactionTerminate(RVSIP_TRANSC_REASON_OUT_OF_RESOURCES, pTransc);

    }
    return rv;
}



#endif



/*-----------------------------------------------------------------
                     PRACK SUPPORT
  -----------------------------------------------------------------*/
#ifndef RV_SIP_PRIMITIVES


/***************************************************************************
 * TransactionUpdateReliableStatus
 * ------------------------------------------------------------------------
 * General: Update the reliable status of an Invite transaction from an
 *          INVITE message received.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdateReliableStatus(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)
{
    
    
    RvBool bHeaderFound = RV_FALSE;
    
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"Require","100rel",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pTransc->relStatus = RVSIP_TRANSC_100_REL_REQUIRED;
        return RV_OK;
    }

  
    /*if we got here there is no Require - look for Supported*/
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"Supported","100rel",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pTransc->relStatus = RVSIP_TRANSC_100_REL_SUPPORTED;
        return RV_OK;
    }

    /*look for compact form Supported*/
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"k","100rel",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pTransc->relStatus = RVSIP_TRANSC_100_REL_SUPPORTED;
        return RV_OK;
    }

    pTransc->relStatus = RVSIP_TRANSC_100_REL_UNDEFINED;
    return RV_OK;

}


/***************************************************************************
 * TransactionSetUnsupportedList
 * ------------------------------------------------------------------------
 * General: Go over the message require headers and add any unsupported
 *          option to the unsupported list in the transaction.If the list
 *          is not empty the transaction will respond with 420 and will add
 *          all unsupported headers in the response message.
 *          The unsuppored list is kept on the transaction page as string.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetUnsupportedList(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)

{
    RvSipOtherHeaderHandle       hRequire;
    RvSipHeaderListElemHandle    listElem;
    RvStatus                    rv;
    HRPOOL                       hMsgPool = SipMsgGetPool(hMsg);
    HPAGE                        hMsgPage = SipMsgGetPage(hMsg);
    RvInt32                     valueOffset,offset;
    RvInt32                     valueLen;


    if(hMsgPool == NULL || hMsgPage == NULL_PAGE)
    {
        return RV_ERROR_UNKNOWN;
    }

    /*get the first Require header*/
    hRequire =  RvSipMsgGetHeaderByName(hMsg,"Require",
                                       RVSIP_FIRST_HEADER,&listElem);
    while (hRequire != NULL)
    {
        if(IsRequireInSupportedList(pTransc,hRequire) == RV_FALSE)
        {

            /*if this is not the first element on the list add ','*/
            if(pTransc->unsupportedList != UNDEFINED)
            {
                rv = RPOOL_AppendFromExternal(pTransc->pMgr->hGeneralPool,
                                             pTransc->memoryPage,
                                             ",", 1, RV_FALSE, &offset,NULL);
                if(rv != RV_OK)
                {
                    return rv;
                }
            }

            valueOffset = SipOtherHeaderGetValue(hRequire);
            valueLen = RPOOL_Strlen(hMsgPool, hMsgPage, valueOffset);

            /* append valueLen to the transaction page*/
            rv = RPOOL_Append(pTransc->pMgr->hGeneralPool,
                              pTransc->memoryPage,
                              valueLen,
                              RV_FALSE,
                              &offset);
           if(rv != RV_OK)
           {
               return rv;
           }
           /*copy the value from the message page to the transaction page*/
           rv = RPOOL_CopyFrom(pTransc->pMgr->hGeneralPool,
                               pTransc->memoryPage,
                               offset,
                               hMsgPool,
                               hMsgPage,
                               valueOffset,
                               valueLen);
           if(rv != RV_OK)
           {
               return rv;
           }
           if(pTransc->unsupportedList == UNDEFINED)
           {
               pTransc->unsupportedList = offset;
#ifdef SIP_DEBUG
               pTransc->strUnsupportedList = (RvChar*)RPOOL_GetPtr(
                                                    pTransc->pMgr->hGeneralPool,
                                                    pTransc->memoryPage,
                                                    pTransc->unsupportedList);
#endif
           }

        }
        /* get the next require hop*/
        hRequire =  RvSipMsgGetHeaderByName(hMsg,"Require",
                                       RVSIP_NEXT_HEADER,&listElem);
    }

    /*add '\0 at the end of the string*/
    if(pTransc->unsupportedList != UNDEFINED)
    {
        rv = RPOOL_AppendFromExternal(pTransc->pMgr->hGeneralPool,
                                      pTransc->memoryPage,
                                      "\0", 1, RV_FALSE, &offset,NULL);
        if(rv != RV_OK)
        {
            return rv;
        }
    }

    return RV_OK;
}
#endif /* #ifndef RV_SIP_PRIMITIVES*/






/***************************************************************************
 * TransactionCopyTimestampToMsg
 * ------------------------------------------------------------------------
 * General: Copy time stamp header from the transaction page to the
 *           message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 *          hMsg - handle to the message.
 ***************************************************************************/
RvStatus TransactionCopyTimestampToMsg(IN Transaction   *pTransc,
                                        IN RvSipMsgHandle hMsg)
{
    RvSipOtherHeaderHandle hTimeStamp;
    RvStatus retStatus;

    if(pTransc->hTimeStamp == NULL)
    {
        return RV_OK;
    }

    retStatus = RvSipOtherHeaderConstructInMsg(hMsg, RV_FALSE,&hTimeStamp);
    if(retStatus != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionCopyTimestampToMsg - Failed for transc 0x%p, failed construct time stamp header",
            pTransc));
        return retStatus;
    }
    retStatus = RvSipOtherHeaderCopy(hTimeStamp,pTransc->hTimeStamp);
    if(retStatus != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "TransactionCopyTimestampToMsg - Failed for transc 0x%p, failed to copy Time Stamp header",
            pTransc));
        return retStatus;
    }
    return RV_OK;

}



/***************************************************************************
 * TransactionCopyTimestampFromMsg
 * ------------------------------------------------------------------------
 * General: Copy time stamp header from the message to the transaction
 *          page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 *          hMsg - handle to the message.
 ***************************************************************************/
RvStatus TransactionCopyTimestampFromMsg(IN Transaction   *pTransc,
                                                 IN RvSipMsgHandle hMsg)
{
    RvSipOtherHeaderHandle hTimeStamp;
       RvSipHeaderListElemHandle hElement;
       RvStatus rv;

    hTimeStamp =  RvSipMsgGetHeaderByName(hMsg,"Timestamp",
                                          RVSIP_FIRST_HEADER,&hElement);
    if(hTimeStamp != NULL)
    {
        rv = RvSipOtherHeaderConstruct(pTransc->pMgr->hMsgMgr,
                                       pTransc->pMgr->hGeneralPool,
                                       pTransc->memoryPage,
                                       &(pTransc->hTimeStamp));
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                     "TransactionCopyTimestampFromMsg - Failed for transc 0x%p, failed construct Timestamp header",
                      pTransc));
            return rv;
        }
        rv = RvSipOtherHeaderCopy(pTransc->hTimeStamp,hTimeStamp);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                     "TransactionCopyTimestampFromMsg - Failed for transc 0x%p, failed to copy Timestamp header",
                      pTransc));
            return rv;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionMethodStricmp
 * ------------------------------------------------------------------------
 * General: The function does insensitive comparsion between 2 strings..
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
RvBool RVCALLCONV TransactionMethodStricmp(
                                IN RvChar* firstStr,
                                IN RvChar* secStr)
{
    RvInt firstLen, secLen;
    RvInt  i = 0;

    if(firstStr == NULL)
    {
        if(secStr == NULL)
            return RV_TRUE;
        else
            return RV_FALSE;
    }

    firstLen = (RvInt)strlen(firstStr);
    secLen   = (RvInt)strlen(secStr);

    if(firstLen != secLen)
        return RV_FALSE;

    while (firstStr[i] != '\0')
    {
        if((firstStr[i] != secStr[i]) &&
            ((firstStr[i]-secStr[i])%('a'-'A') != 0))
                return RV_FALSE;
        else
            ++i;
    }
    return RV_TRUE;
}

/***************************************************************************
 * TransactionGetEnumMethodFromString
 * ------------------------------------------------------------------------
 * General: returns the enumeration that a string represents. If the string
 *          is invalid such as begins with spaces or too long the method
 *          returned is SIP_TRANSACTION_METHOD_UNDEFINED.
 * Return Value: The method as emum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strRequestMethod - the method as string.
 ***************************************************************************/
SipTransactionMethod RVCALLCONV TransactionGetEnumMethodFromString(
                                        IN RvChar   *strRequestMethod)
{

    RvInt32 methodLen = (RvInt32)strlen(strRequestMethod);
    if(methodLen > 30 ||
       methodLen == 0 ||
       strRequestMethod[0] == ' '     ||
       TransactionMethodStricmp(strRequestMethod , "CANCEL") ||
       TransactionMethodStricmp(strRequestMethod , "ACK"))
        return SIP_TRANSACTION_METHOD_UNDEFINED;
    if(TransactionMethodStricmp(strRequestMethod , "REGISTER"))
        return SIP_TRANSACTION_METHOD_REGISTER;
#ifdef RV_SIP_SUBS_ON
    else if(TransactionMethodStricmp(strRequestMethod , "REFER"))
        return SIP_TRANSACTION_METHOD_REFER;
    else if(TransactionMethodStricmp(strRequestMethod , "SUBSCRIBE"))
        return SIP_TRANSACTION_METHOD_SUBSCRIBE;
    else if(TransactionMethodStricmp(strRequestMethod , "NOTIFY"))
        return SIP_TRANSACTION_METHOD_NOTIFY;
#endif /* #ifdef RV_SIP_SUBS_ON */
    else if(TransactionMethodStricmp(strRequestMethod , "BYE"))
        return SIP_TRANSACTION_METHOD_BYE;
    else if(TransactionMethodStricmp(strRequestMethod , "INVITE"))
        return SIP_TRANSACTION_METHOD_INVITE;
#ifndef RV_SIP_PRIMITIVES
    else if(TransactionMethodStricmp(strRequestMethod , "PRACK"))
        return SIP_TRANSACTION_METHOD_PRACK;
#endif
    else
        return SIP_TRANSACTION_METHOD_OTHER;
}

/***************************************************************************
 * TransactionGetStringMethodFromEnum
 * ------------------------------------------------------------------------
 * General: return a string with the method of the given enumeration.
 * Return Value: The method as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eMethod - the method as enum.
 ***************************************************************************/
RvChar*  RVCALLCONV TransactionGetStringMethodFromEnum(
                                            IN SipTransactionMethod eMethod)
{
    switch(eMethod)
    {
    case SIP_TRANSACTION_METHOD_REGISTER:
        return "REGISTER";
#ifdef RV_SIP_SUBS_ON
    case SIP_TRANSACTION_METHOD_REFER:
        return "REFER";
    case SIP_TRANSACTION_METHOD_SUBSCRIBE:
        return "SUBSCRIBE";
    case SIP_TRANSACTION_METHOD_NOTIFY:
        return "NOTIFY";
#endif /* #ifdef RV_SIP_SUBS_ON */
    case SIP_TRANSACTION_METHOD_BYE:
        return "BYE";
    case SIP_TRANSACTION_METHOD_INVITE:
        return "INVITE";
    case SIP_TRANSACTION_METHOD_CANCEL:
        return "CANCEL";
#ifndef RV_SIP_PRIMITIVES
    case SIP_TRANSACTION_METHOD_PRACK:
        return "PRACK";
#endif
    case SIP_TRANSACTION_METHOD_OTHER:
        return NULL;
    default:
        return NULL;
    }
}

/***************************************************************************
 * FinalResponseReason
 * ------------------------------------------------------------------------
 * General: Returnes the enumeration value defined for the class of the
 *          received response code
 * Return Value: The response code class inumeration.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: responseCode - The response code.
 ***************************************************************************/
RvSipTransactionStateChangeReason RVCALLCONV FinalResponseReason(
                                               IN RvUint16 responseCode)
{
    if ((200 <= responseCode) && (300 > responseCode))
    {
        return RVSIP_TRANSC_REASON_RESPONSE_SUCCESSFUL_RECVD;
    }
    else if ((300 <= responseCode) && (400 > responseCode))
    {
        return RVSIP_TRANSC_REASON_RESPONSE_REDIRECTION_RECVD;
    }
    else if ((400 <= responseCode) && (500 > responseCode))
    {
        return RVSIP_TRANSC_REASON_RESPONSE_REQUEST_FAILURE_RECVD;
    }
    else if ((500 <= responseCode) && (600 > responseCode))
    {
        return RVSIP_TRANSC_REASON_RESPONSE_SERVER_FAILURE_RECVD;
    }
    else if ((600 <= responseCode) && (700 > responseCode))
    {
        return RVSIP_TRANSC_REASON_RESPONSE_GLOBAL_FAILURE_RECVD;
    }
    else
    {
        return RVSIP_TRANSC_REASON_UNDEFINED;
    }
}


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * TransactionGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransactionGetStateName (
                                      IN  RvSipTransactionState  eState)
{

    switch(eState)
    {
    case RVSIP_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
        return "Client General Proceeding";
    case RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
        return "Client General Request Sent";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_ACK_SENT:
        return "Client Invite Ack sent";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CALLING:
        return "Client Invite Calling";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD:
        return "Client Invite Final Response Rcvd";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING:
        return "Client Invite Proceeding";
    case RVSIP_TRANSC_STATE_IDLE:
        return "Idle";
    case RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
        return "Server General Final Response Sent";
    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
        return "Server General Request Rcvd";
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD:
        return "Server Invite Request Rcvd";
    case RVSIP_TRANSC_STATE_SERVER_INVITE_FINAL_RESPONSE_SENT:
        return "Server Invite Final Response Sent";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_CANCELLING:
        return "Client Invite Cancelling";
    case RVSIP_TRANSC_STATE_CLIENT_GEN_CANCELLING:
        return "Client General Cancelling";
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_SENT:
        return "Client Cancel Sent";
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_PROCEEDING:
        return "Client Cancel Proceeding";
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_FINAL_RESPONSE_SENT:
        return "Server Cancel Final Response Sent";
    case RVSIP_TRANSC_STATE_TERMINATED:
        return TRANSC_STATE_TERMINATED_STR;
    case RVSIP_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
        return "Client General Final Response Rcvd";
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROCEEDING_TIMEOUT:
        return "Client Invite Proceeding Timeout";
    case RVSIP_TRANSC_STATE_SERVER_INVITE_ACK_RCVD:
        return "Server Invite Ack Received";
    case RVSIP_TRANSC_STATE_CLIENT_CANCEL_FINAL_RESPONSE_RCVD:
        return "Client Cancel Final Response Received";
    case RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        return TRANSC_STATE_CLIENT_MSG_SEND_FAILURE_STR;
    case RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD:
        return "Server Cancel Request Rcvd";
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT:
        return "Server Invite Reliable Provisional Response Sent";
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PRACK_COMPLETED:
        return "Server Invite Prack Completed";
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    case RVSIP_TRANSC_STATE_CLIENT_INVITE_PROXY_2XX_RESPONSE_RCVD:
        return "Client Invite Proxy 2xx Response Rcvd";
    case RVSIP_TRANSC_STATE_SERVER_INVITE_PROXY_2XX_RESPONSE_SENT:
        return "Server Invite Proxy 2xx Response Sent";
    default:
        return "Undefined";
    }
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionGetMsgCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given transaction compression message type
 * Return Value: The string with the message type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eMsgCompType - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransactionGetMsgCompTypeName (
                             IN  RvSipTransmitterMsgCompType eMsgCompType)
{
    switch(eMsgCompType)
    {
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED:
        return "Uncompressed Message";
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED:
        return "SigComp Compressed Message";
    case RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED_INCLUDE_BYTECODE:
        return "SigComp Compressed Message including Bytecode";
    default:
        return "Undefined";
    }
}
#endif /* RV_SIGCOMP_ON */
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

/***************************************************************************
* TransactionLockTranscMgr
* ------------------------------------------------------------------------
* General: Locks the transaction mgr. This function is used when changing parameters
*          inside the transaction that influence the hash key of the transaction.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
***************************************************************************/
void TransactionLockTranscMgr(Transaction *pTransc)
{
        RvMutexLock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
}

/***************************************************************************
* TransactionUnlockTranscMgr
* ------------------------------------------------------------------------
* General: Unlocks the transaction mgr. This function is used when changing parameters
*          inside the transaction that influence the hash key of the transaction.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
***************************************************************************/
void TransactionUnlockTranscMgr(Transaction *pTransc)
{
        RvMutexUnlock(&pTransc->pMgr->hLock,pTransc->pMgr->pLogMgr);
}

/***************************************************************************
* TransactionSetCallID
* ------------------------------------------------------------------------
* General: Sets the Call ID in the transaction. The function locks the transaction
*          manager when doing this.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
*          hPool      - The pool of the call ID
*          hPage      - The page of the call ID
*          offset     - The call ID offset
***************************************************************************/
void TransactionSetCallID(Transaction *pTransc,
                          HRPOOL       hPool,
                          HPAGE        hPage,
                          RvInt32     offset)
{
    TransactionLockTranscMgr(pTransc);
    pTransc->strCallId.hPool = hPool;
    pTransc->strCallId.hPage = hPage;
    pTransc->strCallId.offset = offset;
    TransactionUnlockTranscMgr(pTransc);
}

/***************************************************************************
* TransactionSetResponseCode
* ------------------------------------------------------------------------
* General: Sets the response code in the transaction. The function locks the transaction
*          manager when doing this.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc      - Pointer to the transaction structure
*          responseCode - The response code to set in the transaction
***************************************************************************/
void TransactionSetResponseCode(Transaction *pTransc,
                                RvUint16    responseCode)
{
    TransactionLockTranscMgr(pTransc);
    pTransc->responseCode = responseCode;
    TransactionUnlockTranscMgr(pTransc);
}

/***************************************************************************
* TransactionSetTag
* ------------------------------------------------------------------------
* General: Sets the tag of the to/from header in the transaction. The function locks the
*          transaction manager when doing this.
* Return Value: - RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc   - Pointer to the transaction structure
*          strTag    - The string to set in the tag
*          bIsToTag  - RV_TRUE if the tag is the to-tag, RV_FALSE if the tag is the from tag
***************************************************************************/
RvStatus TransactionSetTag(Transaction *pTransc,
                            RvChar     *strTag,
                            RvBool      bIsToTag)
{
    RvStatus rv = RV_OK;

    TransactionLockTranscMgr(pTransc);
    if(bIsToTag == RV_TRUE)
    {
        rv = RvSipPartyHeaderSetTag(pTransc->hToHeader, strTag);
    }
    else
    {
        rv = RvSipPartyHeaderSetTag(pTransc->hFromHeader, strTag);
    }
    TransactionUnlockTranscMgr(pTransc);
    return rv;
}

/***************************************************************************
 * TransactionSaveReceivedFromAddr
 * ------------------------------------------------------------------------
 * General: saves the address from which the last address was received.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   -- Handle to the transaction.
 *            pRecvFrom -- the address on which a message was received
 ***************************************************************************/
void RVCALLCONV TransactionSaveReceivedFromAddr (
                                      IN  Transaction*         pTransc,
                                      IN  SipTransportAddress* pRecvFrom)
{
    memcpy(&pTransc->receivedFrom,pRecvFrom,sizeof(pTransc->receivedFrom));

}

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * TransactionConstructAndSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Allocate and Sets the given referred-by header in the transaction object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionConstructAndSetReferredByHeader (
                                      IN  Transaction                   *pTransc,
                                      IN  RvSipReferredByHeaderHandle    hReferredBy)
{
    RvStatus rv;
    RvSipReferredByHeaderHandle hAllocatedHeader;

    rv = RvSipReferredByHeaderConstruct(pTransc->pMgr->hMsgMgr,
                               pTransc->pMgr->hGeneralPool,
                               pTransc->memoryPage, &hAllocatedHeader);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionConstructAndSetReferredByHeader - transc 0x%p - Failed to construct referred-by header (rv=%d)",
               pTransc,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    rv = RvSipReferredByHeaderCopy(hAllocatedHeader, hReferredBy);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
               "TransactionConstructAndSetReferredByHeader - transc 0x%p - Failed to copy header (rv=%d)",
               pTransc,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pTransc->hReferredBy = hReferredBy;
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionHandleIncomingSigCompMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: The function should be called when a SigComp-SIP message is
 *          received. According to the SigComp model each SigComp-SIP
 *          message MUST be related to any SigComp compartment. Thus, the
 *          function verifies that the Transaction is currently associated
 *          with a SigComp Compartment. In case that the Transaction is not
 *          part of any compartment, then new one is created for it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc    - The transaction handle.
 *          hConn      - The connection on which the message was received.
 *          pRecvFrom  - The address from which the message was received.
 *          hLocalAddr - The address on which the message was received
 *          hMsg       - Handle to the incoming message, to be related to a 
 *                       Compartment object 
 *          pSigCompMsgInfo - 
 *                      SigComp info related to the received message.
 *                      The information contains indication if the UDVM
 *                      waits for a compartment ID for the given unique
 *                      ID that was related to message by the UDVM.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionHandleIncomingSigCompMsgIfNeeded(
                             IN    Transaction                     *pTransc,
                             IN    RvSipTransportConnectionHandle   hConn,
                             IN    SipTransportAddress             *pRecvFrom,
                             IN    RvSipTransportLocalAddrHandle    hLocalAddr,
                             IN    RvSipMsgHandle                   hMsg, 
                             INOUT SipTransportSigCompMsgInfo      *pSigCompMsgInfo)
{
    RPOOL_Ptr  urnRpoolPtr;
    RvBool     bUrnFound    = RV_FALSE;
    RPOOL_Ptr *pUrnRpoolPtr = NULL;
    RvBool     bUpdateTrx   = RV_FALSE;
    RvStatus   rv           = RV_OK;
	RvSipTransportAddr        rvLocalAddr;
	SipTransportAddress       localAddr;

    
    /* 1. Check if the received message is a SigComp-compressed message */ 
    if (pSigCompMsgInfo == NULL || pSigCompMsgInfo->bExpectComparmentID == RV_FALSE)
    {
        return RV_OK;
    }

    /* 2. Indicate that the received message type is SigComp-compressed */ 
    pTransc->eIncomingMsgComp = RVSIP_COMP_SIGCOMP;

    if (pTransc->bSigCompEnabled == RV_FALSE)
    {
        RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p, SigComp feature is disabled and compressed msg rcvd", pTransc));
    }
    /* 3. Check if the given Transaction is already related to a Compartment.   */
    /* According to RFC5049, mapped to this Transaction, */ 
    /* would have been sent from a single endpoint,both request and response    */
    /* should have the same unique ID (and the same Compartment) */
    if (pTransc->hSigCompCompartment == NULL)
    {   
        /* Indicate that the related Compartment to be related to this Transaction */
        /* should be forwarded to it's Trx object */ 
        bUpdateTrx = RV_TRUE; 

        /* Look for the SIP/SigComp identifier within the incoming message */         
        rv = LookForSigCompUrnInRcvdMsg(pTransc,hMsg, &bUrnFound, &urnRpoolPtr);
        if (rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , failed to look for URN in hMsg 0x%p",
                pTransc,hMsg));
        }

        pUrnRpoolPtr = (bUrnFound == RV_TRUE) ? &urnRpoolPtr : NULL;
    }
    else
    {
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , found compartment 0x%p, there is no need to look for URN",
            pTransc,pTransc->hSigCompCompartment));
    }
    

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
        "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , going to relate hMsg 0x%p,compartment 0x%p with unique id %d",
        pTransc,hMsg,pTransc->hSigCompCompartment,pSigCompMsgInfo->uniqueID));
    
    /* 4. Turn off the expectation indication since even if SipCompartmentRelateCompHandleToInMsg */
    /* fails it internally it automatically frees the temporal compartment resources */ 
    pSigCompMsgInfo->bExpectComparmentID = RV_FALSE;
    
    /* 5. Relate the found or NULL (not found) Compartment to the given decompression unique ID */ 
	rv = RvSipTransportMgrLocalAddressGetDetails (hLocalAddr, sizeof(RvSipTransportAddr), &rvLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , failed to get address 0x%p details",
            pTransc,hLocalAddr)); 
        return rv;
    } 

	localAddr.eTransport = rvLocalAddr.eTransportType;
	RvSipTransportConvertTransportAddr2RvAddress(&rvLocalAddr, &localAddr.addr);

    rv = SipCompartmentRelateCompHandleToInMsg(
                        pTransc->pMgr->hCompartmentMgr,
                        pUrnRpoolPtr,
                        pRecvFrom,
						&localAddr,
						hMsg,
                        hConn,
                        pSigCompMsgInfo->uniqueID,
                        &pTransc->hSigCompCompartment);
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , failed to relate a Compartment (0x%p) to uniqueID %d",
            pTransc,pTransc->hSigCompCompartment,pSigCompMsgInfo->uniqueID)); 
        return rv;
    }

    /* 6. Update the Transmitter if necessary */ 
    if (bUpdateTrx == RV_TRUE && pTransc->hTrx != NULL)
    {
        rv = SipTransmitterSetCompartment(pTransc->hTrx,pTransc->hSigCompCompartment);
        if (rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionHandleIncomingSigCompMsgIfNeeded - Transc 0x%p , failed to forward Compartment (0x%p) to Trx 0x%p",
                pTransc,pTransc->hSigCompCompartment,pTransc->hTrx)); 
            return rv;
        }
    }

	/* 7. update the transaction */
	if (RV_TRUE == bUpdateTrx)
	{
		RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
			"TransactionHandleIncomingSigCompMsgIfNeeded - Attaching Transc 0x%p to ompartment 0x%p",
			pTransc,pTransc->hSigCompCompartment));

		rv = SipCompartmentAttach(pTransc->hSigCompCompartment,(void*)pTransc);
		if (rv != RV_OK)
		{
			RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
				"TransactionHandleIncomingSigCompMsgIfNeeded - Transaction 0x%p, failed to attach to compartment=0x%p",
				pTransc, pTransc->hSigCompCompartment));   
			return rv;
		}
	}
	

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */ 


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionGetOutboundMsgCompType
 * ------------------------------------------------------------------------
 * General: Retrieves the compression type of the Transaction outbound
 *          message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle.
 ***************************************************************************/
RvSipCompType RVCALLCONV TransactionGetOutboundMsgCompType(
                                         IN Transaction   *pTransc)
{
    RvSipTransmitterMsgCompType msgCompType =
             SipTransmitterGetOutboundMsgCompType(pTransc->hTrx);

    return SipTransmitterConvertMsgCompTypeToCompType(msgCompType);
}
#endif /* RV_SIGCOMP_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransactionSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transaction.
 *          As a result of this operation, all messages, sent by this
 *          Transaction, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTransc - Pointer to the Transaction object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TransactionSetSecObj(IN  Transaction*      pTransc,
                              IN  RvSipSecObjHandle hSecObj)
{
    RvStatus        rv;
    TransactionMgr* pTranscMgr = pTransc->pMgr;

    /* If Security Object was already set, detach from it */
    if (NULL != pTransc->hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTranscMgr->hSecMgr, pTransc->hSecObj,
                -1 /*delta*/, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION,
                (void*)pTransc);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionSetSecObj - Transaction 0x%p: Failed to unset existing Security Object %p",
                pTransc, pTransc->hSecObj));
            return rv;
        }
        pTransc->hSecObj = NULL;
    }

    /* Register the Transaction to the new Security Object*/
    if (NULL != hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTranscMgr->hSecMgr, hSecObj,
                1 /*delta*/, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION,
                (void*)pTransc);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionSetSecObj - Transaction 0x%p: Failed to set new Security Object %p",
                pTransc, hSecObj));
            return rv;
        }
    }

    if (NULL != pTransc->hTrx)
    {
        rv = RvSipTransmitterSetSecObj(pTransc->hTrx, hSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pTranscMgr->pLogSrc,(pTranscMgr->pLogSrc,
                "TransactionSetSecObj - Transaction 0x%p - failed to set Security Object %p into Transmitter %p(rv=%d:%s)",
                pTransc, hSecObj, pTransc->hTrx, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    pTransc->hSecObj = hSecObj;

    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * UpdateFromHeader
 * ------------------------------------------------------------------------
 * General: Delete the existing From header, and create a new one similar to
 *            the From header received in the message parameter.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_ERROR_BADPARAM - The message received has no From header
 *                                       in it.
 *                 RV_OutOfResource - Couldn't allocate memory for the new From
 *                                    header in the transaction objetc.
 *                 RV_OK - The From header was updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to set From header.
 *           pMsg - The message from which the new From header is copied.
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateFromHeader(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)
{
    RvSipPartyHeaderHandle hHeader;
    RvStatus              retStatus;
    RvInt32               transcFromTag = UNDEFINED;

    transcFromTag = SipPartyHeaderGetTag(pTransc->hFromHeader);
    hHeader = RvSipMsgGetFromHeader(hMsg);
    if (NULL == hHeader)
    {
        return RV_ERROR_BADPARAM;
    }
    TransactionLockTranscMgr(pTransc);
    if (UNDEFINED != transcFromTag || SipPartyHeaderGetTag(hHeader) == UNDEFINED)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "UpdateFromHeader: transc 0x%p - no need to update transc from tag", pTransc));
        TransactionUnlockTranscMgr(pTransc);
        return RV_OK;
    }
    
    /* Copy the given From header to the transaction's From header */
    retStatus = SipPartyHeaderConstructAndCopy(pTransc->pMgr->hMsgMgr,
                                        pTransc->pMgr->hGeneralPool,
                                        pTransc->memoryPage,
                                        hHeader,
                                        RVSIP_HEADERTYPE_FROM,
                                        &(pTransc->hFromHeader));

    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "UpdateFromHeader: transc 0x%p - copy From header with new from-tag from msg (pool=0x%p page=0x%p)",
        pTransc, pTransc->pMgr->hGeneralPool, pTransc->memoryPage));

    TransactionUnlockTranscMgr(pTransc);
    return retStatus;
}


/***************************************************************************
 * UpdateToHeader
 * ------------------------------------------------------------------------
 * General: Delete the existing To header, and create a new one similar to
 *            the To header received in the message parameter.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_ERROR_BADPARAM - The message received has no To header
 *                                       in it.
 *                 RV_OutOfResource - Couldn't allocate memory for the new To
 *                                    header in the transaction objetc.
 *                 RV_OK - The To header was updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to set To header.
 *           pMsg - The message from which the new To header is copied.
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateToHeader( IN Transaction   *pTransc,
                                           IN RvSipMsgHandle hMsg,
                                           IN RvUint16 responseCode)
{
    RvSipPartyHeaderHandle hHeader;
    RvStatus              retStatus;
#ifdef UPDATED_BY_SPIRENT
    RvBool                bCompactPartyForm=RV_FALSE;
#endif

    hHeader = RvSipMsgGetToHeader(hMsg);
    if (NULL == hHeader)
    {
        return RV_ERROR_BADPARAM;
    }
    TransactionLockTranscMgr(pTransc);

#ifdef UPDATED_BY_SPIRENT
    retStatus = RvSipPartyHeaderGetCompactForm(pTransc->hToHeader,&bCompactPartyForm);
    if(RV_OK != retStatus){
        RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionLock - Transaction 0x%p: Get partyheader compact form failed.",
            pTransc));
        TransactionUnlockTranscMgr(pTransc);
        return retStatus;
    }
#endif
    if(SipPartyHeaderGetTag(hHeader) == UNDEFINED)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "UpdateToHeader: transc 0x%p - msg To header has no to-tag", pTransc));

#ifdef RFC_2543_COMPLIANT
        if(responseCode > 100 && responseCode < 300) 
        {
            /*101-199 - if do not have a to-tag, consider it as an empty tag. */
            SipPartyHeaderSetEmptyTag(pTransc->hToHeader);
        }
#endif /*#ifdef RFC_2543_COMPLIANT*/

        TransactionUnlockTranscMgr(pTransc);
        return RV_OK;
    }

    /* Copy the given To header to the transaction's To header */
    retStatus = SipPartyHeaderConstructAndCopy( pTransc->pMgr->hMsgMgr,
                                                pTransc->pMgr->hGeneralPool,
                                                pTransc->memoryPage,
                                                hHeader,
                                                RVSIP_HEADERTYPE_TO,
                                                &(pTransc->hToHeader));
    
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "UpdateToHeader: transc 0x%p - copy To header with new to-tag from msg (pool=0x%p page=0x%p)",
        pTransc, pTransc->pMgr->hGeneralPool, pTransc->memoryPage));
#ifdef UPDATED_BY_SPIRENT
    if(RV_OK != retStatus){
        RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionLock - Transaction 0x%p: SipPartyHeaderConstructAndCopy failed.",
            pTransc));
        TransactionUnlockTranscMgr(pTransc);
        return retStatus;
    }
    retStatus = RvSipPartyHeaderSetCompactForm(pTransc->hToHeader,bCompactPartyForm);
    if(RV_OK != retStatus){
        RvLogWarning(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionLock - Transaction 0x%p: Set partyheader compact form failed.",
            pTransc));
        TransactionUnlockTranscMgr(pTransc);
        return retStatus;
    }
#endif

    TransactionUnlockTranscMgr(pTransc);
    RV_UNUSED_ARG(responseCode)
    return retStatus;
}


#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * TransactionLock
 * ------------------------------------------------------------------------
 * General: If an owner lock exists locks the owner's lock. else locks the
 *          transaction's lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pTransc - The transaction to lock.
 *            pMgr         - pointer to transaction manager
 *            tripleLock     - triple lock of the transaction
 ***************************************************************************/
static RvStatus TransactionLock(IN Transaction    *pTransc,
                                IN TransactionMgr *pMgr,
                                IN SipTripleLock  *tripleLock,
                                IN RvInt32         identifier)
{

    if ((pTransc == NULL) || (pMgr == NULL) || (tripleLock == NULL))
    {
        return RV_ERROR_NULLPTR;
    }
    RvMutexLock(&tripleLock->hLock,pMgr->pLogMgr);

    if (pTransc->transactionUniqueIdentifier != identifier &&
        pTransc->transactionUniqueIdentifier == 0)
    {
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransactionLock - Transaction 0x%p: transaction object was destructed",
            pTransc));
        RvMutexUnlock(&tripleLock->hLock,pMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}


/***************************************************************************
 * TransactionUnLock
 * ------------------------------------------------------------------------
 * General: If an owner un-lock exists locks the owner's lock. else un-locks
 *          the transaction's lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pLogMgr        - The log mgr pointer
 *           hLock        - The lock to un-lock.
***************************************************************************/
static void RVCALLCONV TransactionUnLock(IN RvLogMgr  *pLogMgr,
                                         IN RvMutex   *hLock)
{
    if (NULL != hLock)
    {
        RvMutexUnlock(hLock,pLogMgr);
    }
}

/***************************************************************************
 * TransactionTerminateUnlock
 * ------------------------------------------------------------------------
 * General: Unlocks processing and transaction locks. This function is called to unlock
 *          the locks that were locked by the LockMsg() function. The UnlockMsg() function
 *          is not called because in this stage the transaction object was already destructed.
 * Return Value: - RV_Succees
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pLogMgr            - The log mgr pointer
 *            pLogSrc        - log handler
 *            hObjLock        - The transaction lock.
 *            hProcessingLock - The transaction processing lock
 ***************************************************************************/
static void RVCALLCONV TransactionTerminateUnlock(IN RvLogMgr			*pLogMgr,
                                                  IN RvLogSource		*pLogSrc,
                                                  IN RvMutex            *hObjLock,
                                                  IN RvMutex            *hProcessingLock)
{
    RvLogSync(pLogSrc ,(pLogSrc ,
        "TransactionTerminateUnlock - Unlocking Object lock 0x%p: Processing Lock 0x%p",
        hObjLock, hProcessingLock));

    if (hObjLock != NULL)
    {
        RvMutexUnlock(hObjLock,pLogMgr);
    }

    if (hProcessingLock != NULL)
    {
        RvMutexUnlock(hProcessingLock,pLogMgr);
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
 * TerminateTransactionEventHandler
 * ------------------------------------------------------------------------
 * General: Causes immediate shut-down of the transaction:
 *  Changes the transaction's state to "terminated" state.
 *  Calls EvStateChanged callback with "terminated" state, and the reason
 *  received as a parameter. Calls Destruct on the transaction being
 *  terminated.
 * Return Value: RV_OK - The transaction was terminated successfully.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 *                 RV_ERROR_UNKNOWN - The transaction was not found in the transaction
 *                              manager, in other words the UnManageTransaction
 *                              call has returned the RV_ERROR_NOT_FOUND return value.
 *                              In practice the Destruct function call returns
 *                              RV_ERROR_UNKNOWN (that way it is detected here).
 *                              Note : The transaction was destructed and can
 *                              not be used any more. The RV_ERROR_UNKNOWN return is
 *                              for indication only.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 *            eReason -      The reason for terminating the transaction.
 ***************************************************************************/
static RvStatus  RVCALLCONV TerminateTransactionEventHandler(IN void *trans,
                                                             IN RvInt32  reason)
{
    RvLogSource*    logSrc;
    RvStatus        retStatus;
    Transaction     *pTransc;
    RvMutex         *hObjLock;
    RvMutex         *hProcessingLock;

    if (trans == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransc = (Transaction *)trans;
    retStatus = TransactionLockMsg(pTransc);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "TerminateTransactionEventHandler - Transc 0x%p - event is out of the termination queue", pTransc));
    
    logSrc          = pTransc->pMgr->pLogSrc;
    hProcessingLock = &pTransc->tripleLock->hProcessLock;
    hObjLock        = &pTransc->tripleLock->hLock;

    /* Call "state changed" callback */
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_TERMINATED,
                             (RvSipTransactionStateChangeReason)reason);
    retStatus = TransactionDestruct(pTransc);

    TransactionTerminateUnlock(pTransc->pMgr->pLogMgr,logSrc, hObjLock,hProcessingLock);
    return retStatus;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * MsgSendFailureEventHandler
 * ------------------------------------------------------------------------
 * General: called from the processing queue. Changes the transaction state
 *          to msg send failure.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      trans - The transaction handle
 *            reason -      The reason of the new state.
 ***************************************************************************/
static RvStatus  RVCALLCONV MsgSendFailureEventHandler( IN void   *trans,
                                                        IN RvInt32 reason,
                                                        IN RvInt32 notUsed,
                                                        IN RvInt32 objUniqueIdentifier)
{


    RvStatus           retStatus;
    Transaction        *pTransc;

    if (trans == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    pTransc = (Transaction *)trans;
    retStatus = TransactionLockMsg(pTransc);
    if (retStatus != RV_OK)
    {
        return retStatus;
    }

    if(objUniqueIdentifier != pTransc->transactionUniqueIdentifier)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "MsgSendFailureEventHandler - Transc 0x%p - different identifier. Ignore the event.",
            pTransc));
        TransactionUnLockMsg(pTransc);
        return RV_OK;
    }

    RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
        "MsgSendFailureEventHandler - Transc 0x%p - event is out of the event queue", pTransc));

    if(RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE == pTransc->eState)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "MsgSendFailureEventHandler - Transc 0x%p - already in msg-sent-failure state. Ignore the event.",
            pTransc));
        TransactionUnLockMsg(pTransc);
        return RV_OK;
    }

#ifdef RV_SIP_IMS_ON
	TransactionSecAgreeMsgSendFailure(pTransc);
#endif
#ifdef RV_SIGCOMP_ON
	/* we need to release the resources of the internal sigcomp compartment,
	as states stored within it are no longer valid for further transmissions */				
	DetachFromInternalSigcompCompartment(pTransc);
#endif

    if(pTransc->eState == RVSIP_TRANSC_STATE_TERMINATED)
    {
        /*ignore the new state*/
        TransactionUnLockMsg(pTransc);
        return RV_OK;
    }
    /* Call "state changed" callback */
    TransactionChangeState(pTransc,
                             RVSIP_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE,
                             (RvSipTransactionStateChangeReason)reason);


    TransactionUnLockMsg(pTransc);
    RV_UNUSED_ARG(notUsed)
    return retStatus;
}
#endif

/***************************************************************************
 * AddMaxForwardsToMsg
 * ------------------------------------------------------------------------
 * General: Adds the Max-Forward:70 header to the message if there is no such
 *          header in the message already.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction haneld
 *          hMsg - The message handle
 ***************************************************************************/
static RvStatus RVCALLCONV AddMaxForwardsToMsg(
                                      IN RvSipMsgHandle hMsg)

{

    RvStatus rv;
    RvSipOtherHeaderHandle hOther;
    RvSipHeaderListElemHandle hElement;

    hOther =  RvSipMsgGetHeaderByName(hMsg,"Max-Forwards",
                                       RVSIP_FIRST_HEADER,&hElement);

    if(hOther != NULL)
    {
        return RV_OK;
    }
    rv = SipMsgAddNewOtherHeaderToMsg(hMsg,"Max-Forwards","70",NULL);
    if(rv != RV_OK)
    {
        return rv;
    }
    return RV_OK;
}



#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddSupportedListToMsg
 * ------------------------------------------------------------------------
 * General: Adds supported list to the message in Supported headers.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle
 *          hMsg - the handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddSupportedListToMsg(IN Transaction*   pTransc,
                                           IN RvSipMsgHandle hMsg)
{
    RvStatus rv;
    RvSipOtherHeaderHandle hOther;
    int                 i;
    HRPOOL              hPool;
    HPAGE               hPage;
    RvInt32            offset;
    RvInt32            valueOffset = UNDEFINED;
    /*if there is no extension list we finished*/
    if(pTransc->pMgr->extensionListSize == 0)
    {
        return RV_OK;
    }
    rv = RvSipOtherHeaderConstructInMsg(hMsg,RV_FALSE,&hOther);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = RvSipOtherHeaderSetName(hOther,"Supported");
    if(rv != RV_OK)
    {
        return rv;
    }
    /*append the supported strings direcly to the page of the other header*/
    hPool = SipOtherHeaderGetPool(hOther);
    hPage = SipOtherHeaderGetPage(hOther);
    if(hPool == NULL || hPage == NULL_PAGE)
    {
        return RV_ERROR_UNKNOWN;
    }

    for(i=0; i<pTransc->pMgr->extensionListSize; i++)
    {

        /*append the supported string*/
        rv = RPOOL_AppendFromExternal(hPool,hPage,
                                      pTransc->pMgr->supportedExtensionList[i],
                                      (RvInt)strlen(pTransc->pMgr->supportedExtensionList[i]),
                                      RV_FALSE,
                                      &offset,
                                      NULL);
        if(rv != RV_OK)
        {
            return rv;
        }
        /*save the offset of the first value*/
        if(valueOffset == UNDEFINED)
        {
            valueOffset = offset;
        }

        /*if this is the last string on the list add '\0' else add ','*/
        if(i+1 == pTransc->pMgr->extensionListSize)
        {
            rv = RPOOL_AppendFromExternal(hPool,hPage,
                                      "\0",
                                      1,
                                      RV_FALSE,
                                      &offset,
                                      NULL);

        }
        else
        {
            rv = RPOOL_AppendFromExternal(hPool,hPage,
                                      ",",
                                      1,
                                      RV_FALSE,
                                      &offset,
                                      NULL);

        }
        if(rv != RV_OK)
        {
            return rv;
        }
    }
    /*set the offset back in the header*/
    rv=SipOtherHeaderSetValue(hOther,NULL,hPool,hPage,valueOffset);

    if(rv != RV_OK)
    {
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * AddUnsupportedListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the unsupported list to the message in Unsupported header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle
 *          hMsg - the handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddUnsupportedListToMsg(IN Transaction*   pTransc,
                                                  IN RvSipMsgHandle hMsg)
{
    RvStatus rv;
    RvSipOtherHeaderHandle hOther;
    rv = RvSipOtherHeaderConstructInMsg(hMsg,RV_FALSE,&hOther);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = RvSipOtherHeaderSetName(hOther,"Unsupported");
    if(rv != RV_OK)
    {
        return rv;
    }

    rv = SipOtherHeaderSetValue(hOther,NULL,pTransc->pMgr->hGeneralPool,
                                pTransc->memoryPage,pTransc->unsupportedList);

    if(rv != RV_OK)
    {
        return rv;
    }
    return RV_OK;
}

/***************************************************************************
 * IsRequireInSupportedList
 * ------------------------------------------------------------------------
 * General: check if a require header value can be found in the supported
 *          list of the transaction mgr.
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle
 *          hRequire - The Require header handle.
 ***************************************************************************/
static RvBool RVCALLCONV IsRequireInSupportedList(
                                IN Transaction             *pTransc,
                                IN RvSipOtherHeaderHandle  hRequire)
{
    RPOOL_Ptr  strValue;
    int        i;
    strValue.hPool  = SipOtherHeaderGetPool(hRequire);
    strValue.hPage  = SipOtherHeaderGetPage(hRequire);
    strValue.offset = SipOtherHeaderGetValue(hRequire);
    if(strValue.hPool  == NULL       ||
       strValue.hPage  == NULL_PAGE  ||
       strValue.offset == UNDEFINED)
    {
        return RV_FALSE;
    }


    for( i=0; i<pTransc->pMgr->extensionListSize; i++)
    {
        if(RPOOL_CmpToExternal(strValue.hPool,strValue.hPage,strValue.offset,
                               pTransc->pMgr->supportedExtensionList[i], RV_FALSE) == RV_TRUE)
        {
            return RV_TRUE;
        }
    }
    return RV_FALSE;
}

/***************************************************************************
 * AddReliableInfoToMsg
 * ------------------------------------------------------------------------
 * General: Adds the Require=100rel and the RSeq to msg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle
 *          hMsg - the handle to the message.
 ***************************************************************************/
static RvStatus RVCALLCONV AddReliableInfoToMsg(
                                    IN Transaction *pTransc,
                                    IN RvSipMsgHandle hMsg)
{
    RvStatus rv;
    RvSipRSeqHeaderHandle  hRSeq;

    /*generate the rseq inital value if needed*/
    TransactionLockTranscMgr(pTransc);
    if(pTransc->localRSeq.bInitialized == RV_FALSE)
    {
        pTransc->localRSeq.val		    = (TransactionRand(pTransc) % 1000)+1;
		pTransc->localRSeq.bInitialized = RV_TRUE;
    }
    else
    {
        pTransc->localRSeq.val++;
    }
    TransactionUnlockTranscMgr(pTransc);

    /*add Require=100rel*/
    rv = SipMsgAddNewOtherHeaderToMsg(hMsg,"Require","100rel",NULL);
    if(rv != RV_OK)
    {
        return rv;
    }
    /*add the RSeq header*/
    rv = RvSipRSeqHeaderConstructInMsg(hMsg,RV_FALSE,&hRSeq);
    if(rv != RV_OK)
    {
        return rv;
    }
    rv = RvSipRSeqHeaderSetResponseNum(hRSeq,pTransc->localRSeq.val);
    if(rv != RV_OK)
    {
        return rv;
    }
    /*increase the rseq for the next usage*/
    return RV_OK;
}


#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * TransactionSetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Sets a handle to the received message.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction to which the mesasge should be set
 *        hReceivedMsg - Handle to the received message.
 ***************************************************************************/
void  RVCALLCONV TransactionSetReceivedMsg(IN Transaction* pTransc,
                                           IN RvSipMsgHandle    hReceivedMsg)
{
    pTransc->hReceivedMsg = hReceivedMsg;
}

/***************************************************************************
 * TransactionGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Return the transaction received message handle
 * Return Value: Handle to the transaction received message.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction to which the mesasge should be set
 ***************************************************************************/
RvSipMsgHandle  RVCALLCONV TransactionGetReceivedMsg(IN Transaction* pTransc)
{
    return(pTransc->hReceivedMsg);
}


/***************************************************************************
 * IsValidMessage
 * ------------------------------------------------------------------------
 * General: Return Indication whether the message (request/response) is a
 *          message.
 * Return Value: The function return RV_TRUE if the message is valid,
 *                otherwise the function return RV_FALSE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hMsg - pointer to validate message.
 *        bIsRequest - indication whether to check the message as request or
 *                     as response.
 ***************************************************************************/
RvBool RVCALLCONV IsValidMessage(IN Transaction* pTransc,
                                  IN RvSipMsgHandle    hMsg,
                                  IN RvBool        bIsRequest)
{
    RvSipMsgType eType;

    if(NULL == hMsg)
    {
        return(RV_FALSE);
    }

    eType = RvSipMsgGetMsgType(hMsg);

    /*Check validity of request message*/
    if(RVSIP_MSG_REQUEST == eType)
    {
        if(RV_FALSE == bIsRequest)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid message is Request",
               pTransc, hMsg, bIsRequest));
            return(RV_FALSE);
        }
        if(NULL == RvSipMsgGetRequestUri(hMsg))
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid Req-Uri",
               pTransc, hMsg, bIsRequest));
            return(RV_FALSE);
        }
    }
    /*Chekc validity of rsponse message*/
    else if(RVSIP_MSG_RESPONSE == eType)
    {
        if(RV_TRUE == bIsRequest)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, message is Response",
               pTransc, hMsg, bIsRequest));
            return(RV_FALSE);
        }
        if(UNDEFINED == RvSipMsgGetStatusCode(hMsg))
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
              "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid Status Code",
               pTransc, hMsg, bIsRequest));
            return(RV_FALSE);
        }
    }
    else
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid message type",
            pTransc, hMsg, bIsRequest));
        return(RV_FALSE);
    }
    /*Check all common headers*/
    if(NULL == RvSipMsgGetToHeader(hMsg))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid To Header",
            pTransc, hMsg, bIsRequest));
        return(RV_FALSE);
    }
    if(NULL == RvSipMsgGetFromHeader(hMsg))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid From Header",
            pTransc, hMsg, bIsRequest));
        return(RV_FALSE);
    }
    if(NULL == RvSipMsgGetCSeqHeader(hMsg))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid CSeq Header",
            pTransc, hMsg, bIsRequest));
        return(RV_FALSE);
    }
    if(UNDEFINED == SipMsgGetCallIdHeader(hMsg))
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
            "IsValidMessage - Transaction=0x%p, Msg=0x%p bIsRequest=%d message not valid, Not valid Call-Id",
            pTransc, hMsg, bIsRequest));
        return(RV_FALSE);
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransc);
#endif

    return(RV_TRUE);
}



/***************************************************************************
 * TransactionCopyStrToTranscPage
 * ------------------------------------------------------------------------
 * General: Copy a given string to the transaction
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - targt transaction to copy the string to
 *          strData - The strgin to be set in the object.
 *                           If Null, the exist string in the
 *                           object will be removed.
 *          strOffset      - Offset of a string on the same page to use in an attach case.
 *                           (else - UNDEFINED)
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 * Out:     pReturnOffset - traget offset.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyStrToTranscPage(
                                                 IN  Transaction       *pTransc,
                                                 OUT RvInt32           *pReturnOffset,
                                                 OUT RvChar           **pDebugInf,
                                                 IN  RvChar*          strData,
                                                 IN  HRPOOL            hPool,
                                                 IN  HPAGE             hPage,
                                                 IN  RvInt32          strOffset)
{
    RvStatus           rv;

    /* Check if the Source object containe any data*/
    if ((NULL != hPool) &&
        (NULL_PAGE != hPage) &&
        (UNDEFINED != strOffset))
    {
        RvUint32 length;

        length = RPOOL_Strlen(hPool, hPage, strOffset) + 1;
        rv = RPOOL_Append((pTransc->pMgr)->hGeneralPool,
                                 pTransc->memoryPage,
                                 length, RV_FALSE, pReturnOffset);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionCopyStrToTranscPage - Failed ,Append failed. Transc= 0x%p hPool 0x%p, hPage 0x%p",
                pTransc, (pTransc->pMgr)->hGeneralPool, pTransc->memoryPage));
            return rv;
        }
        rv = RPOOL_CopyFrom(
                       (pTransc->pMgr)->hGeneralPool,
                       pTransc->memoryPage, *pReturnOffset,
                       hPool, hPage, strOffset, length);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionCopyStrToTranscPage - Failed ,Copy From failed. Transc= 0x%p hPool 0x%p, hPage 0x%p",
                pTransc, (pTransc->pMgr)->hGeneralPool, pTransc->memoryPage));
            return rv;
        }



    }
    else if(NULL != strData)
    {
        RPOOL_ITEM_HANDLE    ptr;
        rv = RPOOL_AppendFromExternal((pTransc->pMgr)->hGeneralPool,
                                    pTransc->memoryPage,
                                    (void*)strData,
                                    (RvInt)strlen(strData)+1,
                                    RV_FALSE,
                                    pReturnOffset,
                                    &ptr);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                "TransactionCopyStrToTranscPage - Failed ,AppendFromExternal failed. Transc= 0x%p hPool 0x%p, hPage 0x%p",
                pTransc, hPool, hPage));
            return (rv);
        }

    }
    else
    {
        *pReturnOffset = UNDEFINED;
        *pDebugInf = NULL;
        return(RV_OK);
    }

#ifdef SIP_DEBUG
    if(NULL != pDebugInf)
    {
        *pDebugInf = RPOOL_GetPtr(pTransc->pMgr->hGeneralPool,
                                  pTransc->memoryPage,
                                 *pReturnOffset);
    }
#endif
    return RV_OK;
}


/***************************************************************************
 * TransactionUpdateTopViaFromMsg
 * ------------------------------------------------------------------------
 * General: Update the transaction top via header from the message which about
 *          to be send.
 *          this function should be called after calling EvMsgToSend.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc    - pointer to transaction to update its top via header.
 *          hMsg       - Source message to update the top via from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdateTopViaFromMsg(
                                            IN Transaction        *pTransc,
                                            IN RvSipMsgHandle   hMsg)
{
    RvStatus            rv = RV_OK;
    RvSipViaHeaderHandle hSourceVia;
    RvSipHeaderListElemHandle hPos;

    /*update branch only to non ack messages*/
    if(RvSipMsgGetRequestMethod(hMsg) != RVSIP_METHOD_ACK)
    {

        TransactionLockTranscMgr(pTransc);
        if(pTransc->hTopViaHeader == NULL)
        {
            rv = RvSipViaHeaderConstruct(pTransc->pMgr->hMsgMgr,
                                         pTransc->pMgr->hGeneralPool,
                                         pTransc->memoryPage,
                                         &(pTransc->hTopViaHeader));
            if(RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "TransactionUpdateTopViaFromMsg - Transaction 0x%p: Error creating via header err= %d.",
                    pTransc, rv));
                TransactionUnlockTranscMgr(pTransc);
                return(rv);
            }
        }
        hSourceVia = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_VIA,
                                             RVSIP_FIRST_HEADER, RVSIP_MSG_HEADERS_OPTION_ALL,
                                             &hPos);
        if(NULL == hSourceVia)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionUpdateTopViaFromMsg - Transaction 0x%p: no top via header in message.",
                pTransc));
            TransactionUnlockTranscMgr(pTransc);
            return RV_ERROR_UNKNOWN;
        }
        if(SipViaHeaderGetStrBadSyntax(hSourceVia) > UNDEFINED)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                      "TransactionUpdateTopViaFromMsg - Transaction 0x%p: defected top via header.",
                      pTransc));
            TransactionUnlockTranscMgr(pTransc);
            return RV_ERROR_UNKNOWN;
        }
        rv = RvSipViaHeaderCopy(pTransc->hTopViaHeader, hSourceVia);

        TransactionUnlockTranscMgr(pTransc);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                      "TransactionUpdateTopViaFromMsg - Transaction 0x%p: Error copying top via header err= %d.",
                      pTransc, rv));
            return(rv);
        }
        RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionUpdateTopViaFromMsg - Transaction 0x%p: Copied the top via header from message into the transc object.",
                  pTransc));
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionChangeState
 * ------------------------------------------------------------------------
 * General: Change the transaction state and call the state chage
 *            callback with the new state.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc- The transaction in which the state has changed.
 *        eState      - The new state.
 *        eReason     - The reason for the state change.
 * Output:(-)
 ***************************************************************************/
void RVCALLCONV TransactionChangeState(IN Transaction                      *pTransc,
                                         IN RvSipTransactionState             eState,
                                         IN RvSipTransactionStateChangeReason eReason)
{

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
             "TransactionChangeState - Transaction 0x%p: state changed to %s",
             pTransc, TransactionGetStateName(eState)));

    /* updating the last state, in order of determining the reason.
       this check is done for states that have already been updated before */
    if (eState != pTransc->eState)
    {
        pTransc->eLastState = pTransc->eState;
    }

    /* Change the transaction's state according to the new state */
    pTransc->eState = eState;

    /* Call "State changed" callback */
    TransactionCallbackStateChangedEv(pTransc,eState,eReason);
}






/***************************************************************************
 * TransactionSetTripleLock
 * ------------------------------------------------------------------------
 * General: replaces transaction and objects under the transaction
 *        triple locks
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    ownerTripleLock  - new triple lock.
 *          pTransc     - The transaction
 ***************************************************************************/
void RVCALLCONV TransactionSetTripleLock(
    IN SipTripleLock *ownerTripleLock,
    IN Transaction   *pTransc)
{

    /* Set the transaction's lock to the owner's lock */
    RvMutexLock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr);
    pTransc->tripleLock = ownerTripleLock;
    
    /* Set transmitter triple lock - the transaction must be locked,
       so the pTransc->hTrx won't become NULL...*/
    if (pTransc->hTrx != NULL)
    {
        SipTransmitterSetTripleLock(pTransc->hTrx,ownerTripleLock);
    }
    
    RvMutexUnlock(&pTransc->transactionTripleLock.hLock,pTransc->pMgr->pLogMgr); 

}

/***************************************************************************
 * TransactionAttachToConnection
 * ------------------------------------------------------------------------
 * General: Attach the Transaction as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc       - pointer to Transaction
 *            hConn         - The connection handle
 *            bUpdateActiveConn - update the transaction active connection with
 *                                the connection we just attached to.
 ***************************************************************************/
RvStatus TransactionAttachToConnection(
                        IN Transaction                    *pTransc,
                        IN RvSipTransportConnectionHandle  hConn,
                        IN RvBool                          bUpdateActiveConn)
{
    RvStatus rv;

    /*attach to new conn*/
    rv = SipTransportConnectionAttachOwner(hConn,
                                (RvSipTransportConnectionOwnerHandle)pTransc,
                                &pTransc->pMgr->connEvHandlers,
                                sizeof(pTransc->pMgr->connEvHandlers));

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAttachToConnection - Failed to attach Transaction 0x%p to connection 0x%p",pTransc,hConn));
        return rv;
    }
    if(bUpdateActiveConn == RV_TRUE)
    {
        pTransc->hActiveConn = hConn;
    }
    return rv;
}


/***************************************************************************
 * TransactionDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transaction from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc    - pointer to Transaction
 *            hConn   - the connection to detach from.
 *            bUpdateActiveConn - if we detached from the active conn, set it
 *                                to NULL.
 ***************************************************************************/
void TransactionDetachFromConnection(
                  IN Transaction                    *pTransc,
                  IN RvSipTransportConnectionHandle  hConn,
                  IN RvBool                          bUpdateActiveConn)
{

    RvStatus rv;
    if(hConn == NULL)
    {
        return;
    }
    RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionDetachFromConnection - Transaction 0x%p detaching from connection 0x%p",pTransc,hConn));

    rv = RvSipTransportConnectionDetachOwner(hConn,
                                            (RvSipTransportConnectionOwnerHandle)pTransc);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                  "TransactionDetachFromConnection - Failed to detach transmitter 0x%p from connection 0x%p",pTransc,hConn));

    }
    /*set null in the transmitter if we detached from the current connection*/
    if(hConn == pTransc->hActiveConn && bUpdateActiveConn == RV_TRUE)
    {
        pTransc->hActiveConn = NULL;
    }
}

/***************************************************************************
 * TransactionSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the transaction timers.
 *          If some of the fields in pTimers are not set (UNDEFINED), this
 *          function will calculate it, or take the values from configuration.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus TransactionSetTimers(IN  Transaction*  pTransc,
                              IN  RvSipTimers*  pNewTimers)
{
    RvStatus  rv = RV_OK;
    RvInt32   offset;

    if(pNewTimers == NULL)
    {
        /* reset the pTimers struct */
        pTransc->pTimers = NULL;
        return RV_OK;
    }

    /* 1. allocate timers struct on the transaction page */
    pTransc->pTimers = (RvSipTimers*)RPOOL_AlignAppend((pTransc->pMgr)->hGeneralPool,
                        pTransc->memoryPage,
                        sizeof(RvSipTimers), &offset);
    if (NULL == pTransc->pTimers)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "TransactionSetTimers - Transaction 0x%p: Failed to append timers struct",
            pTransc));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* fill the given values. and calculate the rest */
    if (UNDEFINED >= (RvInt32)pNewTimers->T1Timeout)
    {
        pTransc->pTimers->T1Timeout = pTransc->pMgr->pTimers->T1Timeout;
    }
    else
    {
        pTransc->pTimers->T1Timeout = pNewTimers->T1Timeout;
    }

    if (DEFAULT_T2 > (RvInt32)pNewTimers->T2Timeout)
    {
        /* T2 must be larger or equal to this configuration value. */
        pTransc->pTimers->T2Timeout = pTransc->pMgr->pTimers->T2Timeout;
    }
    else
    {
        pTransc->pTimers->T2Timeout = pNewTimers->T2Timeout;
    }

    if (UNDEFINED >= (RvInt32)pNewTimers->T4Timeout)
    {
        pTransc->pTimers->T4Timeout = pTransc->pMgr->pTimers->T4Timeout;
    }
    else
    {
        pTransc->pTimers->T4Timeout = pNewTimers->T4Timeout;
    }

    if (UNDEFINED >= pNewTimers->genLingerTimeout)
    {
        pTransc->pTimers->genLingerTimeout = pTransc->pTimers->T1Timeout * 64;
    }
    else
    {
        pTransc->pTimers->genLingerTimeout = pNewTimers->genLingerTimeout;
    }

    if (UNDEFINED >= pNewTimers->inviteLingerTimeout)
    {
        pTransc->pTimers->inviteLingerTimeout = pTransc->pMgr->pTimers->inviteLingerTimeout;
    }
    else
    {
        pTransc->pTimers->inviteLingerTimeout = pNewTimers->inviteLingerTimeout;
    }

    if (UNDEFINED >= pNewTimers->provisionalTimeout)
    {
        pTransc->pTimers->provisionalTimeout = pTransc->pMgr->pTimers->provisionalTimeout;
    }
    else
    {
        pTransc->pTimers->provisionalTimeout = pNewTimers->provisionalTimeout;
    }

    if (UNDEFINED >= pNewTimers->generalRequestTimeoutTimeout)
    {
        pTransc->pTimers->generalRequestTimeoutTimeout = pTransc->pTimers->T1Timeout * 64;
    }
    else
    {
        pTransc->pTimers->generalRequestTimeoutTimeout = pNewTimers->generalRequestTimeoutTimeout;
    }

    if (0 >= pNewTimers->cancelInviteNoResponseTimeout)
    {
        /* cancelInviteNoResponseTimer can't be 0, setting timer to T1*64 */
        if(pTransc->pTimers->T1Timeout > 0)
        {
            pTransc->pTimers->cancelInviteNoResponseTimeout = pTransc->pTimers->T1Timeout * 64;
        }
        else
        {
            pTransc->pTimers->cancelInviteNoResponseTimeout = pTransc->pMgr->pTimers->cancelInviteNoResponseTimeout;
        }
    }
    else
    {
        pTransc->pTimers->cancelInviteNoResponseTimeout = pNewTimers->cancelInviteNoResponseTimeout;
    }

    if(pNewTimers->maxInviteRequestRetransmissions <= 0)
    {
        pTransc->pTimers->maxInviteRequestRetransmissions = pTransc->pMgr->pTimers->maxInviteRequestRetransmissions;
    }
    else
    {
        pTransc->pTimers->maxInviteRequestRetransmissions = pNewTimers->maxInviteRequestRetransmissions;
    }
    pTransc->pTimers->maxGeneralRequestRetransmissions = pNewTimers->maxGeneralRequestRetransmissions;
    pTransc->pTimers->maxInviteResponseRetransmissions = pNewTimers->maxInviteResponseRetransmissions;

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
        "TransactionSetTimers - Transaction 0x%p: timers were set",
        pTransc));
    return rv;
}

/******************************************************************************
 * TranscGetCbName
 * ----------------------------------------------------------------------------
 * General: Print the name of callback represented by the bit turned on in the 
 *          mask results.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:   MaskResults - the bitmap holding the callback representing bit.
 *****************************************************************************/
RvChar* TranscGetCbName(RvInt32 MaskResults)
{
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_CREATED))
        return "Transc-Created-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_STATE_CHANGED))
        return "Transc-State-Changed-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_MSG_TO_SEND))
        return "Transc-Msg-To-Send-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_MSG_RECEIVED))
        return "Transc-Msg-Rcvd-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_APP_INT_CREATED))
        return "Transc-App-Internal-Created-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_CANCELLED))
        return "Transc-Cancelled-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_CALL_CREATED))
        return "Transc-Call-Created-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_IGNORE_REL_PROV))
        return "Transc-Ignor-Rel-Provisional-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_INVITE_TRANSC_MSG_SENT))
        return "Transc-Invite-Msg-Sent-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_SUPPLY_PARAMS))
        return "Transc-Supply-Params-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_AUTH_CREDENTIALS_FOUND))
        return "Transc-Auth-Credentials-Found-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_AUTH_COMPLETED))
        return "Transc-Auth-Completed-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_OPEN_CALL_LEG))
        return "Transc-Open-Call-leg-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_OTHER_URL_FOUND))
        return "Transc-Other-Url-Found-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_FINAL_DEST_RESOLVED))
        return "Transc-Final-Dest-Resolved-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_NEW_CONN_IN_USE))
        return "Transc-New-Conn-In-use-Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_TRANSC_SIGCOMP_NOT_RESPONDED))
        return "Transc-SigComp-Msg-Not-Supported-Ev";
    else
        return "Unknown";
}



/*-------------------------------------------------------------------------*/
/*                            STATIC FUNCTIONS                             */
/*-------------------------------------------------------------------------*/

/***************************************************************************
 * UpdateCancelTranscPairLockingOnDestruction
 * ------------------------------------------------------------------------
 * General: Cancel transaction and cancelled transaction know each other.
 *          This relationship needs to be broken when one of the transactions
 *          terminate.
 *          When the cancel transaction terminates first we set the cancel pair
 *          for each of the transactions to NULL.
 *          When the cancelled transaction terminates first we also change the
 *          cancel lock so that it will no longer use the cancelled transaction
 *          lock. This is just in the case when the cancel uses the cancelled
 *          internal locking (not a call leg lock for example).
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:         pTransc - The transaction handle.
 *                eTransportType - The function should prefere addresses
 *                                          with the given transport but it is not
 *                                          must.
 * Output:        phLocalAddr - handle of the chosen local address.
 ***************************************************************************/
static void RVCALLCONV UpdateCancelTranscPairLockingOnDestruction(Transaction *pTransc)
{

    Transaction* pCancelPair = NULL;
    RvLogSource*  logSrc;

    logSrc = pTransc->pMgr->pLogSrc;

    /*for cancel transaction: Set in the cancelled transaction the indication that
      the cancel was terminated*/
    if(pTransc->eMethod == SIP_TRANSACTION_METHOD_CANCEL &&
       pTransc->hCancelTranscPair != NULL)
    {
        RvLogDebug(logSrc ,(logSrc ,
                 "UpdateCancelTranscPairLockingOnDestruction - Transc 0x%p ,cancelled trasc 0x%p: setting cancelling pair to NULL", pTransc,pCancelPair));

        pCancelPair = (Transaction*)pTransc->hCancelTranscPair;
        pCancelPair->hCancelTranscPair = NULL;
        pTransc->hCancelTranscPair = NULL;
        return;
    }

    /*for non cancel transaction: if this transaction was cancelled, set in the
      cancel transaction indication that this transaction was terminated.
      If the cancel transaction uses this transaction internal locks, change the locks
      so that the cancel will use its own locks from now on*/
    if(pTransc->eMethod != SIP_TRANSACTION_METHOD_CANCEL &&
        pTransc->hCancelTranscPair != NULL &&
        pTransc->tripleLock == &(pTransc->transactionTripleLock))
    {
        RvLogDebug(logSrc ,(logSrc ,
                 "UpdateCancelTranscPairLockingOnDestruction - Transc 0x%p ,canceling trasc 0x%p: setting cancelling pair to NULL and updating locks", pTransc,pCancelPair));
        pCancelPair = (Transaction*)pTransc->hCancelTranscPair;
        pCancelPair->hCancelTranscPair = NULL;
        if(pCancelPair->tripleLock == &(pTransc->transactionTripleLock))
        {
            TransactionSetTripleLock(&(pCancelPair->transactionTripleLock),pCancelPair);
        }
        return;
    }
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
* TransactionOwner2Str
* ------------------------------------------------------------------------
* General: Converts from owner enumeration to string.
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* input: eOwner - the owner type enumeration
***************************************************************************/
static RvChar* TransactionOwner2Str(SipTransactionOwner eOwner)
{
    switch(eOwner)
    {
        case SIP_TRANSACTION_OWNER_NON:
            return "None";
        case SIP_TRANSACTION_OWNER_CALL:
            return "Call";
        case SIP_TRANSACTION_OWNER_TRANSC_CLIENT:
            return "Transc-Client";
        case SIP_TRANSACTION_OWNER_APP:
            return "App";
        case SIP_TRANSACTION_OWNER_UNDEFINED:
            return "Undefined";
        default:
            return "Exception";
    }
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


#ifdef RV_SIGCOMP_ON
/***************************************************************************
* LookForSigCompUrnInRcvdMsg
* ------------------------------------------------------------------------
* General: Searches for SigComp identifier URN (Uniform Resource Name) 
*          that uniquely identifies the application. SIP/SigComp identifiers 
*          are carried in the 'sigcomp-id' SIP URI (Uniform Resource 
*          Identifier) in outgoing request messages (and incoming responses)
*          or within Via header field parameter within incoming request 
*          messages.
*   
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc      - Transaction object pointer
*          hMsg         - Handle to the received request message.
* Output:  pbUrnFound   - Indication if the URN was found.
*          pUrnRpoolPtr - Pointer to the page, pool and offset which
*                         contains a copy of the 
*                         found URN  (might be NULL in case of missing URN).
***************************************************************************/
static RvStatus LookForSigCompUrnInRcvdMsg(IN   Transaction      *pTransc,
                                           IN   RvSipMsgHandle    hMsg, 
                                           OUT  RvBool           *pbUrnFound,
                                           OUT  RPOOL_Ptr        *pUrnRpoolPtr)
{
    RvStatus        rv;
    RvSipMsgType    eMsgType = RvSipMsgGetMsgType(hMsg);
    
    *pbUrnFound = RV_FALSE; 

    RvLogDebug(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
        "LookForSigCompUrnInRcvdMsg - Transc 0x%p is looking for URN in hMsg 0x%p (type=%d)", 
        pTransc, hMsg, eMsgType));

    switch (eMsgType) 
    {
    case RVSIP_MSG_REQUEST:
        rv = SipCommonLookForSigCompUrnInTopViaHeader(
                hMsg,pTransc->pMgr->pLogSrc,pbUrnFound,pUrnRpoolPtr);
        break;
    case RVSIP_MSG_RESPONSE:
        /* Retrieve the URN from the Transaction URI */
        rv = SipCommonLookForSigCompUrnInAddr(
                pTransc->hRequestUri,pTransc->pMgr->pLogSrc,pbUrnFound,pUrnRpoolPtr);        
        break;
    default: 
        rv = RV_ERROR_UNKNOWN;
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "LookForSigCompUrnInRcvdMsg - Transc 0x%p received undefined type of hMsg 0x%p", 
            pTransc, hMsg));
    }
    if (rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
            "LookForSigCompUrnInRcvdMsg - Transc 0x%p couldn't retrieve URN from hMsg 0x%p", 
            pTransc, hMsg));
        return rv; 
    }

    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
* DetachFromInternalSigcompCompartment
* ------------------------------------------------------------------------
* General: This function triggers the SIP compartment to release internal
*          sigcomp resources, so that they cannot be used for future 
*          message transmission. This helps the compressor on the client
*          and the compressor on the server to be synced with states
*          (for example, in case of connection failure)
*   
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc      - Transaction object pointer
***************************************************************************/
static RvStatus DetachFromInternalSigcompCompartment(Transaction *pTransc)
{
	RvSipCompartmentHandle hComp;
	RvStatus               retStatus;

	if (pTransc->hSigCompCompartment != NULL) 
	{
		hComp = pTransc->hSigCompCompartment;
	}
	else
	{
		SipTransmitterGetCompartment(pTransc->hTrx,&hComp);
	}
	if (hComp != NULL)
	{
		retStatus = SipCompartmentMgrDetachFromSigCompCompartment(hComp);
		if (retStatus != RV_OK)
		{
			RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
				"TransactionTransportTrxStateChangeEv - Transaction 0x%p failed to detach from sigcomp compartment 0x%p rv=%d",
				pTransc, retStatus));
		}
	}
	return RV_OK;
}
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
#endif /* #ifdef RV_SIGCOMP_ON */ 


#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif


