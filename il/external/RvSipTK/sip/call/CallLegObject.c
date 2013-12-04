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
 *                              <CallLegObject.c>
 *
 * This file defines the callLeg object, attributes and interface functions.
 * CallLeg represents a SIP call leg as defined in RFC 2543.
 * This means that a CallLeg is defined using it's Call-Id, From and To
 * fields.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#ifndef RV_SIP_PRIMITIVES

#include "rvrandomgenerator.h"
#include "CallLegObject.h"
#include "_SipCallLeg.h"
#include "CallLegRouteList.h"
#include "RvSipCallLeg.h"
#include "CallLegCallbacks.h"
#include "CallLegSubs.h"
#include "CallLegRefer.h"
#include "CallLegSessionTimer.h"
#include "CallLegMsgEv.h"
#include "CallLegForking.h"
#include "CallLegSession.h"
#include "CallLegInvite.h"
#include "_SipTransactionTypes.h"
#include "_SipTransportTypes.h"
#include "_SipTransport.h"
#include "_SipAuthenticator.h"
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include "_SipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"
#include "RvSipReferredByHeader.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"
#include "RvSipCommonList.h"
#include "_SipCommonUtils.h"
#include "_SipMsg.h"
#include "_SipHeader.h"
#include "RvSipHeader.h"
#include "RvSipOtherHeader.h"

#include "RvSipRouteHopHeader.h"
#include "_SipTransmitterMgr.h"
#include "_SipTransmitter.h"


#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctp.h"
#endif

#ifdef RV_SIP_IMS_ON
#include "CallLegSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "_SipSecuritySecObj.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV CallLegSetHeadersInOutboundRequestMsg (
                                    IN  CallLeg             *pCallLeg,
                                    IN  RvSipTranscHandle    hTransc,
                                    IN  SipTransactionMethod eRequestMethod);

static RvStatus RVCALLCONV TerminateCallegEventHandler(IN void *obj, IN RvInt32 reason);

#ifdef RV_SIP_SUBS_ON
static void RVCALLCONV InitCallLegSubsData(IN CallLeg *pCallLeg, IN RvBool  bIsHidden);

static void RVCALLCONV TerminateCallLegSubsReferIfNeeded(
                                        IN  CallLeg *pCallLeg,
                                        OUT RvBool  *pbEmptySubsList);

static void RVCALLCONV NotifyReferGeneratorInTerminateIfNeeded(
                                IN CallLeg                       *pCallLeg,
                                IN RvSipCallLegStateChangeReason  eReason);
#endif /* #ifdef RV_SIP_SUBS_ON */

static RvStatus RVCALLCONV SetSingleHeaderInOutboundReqIfExists(
                                IN  CallLeg        *pCallLeg,
                                IN  RvSipMsgHandle  hMsg,
                                IN  RvSipHeaderType eHeaderType);


#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

static RvStatus RVCALLCONV CallLegLock(IN CallLeg             *pCallLeg,
                                        IN CallLegMgr*         pMgr,
                                        IN SipTripleLock     *tripleLock,
                                        IN RvInt32            identifier);

static void RVCALLCONV CallLegUnLock(IN  RvLogMgr *pLogMgr,
                                     IN  RvMutex  *hLock);
#else
#define CallLegLock(a,b,c,d) (RV_OK)
#define CallLegUnLock(a,b)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/* ---------- TRANSC LIST FUNCTIONS ------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvChar* GetTranscListActionName(CallLegTranscListAction eAction);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static void RVCALLCONV GeneralTranscListTerminateAll(IN CallLeg*          pCallLeg,
                                                     IN RvSipTranscHandle hTransc,
                                                     IN RLIST_ITEM_HANDLE pItem);
static RvBool RVCALLCONV GeneralTranscListRemoveFromList(IN CallLeg*          pCallLeg,
                                                       IN RvSipTranscHandle hTransc,
                                                       IN RvSipTranscHandle hTranscForCompare,
                                                       IN RLIST_ITEM_HANDLE pItem);

static RvBool RVCALLCONV GeneralTranscListFindPrack(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc);

static RvBool RVCALLCONV GeneralTranscListVerifyNoActive(IN CallLeg*          pCallLeg,
                                                         IN RvSipTranscHandle hTransc);

static void RVCALLCONV GeneralTranscListResetAppActive(IN RvSipTranscHandle hTransc);

static RvBool RVCALLCONV GeneralTranscListDetachBye(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RLIST_ITEM_HANDLE pItem);

static void RVCALLCONV GeneralTranscListStopGeneral(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RLIST_ITEM_HANDLE pItem);

static void RVCALLCONV GeneralTranscListSend487OnPending(IN CallLeg*          pCallLeg,
                                                         IN RvSipTranscHandle hTransc);

static void RVCALLCONV DestructAllInvites(IN CallLeg *pCallLeg); 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new call in the Idle state. Sets all values to their
 *          initial values, allocates a memory page for this call.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize call-leg due to a
 *                                   resources problem.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the new call-leg
 *            pMgr - pointer to the manager of this call.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegInitialize(IN  CallLeg          *pCallLeg,
                                       IN  CallLegMgr       *pMgr,
                                       IN  RvSipCallLegDirection eDirection,
                                       IN  RvBool           bIsHidden)
{
    RvStatus          rv;

    /* Note: avoid initiation of call-leg to all zeros (memset) because calllegTripleLock
     value should remains unchanged since stack initialization. */

     /*allocate a page for the usage of this call*/
#if defined(UPDATED_BY_SPIRENT)
    //dfb: the following is to avoid the case that pCallLeg is destructed by
    //another thread and reused without unlocking calllegTripleLock. In this
    //function tripleLock is reset to NULL which leads problem for
    //CallLegUnLockMsg. It fails when pCallLeg->tripleLock is null. At this
    //moment deadlock happens.

    if(pCallLeg){
        int lockcnt;
        RvMutexGetLockCounter(&pCallLeg->callTripleLock.hLock,pMgr->pLogMgr, &lockcnt);
        if(lockcnt){
            RvMutexLock(&pCallLeg->callTripleLock.hLock,pMgr->pLogMgr);
            RvMutexUnlock(&pCallLeg->callTripleLock.hLock,pMgr->pLogMgr);
        }
        RvMutexGetLockCounter(&pCallLeg->callTripleLock.hProcessLock,pMgr->pLogMgr, &lockcnt);
        if(lockcnt){
            RvMutexLock(&pCallLeg->callTripleLock.hProcessLock,pMgr->pLogMgr);
            RvMutexUnlock(&pCallLeg->callTripleLock.hProcessLock,pMgr->pLogMgr);
        }
    }
#endif
    rv = RPOOL_GetPage(pMgr->hGeneralPool,0,&(pCallLeg->hPage));
    if(rv != RV_OK)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*initialize a list of transactions for this call*/
    if(RV_FALSE == bIsHidden)
    {
        pCallLeg->hGeneralTranscList = RLIST_ListConstruct(pMgr->hTranscHandlesPool);
        if(pCallLeg->hGeneralTranscList == NULL)
        {
            RPOOL_FreePage(pMgr->hGeneralPool,pCallLeg->hPage);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        pCallLeg->hGeneralTranscList = NULL;
    }
    
    pCallLeg->pMgr =  pMgr;
    pCallLeg->bIsHidden = bIsHidden;
    memset(&(pCallLeg->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));
    memset(&(pCallLeg->localAddr),0,sizeof(SipTransportObjLocalAddresses));
    memset(&(pCallLeg->receivedFrom),0,sizeof(SipTransportAddress));
	pCallLeg->receivedFrom.eTransport = RVSIP_TRANSPORT_UNDEFINED;
    SipTransportInitObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,&pCallLeg->outboundAddr);
    pCallLeg->eState =                     RVSIP_CALL_LEG_STATE_IDLE;
    pCallLeg->eLastState =                 RVSIP_CALL_LEG_STATE_IDLE;
    pCallLeg->ePrevState =                 RVSIP_CALL_LEG_STATE_IDLE;
    pCallLeg->eNextState =                 RVSIP_CALL_LEG_STATE_IDLE;
    pCallLeg->ePrackState =                RVSIP_CALL_LEG_PRACK_STATE_UNDEFINED;
    pCallLeg->hActiveTransc =              NULL;
    pCallLeg->hAppCallLeg =                NULL;
    pCallLeg->hToHeader =                  NULL;
    pCallLeg->hFromHeader =                NULL;
    pCallLeg->hLocalContact =              NULL;
    pCallLeg->hRemoteContact =             NULL;
    pCallLeg->hRedirectAddress =           NULL;
	SipCommonCSeqSetStep(0,&pCallLeg->localCSeq);
	SipCommonCSeqReset(&pCallLeg->remoteCSeq); 
    pCallLeg->eDirection =                 eDirection;
    pCallLeg->strCallId =                  UNDEFINED;
    pCallLeg->hashElementPtr =             NULL;
    pCallLeg->hRouteList =                 NULL;
    pCallLeg->hListPage        =           NULL_PAGE;
    pCallLeg->hOutboundMsg =               NULL;
    pCallLeg->hReceivedMsg =               NULL;
    pCallLeg->tripleLock                 = NULL;
    pCallLeg->callLegEvHandlers          = &(pMgr->callLegEvHandlers);
    pCallLeg->bAckRcvd                   = RV_FALSE;
    pCallLeg->bTerminateCallLeg          = RV_FALSE;
    pCallLeg->hHeadersListToSetInInitialRequest = NULL;
    pCallLeg->hActiveIncomingPrack       = NULL;
	CallLegResetIncomingRSeq(pCallLeg); 
	SipCommonCSeqReset(&pCallLeg->incomingRel1xxCseq); 
    pCallLeg->hTimeStamp                 = NULL;
#ifdef RV_SIP_AUTH_ON
    pCallLeg->pAuthListInfo   = NULL;
    pCallLeg->hListAuthObj    = NULL;
    pCallLeg->eAuthenticationOriginState = RVSIP_CALL_LEG_STATE_UNDEFINED;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    pCallLeg->cctContext = -1;
    pCallLeg->URI_Cfg_Flag = 0xFF;
    SipCommonCoreRvTimerInit(&pCallLeg->connectTimer);
#endif
/* SPIRENT_END */
#endif /* #ifdef RV_SIP_AUTH_ON */
    pCallLeg->hReplacesHeader                  = NULL;
    pCallLeg->statusReplaces                   = RVSIP_CALL_LEG_REPLACES_UNDEFINED;
    pCallLeg->pSessionTimer                    = NULL;
    pCallLeg->pNegotiationSessionTimer         = NULL;
    pCallLeg->hTcpConn                         = NULL;
    pCallLeg->bIsPersistent                    = pMgr->bIsPersistent;
#ifdef RV_SIGCOMP_ON
    pCallLeg->bSigCompEnabled                  = RV_TRUE;
#endif

#ifdef SIP_DEBUG
    pCallLeg->pCallId                          = NULL;
#endif
    /*forking params */
    pCallLeg->bForkingEnabled = pMgr->bEnableForking;
    pCallLeg->bOriginalCall = RV_TRUE;
    pCallLeg->hOriginalForkedCall = NULL;
    pCallLeg->originalForkedCallUniqueIdentifier = UNDEFINED;
    SipCommonCoreRvTimerInit(&pCallLeg->forkingTimer);;
    pCallLeg->forked1xxTimerTimeout = pMgr->forked1xxTimerTimeout;

    pCallLeg->hOriginalContact = NULL;
    pCallLeg->hOriginalContactPage = NULL;
    pCallLeg->bForceOutboundAddr   = RV_FALSE;
    pCallLeg->bAddAuthInfoToMsg    = RV_TRUE; /*by default, the information is added*/

    pCallLeg->pTimers = NULL;
    pCallLeg->createdRejectStatusCode = 0;
    pCallLeg->bInviteReSentInCB = RV_FALSE;
    pCallLeg->bUseFirstRouteForInitialRequest = RV_FALSE;
    pCallLeg->hActiveTrx = NULL;

#ifdef RV_SIP_IMS_ON
    pCallLeg->hInitialAuthorization = NULL;
	pCallLeg->hSecAgree             = NULL;
	pCallLeg->hMsgRcvdTransc        = NULL;
	pCallLeg->hSecObj               = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/
    pCallLeg->cbBitmap = 0;

    /*initialize a list of invite objects for this call-leg */
    if(RV_FALSE == bIsHidden)
    {
        RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);
        pCallLeg->hInviteObjsList = RLIST_ListConstruct(pMgr->hInviteObjsListPool);
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

        if (NULL == pCallLeg->hInviteObjsList)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInitialize: call-leg 0x%p, failed to allocate invite objects List", pCallLeg));
            RPOOL_FreePage(pMgr->hGeneralPool,pCallLeg->hPage);
            RLIST_ListDestruct(pMgr->hTranscHandlesPool,pCallLeg->hGeneralTranscList);
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    else
    {
        pCallLeg->hInviteObjsList = NULL;
    }
    
    pCallLeg->tripleLock = &pCallLeg->callTripleLock;

#ifdef RV_SIP_SUBS_ON
    InitCallLegSubsData(pCallLeg, bIsHidden);
#endif /* #ifdef RV_SIP_SUBS_ON */
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpInitializeMsgSendingParams(&pCallLeg->sctpParams);
#endif
    /* Check whether the session timer is supported*/
    if (pMgr->eSessiontimeStatus == RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED &&
        RV_FALSE == bIsHidden)
    {
        /*Initialize the session timer object in the call leg*/
        rv = CallLegSessionTimerInitialize(pCallLeg);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegInitialize - Failed for Call-leg 0x%p, Error = %d in function RvSipMinSEHeaderConstruct"
                ,pCallLeg,rv));
            RPOOL_FreePage(pMgr->hGeneralPool,pCallLeg->hPage);
            RLIST_ListDestruct(pMgr->hTranscHandlesPool,pCallLeg->hGeneralTranscList);
            RLIST_ListDestruct(pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList);
            return rv;
        }

    }

    rv = SipTransportLocalAddressInitDefaults(pCallLeg->pMgr->hTransportMgr,
                                              &pCallLeg->localAddr);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegInitialize - Failed for Call-leg 0x%p, Error = %d in function SipTransportLocalAddressInitDefaults"
                  ,pCallLeg,rv));

        RPOOL_FreePage(pMgr->hGeneralPool,pCallLeg->hPage);
        RLIST_ListDestruct(pMgr->hTranscHandlesPool,pCallLeg->hGeneralTranscList);
        RLIST_ListDestruct(pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList);
        return rv;

    }
    rv = RPOOL_GetPage(pMgr->hElementPool,0,&(pCallLeg->hRemoteContactPage));
    if(rv != RV_OK)
    {
        RPOOL_FreePage(pMgr->hGeneralPool,pCallLeg->hPage);
        RLIST_ListDestruct(pMgr->hTranscHandlesPool,pCallLeg->hGeneralTranscList);
        RLIST_ListDestruct(pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList);
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Set unique identifier only on initialization success */
    RvRandomGeneratorGetInRange(pCallLeg->pMgr->seed, RV_INT32_MAX,
        (RvRandom*)&pCallLeg->callLegUniqueIdentifier);

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegInitialize - Call-leg 0x%p was initialized successfuly",
        pCallLeg));

    return RV_OK;

}

/***************************************************************************
 * CallLegVerifyValidity
 * ------------------------------------------------------------------------
 * General: verify that the call-leg we found is in the hash, not terminated.
 * Return Value: RV_TURE - legal. RV_FALSE - not legal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr           - The manager of the new call-leg
 *          pForkedCallLeg - The call-leg
 ***************************************************************************/
RvBool RVCALLCONV CallLegVerifyValidity( IN  CallLegMgr* pMgr,
                                         IN  CallLeg*    pCallLeg,
                                         IN  RvInt32     curIdentifier)
{
    /*the call-leg was destructed and is out of the hash and out the queue
    but it was constructed again so the locking succeed on the wrong
    object*/
    if(pCallLeg->callLegUniqueIdentifier != curIdentifier)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CallLegVerifyValidity - call-leg 0x%p uniqe identifier not equal.",
          pCallLeg));

        return RV_FALSE;
    }

    /*call leg is in the event queue (or will soon be in the queue)for termination
    we continue to process only for subscriptions that belongs to this call-leg*/
    else if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CallLegVerifyValidity - call-leg 0x%p is in state terminated.",
          pCallLeg));

        return RV_FALSE;
    }

    /* if the call-leg was took out of the hash, before we locked it */
    else if(pCallLeg->hashElementPtr == NULL)
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CallLegVerifyValidity - call-leg 0x%p is not in the hash.",
          pCallLeg));

        return RV_FALSE;
    }
    else if(RV_TRUE == CallLegGetIsHidden(pCallLeg))
    {
        RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
          "CallLegVerifyValidity - call-leg 0x%p - hidden call-leg.",
          pCallLeg));

        return RV_FALSE;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif

    return RV_TRUE;
}

/***************************************************************************
 * CallLegShouldRejectInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks if call-leg should reject the transaction. 
 *          This function will be called when the CSeq is out of order /
 *          when a required option is unsupported / When a Replaces header is
 *          in a request other then INVITE.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg handle
 *          hTransc  - The transaction handle
 *          responseCode - The response code
 *          eTranscState  - The transaction state.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV CallLegShouldRejectInvalidRequest(
                            IN  CallLeg               *pCallLeg,
                            IN  RvSipTranscHandle      hTransc,
                            OUT RvUint16                *pRejectCode,
                            OUT RvBool                 *pbCseqTooSmall)
{
    RvStatus				  rv;
    SipTransactionMethod	  eMethod;
    RvInt32					  transcCseq;
    RvBool					  bUnsupportedReq=RV_FALSE;
    RvSipTransactionState     eTranscState;
	RvSipMsgHandle            hMsg;
        
    *pRejectCode = 0;
    *pbCseqTooSmall = RV_FALSE;

    SipTransactionGetMethod(hTransc,&eMethod);
    RvSipTransactionGetCurrentState(hTransc, &eTranscState);

    
    /* check the required extension of remote UA - 420 response */
    RvSipTransactionIsUnsupportedExtRequired(hTransc,&bUnsupportedReq);
    if(bUnsupportedReq == RV_TRUE)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - required extension of remote UA are unsupported",
                pCallLeg));
        *pRejectCode = 420;
    }

	rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* check if a general request contains Replaces header - 400 response */
    if((eTranscState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD) &&
       (pCallLeg->pMgr->statusReplaces != RVSIP_CALL_LEG_REPLACES_UNDEFINED))
    {
        RvSipHeaderListElemHandle elem;
        void                     *hReplacesHeader = NULL;

        hReplacesHeader = RvSipMsgGetHeaderByType(hMsg,
                                                  RVSIP_HEADERTYPE_REPLACES,
                                                  RVSIP_FIRST_HEADER,
                                                  &elem);
        if(hReplacesHeader != NULL)
        {
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - general request contains Replaces header",
                pCallLeg));
            *pRejectCode = 400;
        }

    }

    /* check if the cseq is illegal (if there is no default response) - 400 response */
    RvSipTransactionGetCSeqStep(hTransc,&transcCseq);
    

