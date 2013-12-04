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
 *                              <SubsHighAvailability.c>
 *
 *  The following functions are for High Availability use. It means that
 *  the application can store all the nessecary details of an active subscription
 *  and restore it later to another list of subscriptions.
 *
 *    Author                         Date
 *    ------                        ------
 *  Dikla Dror                      Mar 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#if defined(RV_SIP_SUBS_ON) && defined(RV_SIP_HIGHAVAL_ON)

#include "SubsObject.h"
#include "SubsMgrObject.h"
#include "_SipCallLeg.h"
#include "RvSipEventHeader.h"
#include "_SipCommonHighAval.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define MAX_SUBS_PARAM_IDS_NUM 11

/* Subs HighAval Params IDs
   -----------------------------
   These values MUSTN'T be changed in order to assure backward compatibility.
   Moreover, these IDs MUSTN'T include the HIGH_AVAL_PARAM_INTERNAL_SEP or
   the HIGH_AVAL_PARAM_EXTERNAL_SEP.
*/
#define SUBS_CALL_ID                    "SubscriptionCall"
#define STATE_ID                        "State"
#define OUT_OF_BAND_ID                  "OutOfBand"
#define EVENT_HEADER_ID                 "EventHeader"
#define EVENT_HEADER_EXIST_ID           "EventHeaderExist"
#define SUBS_TYPE_ID                    "SubsType"
#define ALERT_TIMEOUT_ID                "AlertTimeout"
#define EXPIRATION_VAL_ID               "ExpirationVal"
#define REQUESTED_EXPIRATION_VAL_ID     "RequestedExpirationVal"
#define NO_NOTIFY_TIMEOUT_ID            "NoNotifyTimeout"
#define AUTO_REFRESH_ID                 "AutoRefresh"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV StoreSubsEventHeader(
                                     IN    Subscription   *pSubs,
                                     IN    RvUint32        maxBufLen,
                                     INOUT RvChar        **pCurrBufPos,
                                     INOUT RvUint32       *pCurrBufLen);

