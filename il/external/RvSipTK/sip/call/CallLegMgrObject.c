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
 *                              <CallLegMgr.c>
 *     The call-leg manager object holds all the call-leg module resources
 *   Including memory pool, pools of lists, locks and more.
 *   Its main functionality is to manage the call-leg list, to create new
 *   call-legs and to associate new transaction to a specific call-leg according
 *   to the transaction key.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipCallLeg.h"
#include "_SipCallLeg.h"
#include "CallLegRefer.h"
#include "CallLegCallbacks.h"
#include "CallLegMgrObject.h"
#include "CallLegObject.h"
#include "CallLegTranscEv.h"
#include "CallLegMsgEv.h"
#include "CallLegForking.h"
#include "RvSipReferToHeader.h"
#include "RvSipReplacesHeader.h"
#include "_SipReplacesHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "_SipMsg.h"
#include "_SipTransport.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* CallLegMgrHashKey
 * ---------------------------------------------------------------------------
 */

typedef struct {
    CallLegMgr             *pMgr;
    RPOOL_Ptr              pCallId;
    RvSipPartyHeaderHandle *phFrom;
    RvSipPartyHeaderHandle *phTo;
    RvSipCallLegDirection  eDirection;
} CallLegMgrHashKey;

/* CallLegMgrHashElem
 * ---------------------------------------------------------------------------
 */

typedef struct {
    CallLegMgrHashKey   hashKey;
    RvSipReplacesHeaderHandle hReplacesHashKey;
    RvBool              bIsReplacesSearch;
    RvBool              bOnlyEstablishedCall;
    RvBool              *pbOriginalCall;
    RvBool              *pbIsHidden;
} CallLegMgrHashElem;
/* Remark: if reference to the field in Call-Leg object is added to
           CallLegMgrHashElem struct (e.g. 'pbOriginalCall'),ensure that every
           access of the Call-Leg object field is surrounded by Call-Leg Manager
           Lock/Unlock pair.
           This should be done in order to prevent the field update, while
           search in hash is being performed. */

/* CallLegMgrCallHashKeyDetailed
 * ----------------------------------------------------------------------------
 * Contains dialog parameters, that are used for Call identification in hash.
 *
 * hHeaderTo    - handle of the FROM header
 * hHeaderFrom  - handle of the TO header
 * strToTag     - pointer to location details of string, representing TO tag
 * strFromTag   - pointer to location details of string, representing FROM tag
 * strCallID    - pointer to location details of string, representing Call ID
 * eDirection   - direction of dialog (incoming or outgoing)
 */
typedef struct {
    RvSipPartyHeaderHandle   hHeaderTo;
    RvSipPartyHeaderHandle   hHeaderFrom;
    RPOOL_Ptr                strToTag;
    RPOOL_Ptr                strFromTag;
    RPOOL_Ptr                strCallID;
    RvSipCallLegDirection    eDirection;
} CallLegMgrCallHashKeyDetailed;

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvUint HashFunction(void *key);

static void HashFind(IN  CallLegMgr          *pMgr,
                     IN  CallLegMgrHashElem  *pHashElem,
                     IN  HASH_CompareFunc    pfnCompareFunc,
                     OUT RvSipCallLegHandle  **pphCallLeg);

static RvBool HashCompare(IN CallLegMgrCallHashKeyDetailed  *pNewCall,
                          IN CallLegMgrCallHashKeyDetailed  *pExistCall,
                          IN RvLogSource                    *pLogSrc);

static RvBool CallHashCompare(IN void *newHashElement,
                              IN void *oldHashElement);

static RvBool CallHashCompareReplaces( IN void *newHashElement,
                                                IN void *oldHashElement);

static RvBool  CallHashCompareOriginal(IN void *newHashElement,
                                       IN void *oldHashElement);

static RvBool  CallHashCompareHidden(IN void *newHashElement,
                                     IN void *oldHashElement);

static RvBool  CallHashCompareOriginalHidden(IN void *newHashElement,
                                             IN void *oldHashElement);

static void ConvertHashKey2CallDetails(
                            IN   CallLegMgrHashKey              *pHashKey,
                            OUT  CallLegMgrCallHashKeyDetailed  *pCallDetails);

static void ConvertTransactionKey2HashKey(
                            IN   SipTransactionKey      *pTransactionKey,
                            IN   CallLegMgr             *pMgr,
                            IN   RvSipCallLegDirection  eTransactionDirection,
                            OUT  CallLegMgrHashKey      *pHashKey);

static RvBool CallLegMgrGetHashElementIsHidden(IN CallLegMgrHashElem *pHashElem);

#ifdef RFC_2543_COMPLIANT
static RvStatus CallLegMgrDealWithEmptyTag(
									IN  CallLegMgrHashKey              *pExistHashKey,
									IN  CallLegMgrCallHashKeyDetailed  *pExistCallDetails,
									IN  CallLegMgrCallHashKeyDetailed  *pNewCallDetails); 