#ifdef NO_CSEQ_CHECK_FOR_SUBS_NOTIFY/*patch for sanyo*/
                                    /* do not check the cseq value for subscription notify */
     
    if(eMethod != SIP_TRANSACTION_METHOD_NOTIFY ||
       SipTransactionGetSubsInfo(hTransc) == NULL)
    {
#endif /*#ifdef NO_CSEQ_CHECK_FOR_SUBS_NOTIFY */
    if(*pRejectCode == 0)
    {
		RvBool bCSeqIsSmall = SipCommonCSeqIsIntSmaller(&pCallLeg->remoteCSeq,transcCseq,RV_TRUE); 

        if(eMethod != SIP_TRANSACTION_METHOD_CANCEL && bCSeqIsSmall == RV_TRUE)
        {
            CallLegTranscMemory* pMemory;
			RvInt32 remoteCSeq = UNDEFINED;
			SipCommonCSeqGetStep(&pCallLeg->remoteCSeq, &remoteCSeq);
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - message received with cseq %d <= remoteCSeq of call-leg %d",
                pCallLeg, transcCseq,remoteCSeq));
            *pRejectCode = 500;
            *pbCseqTooSmall = RV_TRUE;
            /* such a transaction should not delete the route list on the reject response*/
            rv = CallLegAllocTransactionCallMemory(hTransc, &pMemory);
            if(rv != RV_OK || NULL == pMemory)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - cannot set the transaction context, when an INVITE is received in state %s",
                    pCallLeg, CallLegGetStateName(pCallLeg->eState))); 
            }
            else
            {
                pMemory->dontDestructRouteList = RV_TRUE;
            }         
        }
    }
#ifdef NO_CSEQ_CHECK_FOR_SUBS_NOTIFY 
    }
#endif /*#ifdef NO_CSEQ_CHECK_FOR_SUBS_NOTIFY */

    /* check if the top record-route header is legal (if there is no default response)
       - 400 response */
    if(*pRejectCode == 0)
    {
        RvSipRouteHopHeaderHandle  *phRecordRoute = NULL;
        RLIST_ITEM_HANDLE           pItem        = NULL;

        if(CallLegRouteListIsEmpty(pCallLeg) == RV_FALSE)
        {
            /*get the first address*/
            RLIST_GetHead(NULL, pCallLeg->hRouteList,&pItem);
            phRecordRoute = (RvSipRouteHopHeaderHandle*)pItem;
            if(NULL != phRecordRoute)
            {
                if(RV_TRUE == RvSipHeaderIsBadSyntax((void*)*phRecordRoute,RVSIP_HEADERTYPE_ROUTE_HOP))
                {
                    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                            "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - top record-route is bad-syntax",
                            pCallLeg));
                    *pRejectCode = 400;
                }
            }
        }
    }

#ifdef RV_SIP_IMS_ON
	if (*pRejectCode == 0)
	{
		rv = SipSecAgreeMgrIsInvalidRequest(pCallLeg->pMgr->hSecAgreeMgr, hMsg, pRejectCode);
		if (RV_OK != rv)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - failed to check message 0x%p",
                       pCallLeg, hMsg)); 
			return rv;
		}
		if (*pRejectCode != 0)
		{
			RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegShouldRejectInvalidRequest - Call-leg 0x%p - more than one Via in message with security-agreement required",
                       pCallLeg));
		}
	}
#endif /* #ifdef RV_SIP_IMS_ON */
        
    return RV_OK;
}
                            
/***************************************************************************
 * CallLegTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object, Free the resources taken by it and
 *          remove it from the manager call-leg list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 ***************************************************************************/
void RVCALLCONV CallLegTerminate(IN  CallLeg    *pCallLeg,
                                 IN RvSipCallLegStateChangeReason eReason)
{
    if (pCallLeg == NULL)
    {
        return;
    }
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        return;
    }
    /*terminate all the call-leg transactions*/
    
    if(RV_FALSE== pCallLeg->bIsHidden)
    {
        CallLegTerminateAllGeneralTransc(pCallLeg);
        CallLegTerminateAllInvites(pCallLeg, RV_TRUE);
    }
    pCallLeg->eLastState = pCallLeg->eState;
    pCallLeg->eState = RVSIP_CALL_LEG_STATE_TERMINATED;
    CallLegTerminateIfPossible(pCallLeg, eReason);
}

/***************************************************************************
 * CallLegTerminateIfPossible
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object if possible. If there are transactions in
 *          the call leg transaction list or subscription in the call leg then the call
 *          leg will not be terminated. The last subscription / transaction in the call
 *          leg will terminate the call leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to terminate.
 ***************************************************************************/
void CallLegTerminateIfPossible(IN  CallLeg                     *pCallLeg,
                                IN RvSipCallLegStateChangeReason eReason)
{
    RvBool               isSubsEmpty = RV_TRUE;
    RLIST_ITEM_HANDLE    pTranscItem       = NULL;
    RLIST_POOL_HANDLE    hTranscPool;
    RLIST_ITEM_HANDLE    pInviteItem       = NULL;
    RLIST_POOL_HANDLE    hInvitePool;

    if(RV_FALSE == pCallLeg->bIsHidden)
    {
        hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;
        RLIST_GetHead(hTranscPool, pCallLeg->hGeneralTranscList, &pTranscItem);
        hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
        RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pInviteItem);
    }
#ifdef RV_SIP_SUBS_ON
    TerminateCallLegSubsReferIfNeeded(pCallLeg,&isSubsEmpty);
#endif /* #ifdef RV_SIP_SUBS_ON */

    /* no subscriptions, transactions and invite objects - may free the call-leg */
    if(isSubsEmpty == RV_TRUE && pTranscItem == NULL && pInviteItem == NULL) 
    {
#ifdef RV_SIP_IMS_ON 
		/* notify the security-agreement on call-leg's termination */
        CallLegSecAgreeDetachSecAgree(pCallLeg);
        /* Detach from Security Object */
        if (NULL != pCallLeg->hSecObj)
        {
            RvStatus rv;
            rv = CallLegSetSecObj(pCallLeg, NULL);
            if (RV_OK != rv)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                    "CallLegTerminateIfPossible - CallLeg 0x%p: Failed to unset Security Object %p",
                    pCallLeg, pCallLeg->hSecObj));
            }
        }
#endif /* #ifdef RV_SIP_IMS_ON */

        /*remove call-leg from hash*/
        CallLegRemoveFromHash(pCallLeg);

        /*detach from the connection  - we do not want to get connection events*/
        if(pCallLeg->hTcpConn != NULL)
        {
            RvSipTransportConnectionDetachOwner(pCallLeg->hTcpConn,
                (RvSipTransportConnectionOwnerHandle)pCallLeg);
            pCallLeg->hTcpConn = NULL;
        }

        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTerminateIfPossible - Send Call-leg 0x%p to the event queue for termination"
                ,pCallLeg));
        SipTransportSendTerminationObjectEvent(pCallLeg->pMgr->hTransportMgr,
                                   (void *)pCallLeg, 
                                   &(pCallLeg->terminationInfo),
                                   (RvInt32)eReason,
                                   TerminateCallegEventHandler,
                                   RV_TRUE,
                                   CALL_LEG_STATE_TERMINATED_STR,
                                   CALL_LEG_OBJECT_NAME_STR);
    }
}

/***************************************************************************
 * CallLegRemoveFromHash
 * ------------------------------------------------------------------------
 * General: remove the call-leg from the hash table..
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to remove from hash.
 ***************************************************************************/
void RVCALLCONV CallLegRemoveFromHash(IN CallLeg* pCallLeg)
{
    if(pCallLeg->hashElementPtr != NULL)
    {
        RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRemoveFromHash - Remove Call-leg 0x%p from hash"
            ,pCallLeg));
        HASH_RemoveElement(pCallLeg->pMgr->hHashTable,pCallLeg->hashElementPtr);
        pCallLeg->hashElementPtr = NULL;
        RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    }
}
/***************************************************************************
 * CallLegDestruct
 * ------------------------------------------------------------------------
 * General: Free the call-leg resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to free.
 ***************************************************************************/
void RVCALLCONV CallLegDestruct(IN  CallLeg    *pCallLeg)
{
    RvStatus rv = RV_OK;
#ifdef SIP_DEBUG
    if (NULL == pCallLeg)
    {
        return;
    }
#endif

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDestruct - Call-leg 0x%p destructing..." ,pCallLeg));

    pCallLeg->callLegUniqueIdentifier = 0;

    if(pCallLeg->hHeadersListToSetInInitialRequest != NULL)
    {
        RvSipCommonListDestruct(pCallLeg->hHeadersListToSetInInitialRequest);
        pCallLeg->hHeadersListToSetInInitialRequest = NULL;
    }

    if(pCallLeg->hListPage != NULL_PAGE)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool,pCallLeg->hListPage);
    }

    /* destruct the ACK message, transmitter and transmitter page - if exists */
    if (SipCommonCoreRvTimerStarted(&pCallLeg->forkingTimer))
    {
        CallLegForkingTimerRelease(pCallLeg);
    }

#ifdef RV_SIP_AUTH_ON
    if (NULL != pCallLeg->hListAuthObj)
    {
        SipAuthenticatorAuthListResetPasswords(pCallLeg->hListAuthObj);
        SipAuthenticatorAuthListDestruct(pCallLeg->pMgr->hAuthMgr, pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj);
        pCallLeg->hListAuthObj = NULL;
    }

#endif /* #ifdef RV_SIP_AUTH_ON */

    CallLegSessionTimerReleaseTimer(pCallLeg);

    /*detach from the connection  - we do not want to get connection events*/
    if(pCallLeg->hTcpConn != NULL)
    {
        RvSipTransportConnectionDetachOwner(pCallLeg->hTcpConn,
            (RvSipTransportConnectionOwnerHandle)pCallLeg);
        pCallLeg->hTcpConn = NULL;
    }

#ifdef RV_SIP_IMS_ON 
    /* Detach from Security Object */
    if (NULL != pCallLeg->hSecObj)
    {
        rv = CallLegSetSecObj(pCallLeg, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegDestruct - CallLeg 0x%p: Failed to unset Security Object %p",
                pCallLeg, pCallLeg->hSecObj));
        }
    }
#endif /* #ifdef RV_SIP_IMS_ON */

    /* Destruct the call-leg's Invite objects, otherwise their timer might  */
    /* be expired through (or after) the SIP Stack shutdown, but before the */
    /* common core is destructed */ 
	DestructAllInvites(pCallLeg); 
    
       /*the call should be removed from the manager call list*/
    RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);

    CallLegMgrUpdateObjCounters(pCallLeg->pMgr,CallLegGetIsHidden(pCallLeg),RV_FALSE);
    rv = CallLegMgrCheckObjCounters(pCallLeg->pMgr,CallLegGetIsHidden(pCallLeg),RV_FALSE);
    if (rv != RV_OK)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDestruct: pCallLeg=0x%p destruction may cause inconsistency in CallLegMgr counters",
            pCallLeg));
    }

    /*destruct the list of transactions*/
    if(RV_FALSE == pCallLeg->bIsHidden)
    {
        RLIST_ListDestruct(pCallLeg->pMgr->hTranscHandlesPool,pCallLeg->hGeneralTranscList);
        RLIST_ListDestruct(pCallLeg->pMgr->hInviteObjsListPool,pCallLeg->hInviteObjsList);
    }

    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg)
    {
        RvSipMsgDestruct(pCallLeg->hOutboundMsg);
        pCallLeg->hOutboundMsg = NULL;
    }

    if(pCallLeg->hashElementPtr != NULL)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegDestruct - Call-leg 0x%p was removed from hash (hashElem 0x%p)"
                ,pCallLeg,pCallLeg->hashElementPtr));
        HASH_RemoveElement(pCallLeg->pMgr->hHashTable,pCallLeg->hashElementPtr);
    }
    pCallLeg->hashElementPtr = NULL;
    RLIST_Remove(pCallLeg->pMgr->hCallLegPool, pCallLeg->pMgr->hCallLegList,
                 (RLIST_ITEM_HANDLE)pCallLeg);

    
    /*free the memory page allocated for this call*/
    RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool,pCallLeg->hPage);
    RPOOL_FreePage(pCallLeg->pMgr->hElementPool,pCallLeg->hRemoteContactPage);
    if(pCallLeg->hOriginalContactPage != NULL)
    {
        RPOOL_FreePage(pCallLeg->pMgr->hElementPool,pCallLeg->hOriginalContactPage);
    }

    /*unlock the mgr after releasing all information (including pages).
      otherwise, the call-leg can be reallocated, before releasing the pages...*/    
    RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);

}

/***************************************************************************
 * CallLegGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given CallLeg object.
 *          The call-id is kept on the Call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGenerateCallId(IN  CallLeg*           pCallLeg)
{
    RvStatus          rv;
    HRPOOL             pool      = pCallLeg->pMgr->hGeneralPool;
    RvInt32           offset;
    RvChar            strCallId[SIP_COMMON_ID_SIZE];
    RvInt32           callIdSize;

    rv = SipTransactionGenerateIDStr(pCallLeg->pMgr->hTransportMgr,
                             pCallLeg->pMgr->seed,
                             (void*)pCallLeg,
                             pCallLeg->pMgr->pLogSrc,
                             (RvChar*)strCallId);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegGenerateCallId - Failed for Call-leg 0x%p, ",pCallLeg));
    }

    callIdSize = (RvInt32)strlen(strCallId);

    /*allocate a buffer on the call-leg page that will be used for the
      first part of the call-id and place the call-Id there.*/
	
	rv = RPOOL_AppendFromExternal(pool,pCallLeg->hPage,strCallId, callIdSize+1,RV_FALSE,&offset,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegGenerateCallId - Call-leg 0x%p failed in RPOOL_AppendFromExternal (rv=%d)",pCallLeg,rv));
        return rv;
    }
    pCallLeg->strCallId = offset;

#ifdef SIP_DEBUG
    pCallLeg->pCallId = RPOOL_GetPtr(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                                     pCallLeg->strCallId);
#endif
    return RV_OK;
}
/***************************************************************************
 * CallLegIsInitialRequest
 * ------------------------------------------------------------------------
 * General: returns RV_TURE if this is an initial request - request with
 *          an empty to-tag.
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 ***************************************************************************/
RvBool RVCALLCONV CallLegIsInitialRequest(CallLeg* pCallLeg)
{
    if(0 < RvSipPartyHeaderGetStringLength(pCallLeg->hToHeader, RVSIP_PARTY_TAG))
    {
        /* the call-leg has a to-tag --> this is not an initial request. */
        return RV_FALSE;
    }
    return RV_TRUE;
}
/***************************************************************************
 * CallLegGetRequestURI
 * ------------------------------------------------------------------------
 * General: Retrieves the request URI from a call-leg.
 * Return Value: RV_ERROR_UNKNOWN - Can't find a request URI.
 *               RV_OK - Request URI was retrieved successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *          hTransc  - The handle to the transaction. Could be NULL.
 * Output:  phAddress - Handle to the result request URI address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRequestURI(IN  CallLeg*           pCallLeg,
                                         IN  RvSipTranscHandle  hTransc,
                                         OUT RvSipAddressHandle *phAddress,
                                         OUT RvBool*   pbSendToFirstRouteHeader)
{
    RvStatus rv;
    RvBool   bSendToFirstRouteHeader = RV_FALSE;
    RvBool   bInitialRequest = CallLegIsInitialRequest(pCallLeg);
    
    if (RV_TRUE == pCallLeg->bUseFirstRouteForInitialRequest &&
        RV_TRUE == bInitialRequest)
    {
        rv = RvSipTransactionSendToFirstRoute(hTransc, RV_TRUE);  
    }
    
    if(CallLegRouteListIsEmpty(pCallLeg))
    {
        if(RV_FALSE == bInitialRequest)
        {
            /* we shall use the outbound proxy only for initial requests.
               (proxies that want to get non-initial requests too, should 
               add a record-route header...) */
            RvSipTransactionIgnoreOutboundProxy(hTransc,RV_TRUE);
        }
         /*if there is contact then this is the request URI*/
        rv = CallLegGetRemoteTargetURI(pCallLeg, phAddress);
    }
    else
    {
        rv = CallLegRouteListGetRequestURI(pCallLeg, phAddress, &bSendToFirstRouteHeader);
        if(rv == RV_OK && hTransc != NULL)
        {
            rv = RvSipTransactionSendToFirstRoute(hTransc, bSendToFirstRouteHeader);   
        }
    }
    if(pbSendToFirstRouteHeader != NULL)
    {
        *pbSendToFirstRouteHeader = bSendToFirstRouteHeader;
    }
    return rv;
}

/***************************************************************************
 * CallLegGetRemoteTargetURI
 * ------------------------------------------------------------------------
 * General: Retrieves the remote target request URI from a call-leg.
 *          If there is a remote contact it will be the request URI. If not
 *          The From / To address will be the request URI.
 * Return Value: RV_ERROR_UNKNOWN - Can't find the remote target request URI.
 *               RV_OK - Remote target request URI was retrieved successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 * Output:  phAddress - Handle to the result request URI address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRemoteTargetURI(IN  CallLeg*           pCallLeg,
                                               OUT RvSipAddressHandle *phAddress)
{
    RvSipPartyHeaderHandle  hRemoteParty = NULL;

    if(pCallLeg->hRemoteContact != NULL)
    {
        *phAddress = pCallLeg->hRemoteContact;
        return RV_OK;
    }
    /* If no request URI exists, extract the remote party's address
    from the To or From fields according to the call-leg direction.*/
    if(pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_OUTGOING)
    {
        hRemoteParty = pCallLeg->hToHeader;
    }
    else
    {
        hRemoteParty = pCallLeg->hFromHeader;
    }

    if(hRemoteParty != NULL)
    {
        *phAddress = RvSipPartyHeaderGetAddrSpec(hRemoteParty);
        /*if there is no address in the party header the function fails*/
        if(*phAddress == NULL)
        {
            return RV_ERROR_UNKNOWN;
        }
        else
        {
            return RV_OK;
        }
    }
    else
    {
        *phAddress = NULL;
        return RV_ERROR_UNKNOWN;
    }
}

/***************************************************************************
 * CallLegSaveReceivedFromAddr
 * ------------------------------------------------------------------------
 * General: saves the address from which the last message was received.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg  -- Handle to the call-leg.
 *            pRecvFrom -- the address from which a message was received
 ***************************************************************************/
void RVCALLCONV CallLegSaveReceivedFromAddr(
                                      IN  CallLeg*              pCallLeg,
                                      IN  SipTransportAddress*  pRecvFrom)
{
    memcpy(&pCallLeg->receivedFrom,pRecvFrom,sizeof(pCallLeg->receivedFrom));
}