static RvStatus RVCALLCONV RestoreSubsEventHeader(
                                       IN    RvChar   *pHeaderID,
                                       IN    RvUint32  headerIDLen,
                                       IN    RvChar   *pHeaderValue,
                                       IN    RvUint32  headerValueLen,
                                       INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreActiveSubs(IN  RvChar             *memBuff,
                                             IN  RvUint32            buffLen,
                                             IN  RvBool              bStandAlone,
                                             OUT Subscription       *pSubs);

static void RVCALLCONV ResetActiveSubs(OUT Subscription  *pSubs);

static RvStatus RVCALLCONV GetSubsCallLegSize(IN  Subscription *pSubs,
                                              OUT RvInt32      *pLen);

static RvStatus RVCALLCONV GetSubsGeneralParamsSize(
                                         IN  Subscription *pSubs,
                                         OUT RvInt32      *pLen);

static RvStatus RVCALLCONV StoreSubsCallLeg(
                                     IN    Subscription *pSubs,
                                     IN    RvUint32      buffLen,
                                     INOUT RvChar      **ppCurrPos,
                                     INOUT RvUint32     *pCurrLen);

static RvStatus RVCALLCONV StoreSubsGeneralParams(
                                     IN    Subscription *pSubs,
                                     IN    RvUint32      buffLen,
                                     INOUT RvChar      **ppCurrPos,
                                     INOUT RvUint32     *pCurrLen);

static RvStatus RVCALLCONV RestoreSubsCallLeg(
                                           IN    RvChar   *pSubsCallID,
                                           IN    RvUint32  subsCallIDLen,
                                           IN    RvChar   *pSubsCallValue,
                                           IN    RvUint32  subsCallValueLen,
                                           INOUT void     *pParamObj);

static RvStatus RVCALLCONV RestoreSubsState(
                                        IN    RvChar   *pStateID,
                                        IN    RvUint32  stateIDLen,
                                        IN    RvChar   *pStateVal,
                                        IN    RvUint32  stateValLen,
                                        INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreSubsType(
                                        IN    RvChar   *pSubsTypeID,
                                        IN    RvUint32  subsTypeIDLen,
                                        IN    RvChar   *pSubsTypeVal,
                                        IN    RvUint32  subsTypeValLen,
                                        INOUT void     *pObj);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SubsGetActiveSubsStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of active subs.
 *          (The size of buffer, that should be supply in RvSipSubsStoreActiveSubs()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE in case
 *                           that a CallLeg object retrieves its subscription
 *                           storage size, in order to avoid infinity recursion.
 * Output:  pLen           - size of buffer. will be -1 in case of failure.
 ***************************************************************************/
RvStatus SubsGetActiveSubsStorageSize(IN  RvSipSubsHandle hSubs,
                                      IN  RvBool          bStandAlone,
                                      OUT RvInt32        *pLen)
{
    Subscription *pSubs    = (Subscription *)hSubs;
    RvInt32       currLen  = 0;
    RvInt32       tempLen;
    RvStatus      rv;

    *pLen    = UNDEFINED;

    /* Only ACTIVE Subscriptions are stored */
    if (pSubs->eState != RVSIP_SUBS_STATE_ACTIVE)
    {
		RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
			"SubsGetActiveSubsStorageSize: Subs 0x%p is in state %s (not Active), thus cannot be stored",
			pSubs,SubsGetStateName(pSubs->eState)));
		
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalGetGeneralStoredBuffPrefixLen(&tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;

        rv = GetSubsCallLegSize(pSubs,&tempLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsGetActiveSubsStorageSize: Subs 0x%p failed to find the CallLeg 0x%p size",
              pSubs,pSubs->hCallLeg));
            return rv;
        }
        currLen += tempLen;
    }

    rv = GetSubsGeneralParamsSize(pSubs,&tempLen);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsGetActiveSubsStorageSize: Subs 0x%p failed to find its general parameters size",
              pSubs));
        return rv;
    }
    currLen += tempLen;

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalGetGeneralStoredBuffSuffixLen(&tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;
    }

    *pLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * SubsStoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Gets all the important parameters of an already active subs.
 *          The subs must be in state active.
 *          User must give an empty page that will be fullfill with the information.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE if
 *                           a CallLeg object asks for its subscription
 *                           storage, in order to avoid infinity recursion.
 *          buffLen        - The length of the given buffer.
 * Output:  memBuff        - The buffer that will store the Subs' parameters.
 *          pStoredLen     - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus SubsStoreActiveSubs(IN  RvSipSubsHandle  hSubs,
                             IN  RvBool           bStandAlone,
                             IN  RvUint32         buffLen,
                             OUT void            *memBuff,
                             OUT RvUint32        *pStoredLen)
{
    Subscription             *pSubs    = (Subscription *)hSubs;
    RvChar                   *currPos  = memBuff;
    RvUint32                  currLen  = 0;
    RvStatus                  rv;

    *pStoredLen = 0;

    /* Only ACTIVE Subscriptions are stored */
    if(pSubs->eState != RVSIP_SUBS_STATE_ACTIVE)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalStoreGeneralPrefix(buffLen,&currPos,&currLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsStoreActiveSubs: Subs 0x%p, SipCommonHighAvalStoreGeneralPrefix() failed. status %d",
                pSubs, rv));

            return rv;
        }
        rv = StoreSubsCallLeg(pSubs,buffLen,&currPos,&currLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsStoreActiveSubs: Subs 0x%p failed to store the CallLeg 0x%p",
              pSubs->hCallLeg,pSubs));
            return rv;
        }
    }

    rv = StoreSubsGeneralParams(pSubs,buffLen,&currPos,&currLen);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsStoreActiveSubs: Subs 0x%p failed to store its parameters",
              pSubs));
        return rv;
    }

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalStoreGeneralSuffix(buffLen,&currPos,&currLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsStoreActiveSubs: Subs 0x%p, SipCommonHighAvalStoreGeneralSuffix() failed. status %d",
                pSubs, rv));

            return rv;
        }
    }

    *pStoredLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * SubsRestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Create a new Subs on state Active, and fill all it's parameters,
 *          which are strored on the given page. (the page was updated in the
 *          SubsStoreActiveSubs function).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff        - The buffer that stores all the Subs' parameters.
 *        buffLen        - The buffer length.
 *        bStandAlone    - Indicates if the Subs is independent.
 *                         This parameter should be RV_FALSE if
 *                         a CallLeg object asks for its subscription
 *                         restoration, in order to avoid infinity recursion.
 * Input/Output:
 *        pSubs   - Pointer to the restored subscription.
 ***************************************************************************/