#endif /* #ifdef RFC_2543_COMPLIANT */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * CallLegMgrCreateCallLeg
 * ------------------------------------------------------------------------
 * General: Creates a new call-leg with a given direction. The new call-leg
 *          is initialized with initial values.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Call list is full, no call-leg created.
 *               RV_OK - a new call-leg was created and initialized
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr -       The manager of the new call-leg
 *          eDirection - Direction (incoming/outgoing) of the new callLeg.
 *          bIsHidden   - is this is a hidden call-leg (for independent subscription)
 *                       or not.
 * Output:  ppCallLeg  - Pointer to a newly created callLeg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrCreateCallLeg(
                                  IN    CallLegMgr              *pMgr,
                                  IN    RvSipCallLegDirection    eDirection,
                                  IN    RvBool                   bIsHidden,
                                  OUT   RvSipCallLegHandle      *phCallLeg)
{
    RvStatus            rv;
    CallLeg            *pCallLeg;
    RLIST_ITEM_HANDLE   listItem;

    if(pMgr->hCallLegPool == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegMgrCreateCallLeg - Failed. The call-leg manager was initialized with 0 calls"));
        return RV_ERROR_UNKNOWN;
    }
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    rv = CallLegMgrCheckObjCounters(pMgr,bIsHidden,RV_TRUE);
    if (rv != RV_OK)
    {
        *phCallLeg = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrCreateCallLeg - Cannot create new CallLeg due to CallLegMgr counters check"));
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return rv;
    }
    rv = RLIST_InsertTail(pMgr->hCallLegPool,pMgr->hCallLegList,&listItem);

    /*if there are no more available items*/
    if(rv != RV_OK)
    {
        *phCallLeg = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegMgrCreateCallLeg - Failed to insert new call-leg to list (rv=%d)",
                  rv));
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    else
    {
        rv = CallLegMgrUpdateObjCounters(pMgr,bIsHidden,RV_TRUE);
        if (rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrCreateCallLeg - Failure in CallLegMgrUpdateObjCounters for call-leg, bIsHidden=%d", bIsHidden));
        }
    }

    pCallLeg = (CallLeg*)listItem;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
             "CallLegMgrCreateCallLeg - call-leg 0x%p was created. bIsHidden=%d", pCallLeg, bIsHidden));

    rv = CallLegInitialize(pCallLeg,pMgr,eDirection,bIsHidden);
      /*if there are no more available items*/
    if(rv != RV_OK)
    {
        RLIST_Remove(pMgr->hCallLegPool, pMgr->hCallLegList,
                 (RLIST_ITEM_HANDLE)pCallLeg);
        *phCallLeg = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                  "CallLegMgrCreateCallLeg - Failed to initialize the new call-leg (rv=%d)",
                  rv));
        (bIsHidden == RV_TRUE)?(--pMgr->numOfAllocatedHiddenCalls):(--pMgr->numOfAllocatedCalls);

        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    *phCallLeg = (RvSipCallLegHandle)pCallLeg;
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
    return RV_OK;

}
/***************************************************************************
* CallLegMgrNotifyCallLegCreated
* ------------------------------------------------------------------------
* General: Notifies the application/subscrition module, that a new call-leg
*          was created.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr       - Pointer to the call-leg manager.
*            pCallLeg   - The newly created call-leg
*               hTransc  - The transaction that caused this call leg to be created.
***************************************************************************/
RvStatus CallLegMgrNotifyCallLegCreated(IN  CallLegMgr*           pMgr,
                                      IN  RvSipCallLegHandle    hCallLeg,
                                      IN  RvSipTranscHandle     hTransc)
{
    CallLeg* pCallLeg = (CallLeg*)hCallLeg;

    if(RV_TRUE  == CallLegGetIsHidden(pCallLeg) && 
	   RV_FALSE == CallLegGetIsRefer(pCallLeg))
    {
        return RV_OK;
    }

    /* set the active transaction - for the time that this callback is called,
    to be able getting the received message in the callback (for cpp wrapper)*/
    RvSipTransactionGetReceivedMsg(hTransc, &(pCallLeg->hReceivedMsg));
    CallLegSetActiveTransc(pCallLeg, hTransc);

    /*notify the application that a new incoming call was created*/
    CallLegCallbackCreatedEv(pMgr,(RvSipCallLegHandle)pCallLeg);

    /* reset back the active transaction */
    CallLegResetActiveTransc(pCallLeg);
    pCallLeg->hReceivedMsg = NULL;
    return RV_OK;
}

/***************************************************************************
 * CallLegMgrHashInit
 * ------------------------------------------------------------------------
 * General: Initialize the call-leg hash table. the number of entries is
 *          maxCallLegs*2+1.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to construct hash table
 *               RV_OK - hash table was constructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       The call-leg manager.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrHashInit(CallLegMgr *pMgr)
{
    RvInt32 totalNumOfCalls = pMgr->maxNumOfCalls;

    totalNumOfCalls += pMgr->maxNumOfHiddenCalls;

    pMgr->hashSize = HASH_DEFAULT_TABLE_SIZE(totalNumOfCalls);
    pMgr->hHashTable = HASH_Construct(HASH_DEFAULT_TABLE_SIZE(totalNumOfCalls),
                                      totalNumOfCalls,
                                      HashFunction,
                                      sizeof(CallLeg*),
                                      sizeof(CallLegMgrHashElem),
                                      pMgr->pLogMgr,
                                      "Call-Leg Hash");
    if(pMgr->hHashTable == NULL)
        return RV_ERROR_OUTOFRESOURCES;
    return RV_OK;
}

/***************************************************************************
 * CallLegMgrHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a call-leg into the hash table.
 *          The key is generated from the call-leg information and the
 *          call-leg handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Failed to construct hash table
 *               RV_OK - Call-leg handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg -       Handle of the call-leg to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrHashInsert(IN RvSipCallLegHandle hCallLeg)
{
    RvStatus         rv;
    CallLeg*          pCallLeg = (CallLeg*)hCallLeg;
    CallLegMgrHashKey hashKey;
    RvBool           wasInsert;
    CallLegMgrHashElem hashElem;

    /*return if the element is already in the hash*/
    if(pCallLeg->hashElementPtr != NULL)
    {
        return RV_OK;
    }

    /*prepare hash key*/
    hashKey.pMgr           = pCallLeg->pMgr;
    hashKey.pCallId.hPool  = pCallLeg->pMgr->hGeneralPool;
    hashKey.pCallId.hPage  = pCallLeg->hPage;
    hashKey.pCallId.offset = pCallLeg->strCallId;
    hashKey.phFrom          = &(pCallLeg->hFromHeader);
    hashKey.phTo            = &(pCallLeg->hToHeader);

    hashKey.eDirection     = pCallLeg->eDirection;

    hashElem.bOnlyEstablishedCall = RV_FALSE;
    hashElem.bIsReplacesSearch = RV_FALSE;
    memcpy(((void *)&(hashElem.hashKey)), (void *)&hashKey, sizeof(CallLegMgrHashKey));
    hashElem.hReplacesHashKey = NULL;
    hashElem.pbOriginalCall = &(pCallLeg->bOriginalCall);

#ifdef RV_SIP_SUBS_ON
    hashElem.pbIsHidden = &pCallLeg->bIsHidden;
#endif /* #ifdef RV_SIP_SUBS_ON */

    /*insert to hash and save hash element id in the call-leg*/
    RvMutexLock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);
    rv = HASH_InsertElement(pCallLeg->pMgr->hHashTable,
                            &hashElem,
                            &pCallLeg,
                            RV_FALSE,
                            &wasInsert,
                            &(pCallLeg->hashElementPtr),
                            CallHashCompare);

    RvMutexUnlock(&pCallLeg->pMgr->hMutex,pCallLeg->pMgr->pLogMgr);

    if(rv != RV_OK ||wasInsert == RV_FALSE)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegMgrHashInsert - Call-leg 0x%p - was inserted to hash ",pCallLeg));
    return RV_OK;
}

/***************************************************************************
 * CallLegMgrHashFind
 * ------------------------------------------------------------------------
 * General: find a call-leg in the hash table according to the call-leg key.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr -       Handle of the call-leg manager
 *          pKey -       The call-leg key
 *          eDirection - the direction of the call
 *          bOnlyEstablishedCall - Indicates if we search only call-legs with
 *                       to-tag, or also call-leg without to-tag.
 * Output:  phCallLeg -  Handle to the call-leg found in the hash or null
 *                       if the call-leg was not found.
 ***************************************************************************/