/***************************************************************************
 * CallLegSendRequest
 * ------------------------------------------------------------------------
 * General: Sends a request to the remote party. First the Call-leg's request
 *          URI is determined and then a request is sent using the transaction API.
 * Return Value: RV_ERROR_UNKNOWN - An error occurred. The request message was not
 *                           sent.
 *               RV_OK - Request message was sent successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - The call-leg sending the request.
 *            hTransc - The transaction used to send the request.
 *            eRequestMethod - The request method to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSendRequest(
                            IN CallLeg                   *pCallLeg,
                            IN RvSipTranscHandle          hTransc,
                            IN SipTransactionMethod       eRequestMethod,
                            IN RvChar*                    strMethod)


{
    RvStatus          rv;
    RvSipAddressHandle hReqURI;
    
    /*----------------------------------------------
       get the request URI from the call-leg
    ------------------------------------------------*/
    rv = CallLegGetRequestURI(pCallLeg, hTransc, &hReqURI, NULL);
    if (rv != RV_OK || hReqURI == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSendRequest - Failed to send request %d for Call-leg 0x%p, no request URI available",
                  pCallLeg, eRequestMethod));
        return RV_ERROR_UNKNOWN;
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(eRequestMethod == SIP_TRANSACTION_METHOD_NOTIFY || eRequestMethod == SIP_TRANSACTION_METHOD_SUBSCRIBE)
		SipAddrUrlResetCompIdParam(hReqURI);		
#endif
/* SPIRENT_END */

    /*----------------------------------
         send the request
     -----------------------------------*/
    /* If the application created a message in advance use it*/
    if (NULL != pCallLeg->hOutboundMsg &&
        SipTransactionGetSubsInfo(hTransc) == NULL &&
        SipTransactionIsAppInitiator(hTransc) == RV_FALSE)
    {
        rv = SipTransactionSetOutboundMsg(hTransc, pCallLeg->hOutboundMsg);
        if (RV_OK != rv)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegSendRequest - Failed for Call-leg 0x%p, transc 0x%p failed to set outbound message (error code %d)"
                      ,pCallLeg,hTransc,rv));
            return rv;
        }
        pCallLeg->hOutboundMsg = NULL;
    }

    /* set call-leg parameters in transaction outgoing message */
    rv = CallLegSetHeadersInOutboundRequestMsg(pCallLeg, hTransc, eRequestMethod);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSendRequest - Call-leg 0x%p, transc 0x%p failed to set call-leg params in msg"
                 ,pCallLeg,hTransc));
        return rv;
    }

    if(strMethod == NULL)
    {
        rv = SipTransactionRequest(hTransc,eRequestMethod,hReqURI);
    }
    else
    {
        rv = RvSipTransactionRequest(hTransc,strMethod,hReqURI);
    }
    /*The state at this point should be inviting due to a transaction state
      change.*/
    if (RV_OK != rv)
    {
        /*transaction failed to send request*/
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegSendRequest - Failed for Call-leg 0x%p, transc 0x%p failed to send request 0x%p (error code %d)"
                 ,pCallLeg,hTransc,eRequestMethod,rv));
        return rv;
    }

    return RV_OK;

}


/***************************************************************************
 * CallLegDisconnectWithNoBye
 * ------------------------------------------------------------------------
 * General: This function is used to disconnect a call-leg with out sending
 *          BYE first. Usually it will be called when the disconnection is
 *          a result of an error or a time out. The terminate function should
 *          be called after this function to terminate the call-leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg to disconnect with no bye.
 *            eReason - the disconnecting reason.
 * Output:(-).
 ***************************************************************************/

void RVCALLCONV CallLegDisconnectWithNoBye(
                             IN  CallLeg*                       pCallLeg,
                             IN  RvSipCallLegStateChangeReason  eReason)
{
    if(RVSIP_CALL_LEG_STATE_TERMINATED == pCallLeg->eState ||
        RVSIP_CALL_LEG_STATE_DISCONNECTED == pCallLeg->eState)
    {
        return;
    }

    if(RVSIP_CALL_LEG_STATE_IDLE != pCallLeg->eState)
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_DISCONNECTED, eReason);

    }
    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegDisconnectWithNoBye - Call-leg 0x%p: Disconnected with out BYE with reason %s",
              pCallLeg,CallLegGetStateChangeReasonName(eReason)));
}


/***************************************************************************
 * CallLegChangeState
 * ------------------------------------------------------------------------
 * General: Changes the call-leg state and notify the application about it.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegChangeState(
                                 IN  CallLeg                      *pCallLeg,
                                 IN  RvSipCallLegState             eState,
                                 IN  RvSipCallLegStateChangeReason eReason)
{
    /*when the state is terminated it was already updated in the call-leg*/
    if(eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegChangeState - Call-leg 0x%p state changed: %s->%s, (reason = %s)"
              ,pCallLeg, CallLegGetStateName(pCallLeg->eLastState),CallLegGetStateName(eState),
               CallLegGetStateChangeReasonName(eReason)));

    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegChangeState - Call-leg 0x%p state changed: %s->%s, (reason = %s)"
              ,pCallLeg, CallLegGetStateName(pCallLeg->eState),CallLegGetStateName(eState),
               CallLegGetStateChangeReasonName(eReason)));

    }
    /*change state and reason in the call record*/
    pCallLeg->ePrevState = pCallLeg->eState;
    pCallLeg->eState     = eState;

	if(pCallLeg->ePrevState == RVSIP_CALL_LEG_STATE_OFFERING &&
		pCallLeg->eState == RVSIP_CALL_LEG_STATE_DISCONNECTED)
	{
	   /*an initial invite was rejected, remove call-leg from hash*/
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegChangeState - Call-leg 0x%p - an initial invite was rejected. the call-leg is removed from hash",
			 pCallLeg));
		CallLegRemoveFromHash(pCallLeg);
	}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    // reset the 'connectTimer' if the call is about to transfer from "INVITING" to another state.
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_INVITING &&
       eState != RVSIP_CALL_LEG_STATE_INVITING)
    {
       if (SipCommonCoreRvTimerStarted(&pCallLeg->connectTimer))
       {
           SipCommonCoreRvTimerCancel(&pCallLeg->connectTimer);
       }
    }
#endif //defined(UPDATED_BY_SPIRENT)
//SPIRENT_END


    pCallLeg->eLastState = eState;

    CallLegCallbackChangeCallStateEv(pCallLeg,eState, eReason);

#ifdef RV_SIP_SUBS_ON
    if (RVSIP_CALL_LEG_STATE_TERMINATED == eState)
    {
        NotifyReferGeneratorInTerminateIfNeeded(pCallLeg,eReason);
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * CallLegChangeModifyState
 * ------------------------------------------------------------------------
 * General: Changes the call-leg connect sub state.
 *          The connect sub state is used for incoming and outgoing re-Invite
 *          messages.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            pInvite  - Pointer to the invite object.
 *            eSubState - The Modify state.
 *            eReason  - The state change reason.
 *            hReceivedMsg - The received message, that caused the state change.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV CallLegChangeModifyState(
                                 IN  CallLeg                       *pCallLeg,
                                 IN  CallLegInvite                 *pInvite,
                                 IN  RvSipCallLegModifyState        eModifyState,
                                 IN  RvSipCallLegStateChangeReason  eReason)
{
    if(eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED)
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegChangeModifyState - Call-leg 0x%p. Invite 0x%p. Modify State change: %s->%s (reason=%s)",
            pCallLeg, pInvite, 
            (pInvite==NULL)?"":CallLegGetModifyStateName(pInvite->eLastModifyState),
            CallLegGetModifyStateName(eModifyState),
            CallLegGetStateChangeReasonName(eReason)));
        
    }
    else
    {
        RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegChangeModifyState - Call-leg 0x%p. Invite 0x%p. Modify State change: %s->%s (reason=%s)",
              pCallLeg, pInvite, 
              (pInvite==NULL)?"":CallLegGetModifyStateName(pInvite->eModifyState),
              CallLegGetModifyStateName(eModifyState),
              CallLegGetStateChangeReasonName(eReason)));
    }
    
    if(pCallLeg->pMgr->bOldInviteHandling == RV_TRUE)
    {
        /*for old invite behavior we need to inform on state idle once, in the end of re-invite
          process, and to not inform on states TERMINATED, ACK_SENT and ACK_RCVD (those are new
          states, that were not exist in the old invite implementation).
          since both states ACK_SENT and TERMINATED are usually called, we might inform of state idle
          twice, for this purpose we set bModifyIdleInOldInvite that tells us if we already informed 
          application on state idle or not */
        RvSipCallLegModifyState eStateToInform = eModifyState;
        
        /* Old invite changed state to idle in the end of re-invite process */
        if(pInvite != NULL)
        {
           if(eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED ||
               eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT ||
               eModifyState == RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD)
            {
                if(pInvite->bModifyIdleInOldInvite == RV_TRUE)
                {
                    /* if we already informed on state idle, we have nothing to do here... */
                    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                        "CallLegChangeModifyState - Call-leg 0x%p invite 0x%p. do nothing with state %s. already informed on state Idle",
                        pCallLeg, pInvite, CallLegGetModifyStateName(eModifyState)));
                    return;
                }
                else
                {
                    eStateToInform = RVSIP_CALL_LEG_MODIFY_STATE_IDLE;
                    pInvite->bModifyIdleInOldInvite = RV_TRUE;
                }
            }

            pInvite->eLastModifyState = pInvite->eModifyState;
            pInvite->ePrevModifyState = pInvite->eModifyState;
            pInvite->eModifyState = eModifyState;    

            RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegChangeModifyState - Call-leg 0x%p. Invite 0x%p. (internal) inform of Modify State %s (reason=%s)",
                pCallLeg, pInvite, 
                CallLegGetModifyStateName(eStateToInform),
                CallLegGetStateChangeReasonName(eReason)));
        }
        /* use the old callback function */
        CallLegCallbackChangeModifyStateEv(pCallLeg, eStateToInform, eReason);
    }
    else
    {
        if(pInvite == NULL)
        {
            /* this is the initial invite. do not call the callback */
            return;
        }
        pInvite->eLastModifyState = pInvite->eModifyState;
        pInvite->ePrevModifyState = pInvite->eModifyState;
        pInvite->eModifyState = eModifyState;

        /* use the new callback function */
        CallLegCallbackChangeReInviteStateEv(pCallLeg, pInvite, 
                                            eModifyState, eReason);
    }
}
/***************************************************************************
 * CallLegAttachOnConnection
 * ------------------------------------------------------------------------
 * General: Attach the call-leg as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus CallLegAttachOnConnection(IN CallLeg                        *pCallLeg,
                                    IN RvSipTransportConnectionHandle  hConn)
{

    RvStatus rv;
    RvSipTransportConnectionEvHandlers evHandler;
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler = CallLegConnStateChangedEv;
    /*evHandler.pfnConnStausEvHandler = CallLegConnStatusEv;*/
    /*attach to new conn*/
    rv = SipTransportConnectionAttachOwner(hConn,
                                           (RvSipTransportConnectionOwnerHandle)pCallLeg,
                                           &evHandler,sizeof(evHandler));

    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegAttachOnConnection - Failed to attach CallLeg 0x%p to connection 0x%p",pCallLeg,hConn));
        pCallLeg->hTcpConn = NULL;
    }
    else
    {
        pCallLeg->hTcpConn = hConn;
    }
    return rv;

}

/***************************************************************************
 * CallLegDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the call-leg from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Call-leg
 ***************************************************************************/
void CallLegDetachFromConnection(IN CallLeg        *pCallLeg)
{
    RvStatus rv;
    if(pCallLeg->hTcpConn == NULL)
    {
        return;
    }
    rv = RvSipTransportConnectionDetachOwner(pCallLeg->hTcpConn,
                                            (RvSipTransportConnectionOwnerHandle)pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegDetachFromConnection - Failed to detach call-leg 0x%p from prev connection 0x%p",pCallLeg,pCallLeg->hTcpConn));

    }
    pCallLeg->hTcpConn = NULL;
}


/***************************************************************************
 * CallLegConnStateChangedEv
 * ------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a connection state was changed.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hConn   -   The connection handle
 *          hOwner  -   Handle to the connection owner.
 *          eStatus -   The connection status
 *          eReason -   A reason for the new state or undefined if there is
 *                      no special reason for
 ***************************************************************************/
RvStatus RVCALLCONV CallLegConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hCallLeg,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{

    CallLeg*    pCallLeg    = (CallLeg*)hCallLeg;

    RV_UNUSED_ARG(eReason);

    if(pCallLeg == NULL)
    {
        return RV_OK;
    }
    if (CallLegLockMsg(pCallLeg) != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegConnStateChangedEv - call-leg 0x%p, conn=0x%p,Failed to lock call-leg",
                   pCallLeg,hConn));
        return RV_ERROR_UNKNOWN;
    }

    switch(eState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegConnStateChangedEv - CallLeg 0x%p received a connection closed event",pCallLeg));
        if (pCallLeg->hTcpConn == hConn)
        {
            pCallLeg->hTcpConn = NULL;
        }
        break;
    default:
        break;
    }
    CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}



/***************************************************************************
 * CallLegSetKey
 * ------------------------------------------------------------------------
 * General: Set the call leg key (to, from, call-id).
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - key was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            pKey - The given key.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetKey(
                            IN  CallLeg                       *pCallLeg,
                            IN  SipTransactionKey               *pKey)
{
        RvStatus rv;
        rv = CallLegSetFromHeader(pCallLeg,pKey->hFromHeader);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = CallLegSetToHeader(pCallLeg,pKey->hToHeader);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = CallLegSetCallId(pCallLeg,&(pKey->strCallId));
        if(rv != RV_OK)
        {
            return rv;
        }
        return RV_OK;

}

/*------------------------------------------------------------------------
           S T A T E   C H E C K I N G   F U N C T I O N S
 -------------------------------------------------------------------------*/
/***************************************************************************
 * CallLegIsStateLegalForReInviteHandling
 * ------------------------------------------------------------------------
 * General: Is the call-leg state is legal for handling an incoming re-invite,
 *          and for sending re-invite.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegIsStateLegalForReInviteHandling(CallLeg* pCallLeg)
{
    if(RV_FALSE == pCallLeg->pMgr->bOldInviteHandling)
    {
        if(pCallLeg->hActiveTransc != NULL)
        {
            /* There is an active re-invite or bye... */
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegIsStateLegalForReInviteHandling: call-leg 0x%p - illegal state. active transc=0x%p",
                pCallLeg, pCallLeg->hActiveTransc));
            return RV_FALSE;
        }
        switch(pCallLeg->eState)
        {
        case RVSIP_CALL_LEG_STATE_CONNECTED:
        case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
        case RVSIP_CALL_LEG_STATE_ACCEPTED:
            return RV_TRUE;
        default:
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegIsStateLegalForReInviteHandling: call-leg 0x%p - illegal state %s",
                pCallLeg, CallLegGetStateName(pCallLeg->eState)));
            return RV_FALSE;
        }
    }
    else
    {
        if(RVSIP_CALL_LEG_STATE_CONNECTED == pCallLeg->eState)
        {
            return RV_TRUE;
        }
        return RV_FALSE;
    }
}

/*------------------------------------------------------------------------
           T R A N S A C T I O N   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * CallLegCreateTransaction
 * ------------------------------------------------------------------------
 * General: creates a new transaction, attach the call as owner and insert
 *          the new transaction into the call-leg transactions list.
 * Return Value: RV_ERROR_UNKNOWN - If the call doesn't have all the information
 *                            needed to create the transaction for example
 *                            To header is missing or there is a general
 *                            failure when creating the transaction.
 *               RV_ERROR_OUTOFRESOURCES - Failed to create transaction because of
 *                                   resource problem.
 *               RV_OK        - Transaction was created successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer the call-leg creating the new transaction.
 *          bAddToTranscList - Add the transaction to call-leg transactions list
 *                      or not. (subscription and notify transaction will be saved
 *                      in their objects, and not in the call-leg's list)
 * Output:     *phTransc - Handle to the newly created transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegCreateTransaction(
                                       IN  CallLeg*           pCallLeg,
                                       IN  RvBool            bAddToTranscList,
                                       OUT RvSipTranscHandle  *phTransc)
{

    RvStatus           rv;
    SipTransactionKey key;

    /*a transaction can be created only if to and from are available*/
    if(pCallLeg->hFromHeader == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegCreateTransaction - Failed for Call Leg 0x%p: no From header",pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    if(pCallLeg->hToHeader == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "CallLegCreateTransaction - Failed for Call Leg 0x%p: no To header",pCallLeg));
        return RV_ERROR_UNKNOWN;
    }

    /*If there is still no call-id, generate it.*/
    if(pCallLeg->strCallId == UNDEFINED)
    {
        rv = CallLegGenerateCallId(pCallLeg);
        if(rv != RV_OK)
        {
             RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegCreateTransaction - Call Leg 0x%p: failed to generate new Call-ID (rv=%d)",
                       pCallLeg,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /*increase the CSeq.*/
	rv = SipCommonCSeqPromoteStep(&pCallLeg->localCSeq); 
	if (rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegCreateTransaction - Call Leg 0x%p: failed in SipCommonCSeqPromoteStep (rv=%d)",
			pCallLeg,rv));
		return rv;
	}
    
    /*prepare the transaction key, if the call is incoming the transaction
      should recieve reverse to and  from headers*/
    memset(&key,0,sizeof(key));
    rv = SipCommonCSeqGetStep(&pCallLeg->localCSeq,&key.cseqStep);
	if(rv != RV_OK)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			"CallLegCreateTransaction - Call Leg 0x%p: failure in SipCommonCSeqGet() (rv=%d)",
			pCallLeg,rv));
		return rv;
	}

    key.hFromHeader =  (pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)?
                       pCallLeg->hToHeader : pCallLeg->hFromHeader;
    key.hToHeader   =  (pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)?
                       pCallLeg->hFromHeader : pCallLeg->hToHeader;
    key.strCallId.offset = pCallLeg->strCallId;
    key.strCallId.hPage  = pCallLeg->hPage;
    key.strCallId.hPool  = pCallLeg->pMgr->hGeneralPool;
    key.localAddr        = pCallLeg->localAddr;


    /*create the transaction*/
    rv = SipTransactionCreate(pCallLeg->pMgr->hTranscMgr,&key,(void*)pCallLeg,
        SIP_TRANSACTION_OWNER_CALL,pCallLeg->tripleLock,phTransc);
    if(rv != RV_OK || phTransc == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCreateTransaction - Call Leg 0x%p failed to create transaction (rv=%d)",
              pCallLeg,rv));
        if(rv == RV_ERROR_OUTOFRESOURCES)
            return RV_ERROR_OUTOFRESOURCES;
        else
            return RV_ERROR_UNKNOWN;
    }
    /* set the call-leg outbound proxy to the transaction */
    rv = SipTransactionSetOutboundAddress(*phTransc, &pCallLeg->outboundAddr);
    if(rv != RV_OK)
    {
        SipTransactionTerminate(*phTransc);
        return rv;  /*error message will be printed in the function*/
    }

    /* set the call-leg transaction timers to the transaction */
    if(pCallLeg->pTimers != NULL)
    {
        rv = SipTransactionSetTimers(*phTransc, pCallLeg->pTimers);
        if(rv != RV_OK)
        {
            SipTransactionTerminate(*phTransc);
            return rv;  /*error message will be printed in the function*/
        }
    }
    if(pCallLeg->bForceOutboundAddr == RV_TRUE)
    {
        RvSipTransactionSetForceOutboundAddrFlag(*phTransc,RV_TRUE);
    }

    if(pCallLeg->bIsPersistent == RV_TRUE)
    {
        rv = RvSipTransactionSetPersistency(*phTransc,RV_TRUE);
        if(rv != RV_OK)
        {
            SipTransactionTerminate(*phTransc);
            return rv;  /*error message will be printed in the function*/
        }
    }
    if(pCallLeg->hTcpConn != NULL)
    {

        rv = RvSipTransactionSetConnection(*phTransc,pCallLeg->hTcpConn);
        if(rv != RV_OK)
        {
            SipTransactionTerminate(*phTransc);
            return rv;  /*error message will be printed in the function*/
        }
    }