RvStatus SubsRestoreActiveSubs(IN    void           *memBuff,
                               IN    RvUint32        buffLen,
                               IN    RvBool          bStandAlone,
                               INOUT Subscription   *pSubs)
{
    RvStatus   rv;
    RvChar    *pBuff      = (RvChar *)memBuff;
    RvUint32   prefixLen  = 0;
    RvBool     bUseOldVer = RV_FALSE;

    /* The Subs object must be on IDLE state */
    if(pSubs->eState != RVSIP_SUBS_STATE_IDLE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsRestoreActiveSubs - Trying to restore into not IDLE Subs 0x%p",pSubs));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalHandleStandAloneBoundaries(
                        buffLen,&pBuff,&prefixLen,&bUseOldVer);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsRestoreActiveSubs - Subs 0x%p Failed in SipCommonHighAvalHandleStandAloneBoundaries()",
                pSubs));

            return rv;
        }
    }

    rv = RestoreActiveSubs(pBuff,buffLen-prefixLen,bStandAlone,pSubs);
    if (rv != RV_OK)
    {
        ResetActiveSubs(pSubs);
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsRestoreActiveSubs - Failed to restore into Subs 0x%p",pSubs));
        return rv;
    }

    return RV_OK;
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * StoreSubsEventHeader
 * ------------------------------------------------------------------------
 * General: Store the event header of a Subscription in a buffer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs         - Pointer to the stored subscription.
 *          maxBufLen     - The length of the given buffer.
 * Input/Output:
 *          pCurrBufPos   - A pointer to the current storing buffer.
 *          pCurrBufLen   - A pointer to the current length of storing buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreSubsEventHeader(
                                     IN    Subscription   *pSubs,
                                     IN    RvUint32        maxBufLen,
                                     INOUT RvChar        **ppCurrBufPos,
                                     INOUT RvUint32       *pCurrBufLen)
{
    HRPOOL      hGenPool = pSubs->pMgr->hGeneralPool;
    HPAGE       tempPage;
    RvUint32    encodedLen;
    RvStatus    rv;

    if (pSubs->hEvent != NULL)
    {
        rv = RvSipEventHeaderEncode(pSubs->hEvent,hGenPool,&tempPage,&encodedLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "StoreSubsEventHeader - Subs 0x%p failed to encode Event Header",pSubs));
            return rv;
        }
        rv = SipCommonHighAvalStoreSingleParamFromPage(EVENT_HEADER_ID,
                                                       hGenPool,
                                                       tempPage,
                                                       0,
                                                       RV_TRUE,
                                                       maxBufLen,
                                                       encodedLen,
                                                       ppCurrBufPos,
                                                       pCurrBufLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "StoreSubsEventHeader - Subs 0x%p failed to store the encoded Event Header",pSubs));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreSubsEventHeader
 * ------------------------------------------------------------------------
 * General: Restore the event header of a Subscription from a buffer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pHeaderID       - The party header ID.
 *         headerIDLen     - The party header ID length.
 *         pHeaderValue    - The party header value string.
 *         headerValueLen  - Length of the party header value string.
 * InOut:  pObj            - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreSubsEventHeader(
                                       IN    RvChar   *pHeaderID,
                                       IN    RvUint32  headerIDLen,
                                       IN    RvChar   *pHeaderValue,
                                       IN    RvUint32  headerValueLen,
                                       INOUT void     *pObj)
{
    Subscription *pSubs    = (Subscription *)pObj;
    HRPOOL        hGenPool = pSubs->pMgr->hGeneralPool;
    RvChar        termChar;
    RvStatus      rv;

    if (SipCommonMemBuffExactlyMatchStr(pHeaderID,
                                        headerIDLen,
                                        EVENT_HEADER_ID) != RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "RestoreSubsEventHeader - Subs 0x%p received illegal parameter ID",pSubs));

        return RV_ERROR_BADPARAM;
    }

    rv = RvSipEventHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                   hGenPool,
                                   pSubs->hPage,
                                   &pSubs->hEvent);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsEventHeader - Subs 0x%p failed to construct Event Header for restored",pSubs));

        return rv;
    }

    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pHeaderValue,headerValueLen,&termChar);
    rv = RvSipEventHeaderParse(pSubs->hEvent,pHeaderValue);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,headerValueLen,pHeaderValue);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsEventHeader - Subs 0x%p Failed to parse EventHeader.",pSubs));

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Restore a Subscription from a buffer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff     - The buffer that contains the stored subs.
 *        buffLen     - The buffer size
 *        bStandAlone - Indicates if the Subs is independent.
 *                      This parameter should be RV_FALSE if
 *                      a CallLeg object asks for its subscription
 *                      restoration, in order to avoid infinity recursion.
 * Output:  pSubs     - Pointer to the restored subscription.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreActiveSubs(IN  RvChar             *memBuff,
                                             IN  RvUint32            buffLen,
                                             IN  RvBool              bStandAlone,
                                             OUT Subscription       *pSubs)
{
    RvStatus                    rv;
    SipCommonHighAvalParameter  subsIDsArray[MAX_SUBS_PARAM_IDS_NUM];
    SipCommonHighAvalParameter *pCurrSubsID = subsIDsArray;

    /* Copy the stored data to the Subs object */
    if (bStandAlone == RV_TRUE)
    {
        pCurrSubsID->strParamID       = SUBS_CALL_ID;
        pCurrSubsID->pParamObj        = pSubs;
        pCurrSubsID->pfnSetParamValue = RestoreSubsCallLeg;
        pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/
    }

    pCurrSubsID->strParamID       = EVENT_HEADER_ID;
    pCurrSubsID->pParamObj        = pSubs;
    pCurrSubsID->pfnSetParamValue = RestoreSubsEventHeader;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = STATE_ID;
    pCurrSubsID->pParamObj        = pSubs;
    pCurrSubsID->pfnSetParamValue = RestoreSubsState;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = OUT_OF_BAND_ID;
    pCurrSubsID->pParamObj        = &pSubs->bOutOfBand;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = EVENT_HEADER_EXIST_ID;
    pCurrSubsID->pParamObj        = &pSubs->bEventHeaderExist;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = SUBS_TYPE_ID;
    pCurrSubsID->pParamObj        = pSubs;
    pCurrSubsID->pfnSetParamValue = RestoreSubsType;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = ALERT_TIMEOUT_ID;
    pCurrSubsID->pParamObj        = &pSubs->alertTimeout;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = EXPIRATION_VAL_ID;
    pCurrSubsID->pParamObj        = &pSubs->expirationVal;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = REQUESTED_EXPIRATION_VAL_ID;
    pCurrSubsID->pParamObj        = &pSubs->requestedExpirationVal;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = NO_NOTIFY_TIMEOUT_ID;
    pCurrSubsID->pParamObj        = &pSubs->noNotifyTimeout;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    pCurrSubsID->strParamID       = AUTO_REFRESH_ID;
    pCurrSubsID->pParamObj        = &pSubs->autoRefresh;
    pCurrSubsID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrSubsID += 1; /* Promote the pointer to next Subs ID*/

    if (pCurrSubsID-subsIDsArray > MAX_SUBS_PARAM_IDS_NUM)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "RestoreActiveSubs - Subs 0x%p Subs IDs data (%u items) was written beyond the array boundary",
                  pSubs,pCurrSubsID-subsIDsArray));

        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = SipCommonHighAvalRestoreFromBuffer(
            memBuff,buffLen,subsIDsArray,(RvUint32)(pCurrSubsID-subsIDsArray),pSubs->pMgr->pLogSrc);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "RestoreActiveSubs - Subs 0x%p failed in SipCommonHighAvalRestoreFromBuffer",pSubs));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * ResetActiveSubs
 * ------------------------------------------------------------------------
 * General: Reset the Subscription data.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  pSubs         - Pointer to the restored subscription.
 ***************************************************************************/