void RVCALLCONV CallLegMgrHashFind(
                            IN  CallLegMgr              *pMgr,
                            IN  SipTransactionKey       *pKey,
                            IN  RvSipCallLegDirection   eDirection,
                            IN  RvBool                  bOnlyEstablishedCall,
                            OUT RvSipCallLegHandle      **phCallLeg)
{
    CallLegMgrHashElem  hashElem;

    hashElem.bIsReplacesSearch  = RV_FALSE;
    hashElem.hReplacesHashKey   = NULL;
    ConvertTransactionKey2HashKey(pKey,pMgr,eDirection,&(hashElem.hashKey));

    /* Lock the Call-Leg Manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    hashElem.bOnlyEstablishedCall = bOnlyEstablishedCall;

    HashFind(pMgr,&hashElem,CallHashCompare,phCallLeg);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
}

/******************************************************************************
 * CallLegMgrHashFindOriginal
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Original.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
void RVCALLCONV CallLegMgrHashFindOriginal(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eKeyDirection,
                           OUT RvSipCallLegHandle     **phCallLeg)
{
    CallLegMgrHashElem hashElem;

    hashElem.bIsReplacesSearch      = RV_FALSE;
    hashElem.hReplacesHashKey       = NULL;
    hashElem.bOnlyEstablishedCall   = RV_FALSE;
    ConvertTransactionKey2HashKey(
           pKey, pMgr, eKeyDirection, &(hashElem.hashKey));

    /*lock the call-leg manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    HashFind(pMgr,&hashElem,CallHashCompareOriginal,phCallLeg);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
    return;
}

/******************************************************************************
 * CallLegMgrHashFindHidden
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Hidden.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
void RVCALLCONV CallLegMgrHashFindHidden(
                           IN  CallLegMgr             *pMgr,
                           IN  SipTransactionKey      *pKey,
                           IN  RvSipCallLegDirection  eKeyDirection,
                           OUT RvSipCallLegHandle     **phCallLeg)
{
    CallLegMgrHashElem hashElem;

    hashElem.bIsReplacesSearch      = RV_FALSE;
    hashElem.hReplacesHashKey       = NULL;
    hashElem.bOnlyEstablishedCall   = RV_FALSE;
    ConvertTransactionKey2HashKey(
           pKey, pMgr, eKeyDirection, &(hashElem.hashKey));

    /*lock the call-leg manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    HashFind(pMgr,&hashElem,CallHashCompareHidden,phCallLeg);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
    return;
}

/******************************************************************************
 * CallLegMgrHashFindOriginalHidden
 * ----------------------------------------------------------------------------
 * General: search Hash for Call-Leg with dialog identifiers, identical to
 *          those, supplied by Transaction Key parameter.
 *          In addition the Call-Leg should be Hidden and Original.
 *          Call-Leg is Hidden, if it serves out-of-dialog Subscription.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pKey          - Key for Call-Leg search.
 *          eKeyDirection - Direction of Transaction Key tags. If the direction
 *                          is opposite to direction of candidate Call-Leg,
 *                          the TO and FROM tags will be switched during check.
 * Output:  phCallLeg     - Pointer to memory, where the handle of the found
 *                          Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
void RVCALLCONV CallLegMgrHashFindOriginalHidden(
                                    IN  CallLegMgr              *pMgr,
                                    IN  SipTransactionKey       *pKey,
                                    IN  RvSipCallLegDirection   eKeyDirection,
                                    OUT RvSipCallLegHandle      **phCallLeg)
{
    CallLegMgrHashElem hashElem;

    hashElem.bIsReplacesSearch      = RV_FALSE;
    hashElem.hReplacesHashKey       = NULL;
    hashElem.bOnlyEstablishedCall   = RV_FALSE;
    ConvertTransactionKey2HashKey(
           pKey, pMgr, eKeyDirection, &(hashElem.hashKey));

    /*lock the call-leg manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    HashFind(pMgr,&hashElem,CallHashCompareOriginalHidden,phCallLeg);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
    return;
}


/***************************************************************************
 * CallLegMgrReplacesHashFind
 * ------------------------------------------------------------------------
 * General: find a call-leg in the hash table according to the Replaces
 *          header parameters.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr                   - Handle of the call-leg manager
 *          hReplacesHeader        - Handle to the Replaces header.
 * Output:  phCallLeg              - Handle to the call-leg found in the hash or null
 *                                   if the call-leg was not found.
 *          peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog doesn't fit.
 *                     application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces.
 ***************************************************************************/
void RVCALLCONV CallLegMgrHashFindReplaces(
                           IN  CallLegMgr                *pMgr,
                           IN  RvSipReplacesHeaderHandle  hReplacesHeader,
                           OUT RvSipCallLegHandle       **phCallLeg,
                           OUT RvSipCallLegReplacesReason *peReason)
{
    RvStatus                rv;
    CallLegMgrHashElem      hashElem;
    RvSipCallLegReferState  eReferState = RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE;
    RvSipCallLegState       eState      = RVSIP_CALL_LEG_STATE_IDLE;
    RvSipReplacesEarlyFlagType eEarlyFlag;

    memset(((void *)&(hashElem.hashKey)), 0, sizeof(CallLegMgrHashKey));

    hashElem.hashKey.pMgr      = pMgr;
    hashElem.bIsReplacesSearch = RV_TRUE;
    hashElem.hReplacesHashKey  = hReplacesHeader;

    /*lock the call-leg manager*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    HashFind(pMgr,&hashElem,CallHashCompareReplaces,phCallLeg);

    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    if(NULL == *phCallLeg)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrHashFindReplaces: did not find a call-leg to replace."));
        *peReason = RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_NOT_FOUND;
        return;
    }

    RvSipCallLegGetCurrentState((RvSipCallLegHandle)((CallLeg *)*phCallLeg), &eState);
#ifdef RV_SIP_SUBS_ON
    eReferState = CallLegReferGetCurrentReferState((RvSipCallLegHandle)((CallLeg *)*phCallLeg));
#endif /* #ifdef RV_SIP_SUBS_ON */

    if(eState == RVSIP_CALL_LEG_STATE_TERMINATED)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrHashFindReplaces: found call-leg 0x%p. in state TERMINATED",
            *phCallLeg));
        *phCallLeg = NULL;
        *peReason = RVSIP_CALL_LEG_REPLACES_REASON_FOUND_TERMINATED_DIALOG;
        return;
    }

    if(RV_TRUE == CallLegGetIsHidden((CallLeg *)*phCallLeg) || /* Call was not created with INVITE --> subscription */
       ((eState == RVSIP_CALL_LEG_STATE_IDLE) &&
        (eReferState != RVSIP_CALL_LEG_REFER_STATE_REFER_IDLE)) )
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrHashFindReplaces: found non relevant call-leg 0x%p. state %s. bIsHidden %d",
            *phCallLeg, CallLegGetStateName(eState), CallLegGetIsHidden((CallLeg *)*phCallLeg)));
        *phCallLeg = NULL;
        *peReason = RVSIP_CALL_LEG_REPLACES_REASON_FOUND_NON_INVITE_DIALOG;
        return;
    }
    
#ifndef RVSIP_LIMIT_REPLACES_VALIDITY_CHECKS
    /*cannot replace a call in the offering state - new in replaces 03*/
    if(eState == RVSIP_CALL_LEG_STATE_OFFERING)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrHashFindReplaces: found call-leg 0x%p in state offering. cannot replace it.",
            *phCallLeg));
        *phCallLeg = NULL;
        *peReason = RVSIP_CALL_LEG_REPLACES_REASON_FOUND_INCOMING_EARLY_DIALOG;
        return;
    }