#ifdef RV_SIGCOMP_ON
    if (pCallLeg->bSigCompEnabled == RV_FALSE)
    {
        RvSipTransactionDisableCompression(*phTransc);
    }
#endif /* RV_SIGCOMP_ON */

#if (RV_NET_TYPE & RV_NET_SCTP)
    SipTransactionSetSctpMsgSendingParams(*phTransc,&pCallLeg->sctpParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
    /* Set Security Object to be used  while sending messages */
    if (NULL != pCallLeg->hSecObj)
    {
        rv = RvSipTransactionSetSecObj(*phTransc, pCallLeg->hSecObj);
        if (RV_OK != rv)
        {
            SipTransactionTerminate(*phTransc);
            return rv;
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* IMPORTANT:This code should be called last before return.Don't move it!*/
    if(bAddToTranscList == RV_TRUE)
    {
        /*add the transaction handle to the transaction list*/
        rv = CallLegAddTranscToList(pCallLeg,*phTransc);
        if(rv != RV_OK)
        {
            SipTransactionTerminate(*phTransc);
            return RV_ERROR_OUTOFRESOURCES;  /*error message will be printed in the function*/
        }
    }
    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RvSipTransactionSetCct(*phTransc,pCallLeg->cctContext);
    RvSipTransactionSetURIFlag(*phTransc,pCallLeg->URI_Cfg_Flag);
#endif
    /* SPIRENT END */

    /*transaction was created successfully*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegCreateTransaction - Call Leg 0x%p created new transc 0x%p",
                 pCallLeg,*phTransc));

    return RV_OK;
}



/***************************************************************************
 * CallLegAddTranscToList
 * ------------------------------------------------------------------------
 * General: Adds a given transaction to the call-leg transaction list.
 * Return Value: RV_ERROR_OUTOFRESOURCES - List is full, failed to insert.
 *               RV_OK - Transaction was inserted to list successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hTransc - Handle to the transaction that should be inserted
 *                    into the list.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAddTranscToList(IN  CallLeg*           pCallLeg,
                                           IN  RvSipTranscHandle  hTransc)
{
    RLIST_ITEM_HANDLE  listItem;
    RvSipTranscHandle  *phTranscToList;
    RvStatus rv;

    /* Verify that the CallLeg is not terminated */
    if (pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegAddTranscToList - call-leg 0x%p is terminated and MUSTN'T own transc 0x%p",
              pCallLeg,hTransc));

        return RV_ERROR_ILLEGAL_ACTION;
    }

    /*get a new list element*/
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegAddTranscToList - call-leg 0x%p add transc 0x%p to transc list"
              ,pCallLeg,hTransc));

    RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    rv = RLIST_InsertTail(pCallLeg->pMgr->hTranscHandlesPool,
                                    pCallLeg->hGeneralTranscList,
                                    &listItem);
    RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    if(rv != RV_OK)
    {
       RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegAddTranscToList - Failed for call-leg 0x%p (InsertTail returned 0x%p)"
              ,pCallLeg,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }
    /*insert information to the list element*/
    phTranscToList = (RvSipTranscHandle*)listItem;
    *phTranscToList = hTransc;
    return RV_OK;

}

/***************************************************************************
 * CallLegRemoveTranscFromList
 * ------------------------------------------------------------------------
 * General: Removes a given transaction from the call-leg transaction list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The given transaction was not found
 *                                  in the call-leg transaction list.
 *               RV_ERROR_UNKNOWN - There was an infinite  loop while looking
 *                            for the transaction.
 *               RV_OK - Transaction was removed from the list
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg
 *            hTransc - Handle to the transaction that should be removed
 *                    from the list.
 ***************************************************************************/
RvStatus RVCALLCONV  CallLegRemoveTranscFromList(IN  CallLeg*           pCallLeg,
                                                  IN  RvSipTranscHandle  hTransc)
{
    RvStatus rv;
    
    rv = CallLegActOnGeneralTranscList(pCallLeg, hTransc, TRANSC_LIST_REMOVE_TRANSC_FROM_LIST, NULL);
    
    if(rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "CallLegRemoveTranscFromList - Call-leg 0x%p - transaction 0x%p not found in the list. already terminated",
              pCallLeg,hTransc));
        return RV_ERROR_INVALID_HANDLE; /*element not found*/
    }

    return RV_OK;

}

/***************************************************************************
 * CallLegAllocTransactionCallMemory
 * ------------------------------------------------------------------------
 * General: Allocates memory in the transaction for call-leg usage.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - The transaction handle
 * Output:   ppMemory - Pointer to the allocated memory.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegAllocTransactionCallMemory(IN RvSipTranscHandle hTransc,
                                                      OUT CallLegTranscMemory** ppMemory)
{
    CallLegTranscMemory* pMemory;

    pMemory = (CallLegTranscMemory*)SipTransactionGetContext(hTransc);
    if(NULL == pMemory)
    {
        pMemory = (CallLegTranscMemory*)SipTransactionAllocateOwnerMemory(
                                    hTransc, sizeof(CallLegTranscMemory));
        if(pMemory != NULL)
        {
            pMemory->dontDestructRouteList = RV_FALSE;
            pMemory->pInvite = NULL;
            *ppMemory = pMemory;
            return RV_OK;
        }
        else
        {
            *ppMemory = NULL;
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        *ppMemory = pMemory;
        return RV_OK;
    }
}

/***************************************************************************
 * CallLegTerminateAllGeneralTransc
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, Terminate the transcation
 *          using the transaction API. The transaction is not removed from the
 *          list. It will be removed when the event terminated will be received.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
void RVCALLCONV CallLegTerminateAllGeneralTransc(IN  CallLeg*   pCallLeg)
{
    

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegTerminateAllGeneralTransc - Call-leg 0x%p - terminating all its transactions."
             ,pCallLeg));

    CallLegActOnGeneralTranscList(pCallLeg, NULL, TRANSC_LIST_TERMINATE_ALL_TRANSC, NULL);

}

/***************************************************************************
 * CallLegActOnGeneralTranscList
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, and act on the transactions
 *          as required by the eAction parameter.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTranscForCompare - Handle to a transaction. used by functions that 
 *                   need to compare the transc in the list to a specific transc.
 *        eAction  - The required action on the transactions in the list.
 * Output: phFoundTransc - The transaction handle from the list, for actions 
 *                   that require to find a transaction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegActOnGeneralTranscList(IN  CallLeg*   pCallLeg,
                                                  IN  RvSipTranscHandle hTranscForCompare,
                                                  IN  CallLegTranscListAction eAction,
                                                  OUT RvSipTranscHandle      *phFoundTransc)
{
    RLIST_ITEM_HANDLE       pItem        = NULL;     /*current item*/
    RLIST_ITEM_HANDLE       pNextItem    = NULL;     /*next item*/
    RvSipTranscHandle      *phTransc;
    RLIST_POOL_HANDLE       hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;
    RvBool                  bFoundTransc = RV_FALSE;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = MAX_LOOP(pCallLeg);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegActOnGeneralTranscList - Call-leg 0x%p - actions is %s."
             ,pCallLeg, GetTranscListActionName(eAction)));

    if(phFoundTransc != NULL)
    {
        *phFoundTransc = NULL;
    }

    /*get the first item from the list*/
    RLIST_GetHead(hTranscPool, pCallLeg->hGeneralTranscList, &pItem);

    phTransc = (RvSipTranscHandle*)pItem;

    /*go over all the transactions*/
    while(phTransc != NULL && safeCounter < maxLoops)
    {
        RLIST_GetNext(pCallLeg->pMgr->hTranscHandlesPool,
                      pCallLeg->hGeneralTranscList,
                      pItem,
                      &pNextItem);
       
		switch(eAction)
        {
        case TRANSC_LIST_TERMINATE_ALL_TRANSC:
            GeneralTranscListTerminateAll(pCallLeg, *phTransc, pItem);
            break;
        case TRANSC_LIST_REMOVE_TRANSC_FROM_LIST:
            bFoundTransc = GeneralTranscListRemoveFromList(pCallLeg, hTranscForCompare, *phTransc, pItem);
            if(RV_TRUE == bFoundTransc) 
            {
                /* found the required transaction. stop the loop */
                return RV_OK;
            }
            break;
        case TRANSC_LIST_FIND_PRACK_TRANSC:
            bFoundTransc = GeneralTranscListFindPrack(pCallLeg, *phTransc);
            if(RV_TRUE == bFoundTransc) 
            {
                /* found the required transaction. stop the loop */
                if(phFoundTransc != NULL)
                {
                    *phFoundTransc = *phTransc;
                }
                return RV_OK;
            }
            break;
        case TRANSC_LIST_VERIFY_NO_ACTIVE:
            bFoundTransc = GeneralTranscListVerifyNoActive(pCallLeg, *phTransc);
            if(RV_TRUE == bFoundTransc) 
            {
                /* found the required transaction. stop the loop -
                   in this case, finding an active transaction is an error...*/
                return RV_ERROR_UNKNOWN;
            }
            break;
        case TRANSC_LIST_RESET_APP_HANDLE_ALL_TRANSC:
            GeneralTranscListResetAppActive(*phTransc);
            break;
        case TRANSC_LIST_DETACH_BYE:
            bFoundTransc = GeneralTranscListDetachBye(pCallLeg, *phTransc, pItem);
            if(RV_TRUE == bFoundTransc) 
            {
                /* found the bye transaction. stop the loop */
                return RV_OK;
            }
            break;
        case TRANSC_LIST_STOP_GENERAL_RETRANS:
            GeneralTranscListStopGeneral(pCallLeg,*phTransc, pItem);
            break;
        case TRANSC_LIST_487_ON_GEN_PENDING:
            GeneralTranscListSend487OnPending(pCallLeg, *phTransc);
            break;
        default:
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegActOnGeneralTranscList - Call-leg 0x%p - unknown actions %d." ,pCallLeg, eAction));
        }

        pItem = pNextItem;
        phTransc = (RvSipTranscHandle*)pItem;
        safeCounter++;
    }
    
    /*infinite loop*/
    if (safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegActOnGeneralTranscList - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                  pCallLeg,maxLoops));
    }

    switch (eAction)
    {
    case TRANSC_LIST_REMOVE_TRANSC_FROM_LIST:
    case TRANSC_LIST_FIND_PRACK_TRANSC:
        if(RV_FALSE == bFoundTransc)
        {
            return RV_ERROR_NOT_FOUND;
        }
    default:
        return RV_OK;
    }
}

/***************************************************************************
 * CallLegTerminateAllInvites
 * ------------------------------------------------------------------------
 * General: Go over the call-leg invite list, Terminate the objects
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        bInformOfIdleState - Indicates if we want to inform of Idle state
 *                   (for old invite) when the invite is out of termination queue.
 ***************************************************************************/
void RVCALLCONV CallLegTerminateAllInvites(IN  CallLeg*   pCallLeg,
                                           IN  RvBool     bInformOfIdleState)
{
    RLIST_ITEM_HANDLE       pItem        = NULL;     /*current item*/
    RLIST_ITEM_HANDLE       pNextItem    = NULL;     /*next item*/
    CallLegInvite           *pInvite     = NULL;
    RLIST_POOL_HANDLE       hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = MAX_LOOP(pCallLeg);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegTerminateAllInvites - Call-leg 0x%p - terminating all its invite objects."
             ,pCallLeg));

    /*get the first item from the list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    pInvite = (CallLegInvite*)pItem;

    /*go over all the objects*/
    while(pInvite != NULL && safeCounter < maxLoops)
    {
        if(bInformOfIdleState == RV_FALSE)
        {
            /* set the boolean to TRUE as if we already informed of modify-idle state */
            pInvite->bModifyIdleInOldInvite = RV_TRUE;
            RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegTerminateAllInvites - Call-leg 0x%p - do not inform state idle for invite 0x%p."
             ,pCallLeg, pInvite));
        
        }
        CallLegInviteTerminate(pCallLeg, pInvite, RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
        
        RLIST_GetNext(hInvitePool,
                      pCallLeg->hInviteObjsList,
                      pItem,
                      &pNextItem);
        
        pItem = pNextItem;
        pInvite = (CallLegInvite*)pItem;
        safeCounter++;
    }

    /*infinite loop*/
    if (pInvite != NULL && safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegTerminateAllInvites - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                  pCallLeg,maxLoops));
    }

}


/*-----------------------------------------------------------------------
       C A L L  - L E G:  L O C K   F U N C T I O N S
 ------------------------------------------------------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * CallLegLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks calleg according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
RvStatus RVCALLCONV CallLegLockAPI(IN  CallLeg*   pCallLeg)
{
    RvStatus           retCode;
    SipTripleLock     *tripleLock;
    CallLegMgr        *pMgr;
    RvInt32            identifier;

    if (pCallLeg == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {
        RvMutexLock(&pCallLeg->callTripleLock.hLock,pCallLeg->pMgr->pLogMgr);
        pMgr       = pCallLeg->pMgr;
        tripleLock = pCallLeg->tripleLock;
        identifier = pCallLeg->callLegUniqueIdentifier;
        RvMutexUnlock(&pCallLeg->callTripleLock.hLock,pCallLeg->pMgr->pLogMgr);


        if (tripleLock == NULL)
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CallLegLockAPI - Locking call 0x%p: Triple Lock 0x%p", pCallLeg,
            tripleLock));

        retCode = CallLegLock(pCallLeg,pMgr,tripleLock,identifier);
        if (retCode != RV_OK)
        {
            return retCode;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pCallLeg->tripleLock == tripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "CallLegLockAPI - Locking call 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
            pCallLeg,tripleLock));
        CallLegUnLock(pMgr->pLogMgr,&tripleLock->hLock);

    } 

    retCode = CRV2RV(RvSemaphoreTryWait(&tripleLock->hApiLock,NULL));
    if ((retCode != RV_ERROR_TRY_AGAIN) && (retCode != RV_OK))
    {
        CallLegUnLock(pCallLeg->pMgr->pLogMgr, &tripleLock->hLock);
        return retCode;
    }

    tripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(tripleLock->objLockThreadId == 0)
    {
        tripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&tripleLock->hLock,pCallLeg->pMgr->pLogMgr,
                              &tripleLock->threadIdLockCount);
    }
    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CallLegLockAPI - Locking call-leg 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pCallLeg, tripleLock, tripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * CallLegUnlockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks CallLeg according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV CallLegUnLockAPI(IN  CallLeg*   pCallLeg)
{
    SipTripleLock        *tripleLock;
    CallLegMgr           *pMgr;
    RvInt32               lockCnt = 0;

    if (pCallLeg == NULL)
    {
        return;
    }

    pMgr = pCallLeg->pMgr;
    tripleLock = pCallLeg->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    RvMutexGetLockCounter(&tripleLock->hLock,pCallLeg->pMgr->pLogMgr,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CallLegUnLockAPI - UnLocking call 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pCallLeg, tripleLock, tripleLock->apiCnt,lockCnt));


    if (tripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                   "CallLegUnLockAPI - UnLocking call 0x%p: Triple Lock 0x%p, apiCnt=%d",
                   pCallLeg, tripleLock, tripleLock->apiCnt));
        return;
    }

    if (tripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&tripleLock->hApiLock,NULL);
    }

    tripleLock->apiCnt--;
    /*reset the thread id in the counter that set it previously*/
    if(lockCnt == tripleLock->threadIdLockCount)
    {
        tripleLock->objLockThreadId = 0;
        tripleLock->threadIdLockCount = UNDEFINED;
    }

    CallLegUnLock(pCallLeg->pMgr->pLogMgr, &tripleLock->hLock);
}


/************************************************************************************
 * CallLegLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks call-leg according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
RvStatus RVCALLCONV CallLegLockMsg(IN  CallLeg*   pCallLeg)
{
    RvStatus          retCode      = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *tripleLock;
    CallLegMgr*       pMgr;
    RvInt32           identifier;

    if (pCallLeg == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    RvMutexLock(&pCallLeg->callTripleLock.hLock,pCallLeg->pMgr->pLogMgr);
    tripleLock = pCallLeg->tripleLock;
    pMgr       = pCallLeg->pMgr;
    identifier = pCallLeg->callLegUniqueIdentifier;
    RvMutexUnlock(&pCallLeg->callTripleLock.hLock,pCallLeg->pMgr->pLogMgr);

    if (tripleLock == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "CallLegLockMsg - Exiting without locking CallLeg 0x%p: Triple Lock 0x%p, apiCnt=%d already locked",
                   pCallLeg, tripleLock, tripleLock->apiCnt));

        return RV_OK;
    }

    RvMutexLock(&tripleLock->hProcessLock,pCallLeg->pMgr->pLogMgr);

    for(;;)
    {
        retCode = CallLegLock(pCallLeg,pMgr, tripleLock,identifier);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pCallLeg->pMgr->pLogMgr);
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

        CallLegUnLock(pCallLeg->pMgr->pLogMgr, &tripleLock->hLock);

        retCode = RvSemaphoreWait(&tripleLock->hApiLock,NULL);
        if (retCode != RV_OK)
        {
          RvMutexUnlock(&tripleLock->hProcessLock,pCallLeg->pMgr->pLogMgr);
          return retCode;
        }

        didILockAPI = RV_TRUE;
    } 

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CallLegLockMsg - Locking call 0x%p: Triple Lock 0x%p exiting function",
             pCallLeg, tripleLock));
    return retCode;
}


/************************************************************************************
 * CallLegUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks calleg according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the call-leg.
***********************************************************************************/
void RVCALLCONV CallLegUnLockMsg(IN  CallLeg*   pCallLeg)
{
    SipTripleLock   *tripleLock;
    CallLegMgr      *pMgr;
    RvThreadId       currThreadId = RvThreadCurrentId();

    if (pCallLeg == NULL)
    {
        return;
    }

    pMgr       = pCallLeg->pMgr;
    tripleLock = pCallLeg->tripleLock;

    if ((pMgr == NULL) || (tripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (tripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "CallLegUnLockMsg - Exiting without unlocking CallLeg 0x%p: Triple Lock 0x%p already locked",
                   pCallLeg, tripleLock));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "CallLegUnLockMsg - UnLocking call 0x%p: Triple Lock 0x%p", pCallLeg,
             tripleLock));

    CallLegUnLock(pCallLeg->pMgr->pLogMgr, &tripleLock->hLock);
    RvMutexUnlock(&tripleLock->hProcessLock,pCallLeg->pMgr->pLogMgr);
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */


/*-----------------------------------------------------------------------
       C A L L  - L E G:  G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/


/***************************************************************************
 * CallLegGetCallId
 * ------------------------------------------------------------------------
 * General:Returns the Call-Id of the call-leg.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER - The buffer given by the application
 *                                       was isdufficient.
 *               RV_ERROR_NOT_FOUND           - The call-leg does not contain a call-id
 *                                       yet.
 *               RV_OK            - Call-id was copied into the
 *                                       application buffer. The size is
 *                                       returned in the actualSize param.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg   - The Sip Stack handle to the call-leg.
 *          bufSize    - The size of the application buffer for the call-id.
 * Output:     strCallId  - An application allocated buffer.
 *          actualSize - The call-id actual size.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetCallId (
                            IN  CallLeg              *pCallLeg,
                            IN  RvInt32             bufSize,
                            OUT RvChar              *pstrCallId,
                            OUT RvInt32             *actualSize)
{
    RvStatus retStatus;

    *actualSize = 0;
    if(pCallLeg->strCallId == UNDEFINED)
    {
        pstrCallId[0] = '\0';
        return RV_ERROR_NOT_FOUND;
    }
    *actualSize = RPOOL_Strlen(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                               pCallLeg->strCallId);
    if(bufSize < (*actualSize)+1)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    retStatus = RPOOL_CopyToExternal(pCallLeg->pMgr->hGeneralPool,
                                     pCallLeg->hPage,
                                     pCallLeg->strCallId,
                                     pstrCallId,
                                     *actualSize+1);
    return retStatus;
}


/***************************************************************************
 * CallLegSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the call-leg's call-id. The given call-id is copied into the
 *          call-leg memory page.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Call-id was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg -  Pointer to the call-leg.
 *            strCallId - Null terminating string with the new call id.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetCallId (
                                      IN  CallLeg *     pCallLeg,
                                      IN  RPOOL_Ptr    *strCallId)
{
    RvStatus            rv;
    RvInt32            offset;
    RvInt32             length;
    /*copy the given call-id into the call-leg page and save the pointer.*/
    length = RPOOL_Strlen(strCallId->hPool, strCallId->hPage, strCallId->offset);
    rv = RPOOL_Append(pCallLeg->pMgr->hGeneralPool,pCallLeg->hPage, length+1,
                      RV_FALSE, &offset);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegSetCallId - Call-leg 0x%p can not append. rv=%d",pCallLeg,rv));
        return rv;
    }
    rv = RPOOL_CopyFrom(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, offset,
                        strCallId->hPool, strCallId->hPage, strCallId->offset,
                        length+1);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegSetCallId - Call-leg 0x%p can not copy. rv=%d",pCallLeg,rv));
        return rv;
    }
    pCallLeg->strCallId = offset;
#ifdef SIP_DEBUG
    pCallLeg->pCallId = RPOOL_GetPtr(pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                                     pCallLeg->strCallId);
#endif
    return RV_OK;
}



/***************************************************************************
 * CallLegSetFromHeader
 * ------------------------------------------------------------------------
 * General: Set the From header associated with the call-leg.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hFrom - Handle to an application constructed from header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetFromHeader (
                                      IN  CallLeg *               pCallLeg,
                                      IN  RvSipPartyHeaderHandle  hFrom)
{

    RvStatus rv;
    /*first check if the given header was constructed on the call-leg page
      and if so attach it*/
    CallLegLockMgr(pCallLeg);
    if( SipPartyHeaderGetPool(hFrom) == pCallLeg->pMgr->hGeneralPool &&
        SipPartyHeaderGetPage(hFrom) == pCallLeg->hPage)
    {
        pCallLeg->hFromHeader = hFrom;
        CallLegUnlockMgr(pCallLeg);
        return RV_OK;
    }

    /*the given to header is not on the call-leg page - construct and copy*/
    /*if there is no from header, construct it*/
    rv = SipPartyHeaderConstructAndCopy(pCallLeg->pMgr->hMsgMgr,
                                        pCallLeg->pMgr->hGeneralPool,
                                        pCallLeg->hPage,
                                        hFrom,
                                        RVSIP_HEADERTYPE_FROM,
                                       &(pCallLeg->hFromHeader));
    CallLegUnlockMgr(pCallLeg);
    if(rv != RV_OK)
    {
         RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegSetFromHeader - call-leg 0x%p - Failed to set From header - copy failed (rv=%d)",
                   pCallLeg,rv));
         return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;
}



/***************************************************************************
 * CallLegGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from address associated with the call. Note
 *          that the address is not copied. Instead the call-leg's internal from
 *          header is referenced. Attempting to alter the from address after
 *          call has been initiated might cause unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phFrom - Pointer to the call-leg from header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetFromHeader (
                                      IN  CallLeg *               pCallLeg,
                                      OUT RvSipPartyHeaderHandle  *phFrom)
{
    *phFrom = pCallLeg->hFromHeader;
    return RV_OK;

}


/***************************************************************************
 * CallLegSetToHeader
 * ------------------------------------------------------------------------
 * General: Set the To address associated with the call-leg. Note
 *          that attempting to altert the To address after call has
 *             been initiated might cause unexpected results.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - From header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hTo - Handle to an application constructed to header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetToHeader (
                                      IN  CallLeg *               pCallLeg,
                                      IN  RvSipPartyHeaderHandle  hTo)
{
    RvStatus rv;
    /*first check if the given header was constructed on the call-leg page
    and if so attach it*/
    CallLegLockMgr(pCallLeg);

    if( SipPartyHeaderGetPool(hTo) == pCallLeg->pMgr->hGeneralPool &&
        SipPartyHeaderGetPage(hTo) == pCallLeg->hPage)
    {
        pCallLeg->hToHeader = hTo;
        CallLegUnlockMgr(pCallLeg);
        return RV_OK;
    }

    /*the given to header is a stand alone*/
    /*if there is no to header, construct it*/
    rv = SipPartyHeaderConstructAndCopy(pCallLeg->pMgr->hMsgMgr,
                                        pCallLeg->pMgr->hGeneralPool,
                                        pCallLeg->hPage,
                                        hTo,
                                        RVSIP_HEADERTYPE_TO,
                                        &(pCallLeg->hToHeader));
    
    
    CallLegUnlockMgr(pCallLeg);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSetToHeader - Call-leg 0x%p Failed to copy To header (rv=%d)",
            pCallLeg,rv));
        return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;
}


/***************************************************************************
 * CallLegGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the to address associated with the call. Note
 *          that the header is not copied. Instead the call-leg's
 *          internal to header is referenced. Attempting to altert
 *          the to address after call has been initiated might cause
 *          unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phTo - Pointer to the call-leg to header.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetToHeader(
                                      IN  CallLeg *               pCallLeg,
                                      OUT RvSipPartyHeaderHandle  *phTo)
{
    *phTo = pCallLeg->hToHeader;
    return RV_OK;

}


/***************************************************************************
 * CallLegGetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Get the remote party's contact address. This is the
 *          address supplied by the remote party by which it can be
 *          directly contacted in future requests. Note
 *          that the address is not copied. Instead the call-leg's
 *          internal handle is referenced. Attempting to altert
 *          this information might cause unexpected results.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phContactAddress - Pointer to the call-leg Remote Contact Address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetRemoteContactAddress(
                                IN  CallLeg               *pCallLeg,
                                OUT RvSipAddressHandle    *phContactAddress)
{
    *phContactAddress = pCallLeg->hRemoteContact;
    return RV_OK;
}

/***************************************************************************
 * CallLegSetRemoteContactAddress
 * ------------------------------------------------------------------------
 * General: Set the contact address of the remote party. This is the address
 *          with which the remote party may be contacted with. This method
 *          may be used for outgoing calls where the user wishes to use a
 *          Request-URI that is different from the call-leg's To value.
 * Return Value: RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hContactAddress - Handle to an application constructed address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetRemoteContactAddress (
                                      IN  CallLeg *          pCallLeg,
                                      IN  RvSipAddressHandle hContactAddress)
{

    RvStatus        rv;
    RvSipAddressType newAddrType = RvSipAddrGetAddrType(hContactAddress);

    if(newAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
           RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSetRemoteContactAddress - Failed for call 0x%p, Address type not supported",
                  pCallLeg));
        return RV_ERROR_BADPARAM;
    }

    /*first check if we already have a contact and if so free the page and
      allocate a new one*/
    if(pCallLeg->hRemoteContactPage == NULL)
    {
         rv = RPOOL_GetPage(pCallLeg->pMgr->hElementPool,0,&(pCallLeg->hRemoteContactPage));
         if(rv != RV_OK)
         {
              RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSetRemoteContactAddress - Call-leg 0x%p - Failed to allocate page (rv=%d)",
                 pCallLeg,rv));
              return rv;
         }
    }
    if(pCallLeg->hRemoteContact != NULL)
    {
         RPOOL_FreePage(pCallLeg->pMgr->hElementPool,pCallLeg->hRemoteContactPage);
         pCallLeg->hRemoteContact = NULL;
         pCallLeg->hRemoteContactPage = NULL;
         rv = RPOOL_GetPage(pCallLeg->pMgr->hElementPool,0,&(pCallLeg->hRemoteContactPage));
         if(rv != RV_OK)
         {
              RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                           "CallLegSetRemoteContactAddress - Call-leg 0x%p - Failed to allocate page (rv=%d)",
                         pCallLeg,rv));
              return rv;
         }
    }
    /*construct the new remote contact on the new page*/
    rv = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr,
                            pCallLeg->pMgr->hElementPool,
                            pCallLeg->hRemoteContactPage,
                            newAddrType,
                            &(pCallLeg->hRemoteContact));
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSetRemoteContactAddress - Call-leg 0x%p - Failed to construct address (rv=%d)",
            pCallLeg,rv));
         RPOOL_FreePage(pCallLeg->pMgr->hElementPool,pCallLeg->hRemoteContactPage);
         pCallLeg->hRemoteContactPage = NULL;
         pCallLeg->hRemoteContact = NULL;
        return RV_ERROR_OUTOFRESOURCES;

    }

    /*copy the given address to the call-leg remote contact address*/
    rv = RvSipAddrCopy(pCallLeg->hRemoteContact,hContactAddress);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "CallLegSetRemoteContactAddress - Call-leg 0x%p - Failed  to copy address (rv=%d)",
          pCallLeg,rv));
        RPOOL_FreePage(pCallLeg->pMgr->hElementPool,pCallLeg->hRemoteContactPage);
        pCallLeg->hRemoteContactPage = NULL;
        pCallLeg->hRemoteContact = NULL;
        return RV_ERROR_OUTOFRESOURCES;

    }
    return RV_OK;
}


/***************************************************************************
 * CallLegSetRedirectAddress
 * ------------------------------------------------------------------------
 * General: Set the redirect address. This is the address taken from a 3xx
 *          and will be used to reconnect the call-leg to a different party.
 *          This address will replace the remote contact in state redirected.
 * Return Value: RV_ERROR_BADPARAM - The supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hRedirectAddress - Handle to the redirect address
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetRedirectAddress (
                                      IN  CallLeg *          pCallLeg,
                                      IN  RvSipAddressHandle hRedirectAddress)
{
    RvStatus        rv;
    RvSipAddressType newAddrType = RvSipAddrGetAddrType(hRedirectAddress);
    RvSipAddressType currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;

    if(newAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
           RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSetRedirectAddress - Failed for call 0x%p, Address type not supported",
                  pCallLeg));
        return RV_ERROR_BADPARAM;
    }

    if(pCallLeg->hRedirectAddress != NULL)
    {
        currentAddrType = RvSipAddrGetAddrType(pCallLeg->hRedirectAddress);
    }

    /*if there was no address or the new address type is different then the
      current one a new address should be constructed. otherwise just a copy
      is needed*/
    if(pCallLeg->hRedirectAddress == NULL || newAddrType != currentAddrType)
    {
        rv = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr,
                                pCallLeg->pMgr->hGeneralPool,
                                pCallLeg->hPage,
                                newAddrType,
                                &(pCallLeg->hRedirectAddress));
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSetRedirectAddress - Call-leg 0x%p - Failed to construct address (rv=%d)",
                pCallLeg,rv));
            return RV_ERROR_OUTOFRESOURCES;

        }
    }
    /*copy the given address to the call-leg remote contact address*/
    rv = RvSipAddrCopy(pCallLeg->hRedirectAddress,hRedirectAddress);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSetRedirectAddress - Call-leg 0x%p - Failed  to copy address (rv=%d)",
                  pCallLeg,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

/***************************************************************************
 * CallLegGetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Gets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. If no value is
 *          supplied the From header's address part is taken. The
 *          local contact address is used by the remote party to
 *          directly contact the local party. Note
 *          that the address is not copied. Instead the call-leg's
 *          internal address is referenced.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments: RV_OK
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     phContactAddress - Pointer to the call-leg local Contact Address.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetLocalContactAddress (
                                      IN  CallLeg *       pCallLeg,
                                      OUT RvSipAddressHandle *  phContactAddress)

{
    RvSipPartyHeaderHandle hParty = NULL;
    if(pCallLeg->hLocalContact != NULL)
    {
        *phContactAddress = pCallLeg->hLocalContact;
        return RV_OK;
    }

    /*if there is no contact use the From for outgoing calls and the To for incoming.*/
    hParty = (pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)?
                                      pCallLeg->hToHeader:pCallLeg->hFromHeader;
    if(hParty == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "CallLegGetLocalContactAddress - Call-leg 0x%p - Failed, information not existed", pCallLeg));
        return RV_ERROR_UNKNOWN;
    }
    *phContactAddress = RvSipPartyHeaderGetAddrSpec(hParty);
    return RV_OK;
}

/***************************************************************************
 * CallLegSetLocalContactAddress
 * ------------------------------------------------------------------------
 * General: Sets the local contact address which the SIP stack uses
 *          to identify itself to the remote party. If no value is
 *          supplied the From header's address part is taken. The
 *          local contact address is used by the remote party to
 *          directly contact the local party.
 * Return Value: RV_ERROR_BADPARAM - If the supplied address type is not supported.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Remote Contact Address was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     hContactAddress - Handle to an application constructed address.
 ***************************************************************************/

RvStatus RVCALLCONV CallLegSetLocalContactAddress (
                                     IN  CallLeg *           pCallLeg,
                                     IN  RvSipAddressHandle  hContactAddress)

{
    RvStatus        rv;
    RvSipAddressType newAddrType = RvSipAddrGetAddrType(hContactAddress);
    RvSipAddressType currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;

    if(newAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
           RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "CallLegSetLocalContactAddress - Call-leg 0x%p - Failed, address type not supported",
          pCallLeg));
        return RV_ERROR_BADPARAM;
    }

    /*first check if the given address was constructed on the call-leg page
      and if so attach it*/
    if( SipAddrGetPool(hContactAddress) == pCallLeg->pMgr->hGeneralPool &&
        SipAddrGetPage(hContactAddress) == pCallLeg->hPage)
    {
        pCallLeg->hLocalContact = hContactAddress;
        return RV_OK;
    }

    /*if we already have a local contact take its address type*/
    if(pCallLeg->hLocalContact != NULL)
    {
        currentAddrType = RvSipAddrGetAddrType(pCallLeg->hLocalContact);
    }

    /*if there was no address or the new address type is different then the
      current one a new address should be constructed. otherwise just a copy
      is needed*/
    if(pCallLeg->hLocalContact == NULL || newAddrType != currentAddrType)
    {
        rv = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr,
                                pCallLeg->pMgr->hGeneralPool,
                                pCallLeg->hPage,
                                newAddrType,
                                &(pCallLeg->hLocalContact));
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSetLocalContactAddress - Call-leg 0x%p - Failed to construct address (rv=%d)",
                pCallLeg,rv));
            return RV_ERROR_OUTOFRESOURCES;

        }
    }

    /*copy the given address to the call-leg remote contact address*/
    rv = RvSipAddrCopy(pCallLeg->hLocalContact,hContactAddress);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegSetLocalContactAddress - Call-leg 0x%p - Failed to copy address (rv=%d)",
                  pCallLeg,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}