static void RVCALLCONV ResetActiveSubs(OUT Subscription  *pSubs)
{
    pSubs->eState                 = RVSIP_SUBS_STATE_IDLE;
    pSubs->bOutOfBand             = RV_FALSE;
    pSubs->bEventHeaderExist      = RV_TRUE;
    pSubs->hEvent                 = NULL;
    pSubs->eSubsType              = RVSIP_SUBS_TYPE_UNDEFINED;
    pSubs->alertTimeout           = pSubs->pMgr->alertTimeout;
    pSubs->expirationVal          = UNDEFINED;
    pSubs->requestedExpirationVal = UNDEFINED;
    pSubs->noNotifyTimeout        = pSubs->pMgr->noNotifyTimeout;
    pSubs->autoRefresh            = pSubs->pMgr->autoRefresh;
}

/***************************************************************************
 * GetSubsCallLegSize
 * ------------------------------------------------------------------------
 * General: The function calculates the Subs stored CallLeg length.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSubs - A pointer to the Subscription.
 * Output: pLen  - The total length which is required for the storage
 *                 of the CallLeg.
 ***************************************************************************/
static RvStatus RVCALLCONV GetSubsCallLegSize(IN  Subscription *pSubs,
                                              OUT RvInt32      *pLen)
{
    RvStatus rv;
    RvInt32  callLen;

    *pLen = UNDEFINED;

    rv = SipCallLegGetCallStorageSize(pSubs->hCallLeg,RV_FALSE,&callLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    /* The subs length includes all it parameters and separators */
    /* no additional calculations has to be done */
    *pLen = SipCommonHighAvalGetTotalStoredParamLen(callLen,(RvUint32)strlen(SUBS_CALL_ID));

    return RV_OK;
}

/***************************************************************************
 * GetSubsGeneralParamsSize
 * ------------------------------------------------------------------------
 * General: The function calculates the Subs stored general parameters
 *          length.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSubs - A pointer to the Subscription.
 * Output: pLen  - The total length which is required for the storage
 *                 of the Subscription general parameters.
 ***************************************************************************/
 static RvStatus RVCALLCONV GetSubsGeneralParamsSize(
                                         IN  Subscription *pSubs,
                                         OUT RvInt32      *pLen)
{
    HRPOOL        hGenPool = pSubs->pMgr->hGeneralPool;
    HPAGE         tempPage;
    RvUint32      encodedLen;
    const RvChar *strState;
    const RvChar *strType;
    RvStatus      rv;

    *pLen = 0;
    if (pSubs->hEvent != NULL)
    {
        /* Event Header + NULL */
        rv = RvSipEventHeaderEncode(pSubs->hEvent,hGenPool,&tempPage,&encodedLen);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "GetSubsGeneralParamsSize - Failed to encode Event Header of Subs 0x%p",pSubs));
            return rv;
        }
		/* The page is freed if the Encode function returned RV_OK, even when */
		/* the encoded length is 0 */
		RPOOL_FreePage(hGenPool,tempPage);
        if (encodedLen > 0)
        {
            *pLen += SipCommonHighAvalGetTotalStoredParamLen(
                                encodedLen,(RvUint32)strlen(EVENT_HEADER_ID));
        }
    }

    strState = SubsGetStateName(pSubs->eState);
    *pLen   += SipCommonHighAvalGetTotalStoredParamLen(
                        (RvUint32)strlen(strState),(RvUint32)strlen(STATE_ID));

    *pLen   += SipCommonHighAvalGetTotalStoredBoolParamLen(
                    pSubs->bOutOfBand,(RvUint32)strlen(OUT_OF_BAND_ID));
    *pLen   += SipCommonHighAvalGetTotalStoredBoolParamLen(
                    pSubs->bEventHeaderExist,(RvUint32)strlen(EVENT_HEADER_EXIST_ID));
    strType = SubsGetTypeName(pSubs->eSubsType);
    *pLen  += SipCommonHighAvalGetTotalStoredParamLen(
                    (RvUint32)strlen(strType),(RvUint32)strlen(SUBS_TYPE_ID));

    *pLen  += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                    pSubs->alertTimeout,(RvUint32)strlen(ALERT_TIMEOUT_ID));
    *pLen  += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                    pSubs->expirationVal,(RvUint32)strlen(EXPIRATION_VAL_ID));
    *pLen  += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                    pSubs->requestedExpirationVal,(RvUint32)strlen(REQUESTED_EXPIRATION_VAL_ID));
    *pLen  += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                    pSubs->noNotifyTimeout,(RvUint32)strlen(NO_NOTIFY_TIMEOUT_ID));
    *pLen  += SipCommonHighAvalGetTotalStoredBoolParamLen(
                    pSubs->autoRefresh,(RvUint32)strlen(AUTO_REFRESH_ID));

    return RV_OK;
}