#endif /*#ifndef RVSIP_LIMIT_REPLACES_VALIDITY_CHECKS*/

    /* from draft-replaces-04:
       If the Replaces header field matches a confirmed dialog, it checks
       for the presence of the "early-only" flag in the Replaces header
       field. (This flag allows the UAC to prevent a potentially undesirable
       race condition described in Section 7.1.) If the flag is present, the
       UA rejects the request with a 486 Busy response. Otherwise it accepts
       the new INVITE by sending a 200-class response, and shuts down the
       replaced dialog by sending a BYE. If the Replaces header field
       matches an early dialog that was initiated by the UA, it accepts the
       new INVITE by sending a 200-class response, and shuts down the
       replaced dialog by sending a CANCEL.*/
    rv = RvSipReplacesHeaderGetEarlyFlagParam(hReplacesHeader, &eEarlyFlag);
    if(rv == RV_OK &&
       (eEarlyFlag == RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_1 ||
        eEarlyFlag == RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_EMPTY ||
        eEarlyFlag == RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_TRUE))
    {
        if(eState != RVSIP_CALL_LEG_STATE_INVITING &&
           eState != RVSIP_CALL_LEG_STATE_PROCEEDING &&
           eState != RVSIP_CALL_LEG_STATE_OFFERING)
        {
            /* this is a confirmed dialog */
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CallLegMgrHashFindReplaces: found call-leg 0x%p in state %s, and early-only param exists in replaces header. cannot replace it",
                *phCallLeg, CallLegGetStateName(eState)));
            *phCallLeg = NULL;
            *peReason = RVSIP_CALL_LEG_REPLACES_REASON_FOUND_CONFIRMED_DIALOG;
            return;
        }
    }

    *peReason = RVSIP_CALL_LEG_REPLACES_REASON_DIALOG_FOUND_OK;
     RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "CallLegMgrHashFindReplaces: found call-leg 0x%p to replace.",
        *phCallLeg));
}

/***************************************************************************
 * CallLegMgrFindCallLegByMsg
 * ------------------------------------------------------------------------
 * General: The function search for a call-leg with same identifiers as in
 *          the given message (1xx, 2xx or ACK)
 *          Note that if the function succeed, the retrieved call-leg is locked.
 * Return Value: RV_Status - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr   - The transaction manager handle.
 *          hAckMsg      - The handle to the received ACK message.
 * Output:  pbWasHandled - Indicates if the message was handled in the
 *                         callback.
 *          pCurIdentifier - The call-leg identifier before unlocking the call mgr.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegMgrFindCallLegByMsg(
                     IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hMsg,
                     IN   RvSipCallLegDirection   eDirection,
                     IN   RvBool                  bOnlyEstablishedCall,
                     OUT  RvSipCallLegHandle     *phCallLeg,
                     OUT  RvInt32*               pCurIdentifier)
{
    RvStatus rv = RV_OK;
    CallLeg* pCallLeg = NULL;
    SipTransactionKey   msgKey;
    CallLegMgr* pMgr = (CallLegMgr*)hCallLegMgr;

    *phCallLeg = NULL;

    memset(&msgKey,0,sizeof(SipTransactionKey));
    rv = SipTransactionMgrGetKeyFromMessage(pMgr->hTranscMgr,hMsg,&msgKey);
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrFindCallLegByMsg(hMsg=0x%p): SipTransactionMgrGetKeyFromMessage() failed (rv=%d:%s)",
            hMsg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* --------- Lock the Call-Leg Manager ---------------*/
    RvMutexLock(&pMgr->hMutex,pMgr->pLogMgr);

    CallLegMgrHashFind(pMgr,
                        &msgKey,
                        eDirection,
                        bOnlyEstablishedCall,
                        (RvSipCallLegHandle**)&pCallLeg);

    if(pCallLeg == NULL) /* call-leg was not found */
    {
        RvLogInfo(pMgr->pLogSrc, (pMgr->pLogSrc,
            "CallLegMgrFindCallLegByMsg: mgr 0x%p - didn't find any matching call-leg.",
            pMgr));
        
        /* ----------- Unlock the Call-Leg Manager -----------*/
        RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);
        
        return RV_OK;
    }
    
    /* For validity checking */
    *pCurIdentifier = pCallLeg->callLegUniqueIdentifier;

    /* ----------- Unlock the Call-Leg Manager -----------*/
    RvMutexUnlock(&pMgr->hMutex,pMgr->pLogMgr);

    *phCallLeg = (RvSipCallLegHandle)pCallLeg;
    
    RvLogDebug(pMgr->pLogSrc, (pMgr->pLogSrc,
        "CallLegMgrFindCallLegByMsg: mgr 0x%p - found call-leg 0x%p for msg 0x%p",
        pMgr, pCallLeg, hMsg));

    return RV_OK;

}

/******************************************************************************
 * CallLegMgrCheckObjCounters
 * ----------------------------------------------------------------------------
 * General: Checks if the CallLeg Mgr calls counters are valid, before
 *          a construction of new CallLeg or after a destruction of a CallLeg.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Handle of the Call-Leg Manager.
 *          bIsHidden   - Indicates if the constructed/destructed CallLeg
 *                        is hidden.
 *          bIncrement  - Indicates if the counters examination refers to
 *                        CallLeg construction(RV_TRUE)/destruction(RV_FALSE).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegMgrCheckObjCounters(
                                    IN  CallLegMgr *pMgr,
                                    IN  RvBool      bIsHidden,
                                    IN  RvBool      bIncrement)
{
    RvInt32 *pMaxAllocatedCalls;
    RvInt32 *pAllocatedCalls;

    if(bIsHidden == RV_TRUE)
    {
#ifdef RV_SIP_SUBS_ON
        pMaxAllocatedCalls = &pMgr->maxNumOfHiddenCalls;
        pAllocatedCalls    = &pMgr->numOfAllocatedHiddenCalls;
#else /* #ifdef RV_SIP_SUBS_ON */
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrCheckObjCounters - Hidden Calls (Subuscriptions) are not supported"));
        return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
    }
    else
    {
        pMaxAllocatedCalls = &pMgr->maxNumOfCalls;
        pAllocatedCalls    = &pMgr->numOfAllocatedCalls;
    }

    if (bIncrement == RV_TRUE && *pAllocatedCalls >= *pMaxAllocatedCalls)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrCheckObjCounters - already allocate %d call-legs (hidden=%d), maximum calls configuration %d.",
            *pAllocatedCalls, bIsHidden,*pMaxAllocatedCalls));
        return RV_ERROR_OUTOFRESOURCES;
    }
    else if (bIncrement == RV_FALSE && *pAllocatedCalls < 0)
    {
        RvLogExcep(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrCheckObjCounters: Illegal numOfAllocatedCalls(hidden=%d)=%d",
            bIsHidden,*pAllocatedCalls));
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/******************************************************************************
 * CallLegMgrUpdateObjCounters
 * ----------------------------------------------------------------------------
 * General: Updates the CallLeg Mgr calls counters are valid, after
 *          a construction of new CallLeg or after a destruction of a CallLeg.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Handle of the Call-Leg Manager.
 *          bIsHidden   - Indicates if the constructed/destructed CallLeg
 *                        is hidden.
 *          bIncrement  - Indicates if the counters eximination refers to
 *                        CallLeg construction(RV_TRUE)/destruction(RV_FALSE).
 *****************************************************************************/