/***************************************************************************
 * CallLegGetDirection
 * ------------------------------------------------------------------------
 * General: Returns wether the call-leg is incoming or outgoing.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     peDirection - The call-leg direction.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetDirection(
                                 IN  CallLeg *             pCallLeg,
                                 OUT RvSipCallLegDirection *peDirection)
{
    *peDirection = pCallLeg->eDirection;
    return RV_OK;
}

/***************************************************************************
 * CallLegGetCurrentState
 * ------------------------------------------------------------------------
 * General: Returns the current state of the call-leg.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 * Output:     peState - The call-leg state.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetCurrentState (
                                      IN  CallLeg *          pCallLeg,
                                      OUT RvSipCallLegState* peState)
{
    *peState = pCallLeg->eState;
    return RV_OK;
}

/***************************************************************************
 * CallLegGetTranscByMsg
 * ------------------------------------------------------------------------
 * General: Returns the transaction that this message belongs to.
 *          This function looks for the transaction in the transaction list
 *          of the call-leg and compares each transaction by its isUac,method
 *          and cseq.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *          hMsg     - The given message.
 *          bIsMsgRcvd - RV_TRUE if this is a received message. RV_FALSE otherwise.
 * Output:     phTransc - The transaction handle or NULL if no such transaction
 *                     exists.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetTranscByMsg (
                                 IN  CallLeg                *pCallLeg,
                                 IN  RvSipMsgHandle         hMsg,
                                  IN  RvBool                bIsMsgRcvd,
                                 OUT RvSipTranscHandle      *phTransc)
{
    RvSipCSeqHeaderHandle hCSeq;
    RvInt32               msgCSeq;
    RvInt32               transcCSeq;
    RLIST_ITEM_HANDLE     pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE     pNextItem   = NULL;     /*next item*/
    RLIST_POOL_HANDLE     hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;
    RLIST_POOL_HANDLE     hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;
    RvUint32              safeCounter = 0;
    RvUint32              maxLoops    = MAX_LOOP(pCallLeg);
    RvSipTranscHandle    *transcFromList = NULL;
    RvBool                bIsUAC      = RV_FALSE;
    RvBool                bIsMsgOfUAC = RV_FALSE;
    RvSipMethodType       eMsgMethod,eTranscMsgMethod;
    RvSipMsgType          eMsgType;
	RvStatus			  rv;


    *phTransc = NULL;
    
    /*get the Cseq Method and msg type from the message*/
    eMsgType = RvSipMsgGetMsgType(hMsg);
    if(eMsgType == RVSIP_MSG_UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    hCSeq = RvSipMsgGetCSeqHeader(hMsg);
    if(hCSeq == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

	rv = SipCSeqHeaderGetInitializedCSeq(hCSeq,&msgCSeq); 
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegGetTranscByMsg: Call-Leg 0x%p - SipCSeqHeaderGetInitializedCSeq() failed (CSeq might be uninitialized)", pCallLeg));
        return rv;
	}

    eMsgMethod = RvSipCSeqHeaderGetMethodType(hCSeq);
    if(eMsgMethod == RVSIP_METHOD_UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }
    if(eMsgMethod == RVSIP_METHOD_ACK)
    {
        eMsgMethod = RVSIP_METHOD_INVITE;
    }

    if(bIsMsgRcvd == RV_TRUE)
    {
        bIsMsgOfUAC = (eMsgType == RVSIP_MSG_REQUEST)?RV_FALSE:RV_TRUE;
    }
    else
    {
        bIsMsgOfUAC = (eMsgType == RVSIP_MSG_REQUEST)?RV_TRUE:RV_FALSE;
    }

    /*find a transaction with the same CSeq,method, type - in this call-leg*/
    /*get the first item from the list*/
    if(eMsgMethod != RVSIP_METHOD_INVITE)
    {
        RLIST_GetHead(hTranscPool, pCallLeg->hGeneralTranscList, &pItem);

        transcFromList = (RvSipTranscHandle*)pItem;

        /*go over all the transactions*/
        while(transcFromList != NULL && safeCounter < maxLoops)
        {
            /*get information from the transaction*/
            RvSipTransactionIsUAC(*transcFromList,&bIsUAC);
            RvSipTransactionGetCSeqStep(*transcFromList,&transcCSeq);
            eTranscMsgMethod = SipTransactionGetMsgMethodFromTranscMethod(*transcFromList);
            if(eTranscMsgMethod == RVSIP_METHOD_ACK)
            {
                eTranscMsgMethod = RVSIP_METHOD_INVITE;
            }
            if(eMsgMethod == eTranscMsgMethod &&
               bIsUAC     == bIsMsgOfUAC      &&
               msgCSeq    == transcCSeq)
            {
                /*compare via branch*/
                if(SipTransactionIsEqualBranchToMsgBranch(*transcFromList,hMsg)==RV_TRUE)
                {
                   *phTransc = *transcFromList;
                    break;
                }
            }
            /*get the next item*/
            RLIST_GetNext(pCallLeg->pMgr->hTranscHandlesPool,
                          pCallLeg->hGeneralTranscList,
                          pItem,
                          &pNextItem);

            pItem = pNextItem;
            transcFromList = (RvSipTranscHandle*)pItem;
            safeCounter++;
        }
        /*infinite loop*/
        if (safeCounter == maxLoops)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                      "CallLegGetTranscByMsg - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                      pCallLeg,maxLoops));
            return RV_ERROR_UNKNOWN;
        }
    }
    else /* invite message - search in the invite objects list */
    {
        CallLegInvite* pInvite;

        RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);
        
        pInvite = (CallLegInvite*)pItem;
        
        /*go over all the transactions*/
        while(pInvite != NULL && pInvite->hInviteTransc != NULL && safeCounter < maxLoops)
        {
            /*get information from the transaction*/
            RvSipTransactionIsUAC(pInvite->hInviteTransc ,&bIsUAC);
            RvSipTransactionGetCSeqStep(pInvite->hInviteTransc ,&transcCSeq);
            eTranscMsgMethod = SipTransactionGetMsgMethodFromTranscMethod(pInvite->hInviteTransc);
            if(eTranscMsgMethod == RVSIP_METHOD_ACK)
            {
                eTranscMsgMethod = RVSIP_METHOD_INVITE;
            }
            if(eMsgMethod == eTranscMsgMethod &&
                bIsUAC     == bIsMsgOfUAC      &&
                msgCSeq    == transcCSeq)
            {
                /*compare via branch*/
                if(SipTransactionIsEqualBranchToMsgBranch(pInvite->hInviteTransc ,hMsg)==RV_TRUE)
                {
                    *phTransc = pInvite->hInviteTransc ;
                    break;
                }
            }
            /*get the next item*/
            RLIST_GetNext(hInvitePool, 
                          pCallLeg->hInviteObjsList,
                          pItem,
                          &pNextItem);
            
            pItem = pNextItem;
            pInvite = (CallLegInvite*)pItem;;
            safeCounter++;
        }
        /*infinite loop*/
        if (safeCounter == maxLoops)
        {
            RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegGetTranscByMsg - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                pCallLeg,maxLoops));
            return RV_ERROR_UNKNOWN;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * CallLegResetActiveTransc
 * ------------------------------------------------------------------------
 * General: Reset the call-leg active transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegResetActiveTransc (IN  CallLeg  *pCallLeg)
{
    if(pCallLeg->hActiveTransc != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc, 
              "CallLegResetActiveTransc: Call-leg 0x%p - reset active transc 0x%p",
              pCallLeg, pCallLeg->hActiveTransc));
        pCallLeg->hActiveTransc = NULL;
    }
    return RV_OK;
}

/***************************************************************************
 * CallLegSetActiveTransc
 * ------------------------------------------------------------------------
 * General: Set the call-leg active transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSetActiveTransc (IN  CallLeg  *pCallLeg,
                                            IN  RvSipTranscHandle hTransc)
{
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc, 
              "CallLegSetActiveTransc: Call-leg 0x%p - Set active transc to 0x%p",
              pCallLeg, hTransc));
    pCallLeg->hActiveTransc = hTransc;
    return RV_OK;
}

/*------------------------------------------------------------------------
           N A M E    F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * CallLegGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state. (This function is used in
 *          the High Availability module in addition to printing ability.)
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetStateName (
                                      IN  RvSipCallLegState  eState)
{

    switch(eState)
    {
    case RVSIP_CALL_LEG_STATE_ACCEPTED:
        return CALL_LEG_STATE_ACCEPTED_STR;
    case RVSIP_CALL_LEG_STATE_CONNECTED:
        return CALL_LEG_STATE_CONNECTED_STR;
    case RVSIP_CALL_LEG_STATE_DISCONNECTED:
        return CALL_LEG_STATE_DISCONNECTED_STR;
    case RVSIP_CALL_LEG_STATE_DISCONNECTING:
        return CALL_LEG_STATE_DISCONNECTING_STR;
    case RVSIP_CALL_LEG_STATE_IDLE:
        return CALL_LEG_STATE_IDLE_STR;
    case RVSIP_CALL_LEG_STATE_INVITING:
        return CALL_LEG_STATE_INVITING_STR;
    case RVSIP_CALL_LEG_STATE_PROCEEDING:
        return CALL_LEG_STATE_PROCEEDING_STR;
    case RVSIP_CALL_LEG_STATE_OFFERING:
        return CALL_LEG_STATE_OFFERING_STR;
    case RVSIP_CALL_LEG_STATE_REDIRECTED:
        return CALL_LEG_STATE_REDIRECTED_STR;
    case RVSIP_CALL_LEG_STATE_TERMINATED:
        return CALL_LEG_STATE_TERMINATED_STR;
    case RVSIP_CALL_LEG_STATE_UNAUTHENTICATED:
        return CALL_LEG_STATE_UNAUTHENTICATED_STR;
    case RVSIP_CALL_LEG_STATE_CANCELLING:
        return CALL_LEG_STATE_CANCELLING_STR;
    case RVSIP_CALL_LEG_STATE_CANCELLED:
        return CALL_LEG_STATE_CANCELLED_STR;
    case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE:
        return CALL_LEG_STATE_MSG_SEND_FAILURE_STR;
    case RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT:
        return CALL_LEG_STATE_PROCEEDING_TIMEOUT_STR;
    case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED:
        return CALL_LEG_STATE_REMOTE_ACCEPTED_STR;
    default:
        return CALL_LEG_STATE_UNDEFINED_STR;
    }
}

/***************************************************************************
 * CallLegGetStateEnum
 * ------------------------------------------------------------------------
 * General: Returns the state enum according to given state name string
 * Return Value: The the state enum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strState - The state name string.
 ***************************************************************************/
RvSipCallLegState RVCALLCONV CallLegGetStateEnum(IN RvChar *strState)
{
    if (SipCommonStricmp(strState,CALL_LEG_STATE_ACCEPTED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_ACCEPTED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_CONNECTED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_CONNECTED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_DISCONNECTED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_DISCONNECTED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_DISCONNECTING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_DISCONNECTING;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_IDLE_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_IDLE;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_INVITING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_INVITING;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_PROCEEDING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_PROCEEDING;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_OFFERING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_OFFERING;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_REDIRECTED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_REDIRECTED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_TERMINATED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_TERMINATED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_UNAUTHENTICATED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_UNAUTHENTICATED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_CANCELLING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_CANCELLING;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_CANCELLED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_CANCELLED;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_MSG_SEND_FAILURE_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_PROCEEDING_TIMEOUT_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT;
    }
    else if (SipCommonStricmp(strState,CALL_LEG_STATE_REMOTE_ACCEPTED_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED;
    }

    return RVSIP_CALL_LEG_STATE_UNDEFINED;
}

/***************************************************************************
 * CallLegGetDirectionName
 * ------------------------------------------------------------------------
 * General: The function converts a Direction type enum to string.
 * Return Value: The suitable string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eDirection - The direction type.
 ***************************************************************************/
RvChar *CallLegGetDirectionName(RvSipCallLegDirection eDirection)
{
    switch(eDirection) {
    case RVSIP_CALL_LEG_DIRECTION_INCOMING:
        return CALL_LEG_DIRECTION_INCOMING_STR;
    case RVSIP_CALL_LEG_DIRECTION_OUTGOING:
        return CALL_LEG_DIRECTION_OUTGOING_STR;
    default:
        return CALL_LEG_DIRECTION_UNDEFINED_STR;
    }
}

/***************************************************************************
 * CallLegGetDirectionEnum
 * ------------------------------------------------------------------------
 * General: The function converts a Direction string to enum type.
 * Return Value: The suitable enum value.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strDirection - The direction string.
 ***************************************************************************/
RvSipCallLegDirection CallLegGetDirectionEnum(RvChar *strDirection)
{
    if (SipCommonStricmp(strDirection,CALL_LEG_DIRECTION_INCOMING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_DIRECTION_INCOMING;
    }
    else if (SipCommonStricmp(strDirection,CALL_LEG_DIRECTION_OUTGOING_STR) == RV_TRUE)
    {
        return RVSIP_CALL_LEG_DIRECTION_OUTGOING;
    }

    return RVSIP_CALL_LEG_DIRECTION_UNDEFINED;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 *  CallLegGetModifyStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given connect sub state.
 * Return Value: The string with the connect sub state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eConnectState - The connect substate name as enum.
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetModifyStateName(
                                        IN RvSipCallLegModifyState eConnectState)
{
    switch(eConnectState)
    {
    case RVSIP_CALL_LEG_MODIFY_STATE_IDLE:
        return "Modify Idle";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD:
        return "Modify Re-Invite Rcvd";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_SENT:
        return "Modify Re-Invite Response Sent";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_SENT:
        return "Modify Re-Invite Sent";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING:
        return "Modify Re-Invite Proceeding";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLING:
        return "Modify Re-Invite Cancelling";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RESPONSE_RCVD:
        return "Modify Re-Invite Response Rcvd";
    case RVSIP_CALL_LEG_MODIFY_STATE_MSG_SEND_FAILURE:
        return "Modify Msg Send Failure";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_CANCELLED:
        return "Modify Re-Invite Cancelled";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_PROCEEDING_TIMEOUT:
        return "Modify Re-Invite Proceeding Time Out";
    case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED:
        return "Modify Re-Invite Remote Accepted";
    case RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT:
        return "Modify Re-Invite Ack Sent";
    case RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD:
        return "Modify Re-Invite Ack Rcvd";
    case RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED:
        return "Modify Re-Invite Terminated";
    default:
        return "Undefined";
    }
}


/***************************************************************************
 * CallLegGetStateChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state change reason.
 * Return Value: The string with the state change reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The state change reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetStateChangeReasonName (
                                IN  RvSipCallLegStateChangeReason  eReason)
{
    switch(eReason)
    {
    case RVSIP_CALL_LEG_REASON_LOCAL_INVITING:
        return "Local Inviting";
    case RVSIP_CALL_LEG_REASON_REMOTE_INVITING:
        return "Remote Inviting";
    case RVSIP_CALL_LEG_REASON_LOCAL_REFER:
        return "Local Refer";
    case RVSIP_CALL_LEG_REASON_REMOTE_REFER:
        return "Remote Refer";
    case RVSIP_CALL_LEG_REASON_LOCAL_REFER_NOTIFY:
        return "Local Refer Notify";
    case RVSIP_CALL_LEG_REASON_REMOTE_REFER_NOTIFY:
        return "Remote Refer Notify";
    case RVSIP_CALL_LEG_REASON_LOCAL_ACCEPTED:
        return "Local Accepted";
    case RVSIP_CALL_LEG_REASON_REMOTE_ACCEPTED:
        return "Remote Accepted";
    case RVSIP_CALL_LEG_REASON_REMOTE_ACK:
        return "Remote Ack";
    case RVSIP_CALL_LEG_REASON_REDIRECTED:
        return "Redirected";
    case RVSIP_CALL_LEG_REASON_LOCAL_REJECT:
        return "Local Reject";
    case RVSIP_CALL_LEG_REASON_REQUEST_FAILURE:
        return "Request Failure";
    case RVSIP_CALL_LEG_REASON_SERVER_FAILURE:
        return "Server Failure";
    case RVSIP_CALL_LEG_REASON_GLOBAL_FAILURE:
        return "Global Failure";
    case RVSIP_CALL_LEG_REASON_LOCAL_DISCONNECTING:
        return "Local Disconnecting";
    case RVSIP_CALL_LEG_REASON_DISCONNECTED:
        return "Disconnected";
    case RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECTED:
        return "Remote Disconnected";
    case RVSIP_CALL_LEG_REASON_LOCAL_FAILURE:
        return "Local Failure";
    case RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT:
        return "Time Out";
    case RVSIP_CALL_LEG_REASON_CALL_TERMINATED:
        return "Call Terminated";
    case RVSIP_CALL_LEG_REASON_AUTH_NEEDED:
        return "Server requested authentication";
    case RVSIP_CALL_LEG_REASON_UNSUPPORTED_AUTH_PARAMS:
        return "Cannot Authenticate Due to Unsupported Parameters";
    case RVSIP_CALL_LEG_REASON_LOCAL_CANCELLING:
        return "Local Cancelling";
    case RVSIP_CALL_LEG_REASON_ACK_SENT:
        return "Ack Sent";
    case RVSIP_CALL_LEG_REASON_REMOTE_CANCELED:
        return "Remote Canceled";
    case RVSIP_CALL_LEG_REASON_CALL_CONNECTED:
        return "Call Connected";
    case RVSIP_CALL_LEG_REASON_REMOTE_PROVISIONAL_RESP:
        return "Remote Provisional Response";
    case RVSIP_CALL_LEG_REASON_REMOTE_REFER_REPLACES:
        return "Remote REFER with Replaces";
    case RVSIP_CALL_LEG_REASON_REMOTE_INVITING_REPLACES:
        return "Remote INVITE with Replaces";
    case RVSIP_CALL_LEG_REASON_REMOTE_DISCONNECT_REQUESTED:
        return "Remote Disconnected Request";
    case RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_REJECT:
        return "Disconnect Local Reject";
    case RVSIP_CALL_LEG_REASON_DISCONNECT_REMOTE_REJECT:
        return "Disconnect Remote Reject";
    case RVSIP_CALL_LEG_REASON_DISCONNECT_LOCAL_ACCEPT:
        return "Disconnect Local Accept";
    case RVSIP_CALL_LEG_REASON_NETWORK_ERROR:
        return "Network Error";
    case RVSIP_CALL_LEG_REASON_503_RECEIVED:
        return "503 received";
    case RVSIP_CALL_LEG_REASON_GIVE_UP_DNS:
        return "Give Up DNS";
    case RVSIP_CALL_LEG_REASON_CONTINUE_DNS:
        return "Continue DNS";
    case RVSIP_CALL_LEG_REASON_OUT_OF_RESOURCES:
        return "Out Of Resources";
    case RVSIP_CALL_LEG_REASON_FORKED_CALL_NO_FINAL_RESPONSE:
        return "Forked Call-leg No Final Response";
    default:
        return "Undefined";
    }
}


/***************************************************************************
 * CallLegGetCallLegTranscStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state that belongs to a call-leg
 *          transaction.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetCallLegTranscStateName (
                                      IN  RvSipCallLegTranscState  eState)
{
    switch(eState)
    {
    case RVSIP_CALL_LEG_TRANSC_STATE_IDLE:
        return "Idle";
    case RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD:
        return "Request Rcvd";
    case RVSIP_CALL_LEG_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT:
        return "Final Response Sent";
    case RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT:
        return "Request Sent";
    case RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_PROCEEDING:
        return "Proceeding";
    case RVSIP_CALL_LEG_TRANSC_STATE_TERMINATED:
        return "Termintated";
    case RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_GEN_FINAL_RESPONSE_RCVD:
        return "Final Response Rcvd";
    case RVSIP_CALL_LEG_TRANSC_STATE_CLIENT_MSG_SEND_FAILURE:
        return "Msg Send Failure";
     default:
        return "Undefined";
    }
}



/***************************************************************************
 * CallLegGetPrackStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given pracl state.
 * Return Value: The string with the prack state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     ePrackState - The prack state.
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetPrackStateName(
                                   IN  RvSipCallLegPrackState  ePrackState)
{
    switch(ePrackState)
    {
    case RVSIP_CALL_LEG_PRACK_STATE_REL_PROV_RESPONSE_RCVD:
        return "Rel Prov Response Rcvd";
    case  RVSIP_CALL_LEG_PRACK_STATE_PRACK_SENT:
        return "Prack Sent";
    case RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD:
        return "Prack Final Response Rcvd";
    case RVSIP_CALL_LEG_PRACK_STATE_PRACK_RCVD:
        return "Prack Rcvd";
    case RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_SENT:
        return "Prack Final Response Sent";
    default:
        return "Undefined";
    }

}

/***************************************************************************
 * CallLegGetCallLegByeTranscStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state that belongs to a call-leg
 *          bye transaction.
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV CallLegGetCallLegByeTranscStateName (
                                      IN  RvSipCallLegByeState  eState)
{
    switch(eState)
    {
    case RVSIP_CALL_LEG_BYE_STATE_IDLE:
        return "Idle";
    case RVSIP_CALL_LEG_BYE_STATE_REQUEST_RCVD:
        return "Bye Request Rcvd";
    case RVSIP_CALL_LEG_BYE_STATE_RESPONSE_SENT:
        return "Bye Response Sent";
    case RVSIP_CALL_LEG_BYE_STATE_TERMINATED:
        return "Bye Terminated";
    case RVSIP_CALL_LEG_BYE_STATE_DETACHED:
        return "Bye Detached";
    default:
        return "Undefined";
    }
}
#endif  /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */



/***************************************************************************
 * CallLegUpdateReplacesStatus
 * ------------------------------------------------------------------------
 * General: Update the replaces status of a Call-Leg from a received
 *          INVITE message or REFER message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - The call-leg handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegUpdateReplacesStatus(
                                            IN CallLeg       *pCallLeg,
                                            IN RvSipMsgHandle hMsg)
{

    RvBool bHeaderFound = RV_FALSE;
    
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"Require","replaces",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pCallLeg->statusReplaces = RVSIP_CALL_LEG_REPLACES_REQUIRED;
        return RV_OK;
    }

  
    /*if we got here there is no Require - look for Supported*/
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"Supported","replaces",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pCallLeg->statusReplaces = RVSIP_CALL_LEG_REPLACES_SUPPORTED;
        return RV_OK;
    }

    /*look for compact form Supported*/
    bHeaderFound = SipMsgDoesOtherHeaderExist(hMsg,"k","replaces",NULL);

    if(bHeaderFound == RV_TRUE)
    { 
        pCallLeg->statusReplaces = RVSIP_CALL_LEG_REPLACES_SUPPORTED;
        return RV_OK;
    }

    pCallLeg->statusReplaces = RVSIP_CALL_LEG_REPLACES_UNDEFINED;
    return RV_OK;

}


/***************************************************************************
 * CallLegResetAllInviteObj
 * ------------------------------------------------------------------------
 * General: Go over the call-leg's invite list and nullify its active transaction.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
void RVCALLCONV CallLegResetAllInviteObj(IN CallLeg *pCallLeg)
{
    RLIST_ITEM_HANDLE       pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE       pNextItem   = NULL;     /*next item*/
    CallLegInvite          *pInvite     = NULL;
    RLIST_POOL_HANDLE       hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = MAX_LOOP(pCallLeg);

    /* Check if there are Invite objects to be reset*/ 
    if (hInvitePool == NULL || pCallLeg->hInviteObjsList == NULL)
    {
        return; 
    }

    /*get the first item from the list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    pInvite = (CallLegInvite*)pItem;

    /*go over all the objects*/
    while(pInvite != NULL && safeCounter < maxLoops)
    {
		pInvite->hInviteTransc = NULL;
                
        RLIST_GetNext(hInvitePool,
                      pCallLeg->hInviteObjsList,
                      pItem,
                      &pNextItem);
        
        pItem = pNextItem;
        pInvite = (CallLegInvite*)pItem;
        safeCounter++;
    }
}


/***************************************************************************
 * CallLegSetHeadersInOutboundRequestMsg
 * ------------------------------------------------------------------------
 * General: Sets the Referred-By header to the call-leg,
 *          this header will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to alocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg          - Pointer to the call-leg.
 *           hReferredByHeader - Handle to the Referred-By hedaer to be set.
 ***************************************************************************/
static RvStatus RVCALLCONV CallLegSetHeadersInOutboundRequestMsg (
                                    IN  CallLeg             *pCallLeg,
                                    IN  RvSipTranscHandle    hTransc,
                                    IN  SipTransactionMethod eRequestMethod)
{
    RvSipMsgHandle hMsg = NULL;
    RvStatus      rv;

    rv = RvSipTransactionGetOutboundMsg(hTransc, &hMsg);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSetHeadersInOutboundRequestMsg - call-leg 0x%p - Failed to get outbound msg from transc 0x%p",
            pCallLeg, hTransc));
        return rv;
    }

    /* IMPORTANT: Headers are added ONLY to INVITE request in outgoing message */
    if(SIP_TRANSACTION_METHOD_INVITE != eRequestMethod)
    {
        return RV_OK;
    }

    /* Add referred-by */