/***************************************************************************
 * StoreSubsCallLeg
 * ------------------------------------------------------------------------
 * General: The function stores the Subscription CallLeg.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs       - Handle to the Subscription.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreSubsCallLeg(
                                     IN    Subscription *pSubs,
                                     IN    RvUint32      buffLen,
                                     INOUT RvChar      **ppCurrPos,
                                     INOUT RvUint32     *pCurrLen)
{
    RvStatus rv;
    RvInt32  callLen;
    RvInt32  actualCallLen;

    /* First, retrieve the CallLeg length */
    rv = SipCallLegGetCallStorageSize(pSubs->hCallLeg,RV_FALSE,&callLen);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Store the CallLeg prefix */
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            SUBS_CALL_ID,buffLen,callLen,ppCurrPos,pCurrLen);
    if (RV_OK != rv)
    {
        return rv;
    }
    /* Store the CallLeg itself */
    rv = SipCallLegStoreCall(
            pSubs->hCallLeg,RV_FALSE,buffLen-(*pCurrLen),*ppCurrPos,&actualCallLen);
    if (RV_OK != rv)
    {
        return rv;
    }
    /* Verify that the current Subscription length and */
    /* the actual length equal */
    if (actualCallLen != callLen)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "StoreSubsCallLeg - Subs 0x%p got different CallLeg length (%d, %d)",
            pSubs,callLen,actualCallLen));

        return RV_ERROR_UNKNOWN;
    }
    /* Promote the current position and length */
    *ppCurrPos += actualCallLen;
    *pCurrLen  += actualCallLen;
    /* Store the Subscription suffix */
    rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen) ;
    if (RV_OK != rv)
    {
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * StoreSubsGeneralParams
 * ------------------------------------------------------------------------
 * General: The function stores the Subs general parameters.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubs       - Handle to the Subscription.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreSubsGeneralParams(
                                     IN    Subscription *pSubs,
                                     IN    RvUint32      buffLen,
                                     INOUT RvChar      **ppCurrPos,
                                     INOUT RvUint32     *pCurrLen)
{
    const RvChar *strState;
    const RvChar *strType;
    RvStatus      rv;

    /* Store the basic parameters */
    strState = SubsGetStateName(pSubs->eState);
    rv       = SipCommonHighAvalStoreSingleParamFromStr(
                    STATE_ID,strState,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleBoolParam(
            OUT_OF_BAND_ID,pSubs->bOutOfBand,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleBoolParam(
            EVENT_HEADER_EXIST_ID,pSubs->bEventHeaderExist,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    strType = SubsGetTypeName(pSubs->eSubsType);
    rv      = SipCommonHighAvalStoreSingleParamFromStr(
                SUBS_TYPE_ID,strType,buffLen,ppCurrPos,pCurrLen);

    rv = SipCommonHighAvalStoreSingleInt32Param(
                ALERT_TIMEOUT_ID,pSubs->alertTimeout,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleInt32Param(
                EXPIRATION_VAL_ID,pSubs->expirationVal,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleInt32Param(REQUESTED_EXPIRATION_VAL_ID,
                                                pSubs->requestedExpirationVal,
                                                buffLen,
                                                ppCurrPos,
                                                pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleInt32Param(
                NO_NOTIFY_TIMEOUT_ID,pSubs->noNotifyTimeout,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleBoolParam(
                AUTO_REFRESH_ID,pSubs->autoRefresh,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = StoreSubsEventHeader(pSubs,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "StoreSubsGeneralParams - Subs 0x%p failed to store Event Header",pSubs));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreSubsCallLeg
 * ------------------------------------------------------------------------
 * General: The function restores a Subscription CallLeg.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubsCallID      - The ID of the configured parameter.
 *        subsCallIDLen    - The length of the parameter ID.
 *        pSubsCallValue   - The value to be set in the paramObj data member.
 *        subsCallValueLen - The length of the parameter value.
 * InOut: pParamObj        - A reference to an object that might be
 *                           affected by the parameter value.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreSubsCallLeg(
                                           IN    RvChar   *pSubsCallID,
                                           IN    RvUint32  subsCallIDLen,
                                           IN    RvChar   *pSubsCallValue,
                                           IN    RvUint32  subsCallValueLen,
                                           INOUT void     *pParamObj)
{
    Subscription *pSubs = (Subscription *)pParamObj;
    RvStatus      rv    = RV_OK;

    if (SipCommonMemBuffExactlyMatchStr(pSubsCallID,
                                        subsCallIDLen,
                                        SUBS_CALL_ID) != RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsCallLeg - Subs 0x%p received illegal ID",pSubs));

        return RV_ERROR_BADPARAM;
    }

    rv = SipCallLegRestoreCall(pSubsCallValue,subsCallValueLen,RV_FALSE,pSubs->hCallLeg);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsCallLeg - Subs 0x%p failed to restore it CallLeg 0x%p",
            pSubs,pSubs->hCallLeg));

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreSubsState
 * ------------------------------------------------------------------------
 * General: The function restores the Subscription State.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pStateID    - The ID of the configured parameter.
 *        stateIDLen  - The length of the parameter ID.
 *        pStateVal   - The parameter value string.
 *        stateValLen - The length of the parameter value string.
 * InOut: pObj        - Pointer to the restored Subscription obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreSubsState(
                                        IN    RvChar   *pStateID,
                                        IN    RvUint32  stateIDLen,
                                        IN    RvChar   *pStateVal,
                                        IN    RvUint32  stateValLen,
                                        INOUT void     *pObj)
{
    Subscription *pSubs = (Subscription *)pObj;
    RvChar        termChar;

    if (SipCommonMemBuffExactlyMatchStr(pStateID,stateIDLen,STATE_ID) != RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsState - Subs 0x%p received illegal ID",pSubs));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pStateVal,stateValLen,&termChar);
    pSubs->eState = SubsGetStateEnum(pStateVal);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,stateValLen,pStateVal);

    return RV_OK;
}

/***************************************************************************
 * RestoreSubsType
 * ------------------------------------------------------------------------
 * General: The function restores the Subscription Type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pSubsTypeID    - The ID of the configured parameter.
 *        subsTypeIDLen  - The length of the parameter ID.
 *        pSubsTypeVal   - The parameter value string.
 *        subsTypeValLen - The length of the parameter value string.
 * InOut: pObj           - Pointer to the restored Subscription obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreSubsType(
                                        IN    RvChar   *pSubsTypeID,
                                        IN    RvUint32  subsTypeIDLen,
                                        IN    RvChar   *pSubsTypeVal,
                                        IN    RvUint32  subsTypeValLen,
                                        INOUT void     *pObj)
{
    Subscription *pSubs = (Subscription *)pObj;
    RvChar        termChar;

    if (SipCommonMemBuffExactlyMatchStr(pSubsTypeID,subsTypeIDLen,SUBS_TYPE_ID) != RV_TRUE)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "RestoreSubsType - Subs 0x%p received illegal ID",pSubs));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pSubsTypeVal,subsTypeValLen,&termChar);
    pSubs->eSubsType = SubsGetTypeEnum(pSubsTypeVal);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,subsTypeValLen,pSubsTypeVal);

    return RV_OK;
}

#endif /* #if defined(RV_SIP_SUBS_ON) && defined(RV_SIP_HIGHAVAL_ON) */
#endif /* RV_SIP_PRIMITIVES */