RvStatus RVCALLCONV CallLegMgrUpdateObjCounters(
                                    IN  CallLegMgr *pMgr,
                                    IN  RvBool      bIsHidden,
                                    IN  RvBool      bIncrement)
{
    if (bIsHidden == RV_TRUE)
    {
#ifdef RV_SIP_SUBS_ON
        (bIncrement == RV_TRUE) ? (++pMgr->numOfAllocatedHiddenCalls):
                                  (--pMgr->numOfAllocatedHiddenCalls);
#else /* #ifdef RV_SIP_SUBS_ON */
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CallLegMgrUpdateObjCounters - Hidden Calls (Subuscriptions) are not supported"));
        return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_SUBS_ON */
    }
    else
    {
        (bIncrement == RV_TRUE) ? (++pMgr->numOfAllocatedCalls):
                                  (--pMgr->numOfAllocatedCalls);
    }

    return RV_OK;
}

/******************************************************************************
 * CallLegMgrGetSubsMgr
 * ----------------------------------------------------------------------------
 * General: Retrieves the call-leg's mgr handle of subs mgr
 * Return Value: The call-leg's mgr handle of subs mgr.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr  - Handle of the Call-Leg Manager.
 *****************************************************************************/
RvSipSubsMgrHandle RVCALLCONV CallLegMgrGetSubsMgr(IN  CallLegMgr *pMgr) 
{
#ifdef RV_SIP_SUBS_ON
	return pMgr->hSubsMgr;
#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(pMgr); 

	return NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/*--------------------HASH FUNCTION CALLBACKS----------------------*/
/***************************************************************************
 * HashFunction
 * ------------------------------------------------------------------------
 * General: call back function for calculating the hash value from the
 *          hash key.
 * Return Value: The value generated from the key.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     key  - The hash key (of type CallLegMgrHashKey)
 ***************************************************************************/
static RvUint  HashFunction(void *key)
{
    CallLegMgrHashElem *hashElem = (CallLegMgrHashElem *)key;
    RvInt32    i,j;
    RvUint     s = 0;
    RvInt32    mod = hashElem->hashKey.pMgr->hashSize;
    RvInt32    callIdSize;
    RPOOL_Ptr  callId;
    RvInt32    offset;
    RvInt32    timesToCopy;
    RvInt32    leftOvers;
    RvChar     buffer[33];
    static const int base = 1 << (sizeof(char)*8);
    
    if(hashElem->bIsReplacesSearch == RV_FALSE)
    {
        callId.hPool = ((CallLegMgrHashKey *)&(hashElem->hashKey))->pCallId.hPool;
        callId.hPage = ((CallLegMgrHashKey *)&(hashElem->hashKey))->pCallId.hPage;
        callId.offset = ((CallLegMgrHashKey *)&(hashElem->hashKey))->pCallId.offset;
    }
    else
    {
        callId.hPool   = SipReplacesHeaderGetPool((RvSipReplacesHeaderHandle)hashElem->hReplacesHashKey);
        callId.hPage   = SipReplacesHeaderGetPage((RvSipReplacesHeaderHandle)hashElem->hReplacesHashKey);
        callId.offset  = SipReplacesHeaderGetCallID((RvSipReplacesHeaderHandle)hashElem->hReplacesHashKey);
    }
    offset = callId.offset;

    callIdSize = RPOOL_Strlen(callId.hPool,
                              callId.hPage,
                              callId.offset);

    timesToCopy = callIdSize/32;
    leftOvers   = callIdSize%32;

    for(i=0; i<timesToCopy; i++)
    {

        memset(buffer,0,33);
        RPOOL_CopyToExternal(callId.hPool,
                             callId.hPage,
                             offset,
                             buffer,
                             32);
        buffer[32] = '\0';
        for (j=0; j<32; j++) {
            s = (s*base+buffer[j])%mod;
        }
        offset+=32;
    }
    if(leftOvers > 0)
    {
        memset(buffer,0,33);
        RPOOL_CopyToExternal(callId.hPool,
                             callId.hPage,
                             offset,
                             buffer,
                             leftOvers);
        buffer[leftOvers] = '\0';
        for (j=0; j<leftOvers; j++) {
            s = (s*base+buffer[j])%mod;
        }
    }
    return s;
}

/******************************************************************************
 * HashFind
 * ----------------------------------------------------------------------------
 * General: search hash for element, which is identical to provided
 *          as a parameter element.Element comparison is performed by function,
 *          which is supplied as a parameter.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr          - Handle of the Call-Leg Manager.
 *          pHashElem     - Pointer to the element, for which hash is searched.
 *          pfnCompareFunc- Element comparison function.
 * Output:  pphCallLeg    - Pointer to memory, where the pointer to handle of
 *                          the found Call-Leg will be stored by function.
 *                          If no Call-Leg was found - NULL will stored.
 *****************************************************************************/
static void HashFind(IN  CallLegMgr          *pMgr,
                     IN  CallLegMgrHashElem  *pHashElem,
                     IN  HASH_CompareFunc    pfnCompareFunc,
                     OUT RvSipCallLegHandle  **pphCallLeg)
{
    RvStatus rv;
    RvBool   bCallWasFound;
    void     *elementPtr;

    *pphCallLeg = NULL;

    if (NULL == pMgr || NULL == pMgr->hHashTable)
    {
        return;
    }

    bCallWasFound = HASH_FindElement(pMgr->hHashTable, pHashElem,
                                     pfnCompareFunc, &elementPtr);

    /*return if record not found*/
    if(RV_FALSE==bCallWasFound  ||  NULL==elementPtr)
    {
        return;
    }
    rv = HASH_GetUserElement(pMgr->hHashTable,elementPtr,(void*)pphCallLeg);
    if(rv != RV_OK)
    {
        *pphCallLeg = NULL;
        return;
    }
    return;
}

/***************************************************************************
* CallHashCompare
* ------------------------------------------------------------------------
* General: Used to compare to keys that where mapped to the same hash
*          value in order to decide whether they are the same - the compare
*          is to a regular call (not a replaces comparison).
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashElement  - a new hash element
*            oldHashElement  - an element from the hash
***************************************************************************/
static RvBool CallHashCompare(IN void *newHashElement,
                              IN void *oldHashElement)
{
    CallLegMgrHashElem *pNewHashElem = (CallLegMgrHashElem*)newHashElement;
    CallLegMgrHashKey  *pExistHashKey = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)oldHashElement)->hashKey;
    CallLegMgrHashKey  *pNewHashKey   = (CallLegMgrHashKey *)&(pNewHashElem->hashKey);
    CallLegMgrCallHashKeyDetailed  existCallDetails;
    CallLegMgrCallHashKeyDetailed  newCallDetails;