#ifdef RV_SIP_SUBS_ON    
    rv = SetSingleHeaderInOutboundReqIfExists(pCallLeg,hMsg,RVSIP_HEADERTYPE_REFERRED_BY);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSetHeadersInOutboundRequestMsg - call-leg 0x%p - Failed to set referred-by to msg (rv=%d)",
            pCallLeg,rv));
            /* we can continue in sending the message,
        even without the referred-by header */
    }
#endif /*#ifdef RV_SIP_SUBS_ON*/

    /* Add replaces */
    rv = SetSingleHeaderInOutboundReqIfExists(pCallLeg,hMsg,RVSIP_HEADERTYPE_REPLACES);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegSetHeadersInOutboundRequestMsg - call-leg 0x%p - Failed to set replaces to msg",
            pCallLeg));
        return rv;
    }

    /* Add headers list */
    if(pCallLeg->hHeadersListToSetInInitialRequest != NULL)
    {
        rv = SipMsgPushHeadersFromList(hMsg, pCallLeg->hHeadersListToSetInInitialRequest);
        if(RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegSetHeadersInOutboundRequestMsg - call-leg 0x%p - Failed to set headers list to msg",
                pCallLeg));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
* CallLegLockMgr
* ------------------------------------------------------------------------
* General: Locks the call-leg mgr. This function is used when changing parameters
*          inside the call-leg that influence the hash key of the call.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg    - Pointer to the call-leg structure
***************************************************************************/
void CallLegLockMgr(CallLeg *pCallLeg)
{
    if(pCallLeg->hashElementPtr != NULL)
    {
        RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    }
}

/***************************************************************************
* CallLegUnlockMgr
* ------------------------------------------------------------------------
* General: Unlocks the call-leg mgr. This function is used when changing parameters
*          inside the call-leg that influence the hash key of the call.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg    - Pointer to the call-leg structure
***************************************************************************/
void CallLegUnlockMgr(CallLeg *pCallLeg)
{
    if(pCallLeg->hashElementPtr != NULL)
    {
        RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    }
}


/***************************************************************************
 * CallLegGetCallLegOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets the outbound details such as address, transport, host name
 *          etc., (if they were set) that the CallLeg uses when sending a
 *          request to destintaion address.
 *          For each outbound detail, if it wasn't configured before in the
 *          CallLeg, then general SIP Stack outbound values are returned
 *          If the call-leg is not using an outbound address NULL values
 *          are returned.
 *          NOTE: You must supply a valid consecutive buffer to get the
 *                outbound strings (host name and ip address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg       - Handle to the call-leg.
 *          sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure, containing data that is going to be
 *                           retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegGetCallLegOutboundDetails(
                            IN  CallLeg                         *pCallLeg,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg)
{
    RvStatus    rv;
    RvUint16    port;

    pOutboundCfg->port = UNDEFINED;
    RV_UNUSED_ARG(sizeOfCfg);

    /* Retrieving the outbound host name */
    rv = SipTransportGetObjectOutboundHost(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           pOutboundCfg->strHostName,
                                           &port);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "CallLegGetCallLegOutboundDetails - Failed for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        return rv;
    }
    pOutboundCfg->port = ((RvUint16)UNDEFINED==port || port==0) ? UNDEFINED:port;

    /* Retrieving the outbound ip address */
    rv = SipTransportGetObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,
                                           &pCallLeg->outboundAddr,
                                           pOutboundCfg->strIpAddress,
                                           &port);
    if(rv != RV_OK)
    {
          RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "CallLegGetCallLegOutboundDetails - Failed to get outbound address of call-leg 0x%p,(rv=%d)",
                     pCallLeg, rv));
          return rv;
    }
    if (UNDEFINED == pOutboundCfg->port)
    {
        pOutboundCfg->port = ((RvUint16)UNDEFINED==port) ? UNDEFINED:port;
    }

    /* Retrieving the outbound transport type */
    rv = SipTransportGetObjectOutboundTransportType(pCallLeg->pMgr->hTransportMgr,
                                                    &pCallLeg->outboundAddr,
                                                    &pOutboundCfg->eTransport);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegGetCallLegOutboundDetails - Failed to get transport for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        return rv;
    }

#ifdef RV_SIGCOMP_ON
    /* Retrieving the outbound compression type */
    rv = SipTransportGetObjectOutboundCompressionType(pCallLeg->pMgr->hTransportMgr,
                                                      &pCallLeg->outboundAddr,
                                                      &pOutboundCfg->eCompression);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegGetCallLegOutboundDetails - Failed to get compression for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        return rv;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegGetCallLegOutboundDetails - outbound details of call-leg 0x%p: comp=%s",
              pCallLeg,SipTransportGetCompTypeName(pOutboundCfg->eCompression)));
	
	/* retrieving the outbound sigcomp-id */
	rv = SipTransportGetObjectOutboundSigcompId(pCallLeg->pMgr->hTransportMgr,
												&pCallLeg->outboundAddr,
												pOutboundCfg->strSigcompId);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegGetCallLegOutboundDetails - Failed to get sigcomp-id for CallLeg 0x%p (rv=%d)", pCallLeg,rv));
        return rv;
    }

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegGetCallLegOutboundDetails - outbound details of call-leg 0x%p: sigcomp-id=%s",
              pCallLeg,pOutboundCfg->strSigcompId));

#endif /* RV_SIGCOMP_ON */

    RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "CallLegGetCallLegOutboundDetails - outbound details of call-leg 0x%p: address=%s, host=%s, port=%d, trans=%s",
              pCallLeg, pOutboundCfg->strIpAddress,pOutboundCfg->strHostName, pOutboundCfg->port,
              SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));

    return RV_OK;
}

/***************************************************************************
* CallLegIsUpdateTransc
* ------------------------------------------------------------------------
* General: Check if the CallLeg transaction method is UPDATE
* Return Value: RV_TRUE/RV_FALSE
* ------------------------------------------------------------------------
* Arguments:
* Input:  hTransc          - Handle to the Transaction.
* Output: -
***************************************************************************/
RvBool RVCALLCONV CallLegIsUpdateTransc(IN RvSipTranscHandle hCallTransc)
{
    SipTransactionMethod  eMethod;
    RvChar                strMethodBuffer[SIP_METHOD_LEN]   = {0};

    SipTransactionGetMethod(hCallTransc,&eMethod);
    RvSipTransactionGetMethodStr(hCallTransc,SIP_METHOD_LEN,strMethodBuffer);

    if (eMethod == SIP_TRANSACTION_METHOD_OTHER &&
        TransactionMethodStricmp(strMethodBuffer , CALL_LEG_UPDATE_METHOD_STR))
    {
        return RV_TRUE;
    }

    return RV_FALSE;
}

/***************************************************************************
 * CallLegSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the call-leg's transactions timers.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pCallLeg - Handle to the call-leg object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus CallLegSetTimers(IN  CallLeg*      pCallLeg,
                          IN  RvSipTimers*  pNewTimers)
{
    RvStatus  rv = RV_OK;
    RvInt32   offset;

    if(pNewTimers == NULL)
    {
        /* reset the timers struct */
        pCallLeg->pTimers = NULL;
        RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegSetTimers - Call-leg 0x%p: reset the timers struct",
            pCallLeg));

        return RV_OK;
    }

    /* 1. allocate timers struct on the call-leg page */
    if(pCallLeg->pTimers == NULL)
    {
        pCallLeg->pTimers = (RvSipTimers*)RPOOL_AlignAppend(
                            pCallLeg->pMgr->hGeneralPool,
                            pCallLeg->hPage,
                            sizeof(RvSipTimers), &offset);
        if (NULL == pCallLeg->pTimers)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSetTimers - Call-leg 0x%p: Failed to append timers struct",
                pCallLeg));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }

    /* calculation of default values is done in transaction layer */
    memcpy((void*)pCallLeg->pTimers, (void*)pNewTimers, sizeof(RvSipTimers));

    if(pCallLeg->pTimers->inviteLingerTimeout < 0)
    {
        /* need to set a default value here, because this value is used 
           in call-leg layer (and not transaction) to set the ACK timer */
        pCallLeg->pTimers->inviteLingerTimeout = pCallLeg->pMgr->inviteLingerTimeout;
    }

     RvLogInfo(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
            "CallLegSetTimers - Call-leg 0x%p: set the timers struct",
            pCallLeg));

    return rv;
}

/***************************************************************************
 * CallLegResetIncomingRSeq
 * ------------------------------------------------------------------------
 * General: Resets the CallLeg incoming RSeq variable
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Handle to the call-leg object.
 ***************************************************************************/
void CallLegResetIncomingRSeq(IN  CallLeg      *pCallLeg)
{
	pCallLeg->incomingRSeq.val          = 0;
	pCallLeg->incomingRSeq.bInitialized = RV_FALSE; 
}

/***************************************************************************
 * CallLegSetIncomingRSeq
 * ------------------------------------------------------------------------
 * General: Sets the CallLeg incoming RSeq variable a new value
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Handle to the call-leg object.
 ***************************************************************************/
void CallLegSetIncomingRSeq(IN  CallLeg      *pCallLeg,
							IN  RvUint32	  rseqVal)
{
	pCallLeg->incomingRSeq.val          = rseqVal;
	pCallLeg->incomingRSeq.bInitialized = RV_TRUE; 
}

/***************************************************************************
 * CallLegGetIsHidden
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE is the call-leg is hidden otherwise RV_FALSE
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegGetIsHidden(IN CallLeg  *pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
	return pCallLeg->bIsHidden; 
#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(pCallLeg);

	return RV_FALSE; 
#endif /* #ifdef RV_SIP_SUBS_ON */ 
}


/***************************************************************************
 * CallLegGetIsRefer
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE is the call-leg is refer otherwise RV_FALSE
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvBool RVCALLCONV CallLegGetIsRefer(IN CallLeg *pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
	return pCallLeg->bIsReferCallLeg;
#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(pCallLeg);
	
	return RV_FALSE; 
#endif /* #ifdef RV_SIP_SUBS_ON */ 
}

/***************************************************************************
 * CallLegGetActiveReferSubs
 * ------------------------------------------------------------------------
 * General: Returns the call-leg's active refer subscription handle
 * Return Value: The active refer subs
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
RvSipSubsHandle RVCALLCONV CallLegGetActiveReferSubs(IN CallLeg *pCallLeg)
{
#ifdef RV_SIP_SUBS_ON
	return pCallLeg->hActiveReferSubs;

#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(pCallLeg);
	
	return NULL; 
#endif /* #ifdef RV_SIP_SUBS_ON */ 
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * CallLegSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Call-Leg.
 *          As a result of this operation, all messages, sent by this Call-Leg,
 *          will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pCallLeg - Pointer to the Call-Leg object.
 *          hSecObj  - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus CallLegSetSecObj(IN  CallLeg*          pCallLeg,
                          IN  RvSipSecObjHandle hSecObj)
{
    RvStatus rv;
    CallLegMgr* pCallLegMgr = pCallLeg->pMgr;

    /* If Security Object was already set, detach from it */
    if (NULL != pCallLeg->hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pCallLegMgr->hSecMgr,
                pCallLeg->hSecObj, -1/*delta*/,
                RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG, (void*)pCallLeg);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSetSecObj - Call-leg 0x%p: Failed to unset existing Security Object %p",
                pCallLeg, pCallLeg->hSecObj));
            return rv;
        }
        pCallLeg->hSecObj = NULL;
    }

    /* Register the Call_Leg to the new Security Object*/
    if (NULL != hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pCallLegMgr->hSecMgr,
                hSecObj, 1/*delta*/, RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
                (void*)pCallLeg);
        if (RV_OK != rv)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc ,(pCallLeg->pMgr->pLogSrc ,
                "CallLegSetSecObj - Call-leg 0x%p: Failed to set new Security Object %p",
                pCallLeg, hSecObj));
            return rv;
        }
    }

    pCallLeg->hSecObj = hSecObj;

    return RV_OK;
}
#endif /*#ifndef RV_SIP_IMS_ON*/

/******************************************************************************
 * CallLegGetCbName
 * ----------------------------------------------------------------------------
 * General: Print the name of callback represented by the bit turned on in the 
 *          mask results.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  MaskResults - the bitmap holding the callback representing bit.
 *****************************************************************************/
RvChar* CallLegGetCbName(RvInt32 MaskResults)
{
    /* check which bit is turned on in the mask result, and print its name */
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_CALL_CREATED))
        return "Call Created Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_CALL_STATE_CHANGED))
        return "Call State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_MSG_TO_SEND))
        return "Call Msg To Send Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_MSG_RCVD))
        return "Call Msg Rcvd Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_MODIFY_STATE_CHANGED))
        return "Call Modify State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_REINVITE_CREATED))
         return "Call Re-invite Created Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_REINVITE_STATE_CHANGED))
        return "Call Re-Invite State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_TRANSC_CREATED))
        return "Call Transc Created Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_TRANSC_STATE_CHANGED))
        return "Call Transc State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_BYE_CREATED))
        return "Call Bye Created Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_BYE_STATE_CHANGED))
        return "Call Bye State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_REFER_STATE_CHANGED))
        return "Call Refer State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_REFER_NOTIFY))
        return "Call Refer Notify Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_PRACK_STATE_CHANGED))
        return "Call Prack State Changed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_AUTH_CREDENTIALS_FOUND))
        return "Call Auth Credentials Found Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_AUTH_COMPLETED))
        return "Call Auth Completed Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_SESSION_TIMER_REFRESH_ALERT))
        return "Call Session-Timer Alert Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_SESSION_TIMER_NOTIFICATION))
        return "Call Session-Timer Notification Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_SESSION_TIMER_NEGOTIATION_FAULT))
        return "Call Session-Timer Negotiation Fault Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_OTHER_URL_FOUND))
        return "Call Other Url Found Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_FINAL_DEST_RESOLVED))
        return "Call Final Dest Resolved Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_SIGCOMP_NOT_RESPONDED))
        return "Call SigComp not Responded Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_NESTED_INITIAL_REQ))
        return "Call Nested invite req Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_NEW_CONN_IN_USE))
        return "Call New Conn in Use Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_CREATED_DUE_TO_FORKING))
        return "Call Created Due To Forking Ev";
    if(0 < BITS_GetBit((void*)&MaskResults, CB_CALL_PROV_RESP_RCVD))
        return "Call Provisional Response Rcvd Ev";
    else
        return "Unknown";
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/***************************************************************************
 * CallLegLock
 * ------------------------------------------------------------------------
 * General: Locks calleg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg    - pointer to Calleg to be locked
 *            pMgr        - pointer to calleg manager
 *            hLock        - calleg lock
 *          identifier  - the call-leg unique identifier
 ***************************************************************************/
static RvStatus RVCALLCONV CallLegLock(IN CallLeg             *pCallLeg,
                                        IN CallLegMgr*         pMgr,
                                        IN SipTripleLock     *tripleLock,
                                        IN RvInt32            identifier)
{


    if ((pCallLeg == NULL) || (tripleLock == NULL) || (pMgr == NULL))
    {
        return RV_ERROR_BADPARAM;
    }


    RvMutexLock(&tripleLock->hLock,pCallLeg->pMgr->pLogMgr); /*lock the call-leg*/

    if (pCallLeg->callLegUniqueIdentifier != identifier ||
        pCallLeg->callLegUniqueIdentifier == 0)
    {
        /*The CallLeg is invalid*/
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegLock - Call 0x%p: CallLeg object was destructed", pCallLeg));
        RvMutexUnlock(&tripleLock->hLock,pCallLeg->pMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}

/***************************************************************************
 * CallLegUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks a given lock.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLogMgr  - The log mgr pointer
 *            hLock    - A lock to unlock.
 ***************************************************************************/
static void RVCALLCONV CallLegUnLock(IN  RvLogMgr *pLogMgr,
                                     IN  RvMutex  *hLock)
{
   RvMutexUnlock(hLock,pLogMgr); /*unlock the call-leg*/
}
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/***************************************************************************
 * TerminateCallegEventHandler
 * ------------------------------------------------------------------------
 * General: Terminates a CallLeg object, Free the resources taken by it and
 *          remove it from the manager call-leg list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     obj        - pointer to Calleg to be deleted.
 *          reason    - reason of the calleg termination
 ***************************************************************************/
static RvStatus RVCALLCONV TerminateCallegEventHandler(IN void *obj, 
                                                       IN RvInt32  reason)
{
    CallLeg                *pCallLeg;
    RvStatus            retCode;


    pCallLeg = (CallLeg *)obj;
    if (pCallLeg == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    retCode = CallLegLockMsg(pCallLeg);
    if (retCode != RV_OK)
    {
        return retCode;
    }
    
    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "TerminateCallegEventHandler - Call-leg 0x%p is out of termination queue...",pCallLeg));

    if (RV_FALSE == CallLegGetIsHidden(pCallLeg) ||
		RV_TRUE  == CallLegGetIsRefer(pCallLeg))
    {
        CallLegChangeState(pCallLeg,RVSIP_CALL_LEG_STATE_TERMINATED, (RvSipCallLegStateChangeReason)reason);
    }
    CallLegDestruct(pCallLeg);
    CallLegUnLockMsg(pCallLeg);
    return RV_OK;

}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * InitCallLegSubsData
 * ------------------------------------------------------------------------
 * General: Initializes the CallLeg data members which are releated to
 *          Subscriptions.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - Pointer to the initialized CallLeg
 ***************************************************************************/
static void RVCALLCONV InitCallLegSubsData(IN CallLeg *pCallLeg,
                                           IN RvBool  bIsHidden)
{
    pCallLeg->hActiveReferSubs    = NULL;
    pCallLeg->hReferSubsGenerator = NULL;
    pCallLeg->hReferredByHeader   = NULL;
    pCallLeg->bFirstReferExists   = RV_FALSE;
    pCallLeg->bIsReferCallLeg     = RV_FALSE;
    pCallLeg->hSubsList           = NULL;
    pCallLeg->bIsHidden           = bIsHidden;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * TerminateCallLegSubsReferIfNeeded
 * ------------------------------------------------------------------------
 * General: Removes all the subscriptions (if application works with the
 *          'old' refer), which are related to a given CallLeg
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg        - Pointer to the initialized CallLeg
 * Output: pbEmptySubsList - Indication if the CallLeg subs-list was empty
 ***************************************************************************/
static void RVCALLCONV TerminateCallLegSubsReferIfNeeded(
                                        IN  CallLeg *pCallLeg,
                                        OUT RvBool  *pbEmptySubsList)
{
    *pbEmptySubsList = CallLegSubsIsSubsListEmpty((RvSipCallLegHandle)pCallLeg);
    if(*pbEmptySubsList == RV_FALSE &&
        pCallLeg->pMgr->bDisableRefer3515Behavior == RV_TRUE)
    {
        /* if application works with the 'old' refer, then refer subscriptions should
           be terminated here */
        CallLegSubsTerminateAllReferSubs(pCallLeg);
    }
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * NotifyReferGeneratorInTerminateIfNeeded
 * ------------------------------------------------------------------------
 * General: Notify a non-NULL REFER generator about the CallLeg
 *          terminated state.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - Pointer to the initialized CallLeg.
 *         eReason  - The TERMINATED state reason.
 ***************************************************************************/
static void RVCALLCONV NotifyReferGeneratorInTerminateIfNeeded(
                                IN CallLeg                       *pCallLeg,
                                IN RvSipCallLegStateChangeReason  eReason)
{
    RvSipSubsReferNotifyReadyReason eReferReason;

    if (NULL == pCallLeg->hReferSubsGenerator)
    {
        return;
    }

    switch (eReason)
    {
    case RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT:
        eReferReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_TIMEOUT;
        break;
    case RVSIP_CALL_LEG_REASON_CALL_TERMINATED:
        eReferReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_OBJ_TERMINATED;
        break;
    default:
        eReferReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_ERROR_TERMINATION;
    }

    /* notify the associate refer subscription about the response */
    CallLegReferNotifyReferGenerator(pCallLeg, eReferReason, NULL);
}
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * SetSingleHeaderInOutboundReqIfExists
 * ------------------------------------------------------------------------
 * General: Sets a ReferredBy header in a CallLeg's outbound request
 *          message (INVITE) if it exists in the CallLeg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg    - Pointer to the initialized CallLeg.
 *         hMsg        - The CallLeg's outbound message.
 *         eHeaderType - Type of the header to be set.
 ***************************************************************************/
static RvStatus RVCALLCONV SetSingleHeaderInOutboundReqIfExists(
                                        IN  CallLeg        *pCallLeg,
                                        IN  RvSipMsgHandle  hMsg,
                                        IN  RvSipHeaderType eHeaderType)
{
    RvStatus                  rv;
    RvSipHeaderListElemHandle hElem;
    void                     *pOrigHeader;
    void                     *pNewHeader;

    switch (eHeaderType)
    {
#ifdef RV_SIP_SUBS_ON
    case RVSIP_HEADERTYPE_REFERRED_BY:
        pOrigHeader = (void *)pCallLeg->hReferredByHeader;
        break;
#endif /* #ifdef RV_SIP_SUBS_ON */
    case RVSIP_HEADERTYPE_REPLACES:
        /* Set the header only in not-connected state */
        if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ||
           pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)
        {
            return RV_OK;
        }
        pOrigHeader = (void *)pCallLeg->hReplacesHeader;
        break;
    default:
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetSingleHeaderInOutboundReqIfExists - call-leg 0x%p - cannot set header type %s in outbound request",
            pCallLeg,SipHeaderName2Str(eHeaderType)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (pOrigHeader == NULL)
    {
        return RV_OK;
    }
    rv = RvSipMsgPushHeader(hMsg,RVSIP_FIRST_HEADER,pOrigHeader,eHeaderType,&hElem,&pNewHeader);
    if(RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "SetSingleHeaderInOutboundReqIfExists - call-leg 0x%p - Failed to set %s header to msg (rv=%d)",
            pCallLeg,SipHeaderName2Str(eHeaderType),rv));
        return rv;
    }

    return RV_OK;
}

/* ---------- TRANSC LIST FUNCTIONS ------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * GetTranscListActionName
 * ------------------------------------------------------------------------
 * General: Gets the CallLegTranscListAction name as string
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eAction - action.
 ***************************************************************************/
static RvChar* GetTranscListActionName(CallLegTranscListAction eAction)
{
    switch (eAction)
    {
    case TRANSC_LIST_TERMINATE_ALL_TRANSC:
        return "Terminate All Transc";
    case TRANSC_LIST_REMOVE_TRANSC_FROM_LIST:
        return "Remove Transc From List";
    case TRANSC_LIST_FIND_PRACK_TRANSC:
        return "Find Prack Transc";
    case TRANSC_LIST_VERIFY_NO_ACTIVE:
        return "Verify No Active";
    case TRANSC_LIST_RESET_APP_HANDLE_ALL_TRANSC:
        return "Reset App Handle In all Transcs";
    case TRANSC_LIST_DETACH_BYE:
        return "Detach Bye";
    case TRANSC_LIST_STOP_GENERAL_RETRANS:
        return "Stop Gen Retransmissions";
    case TRANSC_LIST_487_ON_GEN_PENDING:
        return "Send 487 On Pending Gen";
    default:
        return "Undefined";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/***************************************************************************
 * CallLegTerminateAllGeneralTransc
 * ------------------------------------------------------------------------
 * General: Terminate the transaction found in the list, using the transaction API.
 *          The transaction is not removed from the list. 
 *          It will be removed when the event terminated will be received.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
static void RVCALLCONV GeneralTranscListTerminateAll(IN CallLeg*          pCallLeg,
                                                     IN RvSipTranscHandle hTransc,
                                                     IN RLIST_ITEM_HANDLE pItem)
{
    RvSipTranscOwnerHandle  hTranscOwner = NULL;
    RvStatus                rv;
    RLIST_POOL_HANDLE       hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;    
    
    /*terminate the transaction*/
    RvSipTransactionTerminate(hTransc);
	
	/* Find if the owner of the Transaction is the current CallLeg */
    rv = RvSipTransactionGetOwner(hTransc,&hTranscOwner);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "GeneralTranscListTerminateAll - Call-leg 0x%p - failed to retrieve the owner of Transc 0x%p.",
                   pCallLeg,hTransc));
    }
    /* If the Transc is not part of the Call-Leg it should */
    /* be removed from its list */
    if ((CallLeg *)hTranscOwner != pCallLeg)
    {
        RLIST_Remove(hTranscPool,pCallLeg->hGeneralTranscList,pItem);
    }
}


/***************************************************************************
 * GeneralTranscListRemoveFromList
 * ------------------------------------------------------------------------
 * General: Terminate the transaction found in the list, using the transaction API.
 *          The transaction is not removed from the list. 
 *          It will be removed when the event terminated will be received.
 * Return Value: TRUE - if the transaction was found
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
static RvBool RVCALLCONV GeneralTranscListRemoveFromList(IN CallLeg*          pCallLeg,
                                                       IN RvSipTranscHandle hTransc,
                                                       IN RvSipTranscHandle hTranscForCompare,
                                                       IN RLIST_ITEM_HANDLE pItem)
{
    RLIST_POOL_HANDLE       hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;    
    
    if(hTransc != NULL && hTransc == hTranscForCompare)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
              "GeneralTranscListRemoveFromList - call-leg 0x%p remove transc 0x%p from transc list"
              ,pCallLeg,hTransc));
        RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
        RLIST_Remove(hTranscPool, pCallLeg->hGeneralTranscList,pItem);
        RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
        return RV_TRUE;
    }

    return RV_FALSE;
}

/***************************************************************************
 * GeneralTranscListFindPrack
 * ------------------------------------------------------------------------
 * General: Search for a PRACK transaction in the list, to respond on. 
 * Return Value: TRUE - if the transaction was found
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
static RvBool RVCALLCONV GeneralTranscListFindPrack(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc)
{
    RvSipTranscHandle     hPrackParent = NULL;
    RvSipTransactionState ePrackState = RVSIP_TRANSC_STATE_UNDEFINED;

    /* Finds the PRACK transaction that suits the active transaction. */

    SipTransactionGetPrackParent(hTransc, &hPrackParent);
    RvSipTransactionGetCurrentState(hTransc,&ePrackState);

    if (hPrackParent == pCallLeg->hActiveTransc &&
        ePrackState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD)
    {
        return RV_TRUE;
    }
    return RV_FALSE;
}


/***************************************************************************
 * GeneralTranscListVerifyNoActive
 * ------------------------------------------------------------------------
 * General: The function verify that there is no transaction in active state
 *          in the call-leg.
 * Return Value: TRUE - if active transaction was found
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
static RvBool RVCALLCONV GeneralTranscListVerifyNoActive(IN CallLeg*          pCallLeg,
                                                         IN RvSipTranscHandle hTransc)
{
    if(RV_TRUE == SipTransactionIsInActiveState(hTransc))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
           "GeneralTranscListVerifyNoActive: call-leg 0x%p. active transaction 0x%p was found",
            pCallLeg, hTransc));
        /* found an active transaction */
        return RV_TRUE;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

    return RV_FALSE;    
}

/***************************************************************************
 * GeneralTranscListResetAppActive
 * ------------------------------------------------------------------------
 * General: Set AppTranscHandle of all transactions in transaction list
 *          to be NULL.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
static void RVCALLCONV GeneralTranscListResetAppActive(IN RvSipTranscHandle hTransc)
{     
    RvSipTransactionSetAppHandle(hTransc, NULL);
}

/***************************************************************************
 * GeneralTranscListDetachBye
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, finds a BYE transaction
 *          detach it, and remove it from call-leg list.
 * Return Value: TRUE - if the BYE transaction was found
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
static RvBool RVCALLCONV GeneralTranscListDetachBye(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RLIST_ITEM_HANDLE pItem)
{
    RLIST_POOL_HANDLE       hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;
    RvSipTransactionState   eState  = RVSIP_TRANSC_STATE_UNDEFINED;
    SipTransactionMethod    eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    RvStatus rv = RV_OK;
           
	RvSipTransactionGetCurrentState(hTransc, &eState);
    SipTransactionGetMethod(hTransc, &eMethod);

    if(eState == RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT  &&
       eMethod == SIP_TRANSACTION_METHOD_BYE)
    {
        /* Detach the transaction*/
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GeneralTranscListDetachBye - Call-leg 0x%p - Detaching Bye transaction 0x%p.", pCallLeg, hTransc));
        rv = RvSipTransactionDetachOwner(hTransc);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "GeneralTranscListDetachBye - Call-leg 0x%p - Failed to detach Bye transaction 0x%p.", pCallLeg, hTransc));
        }
        else
        {
            RLIST_Remove(hTranscPool,pCallLeg->hGeneralTranscList,pItem);
        } 
        return RV_TRUE;
    }
    
    return RV_FALSE;
}

/***************************************************************************
 * GeneralTranscListStopGeneral
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, and terminate all the 
 *          transactions that are sending messages, to stop retransmissions.
 *          The transaction is not removed from the list. 
 *          It will be removed when the event terminated will be received.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
static void RVCALLCONV GeneralTranscListStopGeneral(IN CallLeg*          pCallLeg,
                                                    IN RvSipTranscHandle hTransc,
                                                    IN RLIST_ITEM_HANDLE pItem)
{
    RvSipTranscOwnerHandle  hTranscOwner = NULL;
    RLIST_POOL_HANDLE       hTranscPool = pCallLeg->pMgr->hTranscHandlesPool;
    RvSipTransactionState   eState = RVSIP_TRANSC_STATE_UNDEFINED;
    RvStatus rv;
       
	RvSipTransactionGetCurrentState(hTransc, &eState);
    
    if(eState == RVSIP_TRANSC_STATE_CLIENT_GEN_REQUEST_SENT ||
       eState == RVSIP_TRANSC_STATE_SERVER_GEN_FINAL_RESPONSE_SENT)
    {
        /*terminate the transaction*/
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "GeneralTranscListStopGeneral - Call-leg 0x%p - terminate transaction 0x%p.", pCallLeg, hTransc));
        RvSipTransactionTerminate(hTransc);
   
        /* Find if the owner of the Transaction is the current CallLeg */
        rv = RvSipTransactionGetOwner(hTransc,&hTranscOwner);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                       "GeneralTranscListStopGeneral - Call-leg 0x%p - failed to retrieve the owner of Transc 0x%p.",
                       pCallLeg,hTransc));
        }
        else if ((CallLeg *)hTranscOwner != pCallLeg)
        {
            /* If the transc is not part of the Call-Leg it should be removed from its list */
            RLIST_Remove(hTranscPool,pCallLeg->hGeneralTranscList,pItem);
        }
    }
}

/***************************************************************************
 * GeneralTranscListSend487OnPending
 * ------------------------------------------------------------------------
 * General: Go over the call-leg transaction list, Send 487 for all the transactions
 *          (except BYE) that are in state GEN_REQUEST_RCVD. 
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
static void RVCALLCONV GeneralTranscListSend487OnPending(IN CallLeg*          pCallLeg,
                                                         IN RvSipTranscHandle hTransc)
{
    RvSipTransactionState   eState = RVSIP_TRANSC_STATE_UNDEFINED;
    SipTransactionMethod    eMethod = SIP_TRANSACTION_METHOD_UNDEFINED;
    RvStatus rv;
       
	RvSipTransactionGetCurrentState(hTransc, &eState);
    SipTransactionGetMethod(hTransc, &eMethod);
   
    if(eState == RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD &&
       eMethod != SIP_TRANSACTION_METHOD_BYE)
    {
        /*send 487 on the transaction*/
        RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
         "GeneralTranscListSend487OnPending - Call-leg 0x%p - respond with 487 to pending transc 0x%p ", 
         pCallLeg, hTransc));
        rv = RvSipTransactionRespond(hTransc,487,NULL);
        if (rv != RV_OK)
        {
            /*transaction failed to send respond*/
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                 "GeneralTranscListSend487OnPending - Call-leg 0x%p - Failed to send 487 for pending transc 0x%p (rv=%d)",
                  pCallLeg, hTransc, rv));
        }
    } 
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif
}

/***************************************************************************
 * DestructAllInvites
 * ------------------------------------------------------------------------
 * General: Go over the call-leg's invite list and destruct the objects
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
static void RVCALLCONV DestructAllInvites(IN CallLeg *pCallLeg)
{
    RLIST_ITEM_HANDLE       pItem       = NULL;     /*current item*/
    RLIST_ITEM_HANDLE       pNextItem   = NULL;     /*next item*/
    CallLegInvite          *pInvite     = NULL;
    RLIST_POOL_HANDLE       hInvitePool = pCallLeg->pMgr->hInviteObjsListPool;

    RvUint32 safeCounter = 0;
    RvUint32 maxLoops = MAX_LOOP(pCallLeg);

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
             "DestructAllInvites - Call-leg 0x%p - destructing all its invite objects.",pCallLeg));

    /* Check if there are Invite objects to be destructed at all */ 
    if (hInvitePool == NULL || pCallLeg->hInviteObjsList == NULL)
    {
        return; 
    }

    /*get the first item from the list*/
    RLIST_GetHead(hInvitePool, pCallLeg->hInviteObjsList, &pItem);

    pInvite = (CallLegInvite*)pItem;

    /*go over all the objects*/
    while(pInvite != NULL && safeCounter < maxLoops)
    {
        CallLegInviteDestruct(pCallLeg,pInvite);
                
        RLIST_GetNext(hInvitePool,
                      pCallLeg->hInviteObjsList,
                      pItem,
                      &pNextItem);
        
        pItem = pNextItem;
        pInvite = (CallLegInvite*)pItem;
        safeCounter++;
    }

    /*infinite loop*/
    if (pInvite != NULL && safeCounter == maxLoops)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "DestructAllInvites - Call-leg 0x%p - Reached MaxLoops %d, inifinte loop",
                  pCallLeg,maxLoops));
    }

}



#endif /*RV_SIP_PRIMITIVES */