#ifdef RFC_2543_COMPLIANT
	RvStatus                       rv;
#endif /* #ifdef RFC_2543_COMPLIANT */

    ConvertHashKey2CallDetails(pNewHashKey,  &newCallDetails);
    ConvertHashKey2CallDetails(pExistHashKey,&existCallDetails);

    if(RV_TRUE == pNewHashElem->bOnlyEstablishedCall)
    {
       if(UNDEFINED == existCallDetails.strToTag.offset
#ifdef RFC_2543_COMPLIANT
           && SipPartyHeaderGetEmptyTag(*(pExistHashKey->phTo)) == RV_FALSE
#endif /*#ifdef RFC_2543_COMPLIANT*/
           )
        {
            RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
						"CallHashCompare - Failed. no to-tag. not established call"));
            return RV_FALSE;
        }
    }

#ifdef RFC_2543_COMPLIANT
	rv = CallLegMgrDealWithEmptyTag(pExistHashKey, &existCallDetails, &newCallDetails);
	if (RV_OK != rv)
	{
		RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
					"CallHashCompare - Failed in CallLegMgrDealWithEmptyTag"));
        return RV_FALSE;
	}
#endif /*#ifdef RFC_2543_COMPLIANT*/

    return HashCompare(
        &newCallDetails, &existCallDetails, pExistHashKey->pMgr->pLogSrc);
}

/******************************************************************************
 * CallHashCompareOriginal
 * ----------------------------------------------------------------------------
 * General: checks, if 'oldHashElement' points to Original Call-Leg with dialog
 *          identifiers, equal to those in 'newHashElement'.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: RV_TRUE, if 'oldHashElement' points to Call-Leg, which
 *          satisfies explained in 'General' conditions, RV_FALSE otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   newHashElement - Pointer to the element with search criterias.
 *          oldHashElement - Pointer to the element, existing in hash.
 * Output:  none.
 *****************************************************************************/
static RvBool CallHashCompareOriginal(IN void *newHashElement,
                                       IN void *oldHashElement)
{
    CallLegMgrHashKey  *pExistHashKey = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)oldHashElement)->hashKey;
    CallLegMgrHashKey  *pNewHashKey   = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)newHashElement)->hashKey;
    CallLegMgrCallHashKeyDetailed  existCallDetails;
    CallLegMgrCallHashKeyDetailed  newCallDetails;

    ConvertHashKey2CallDetails(pNewHashKey,  &newCallDetails);
    ConvertHashKey2CallDetails(pExistHashKey,&existCallDetails);

    /* Check direction */
    if (RVSIP_CALL_LEG_DIRECTION_OUTGOING != existCallDetails.eDirection)
    {
        RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
            "CallHashCompareOriginal - Failed. Not outgoing Call-Leg"));
        return RV_FALSE;
    }
    /* Check if existing Call-Leg is outgoing */
    if(RV_FALSE == *(((CallLegMgrHashElem*)oldHashElement)->pbOriginalCall))
    {
        RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
            "CallHashCompareOriginal - Failed. Not original Call-Leg"));
        return RV_FALSE;
    }

    /* Neutralize TO-tag before using of HashCompare() */
    existCallDetails.strToTag.offset = UNDEFINED;

    return HashCompare(
        &newCallDetails, &existCallDetails, pExistHashKey->pMgr->pLogSrc);
}

/******************************************************************************
 * CallHashCompareHidden
 * ----------------------------------------------------------------------------
 * General: checks, if 'oldHashElement' points to Original and Hidden Call-Leg
 *          with dialog identifiers, equal to those in 'newHashElement'.
 *          Call-Leg is Hidden, if it serves out-of-dialog Subscription.
 * Return Value: RV_TRUE, if 'oldHashElement' points to Call-Leg, which
 *          satisfies explained in 'General' conditions, RV_FALSE otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   newHashElement - Pointer to the element with search criterias.
 *          oldHashElement - Pointer to the element, existing in hash.
 * Output:  none.
 *****************************************************************************/
static RvBool  CallHashCompareHidden(IN void *newHashElement,
                                     IN void *oldHashElement)
{
    /* Check if existing Call-Leg is hidden */
    if(RV_FALSE == CallLegMgrGetHashElementIsHidden((CallLegMgrHashElem*)oldHashElement))
    {
        CallLegMgrHashKey  *pExistHashKey;

        pExistHashKey = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)oldHashElement)->hashKey;

        RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
            "CallHashCompareHidden - Failed. Not hidden Call-Leg"));
        return RV_FALSE;
    }
    /* Check if existing Call-Leg is Original and is matched to new one */
    return CallHashCompare(newHashElement,oldHashElement);
}

/******************************************************************************
 * CallHashCompareOriginalHidden
 * ----------------------------------------------------------------------------
 * General: checks, if 'oldHashElement' points to Original and Hidden Call-Leg
 *          with dialog identifiers, equal to those in 'newHashElement'.
 *          Call-Leg is Hidden, if it serves out-of-dialog Subscription.
 *          Original Call-Leg is a Call-Leg, which was not created as a result
 *          of forked message receiption (e.g. forked 200 response receiption).
 *          Call-Leg, created by Application or by incoming INVITE are Original
 *          When database is searched for outgoing Original Call-Leg,
 *          it's TO-tag is ignored.
 * Return Value: RV_TRUE, if 'oldHashElement' points to Call-Leg, which
 *          satisfies explained in 'General' conditions, RV_FALSE otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   newHashElement - Pointer to the element with search criterias.
 *          oldHashElement - Pointer to the element, existing in hash.
 * Output:  none.
 *****************************************************************************/
static RvBool  CallHashCompareOriginalHidden(IN void *newHashElement,
                                             IN void *oldHashElement)
{
    /* Check if existing Call-Leg is hidden */
    if(RV_FALSE == CallLegMgrGetHashElementIsHidden((CallLegMgrHashElem*)oldHashElement))
    {
        CallLegMgrHashKey  *pExistHashKey;

        pExistHashKey = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)oldHashElement)->hashKey;

        RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
            "CallHashCompareOriginalHidden - Failed. Not hidden Call-Leg"));
        return RV_FALSE;
    }
    /* Check if existing Call-Leg is Original and is matched to new one */
    return CallHashCompareOriginal(newHashElement,oldHashElement);
}

/***************************************************************************
* CallHashCompareReplaces
* ------------------------------------------------------------------------
* General: Used to compare two keys that where mapped to the same hash
*          value in order to decide whether they are the same - the compare
*          is when searching for a call that matches to a replaces header
* Return Value: TRUE if the keys are equal, FALSE otherwise.
* ------------------------------------------------------------------------
* Arguments:
* Input:     newHashElement  - a new hash element (made from a replaces header)
*           oldHashElement  - an element from the hash
***************************************************************************/
static RvBool CallHashCompareReplaces( IN void *newHashElement,
                                                IN void *oldHashElement)
{
    CallLegMgrHashElem *pNewHashElem = (CallLegMgrHashElem*)newHashElement;
    CallLegMgrHashKey  *pExistHashKey = (CallLegMgrHashKey *)&((CallLegMgrHashElem*)oldHashElement)->hashKey;
    CallLegMgrCallHashKeyDetailed  existCallDetails;
    CallLegMgrCallHashKeyDetailed  newCallDetails;
    RvSipReplacesHeaderHandle  *hHeader = (RvSipReplacesHeaderHandle*)&(pNewHashElem->hReplacesHashKey);


    /* Fill Call Details to be compared of the existing in hash Call-Leg */
    ConvertHashKey2CallDetails(pExistHashKey,&existCallDetails);
    /* Fill Call Details to be compared of the new Call-Leg */
    newCallDetails.strFromTag.offset = SipReplacesHeaderGetFromTag(*hHeader);
    newCallDetails.strFromTag.hPage = SipReplacesHeaderGetPage(*hHeader);
    newCallDetails.strFromTag.hPool = SipReplacesHeaderGetPool(*hHeader);
    newCallDetails.strToTag.offset = SipReplacesHeaderGetToTag(*hHeader);
    newCallDetails.strToTag.hPage = SipReplacesHeaderGetPage(*hHeader);
    newCallDetails.strToTag.hPool = SipReplacesHeaderGetPool(*hHeader);
    newCallDetails.strCallID.offset = SipReplacesHeaderGetCallID(*hHeader);
    newCallDetails.strCallID.hPage = SipReplacesHeaderGetPage(*hHeader);
    newCallDetails.strCallID.hPool = SipReplacesHeaderGetPool(*hHeader);
    /* set direction of new call to INCOMING, so the rules in HashCompare function
       will fit the replaces compare needs */
    newCallDetails.eDirection = RVSIP_CALL_LEG_DIRECTION_INCOMING;

    /* If the TO tag or FROM tag in the Replaces header is '0',
    it matches both '0' or NULL */
    if(existCallDetails.eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)
    {
        if((RPOOL_CmpToExternal(newCallDetails.strFromTag.hPool,
                                newCallDetails.strFromTag.hPage,
                                newCallDetails.strFromTag.offset,
                                "0", RV_FALSE) == RV_TRUE) &&
           (existCallDetails.strFromTag.offset == UNDEFINED))
        {
            newCallDetails.strFromTag.offset = UNDEFINED;
        }
    }
    else
    {
        if((RPOOL_CmpToExternal(newCallDetails.strToTag.hPool,
                                newCallDetails.strToTag.hPage,
                                newCallDetails.strToTag.offset,
                                "0", RV_FALSE) == RV_TRUE) &&
           (existCallDetails.strFromTag.offset == UNDEFINED))
        {
            newCallDetails.strToTag.offset = UNDEFINED;
        }
    }

    return HashCompare(
        &newCallDetails, &existCallDetails, pExistHashKey->pMgr->pLogSrc);
}

/******************************************************************************
* HashCompare
* -----------------------------------------------------------------------------
* General: compare call details, extracted from hash keys, mapped to the same
*          hash value: TO and FROM tags, Call-Ids.
* Return Value: RV_TRUE if the details are equal, RV_FALSE otherwise.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pNewCall    - pointer to details of Call, which is sought in hash
*           pExistCall  - pointer to details of Call, which exists in hash
*           pLogSrc     - pointer to Log Source object, used for logging
******************************************************************************/
static RvBool HashCompare(IN CallLegMgrCallHashKeyDetailed  *pNewCall,
                          IN CallLegMgrCallHashKeyDetailed  *pExistCall,
                          IN RvLogSource                    *pLogSrc)
{
    RvBool fromTagsEqual;
    RvBool toTagsEqual;
    RvBool callIDsEqual;

    if(pExistCall->strToTag.offset == UNDEFINED)
    {
        if(pNewCall->eDirection == pExistCall->eDirection)
        {
            pNewCall->strToTag.offset = UNDEFINED;
        }
        else
        {
            pNewCall->strFromTag.offset = UNDEFINED;
        }
    }

    if((pNewCall->strToTag.offset == UNDEFINED) &&
       (pNewCall->eDirection == pExistCall->eDirection))
    {
        pExistCall->strToTag.offset = UNDEFINED;
    }

    if(pNewCall->eDirection == pExistCall->eDirection)
    {
        fromTagsEqual = RPOOL_Strcmp(
            pNewCall->strFromTag.hPool,  pNewCall->strFromTag.hPage,  pNewCall->strFromTag.offset,
            pExistCall->strFromTag.hPool,pExistCall->strFromTag.hPage,pExistCall->strFromTag.offset);

        toTagsEqual = RPOOL_Strcmp(
            pNewCall->strToTag.hPool,  pNewCall->strToTag.hPage,  pNewCall->strToTag.offset,
            pExistCall->strToTag.hPool,pExistCall->strToTag.hPage,pExistCall->strToTag.offset);
    }
    else
    {
        toTagsEqual = RPOOL_Strcmp(
            pNewCall->strFromTag.hPool,pNewCall->strFromTag.hPage,pNewCall->strFromTag.offset,
            pExistCall->strToTag.hPool,pExistCall->strToTag.hPage,pExistCall->strToTag.offset);

        fromTagsEqual = RPOOL_Strcmp(
            pNewCall->strToTag.hPool,    pNewCall->strToTag.hPage,    pNewCall->strToTag.offset,
            pExistCall->strFromTag.hPool,pExistCall->strFromTag.hPage,pExistCall->strFromTag.offset);
    }

    callIDsEqual = RPOOL_Strcmp(
        pNewCall->strCallID.hPool,  pNewCall->strCallID.hPage,  pNewCall->strCallID.offset,
        pExistCall->strCallID.hPool,pExistCall->strCallID.hPage,pExistCall->strCallID.offset);

    if(fromTagsEqual != RV_TRUE)
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompare - Failed. From headers 0x%p and 0x%p have different tags",
            pNewCall->hHeaderFrom,
            (pNewCall->eDirection==pExistCall->eDirection)?pExistCall->hHeaderFrom:pExistCall->hHeaderTo));
        return RV_FALSE;
    }
    if(toTagsEqual != RV_TRUE)
    {
        RvLogDebug(pLogSrc,(pLogSrc,
            "HashCompare - Failed. To headers 0x%p and 0x%p have different tags",
            pNewCall->hHeaderTo,
            (pNewCall->eDirection==pExistCall->eDirection)?pExistCall->hHeaderTo:pExistCall->hHeaderFrom));
        return RV_FALSE;
    }
    if(callIDsEqual != RV_TRUE)
    {
        RvLogDebug(pLogSrc,(pLogSrc,"HashCompare - Failed. Call-IDs are not equal"));
        return RV_FALSE;
    }

    RvLogDebug(pLogSrc,(pLogSrc,"HashCompare - Succeed"));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif
    return RV_TRUE;
}

/******************************************************************************
* ConvertHashKey2CallDetails
* -----------------------------------------------------------------------------
* General:  Extracts details of Call, that should be used for Call
*           identification, from hash key.
*
* Return Value: none.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pHashKey     - pointer to hash key
* Output:   pCallDetails - pointer to structure, which should be filled with
*                          Call details
******************************************************************************/
static void ConvertHashKey2CallDetails(
                            IN  CallLegMgrHashKey               *pHashKey,
                            OUT CallLegMgrCallHashKeyDetailed   *pCallDetails)
{
    pCallDetails->strFromTag.offset = SipPartyHeaderGetTag(*(pHashKey->phFrom));
    pCallDetails->strFromTag.hPage  = SipPartyHeaderGetPage(*(pHashKey->phFrom));
    pCallDetails->strFromTag.hPool  = SipPartyHeaderGetPool(*(pHashKey->phFrom));

    pCallDetails->strToTag.offset   = SipPartyHeaderGetTag(*(pHashKey->phTo));
    pCallDetails->strToTag.hPage    = SipPartyHeaderGetPage(*(pHashKey->phTo));
    pCallDetails->strToTag.hPool    = SipPartyHeaderGetPool(*(pHashKey->phTo));

    pCallDetails->strCallID.hPool   = pHashKey->pCallId.hPool;
    pCallDetails->strCallID.hPage   = pHashKey->pCallId.hPage;
    pCallDetails->strCallID.offset  = pHashKey->pCallId.offset;

    pCallDetails->eDirection   = pHashKey->eDirection;
    pCallDetails->hHeaderFrom  = *(pHashKey->phFrom);
    pCallDetails->hHeaderTo    = *(pHashKey->phTo);

    return;
}

/******************************************************************************
* ConvertTransactionKey2HashKey
* -----------------------------------------------------------------------------
* General:  Fills Call-Leg Hash key with details,extracted from Transaction key
*
* Return Value: none.
* -----------------------------------------------------------------------------
* Arguments:
* Input:    pTransactionKey - pointer to Transaction key
*           pMgr            - pointer to Call-Leg Manager
*           eTransactionDirection - direction of Transaction
*                             OUTGOING for UAC and INCOMING for UAS transaction
* Output:   pHashKey        - pointer to Call-Leg Hash key
******************************************************************************/
static void ConvertTransactionKey2HashKey(
                            IN   SipTransactionKey      *pTransactionKey,
                            IN   CallLegMgr             *pMgr,
                            IN   RvSipCallLegDirection  eTransactionDirection,
                            OUT  CallLegMgrHashKey      *pHashKey)
{
    CallLegMgrHashKey   hashKey;

    hashKey.pMgr           = pMgr;
    hashKey.pCallId.hPool  = pTransactionKey->strCallId.hPool;
    hashKey.pCallId.hPage  = pTransactionKey->strCallId.hPage;
    hashKey.pCallId.offset = pTransactionKey->strCallId.offset;
    hashKey.phFrom         = &(pTransactionKey->hFromHeader);
    hashKey.phTo           = &(pTransactionKey->hToHeader);
    hashKey.eDirection     = eTransactionDirection;

    memcpy(pHashKey,&hashKey, sizeof(CallLegMgrHashKey));
    return;
}

/***************************************************************************
 * CallLegMgrGetHashElementIsHidden
 * ------------------------------------------------------------------------
 * General: Returns the call-leg's hashElem isHidden value
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 ***************************************************************************/
static RvBool CallLegMgrGetHashElementIsHidden(IN CallLegMgrHashElem *pHashElem)
{ 
#ifdef RV_SIP_SUBS_ON
	return *(pHashElem->pbIsHidden);
#else /* #ifdef RV_SIP_SUBS_ON */ 
	RV_UNUSED_ARG(pHashElem);
	
	return RV_FALSE; 
#endif /* #ifdef RV_SIP_SUBS_ON */ 
}

#ifdef RFC_2543_COMPLIANT
/***************************************************************************
 * CallLegMgrDealWithEmptyTag
 * ------------------------------------------------------------------------
 * General: Used to protect the call-leg identifiers when at least one of the
 *          UASs does not insert tag to its party identifier. For example,
 *          A call-leg that received 200 with no To tag. From now on we wish
 *          to treat this call-leg as having an empty tag, so in case that another
 *          200 is received with tag XXX it will be considered as forked response.
 * Return Value: RV_TRUE/RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pExistHashKey     - The hash key of the call-leg that was extracted
 *                                from the hash and is being compared to this call-leg
 *            pExistCallDetails - The hash key details of the call-leg that was extracted
 *                                from the hash and is being compared to this call-leg
 *            pNewCallDetails   - The hash key details of the call-leg that is being searched
 *                                for in the hash
 ***************************************************************************/
static RvStatus CallLegMgrDealWithEmptyTag(
									IN  CallLegMgrHashKey              *pExistHashKey,
									IN  CallLegMgrCallHashKeyDetailed  *pExistCallDetails,
									IN  CallLegMgrCallHashKeyDetailed  *pNewCallDetails)
{
	RPOOL_Ptr				*pRemoteTag;
	RvSipPartyHeaderHandle	 hRemoteParty;
	RvStatus				 rv;

	/* Check the direction of the call-leg in order to identify the remote tag parameter
	   that is currently set to the existing call-leg */
	if (pExistCallDetails->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)
	{
		pRemoteTag		= &pExistCallDetails->strFromTag;
		hRemoteParty	= *pExistHashKey->phFrom;
	}
	else /* outgoing */
	{
		pRemoteTag		= &pExistCallDetails->strToTag;
		hRemoteParty	= *pExistHashKey->phTo;
	}

	/* We seek a specific case where there is no remote tag set to the existing call-leg, 
	   but this empty tag should be considered as an actual tag for comparison, i.e., if 
	   the new call-leg has any tag other than empty it will NOT be considered as equal.
	   See bug RVrep00051799 for an example. 
	   If SipPartyHeaderGetEmptyTag() is RV_TRUE it indicates that the empty tag should be
	   considered as an actual tag. */
	if ((pRemoteTag->offset == UNDEFINED) &&
		(SipPartyHeaderGetEmptyTag(hRemoteParty) == RV_TRUE) &&
		(pNewCallDetails->strFromTag.offset != UNDEFINED))
	{
		RPOOL_ITEM_HANDLE hTempPage;

		/* We set an actual empty tag "" to the remote tag so that the HashCompare would
		   return false when there is a non-empty tag in the new call-leg */
		rv = RPOOL_AppendFromExternal(pRemoteTag->hPool, 
									  pRemoteTag->hPage, 
									  "", 1, RV_FALSE, 
									  &pRemoteTag->offset, 
									  &hTempPage);
		if (RV_OK != rv)
		{
			RvLogDebug(pExistHashKey->pMgr->pLogSrc,(pExistHashKey->pMgr->pLogSrc,
						"CallLegMgrDealWithEmptyTag - Failed. could not set temporary tag to the hash key"));
            return rv;
		}
	}

	return RV_OK;
}
#endif /* #ifdef RFC_2543_COMPLIANT */

#endif /*RV_SIP_PRIMITIVES */



