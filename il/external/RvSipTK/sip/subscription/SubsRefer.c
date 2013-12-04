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
 *                              <SubsRefer.c>
 *
 * This file contains implementation for the refer subscription object.
 * .
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 October 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON

#include "SubsObject.h"
#include "SubsNotify.h"
#include "SubsCallbacks.h"
#include "SubsRefer.h"
#include "_SipCallLegMgr.h"
#include "_SipCallLeg.h"
#include "RvSipEventHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipReplacesHeader.h"
#include "_SipReferToHeader.h"
#include "_SipReferredByHeader.h"
#include "_SipReplacesHeader.h"
#include "_SipPartyHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipCallLeg.h"
#include "_SipMsg.h"
#include "_SipOtherHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipBody.h"
#include "_SipCommonUtils.h"
#include "_SipTransport.h"
#include "RvSipCommonList.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV InitSubsReferInfo(IN Subscription        *pSubs);

static RvStatus SubsReferSetReferEventId(Subscription* pSubs,
                                          RvSipTranscHandle hTransc);

static RvStatus RVCALLCONV GetMethodFromReferToHeader(IN  Subscription*    pSubs,
                                                       OUT RvSipMethodType* peMethod);

static RvStatus RVCALLCONV ReferSetParamsInNewObjDialog(Subscription* pSubs,
                                                         RvSipCallLegHandle hDialog);

static RvStatus RVCALLCONV SubsReferCreateNewCallLegResultObj (
                                      IN  Subscription         *pSubs,
                                      IN  RvSipAppCallLegHandle hAppCallLeg,
                                      OUT RvSipCallLegHandle   *phNewCallLeg);

static RvStatus RVCALLCONV SubsReferAttachResultCallLeg (
                                      IN  Subscription*      pSubs,
                                      IN  RvSipCallLegHandle hNewCallLeg);

static RvStatus RVCALLCONV SubsReferSetReplaces (
                                      IN  Subscription*      pSubs,
                                      IN  RvSipCallLegHandle hNewCallLeg);

static RvStatus RVCALLCONV SubsReferCreateNewSubsObj (
                                      IN  Subscription      *pSubs,
                                      IN  RvSipAppSubsHandle hAppSubs,
                                      OUT RvSipSubsHandle   *phNewSubs);

static RvStatus RVCALLCONV SubsReferAttachResultSubs (
                                      IN  Subscription*   pSubs,
                                      IN  RvSipSubsHandle hNewSubs,
                                      IN  RvSipMethodType eMethod);

static RvStatus RVCALLCONV SubsReferCreateNewTranscObj(
                                      IN  Subscription         *pSubs,
                                      IN  RvSipAppTranscHandle hAppTransc,
                                      OUT RvSipTranscHandle   *phNewTransc);

static RvStatus RVCALLCONV ReferSetParamsInNewTransc(Subscription*     pSubs,
                                                      RvSipTranscHandle hTransc);

static RvStatus RVCALLCONV SubsReferUpdateReplacesHeader(Subscription         *pSubs,
                                                 RvSipAddressHandle    hReferToUrl,
                                                 RvSipCommonListHandle hHeadersList);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SubsReferSetInitialReferParams
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with its mandatory parameters:
 *          refer-to and referred-by headers.
 *          If the subscription was not created inside of a call-leg you must
 *          initiate its dialog parameters first, by calling RvSipSubsDialogInit()
 *          before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request. Call RvSipSubsRefer() for sending a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE    - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION    - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES   - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR       - Bad pointer was given by the application.
 *               RV_OK          - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs       - Handle to the subscription the user wishes to initialize.
 *          hReferTo    - Handle to a refer-to header to set in the subscription.
 *          hReferredBy - Handle to a referred-by header to set in the subscription.
 ***************************************************************************/
RvStatus SubsReferSetInitialReferParams(
                                 IN  Subscription*               pSubs,
                                 IN  RvSipReferToHeaderHandle    hReferTo,
                                 IN  RvSipReferredByHeaderHandle hReferredBy)
{
    RvStatus  rv;

    /* allocate the referInfo structure on the subscription page. set "refer"
    in subscription event header, but does not set the event id yet
    (outgoing refer id will be set only on refer transaction creation) */
    rv = InitSubsReferInfo(pSubs);
    if(RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferSetInitialReferParams - subscription 0x%p - Failed init refer info (status %d)",
                pSubs, rv));
        return rv;
    }

    /* refer-to */
    if(hReferTo != NULL)
    {
        rv = SubsReferSetReferToHeader(pSubs, hReferTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferSetInitialReferParams - subscription 0x%p - Failed to set Refer-To header (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    /* hReferredBy */
    if(hReferredBy != NULL)
    {
        rv = SubsReferSetReferredByHeader(pSubs, hReferredBy);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferSetInitialReferParams - subscription 0x%p - Failed to set To header (status %d)",
                pSubs, rv));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * SubsReferInit
 * ------------------------------------------------------------------------
 * General: Initiate a refer subscription with mandatory parameters:
 *          refer-to and referred-by headers of the refer subscription.
 *          If the subscription was not created inside of a call-leg you must
 *          call RvSipSubsDialogInitStr() before calling this function.
 *            This function initialized the refer subscription, but do not send
 *          any request.
 *          You should call RvSipSubsRefer() in order to send a refer request.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the subscription is invalid.
 *                 RV_ERROR_ILLEGAL_ACTION - Invalid subscription state for this action.
 *               RV_ERROR_OUTOFRESOURCES - subscription failed to create a new header.
 *               RV_ERROR_BADPARAM - Bad parameter was given by the application.
 *               RV_ERROR_NULLPTR - Bad pointer was given by the application.
 *               RV_OK - Initialization was done successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSubs         - Handle to the subscription the user wishes to initialize.
 *          strReferTo    - String of the Refer-To header value.
 *                          for Example: "A<sip:176.56.23.4:4444;method=INVITE>"
 *          strReferredBy - String of the Referred-By header value.
 *                          for Example: "<sip:176.56.23.4:4444>"
 *          strReplaces  - The Replaces header to be set in the Refer-To header of
 *                         the REFER request. The Replaces header string doesn't
 *                           contain the 'Replaces:'.
 *                         The Replaces header will be kept in the Refer-To header in
 *                           the subscription object.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferInit(
                         IN  RvSipSubsHandle             hSubs,
                         IN  RvSipReferToHeaderHandle    hReferToHeader,
                         IN  RvSipReferredByHeaderHandle hReferredByHeader,
                         IN  RvChar*                    strReferTo,
                         IN  RvChar*                    strReferredBy,
                         IN  RvChar*                    strReplaces)
{
    RvStatus               rv;
    Subscription           *pSubscription = (Subscription*)hSubs;
    RvSipReferToHeaderHandle     hReferTo = NULL;
    RvSipReferredByHeaderHandle  hReferredBy = NULL;
    HRPOOL                       hpool;
    RvChar                 strHelpToLower[12];
    RvInt32                sizeToMemcpy = 0;

    if((pSubscription->eState != RVSIP_SUBS_STATE_IDLE &&
        pSubscription->bOutOfBand == RV_FALSE) ||
       (pSubscription->eState != RVSIP_SUBS_STATE_ACTIVE &&
        pSubscription->bOutOfBand == RV_TRUE))
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "SubsReferInit - subs 0x%p - illegal action at state %s",
            pSubscription,SubsGetStateName(pSubscription->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL==hReferToHeader && NULL==strReferTo)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
            "SubsReferInit - subs 0x%p: hReferTo(%p) and strReferTo(%p) are NULL",
            hSubs, hReferToHeader, strReferTo));
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "SubsReferInit - subscription 0x%p, initializing refer subscription...",
              hSubs));

    if(hReferToHeader != NULL) /* given headers parameters */
    {
        hReferTo = hReferToHeader;
        hReferredBy = hReferredByHeader;
    }
    else
    {
        /* getting temp page */
        hpool = pSubscription->pMgr->hGeneralPool;

        /* refer-to */
        if(strReferTo != NULL)
        {
            rv = RvSipReferToHeaderConstruct(pSubscription->pMgr->hMsgMgr,
                                            hpool, pSubscription->hPage, &hReferTo);
            if(rv != RV_OK)
            {
                RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                      "SubsReferInit - subscription 0x%p, failed to construct refer-to header ",hSubs));
                return rv;
            }
            /*we need to copy the string prefix to see if we got a full header or just the
              header value. */
            sizeToMemcpy = (RvInt32)strlen(strReferTo);
            sizeToMemcpy = (sizeToMemcpy<9)? sizeToMemcpy:9;

            memcpy(strHelpToLower, strReferTo, sizeToMemcpy);
            SipCommonStrToLower(strHelpToLower, sizeToMemcpy);

            if((memcmp((void *)"refer-to:", (void *)strHelpToLower, sizeToMemcpy) == 0) ||
               (memcmp((void *)"r:", (void *)strHelpToLower, 2) == 0))
            {
                rv = RvSipReferToHeaderParse(hReferTo, strReferTo);
            }
            else
            {
                rv = RvSipReferToHeaderParseValue(hReferTo, strReferTo);
            }
            if(rv != RV_OK)
            {
                RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                      "SubsReferInit - subscription 0x%p, failed to parse refer-to header ",hSubs));
                return rv;
            }
        }
        /* referred-by */
        if(strReferredBy != NULL)
        {
            rv = RvSipReferredByHeaderConstruct(pSubscription->pMgr->hMsgMgr,
                                                hpool, pSubscription->hPage, &hReferredBy);
            if(rv != RV_OK)
            {
                RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                      "SubsReferInit - subscription 0x%p, failed to construct referred-by header ",hSubs));
                return rv;
            }

            sizeToMemcpy = (RvInt32)strlen(strReferredBy);
            sizeToMemcpy = (sizeToMemcpy<12)? sizeToMemcpy:12;
            memcpy(strHelpToLower, strReferredBy, sizeToMemcpy);
            SipCommonStrToLower(strHelpToLower, sizeToMemcpy);
            if((memcmp((void *)"referred-by:", (void *)strHelpToLower, sizeToMemcpy) == 0) ||
               (memcmp((void *)"b:", (void *)strHelpToLower, 2) == 0))
            {
                rv = RvSipReferredByHeaderParse(hReferredBy, strReferredBy);
            }
            else
            {
                rv = RvSipReferredByHeaderParseValue(hReferredBy, strReferredBy);
            }
            if(rv != RV_OK)
            {
                RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
                      "SubsReferInit - subscription 0x%p, failed to parse referred-by header ",hSubs));
                return rv;
            }
        }

    }

    rv = SubsReferSetInitialReferParams(pSubscription, hReferTo, hReferredBy);
    if(rv != RV_OK)
    {
        RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "SubsReferInit - subscription 0x%p, failed to init refer subscription",hSubs));
        return rv;
    }

    if((strReplaces != NULL) &&
       (SipCallLegMgrGetReplacesStatus(pSubscription->pMgr->hCallLegMgr) != RVSIP_CALL_LEG_REPLACES_UNDEFINED))
    {
        rv = SubsReferSetReplacesInReferToHeader(pSubscription, strReplaces);
        if (RV_OK != rv)
        {
            RvLogError(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "SubsReferInit - subscription 0x%p, failed to set replaces header ",hSubs));
            return rv;
        }
    }

    RvLogDebug(pSubscription->pMgr->pLogSrc,(pSubscription->pMgr->pLogSrc,
              "SubsReferInit - Refer Subscription 0x%p was initialized successfully",hSubs));
    return rv;
}

/***************************************************************************
* SubsReferIncomingReferInitialize
* ------------------------------------------------------------------------
* General: The function initiate a refer subscription, that was created as a result
*          of an incoming REFER request.
*          The function allocates a ReferInfo structure on the subscription page,
*          and initialize a it's parameters (refer-to and referred-by headers,
*          event="refer" and event-id).
* Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize subscription due to a
*                                   resources problem.
*               RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:     pMgr    - pointer the the subscription manager.
*           pSubs   - pointer to the subscription.
*           hTransc - Handle to the refer transaction.
*           cseq    - the cseq step of the transaction.
*            hMsg    - handle to the received message.
***************************************************************************/
RvStatus RVCALLCONV SubsReferIncomingReferInitialize(
                                         IN SubsMgr             *pMgr,
                                         IN Subscription        *pSubs,
                                         IN RvSipTranscHandle    hTransc,
                                         IN RvSipMsgHandle       hMsg)
{
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvStatus                    rv;
    RvSipReferToHeaderHandle    hReferTo;
    RvSipReferredByHeaderHandle hReferredBy;

    RV_UNUSED_ARG(pMgr);
    /* allocate the refer-info structure on subscription page, and sets the "refer" event */
    rv = InitSubsReferInfo(pSubs);
    if(RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferIncomingReferInitialize - subscription 0x%p - Failed init refer info (status %d)",
                pSubs, rv));
        return rv;
    }

    /* set the event id */
    rv = SubsReferSetReferEventId(pSubs,hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferIncomingReferInitialize - subscription 0x%p - Failed to set Event id (rv=%d)",
               pSubs,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* set refer headers */
    /* (no need to check validity of headers.
        this will be done in the subscription 'change-state' stage) */

    /* Get the Refer-To header from the message and copy it to the Refer-To
       field of the subscription */
    hReferTo = (RvSipReferToHeaderHandle)RvSipMsgGetHeaderByType(
                                            hMsg, RVSIP_HEADERTYPE_REFER_TO,
                                            RVSIP_FIRST_HEADER, &hListElem);
    if (NULL != hReferTo)
    {
        rv = SubsReferSetReferToHeader(pSubs, hReferTo);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferIncomingReferInitialize - subscription 0x%p - Failed to set Refer-To header in subscription",
               pSubs));
            pSubs->pReferInfo->hReferTo = NULL;
            return rv;
        }
    }
    /* Get the Referred-By header from the message and copy it to the Referred-By
       field of the subscription  */
    hReferredBy = (RvSipReferredByHeaderHandle)RvSipMsgGetHeaderByType(
                                                hMsg, RVSIP_HEADERTYPE_REFERRED_BY,
                                                RVSIP_FIRST_HEADER, &hListElem);
    if (NULL != hReferredBy)
    {
        rv = SubsReferSetReferredByHeader(pSubs, hReferredBy);
        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferIncomingReferInitialize - subscription 0x%p - Failed to set Referred-By header in subscription",
                pSubs));
            pSubs->pReferInfo->hReferredBy = NULL;
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * SubsReferSetReferToHeader
 * ------------------------------------------------------------------------
 * General: Sets the Refer-To header of the refer subscription.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription.
 *          hReferTo   - Handle to a constructed header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReferToHeader(IN Subscription*            pSubs,
                                               IN RvSipReferToHeaderHandle hReferTo)
{
    RvStatus rv;
    HRPOOL    hPool;

    hPool = pSubs->pMgr->hGeneralPool;

    if(hReferTo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((SipReferToHeaderGetPool(hReferTo) == hPool) &&
       (SipReferToHeaderGetPage(hReferTo) == pSubs->hPage))
    {
        /* the header was already constructed on the subscription page.
        an assignment is enough */
        pSubs->pReferInfo->hReferTo = hReferTo;
        return RV_OK;
    }

    /* construct the refer-to header of the subscription */
    rv = RvSipReferToHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                    hPool,
                                    pSubs->hPage,
                                    &pSubs->pReferInfo->hReferTo);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferToHeader - subscription 0x%p - Failed to construct Refer-To header (rv=%d)",
               pSubs,rv));
        return rv;
    }

    /* copy the refer-to header from msg page to the subscription page */
    rv = RvSipReferToHeaderCopy(pSubs->pReferInfo->hReferTo, hReferTo);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferToHeader - subscription 0x%p - Failed to copy Refer-To header (rv=%d)",
               pSubs,rv));
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * SubsReferSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Sets the Referred-By header of the refer subscription.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the subscription.
 *          hReferredBy - Handle to a constructed header.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReferredByHeader(
                                  IN Subscription*               pSubs,
                                  IN RvSipReferredByHeaderHandle hReferredBy)
{
    RvStatus rv;
    HRPOOL    hPool;

    hPool = pSubs->pMgr->hGeneralPool;

    if(hReferredBy == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if((SipReferredByHeaderGetPool(hReferredBy) == hPool) &&
       (SipReferredByHeaderGetPage(hReferredBy) == pSubs->hPage))
    {
        /* the header was already constructed on the subscription page.
        an assignment is enough */
        pSubs->pReferInfo->hReferredBy = hReferredBy;
        return RV_OK;
    }

    /* construct the Referred-By header of the subscription */
    rv = RvSipReferredByHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                    hPool,
                                    pSubs->hPage,
                                    &pSubs->pReferInfo->hReferredBy);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferredByHeader - subscription 0x%p - Failed to construct Referred-By header (rv=%d)",
               pSubs,rv));
        return rv;
    }

    /* copy the referred-by header from msg page to the subscription page */
    rv = RvSipReferredByHeaderCopy(pSubs->pReferInfo->hReferredBy, hReferredBy);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferredByHeader - subscription 0x%p - Failed to copy Referred-By header (rv=%d)",
               pSubs,rv));
        return rv;
    }
    return RV_OK;
}


/***************************************************************************
 * SubsReferSetReplacesInReferToHeader
 * ------------------------------------------------------------------------
 * General: Sets the Replaces header in the Refer-To header of the call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs        - Pointer to the refer subscription.
 *            strReplaces  - The Replaces header value as string to set in the
 *                         Refer-To header of the subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetReplacesInReferToHeader(IN Subscription *pSubs,
                                                         IN RvChar      *strReplaces)
{
    RvStatus rv = RV_OK;
    RvInt32  sizeToMemcpy = 0;
    RvChar   strHelper[9];
    RvSipReplacesHeaderHandle hReplacesHeader = NULL;

    if(strReplaces == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    /* Construct Replaces header in the subscription refer-to header */
    hReplacesHeader = SipReferToHeaderGetReplacesHeader(pSubs->pReferInfo->hReferTo);
    if(hReplacesHeader == NULL)
    {
        rv = RvSipReplacesHeaderConstructInReferToHeader(pSubs->pReferInfo->hReferTo, &hReplacesHeader);

        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                      "SubsReferSetReplacesInReferToHeader - subs 0x%p - Failed to construct Replaces header",
                      pSubs));
            return rv;
        }
    }

    /*we need to copy the string prefix to see if we got a full header or just the
      header value. */
    sizeToMemcpy = (RvInt32)strlen(strReplaces);
    sizeToMemcpy = (sizeToMemcpy<9) ? sizeToMemcpy : 9;
    memcpy(strHelper, strReplaces, sizeToMemcpy);
    SipCommonStrToLower(strHelper, sizeToMemcpy);
    if(memcmp((void *)"replaces:", (void *)strHelper, sizeToMemcpy) == 0)
    {
        rv = RvSipReplacesHeaderParse(hReplacesHeader, strReplaces);
    }
    else
    {
        rv = RvSipReplacesHeaderParseValue(hReplacesHeader, strReplaces);
    }
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsReferSetReplacesInReferToHeader - subs 0x%p - Failed to parse Replaces header",
              pSubs));
            return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * SubsReferCheckReferRequestValidity
 * ------------------------------------------------------------------------
 * General: check the validity of a refer request
 *          a message is valid if it has exactly one refer-to header,
 *          and one or zero referred-by header.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs         - Pointer to the subscription.
 *          hMsg          - Handle to incoming refer message.
 * Output:  pResponseCode - response code for rejecting the message,
 *                          0 if the message is valid.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferCheckReferRequestValidity(
                                                  IN Subscription*     pSubs,
                                                  IN RvSipMsgHandle    hMsg,
                                                  OUT RvUint16*        pResponseCode)
{
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipReferToHeaderHandle    hReferTo;
    RvSipReferredByHeaderHandle hReferredBy;

    *pResponseCode = 0;

    /* check that there is exactly one refer-to header */
    hReferTo = (RvSipReferToHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                            hMsg, RVSIP_HEADERTYPE_REFER_TO,
                                            RVSIP_FIRST_HEADER,
                                            RVSIP_MSG_HEADERS_OPTION_ALL,
                                            &hListElem);
    if (NULL != hReferTo)
    {
        RvSipReferToHeaderHandle hTempReferToHeader;

        hTempReferToHeader = (RvSipReferToHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                            hMsg, RVSIP_HEADERTYPE_REFER_TO,
                                            RVSIP_NEXT_HEADER,
                                            RVSIP_MSG_HEADERS_OPTION_ALL,
                                            &hListElem);
        if (NULL != hTempReferToHeader)
        {
            /* illegal message: two refer-to headers */
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferCheckReferRequestValidity - subscription 0x%p - Found two Refer-To headers in incoming REFER msg. reject with 400",
               pSubs));

            *pResponseCode = 400;
            return RV_OK;
        }
        else
        {
            if(SipReferToHeaderGetStrBadSyntax(hReferTo) > UNDEFINED)
            {
                /* illegal message: bad-syntax single refer-to header. */
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsReferCheckReferRequestValidity - subscription 0x%p - Found single bad-syntax Refer-To headers in incoming REFER msg. reject with 400",
                   pSubs));

                *pResponseCode = 400;
                return RV_OK;
            }
        }
    }
    else
    {
        /* no refer-to header */
        *pResponseCode = 400;
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferCheckReferRequestValidity - subs 0x%p - no legal refer-to header in REFER request. reject with %d",
                pSubs, *pResponseCode));
        return RV_OK;
    }
    /* check that there is no more than one referred-by header.  */
    hReferredBy = (RvSipReferredByHeaderHandle)RvSipMsgGetHeaderByType(
                                                hMsg, RVSIP_HEADERTYPE_REFERRED_BY,
                                                RVSIP_FIRST_HEADER, &hListElem);
    if (NULL != hReferredBy)
    {
        RvSipReferredByHeaderHandle hTempReferredByHeader;

        hTempReferredByHeader = (RvSipReferredByHeaderHandle)RvSipMsgGetHeaderByType(
                                            hMsg, RVSIP_HEADERTYPE_REFERRED_BY,
                                            RVSIP_NEXT_HEADER, &hListElem);
        if (NULL != hTempReferredByHeader)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferCheckReferRequestValidity - subscription 0x%p - Found two Referred-By headers in incoming REFER msg. reject with 400",
               pSubs));

            *pResponseCode = 400;
            return RV_OK;
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSubs);
#endif

    return RV_OK;
}
/***************************************************************************
 * SubsReferDecideOnReferRcvdReason
 * ------------------------------------------------------------------------
 * General: decide whether to use RVSIP_SUBS_REASON_REFER_RCVD reason or
 *          RVSIP_SUBS_REASON_REFER_RCVD_WITH_REPLACES when informing application
 *          that a new refer message was received.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs - Pointer to the refer subscription.
 *          hMsg  - Handle to incoming refer message.
 ***************************************************************************/
RvSipSubsStateChangeReason RVCALLCONV SubsReferDecideOnReferRcvdReason(
                                                  IN Subscription*     pSubs,
                                                  IN RvSipMsgHandle    hMsg)
{
    RvStatus rv;

    /* update the subscription dialog if the remote party supports replaces.
    (user may get this information using RvSipCallLegGetReplacesStatus...)*/
    rv = SipCallLegUpdateReplacesStatus(pSubs->hCallLeg, hMsg);
    if(rv != RV_OK)
    {
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferDecideOnReferRcvdReason: subs 0x%p - SipCallLegUpdateReplacesStatus failed (rv=%d)",
            pSubs,rv));
    }

    /* check that local party supports replaces. */
    if(SipCallLegMgrGetReplacesStatus(pSubs->pMgr->hCallLegMgr) != RVSIP_CALL_LEG_REPLACES_UNDEFINED)
    {
        RvSipReplacesHeaderHandle hReplacesHeader = NULL;
        hReplacesHeader =  SipReferToHeaderGetReplacesHeader(pSubs->pReferInfo->hReferTo);
        if(hReplacesHeader != NULL)
        {
            return RVSIP_SUBS_REASON_REFER_RCVD_WITH_REPLACES;
        }
    }
    return RVSIP_SUBS_REASON_REFER_RCVD;
}


/***************************************************************************
 * SubsReferSetRequestParamsInMsg
 * ------------------------------------------------------------------------
 * General: The function sets the refer-to and referred-by headers of the
 *          refer subscription to the refer request message.
 *          The function also updates the event id of the refer subscription.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs   - Pointer to the subscription.
 *          hTransc - Handle of the refer transaction.
 *          hMsg    - Handle to the outgoing refer message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetRequestParamsInMsg(
                                              IN Subscription*        pSubs,
                                              IN RvSipTranscHandle    hTransc,
                                              IN RvSipMsgHandle       hMsg)
{
    RvStatus                   rv;
    RvSipHeaderListElemHandle   hElem;
    void                       *hHeader,*pTempHeader;

    /* set the event-id of this subscription */
    rv = SubsReferSetReferEventId(pSubs, hTransc);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferSetRequestParamsInMsg: subs 0x%p - failed to set refer event id", pSubs));
        return rv;
    }

    /* set refer-to header in msg */
    if(pSubs->pReferInfo->hReferTo != NULL)
    {
        /* before pushing refer-to header, check that there is no other refer-to header
        in this message */
        pTempHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_REFER_TO,
                                                 RVSIP_FIRST_HEADER,
                                                 RVSIP_MSG_HEADERS_OPTION_ALL,
                                                 &hElem);
        if(pTempHeader == NULL)
        {
            /* there is no refer-to header. insert header to msg */
            rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER, (void*)pSubs->pReferInfo->hReferTo,
                                    RVSIP_HEADERTYPE_REFER_TO, &hElem, &hHeader);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsReferSetRequestParamsInMsg - Subscription 0x%p - Failed to push refer-to header",pSubs));
                return rv;
            }
        }
        else
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsReferSetRequestParamsInMsg - Subscription 0x%p - msg already has refer-to header",pSubs));
        }
    }
    else
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
              "SubsReferSetRequestParamsInMsg - Subscription 0x%p - no refer-to header in subscription. mandatory parameter!!!",pSubs));
        return RV_ERROR_BADPARAM;
    }

    /* set referred-by header in msg */
    if(pSubs->pReferInfo->hReferredBy != NULL)
    {
        /* before pushing refer-to header, check that there is no other refer-to header
        in this message */
        pTempHeader = RvSipMsgGetHeaderByTypeExt(hMsg, RVSIP_HEADERTYPE_REFERRED_BY,
                                                 RVSIP_FIRST_HEADER,
                                                 RVSIP_MSG_HEADERS_OPTION_ALL,
                                                 &hElem);
        if(pTempHeader == NULL)
        {
            /* there is no refer-to header. insert header to msg */
            rv = RvSipMsgPushHeader(hMsg, RVSIP_LAST_HEADER, (void*)pSubs->pReferInfo->hReferredBy,
                                    RVSIP_HEADERTYPE_REFERRED_BY, &hElem, &hHeader);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsReferSetRequestParamsInMsg - Subscription 0x%p - Failed to push referred-by header",pSubs));
                return rv;
            }
        }
        else
        {
            RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsReferSetRequestParamsInMsg - Subscription 0x%p - msg already has referred-by header",pSubs));
        }
    }

    return RV_OK;

}

/***************************************************************************
 * SubsReferAttachNewReferResultObject
 * ------------------------------------------------------------------------
 * General: Accepting of the REFER request, creates a new object as a result,
 *          which is attached to the refer subscription.
 *          The type of the new object is according to the method parameter
 *          in the URL of the Refer-To header.
 *          if method=SUBSCRIBE or method=REFER - a new subscription will be created.
 *          if method=INVITE or no method parameter exists in the Refer-To URL -
 *          a new call-leg will be created.
 *          for all other methods - a new transaction object will be created.
 *          This function attach only dialog-creating objects (call-leg or
 *          subscription), to the refer subscription. (meaning that the notifyReadyEv
 *          will be called only for new call-leg and subscriptions).
 *          application should check the method parameter, in order to know which
 *          application object to allocate, and which handle type to supply
 *          this function as an output parameter.
 *          The function sets the following parameters to the dialog of the new
 *          object :
 *          Call-Id: The Call-Id of the REFER request,
 *          To:      The Refer-To header of the REFER request,
 *          From:    The local contact,
 *          Referred-By: The Referred-By header of the RFEER request.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the subscription the user wishes to accept.
 *          hAppNewObj - Handle to the new application object. (RvSipAppCallLegHandle
 *                       in case of method=INVITE or no method parameter,
 *                       RvSipAppSubsHandle in case of method=SUBSCRIBE /REFER)
 *          eForceObjType - In case application wants to force the stack to create
 *                       a specific object type (e.g. create a transaction and
 *                       not call-leg for method=INVITE in the refer-to url) it
 *                       can set the type in this argument. otherwise it should be NULL.
 * Output:  peCreatedObjType - The type of object that was created by stack.
 *                       if application gave eForceObjType parameter, it will be
 *                       equal to eForceObjType. Otherwise it will be according
 *                       to the method parameter in the refer-to url.
 *          phNewObj   - Handle to the new object that was created by SIP Stack.
 *                       (RvSipCallLegHandle in case of method=INVITE or no
 *                       method parameter,
 *                       RvSipSubsHandle in case of method=SUBSCRIBE/REFER)
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAttachNewReferResultObject (
                            IN    Subscription*              pSubs,
                            IN    void*                       hAppNewObj,
                            IN    RvSipCommonStackObjectType  eForceObjType,
                            OUT   RvSipCommonStackObjectType *peCreatedObjType,
                            OUT   void*                      *phNewObj)
{
    RvStatus           rv = RV_OK;
    RvSipMethodType    eMethod;
    RvSipSubsReferNotifyReadyReason  eReason;
    RvInt16            responseCode;

    /* 1 decide of the new object type */
    if(eForceObjType != RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG &&
       eForceObjType != RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION &&
       eForceObjType != RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION)
    {
        /* in case the parameter was not initialized by application */
        *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;
    }
    else
    {
        *peCreatedObjType = eForceObjType;
    }

    if(RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED == *peCreatedObjType)
    {
        /* 1. take the method param from refer-To URI */
        rv = GetMethodFromReferToHeader(pSubs, &eMethod);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferAttachNewReferResultObject: subs 0x%p - failed to get method param. cannot allocate new object", pSubs));
            return RV_ERROR_UNKNOWN;
        }
        switch(eMethod)
        {
        case RVSIP_METHOD_INVITE:
        case RVSIP_METHOD_ACK:
        case RVSIP_METHOD_CANCEL:
        case RVSIP_METHOD_UNDEFINED:
            /* new call-leg */
            *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG;
            break;
        case RVSIP_METHOD_SUBSCRIBE:
        case RVSIP_METHOD_REFER:
            /* new refer subscription */
            *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION;
            break;
        default:
            /* new transaction */
            *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION;
            break;
        }
    }

    /* create new object */
    switch(*peCreatedObjType)
    {
    case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG:
        /* new call-leg */
        rv = SubsReferCreateNewCallLegResultObj(pSubs, (RvSipAppCallLegHandle)hAppNewObj, (RvSipCallLegHandle*)phNewObj);
        break;
    case RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION:
        /* new subscription */
        rv = SubsReferCreateNewSubsObj(pSubs, (RvSipAppSubsHandle)hAppNewObj, (RvSipSubsHandle*)phNewObj);
        break;
    case RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION:
        /* new transaction */
        rv = SubsReferCreateNewTranscObj(pSubs, (RvSipAppTranscHandle)hAppNewObj, (RvSipTranscHandle*)phNewObj);
        break;
    default:
        /* bug */
        break;
    }

    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferAttachNewReferResultObject - Subs 0x%p - failed to create new object",pSubs));

        eReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_ERROR_TERMINATION;
        responseCode = UNDEFINED;
        *peCreatedObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;
        *phNewObj = NULL;
    }
    else
    {
        eReason = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_INITIAL_NOTIFY;
        responseCode = 100;
    }

    SubsCallbackReferNotifyReadyEv(pSubs, eReason, responseCode, NULL);
    return RV_OK;
}

/***************************************************************************
 * RvSipNotifySetReferInfoInMsg
 * ------------------------------------------------------------------------
 * General: build the body for a notify request of a refer subscription.
 *          The function sets the notify outbound message, with correct
 *          body and content-type header.
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the notification is invalid.
 *                 RV_ERROR_UNKNOWN       -  Failed to set the notify body.
 *               RV_OK       -  set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hNotify    - Handle to the notification object.
 *          statusCode - status code to be set in NOTIFY message body.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferSetNotifyBody(IN  Notification*   pNotify,
                                            IN  RvInt16        statusCode)
{
    RvStatus rv = RV_OK;
    RvChar   strStatusLine[128];
    RvSipMsgHandle hMsg;
    RvChar   CR = 0x0D;
    RvChar   LF = 0x0A;

    rv = SubsNotifyGetOutboundMsg(pNotify, &hMsg);
    if(rv != RV_OK)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                  "SubsReferSetNotifyBody - subs 0x%p notify 0x%p , Failed to get notify outbound message",
                  pNotify->pSubs,pNotify));
        return rv;
    }
    /* Add Content-Type header to the message */
    rv = RvSipMsgSetContentTypeHeader(hMsg, "message/sipfrag");
    if (RV_OK != rv)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                  "SubsReferSetNotifyBody - subs 0x%p notify 0x%p , Failed to add Content-Type header to message 0x%p",
                  pNotify->pSubs,pNotify,hMsg));
        return rv;
    }

    /* Create and add status line to the message body */
    RvSprintf(strStatusLine, "SIP/2.0 %d %s%c%c%c%c", statusCode, SipMsgStatusCodeToString(statusCode),
            CR, LF, CR, LF);
    rv = RvSipMsgSetBody(hMsg, strStatusLine);
    if (RV_OK != rv)
    {
        RvLogError(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
                  "SubsReferSetNotifyBody - subs 0x%p notify 0x%p , Failed to add body to message 0x%p",
                  pNotify->pSubs,pNotify,hMsg));
        return rv;
    }

    RvLogDebug(pNotify->pSubs->pMgr->pLogSrc,(pNotify->pSubs->pMgr->pLogSrc,
              "SubsReferSetNotifyBody - subs 0x%p notify 0x%p , message/sipfrag body was added to msg 0x%p successfully",
              pNotify->pSubs,pNotify,hMsg));
    return RV_OK;
}


/***************************************************************************
 * SubsReferAttachResultObjToReferSubsGenerator
 * ------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAttachResultObjToReferSubsGenerator (
                            IN    Subscription*              pSubs,
                            IN    RvSipCommonStackObjectType eObjType,
                            IN    void*                      pObj)
{
    if(NULL == pSubs->pReferInfo )
    {
        return RV_OK;
    }

    /*1. Set the new object to point to the refer subscription, and change
    the new obj triple lock to point to refer subscription triple lock */
    switch(eObjType)
    {
    case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG:
        SipCallLegAttachReferResultCallLegToReferSubsGenerator((RvSipCallLegHandle)pObj,
                                                               (RvSipSubsHandle)pSubs,
                                                               pSubs->tripleLock);
        break;
    case RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION:
        SubsAttachReferResultSubsToReferSubsGenerator((RvSipSubsHandle)pObj,
                                                       (RvSipSubsHandle)pSubs,
                                                       pSubs->tripleLock);
        break;
    default:
        break;
    }

    /*2. Set the refer subscription to point to the new object */
    pSubs->pReferInfo->pReferResultObj = pObj;
    pSubs->pReferInfo->eReferResultObjType = eObjType;

    return RV_OK;
}

/***************************************************************************
 * SubsReferDetachReferResultObjAndReferSubsGenerator
 * ------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferDetachReferResultObjAndReferSubsGenerator (
                            IN    Subscription*              pSubs)
{
    if(NULL == pSubs->pReferInfo || NULL == pSubs->pReferInfo->pReferResultObj)
    {
        return RV_OK;
    }

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsReferDetachReferResultObjAndReferSubsGenerator: detach refer subs 0x%p from obj 0x%p...",
           pSubs, pSubs->pReferInfo->pReferResultObj));

    /*1. Delete the refer result object pointing to the refer subscription,
    and change it's triple lock to point back to itself triple lock */
    switch(pSubs->pReferInfo->eReferResultObjType)
    {
    case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG:
        SipCallLegDetachReferResultCallLegFromReferSubsGenerator(
                                (RvSipCallLegHandle)pSubs->pReferInfo->pReferResultObj);
        break;
    case RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION:
        SubsDetachReferResultSubsFromReferSubsGenerator(
                                (RvSipSubsHandle)pSubs->pReferInfo->pReferResultObj);
        break;
    default:
        break;
    }

    /*2. Deleted the refer subscription pointing to the associated object */
    pSubs->pReferInfo->pReferResultObj     = NULL;
    pSubs->pReferInfo->eReferResultObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;
    return RV_OK;

}

/***************************************************************************
 * SubsReferLoadFromNotifyRequestRcvdMsg
 * ------------------------------------------------------------------------
 * General: Load the refer final status from incoming NOTIFY request message.
 *          This parameter is the response code that was received in the
 *          message body.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pNotify - Pointer to the Notification Object;
 *            hMsg    - Handle to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferLoadFromNotifyRequestRcvdMsg(
                                IN  Notification           *pNotify,
                                IN  RvSipMsgHandle          hMsg)
{
    RvStatus rv;
    HRPOOL   hMsgPool;
    HPAGE    hMsgPage;
    RvInt32  strMsgBody;
    RvInt32  length;
    RvInt32  i;
    RvUint32 temp;
    RvInt    stat;
    RvInt32  responseCode;
    RvChar   strHelper[128];


   if (pNotify->pSubs->pReferInfo == NULL)
   {
        /* not a refer notify */
       return RV_OK;
   }

   pNotify->pSubs->pReferInfo->referFinalStatus = 0;

   /* Get the NOTIFY message body */
   length = RvSipMsgGetStringLength(hMsg,RVSIP_MSG_BODY);
   if (0 == length)
   {
       return RV_ERROR_UNKNOWN;
   }
   strMsgBody = SipMsgGetBody(hMsg);
   hMsgPool   = SipMsgGetPool(hMsg);
   hMsgPage   = SipMsgGetPage(hMsg);

   /* Copy 127 characters from the message body */
   if (length > 128)
   {
       length = 128;
   }
   rv = RPOOL_CopyToExternal(hMsgPool, hMsgPage, strMsgBody, strHelper, length-1);
   if (RV_OK != rv)
   {
       return RV_ERROR_UNKNOWN;
   }
   strHelper[length-1] = '\0';

   /* Look for the first 'S' in the message body. This is done to skip
      CR, LF spaces and thus. */
   for (i=0; i<length; i++)
   {
       if ('S' == strHelper[i])
       {
           break;
       }
   }
   if (i == length)
   {
       return RV_ERROR_UNKNOWN;
   }
   /* Read the status code from the status line which is in the format
      of SIP-version SP Status-Code...
      SIP-version = "SIP/" 1*DIGIT "." 1*DIGIT */
   stat = sscanf(strHelper+i, "SIP/%d.%d %d", &temp, &temp, (RvUint32*)&responseCode);
   if (3 != stat)
   {
       return RV_ERROR_UNKNOWN;
   }

   /* Update the refer final status field of the call-leg */
   pNotify->pSubs->pReferInfo->referFinalStatus = (RvUint16)responseCode;
   return RV_OK;
}

/***************************************************************************
* SubsReferGeneratorCallNotifyReady
* ------------------------------------------------------------------------
* General: Calls to pfnReferNotifyReadyEvHandler
* Return Value: RV_OK only.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pSubs        - Pointer to the subscription object.
*          eReason      - reason for the notify ready.
*          hResponseMsg - Handle to the received response message.
****************************************************************************/
RvStatus RVCALLCONV SubsReferGeneratorCallNotifyReady(
                                   IN  RvSipSubsHandle    hSubs,
                                   IN  RvSipSubsReferNotifyReadyReason eReason,
                                   IN  RvSipMsgHandle     hResponseMsg)
{
    RvInt16 responseCode = UNDEFINED;
    RvSipSubsReferNotifyReadyReason eReasonToSet;
    Subscription*   pSubs = (Subscription*)hSubs;

    if(hResponseMsg != NULL && eReason == RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNDEFINED)
    {
        responseCode = RvSipMsgGetStatusCode(hResponseMsg);
        if(responseCode >= 100 && responseCode < 200)
        {
            /* 1xx */
            eReasonToSet = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_1XX_RESPONSE_MSG_RCVD;
        }
        else
        {
            eReasonToSet = RVSIP_SUBS_REFER_NOTIFY_READY_REASON_FINAL_RESPONSE_MSG_RCVD;
        }
    }
    else
    {
        eReasonToSet = eReason;
    }

    SubsCallbackReferNotifyReadyEv(pSubs, eReasonToSet,
                                    responseCode, hResponseMsg);

    if(eReasonToSet != RVSIP_SUBS_REFER_NOTIFY_READY_REASON_1XX_RESPONSE_MSG_RCVD &&
       eReasonToSet != RVSIP_SUBS_REFER_NOTIFY_READY_REASON_INITIAL_NOTIFY)
    {
        /* final notify indication. detach the refer subscription and it's object */
        SubsReferDetachReferResultObjAndReferSubsGenerator((Subscription*)hSubs);
    }
    return RV_OK;
}

/***************************************************************************
 * SubsReferAnalyseSubsReferToHeader
 * ------------------------------------------------------------------------
 * General: The function analyze the refer-to header, of a REFER request.
 *          It takes the headers string from the header's url address object,
 *          and if exists, search for replaces header in it.
 *          if replaces header was found, remove it from headers list, and
 *          save it directly in the refer-to header.
 *          at the end of the function, the headers list is saved in the
 *          subscription object for further usage. (setting the headers in
 *          the new object that will be created when accepting the refer request).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 ***************************************************************************/
RvStatus RVCALLCONV SubsReferAnalyseSubsReferToHeader(Subscription* pSubs)
{
    RvChar* pBuffer;
    RvUint32 buffLen;
    RvStatus rv;
    RvSipAddressHandle hReferToUrl;
    RvInt32    urlHeaderLen;
    RvSipCommonListHandle hHeadersList;
    RvBool     bListEmpty;

    /* 1. check that there are headers in the refer-to address */
    hReferToUrl = RvSipReferToHeaderGetAddrSpec(pSubs->pReferInfo->hReferTo);
    if(NULL == hReferToUrl)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - failed to get address from refer-to header",pSubs));
        return RV_ERROR_NOT_FOUND;
    }

    if(RvSipAddrGetAddrType(hReferToUrl) != RVSIP_ADDRTYPE_URL)
    {
        return RV_OK;
    }

    urlHeaderLen = RvSipAddrGetStringLength(hReferToUrl, RVSIP_ADDRESS_HEADERS);
    if(urlHeaderLen <= 0)
    {
        RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - no headers parameter in refer-to url",
            pSubs));
        return RV_OK;
    }

    /* 2. get buffer from transport manager  */
    rv = SipTransportMgrAllocateRcvBuffer(pSubs->pMgr->hTransportMgr, &pBuffer, &buffLen);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - failed to get buffer from transport mgr (rv=%d)",
            pSubs, rv));
        return rv;
    }

    /* 3. get headers parameter from refer-to url, to the buffer */
    rv = RvSipAddrUrlGetHeaders(hReferToUrl, pBuffer, buffLen, &buffLen);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - failed to get headers parameter (rv=%d)",
            pSubs, rv));
        SipTransportMgrFreeRcvBuffer(pSubs->pMgr->hTransportMgr, pBuffer);
        return rv;
    }

    /* 4. allocate a list, that will contain the headers */
    rv = RvSipCommonListConstruct(pSubs->pMgr->hElementPool, (RV_LOG_Handle)pSubs->pMgr->pLogMgr, &hHeadersList);
    if (RV_OK != rv)
    {
        SipTransportMgrFreeRcvBuffer(pSubs->pMgr->hTransportMgr, pBuffer);
        return rv;
    }

    /* 5. parse the headers string to a headers list */
    rv = RvSipAddrUrlGetHeadersList(hReferToUrl, hHeadersList, pBuffer);
    if (RV_OK != rv)
    {
        RvSipCommonListDestruct(hHeadersList);
        SipTransportMgrFreeRcvBuffer(pSubs->pMgr->hTransportMgr, pBuffer);
        return rv;
    }

    /* 5. finished to parse the url headers, can free the buffer */
    SipTransportMgrFreeRcvBuffer(pSubs->pMgr->hTransportMgr, pBuffer);

    /* 6. search for replaces header in the list, and if found, save it directly in
          the refer-to header, and remove it from list */
    rv = SubsReferUpdateReplacesHeader(pSubs, hReferToUrl, hHeadersList);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - failed in SubsReferUpdateReplacesHeader (rv=%d)",
            pSubs, rv));
        RvSipCommonListDestruct(hHeadersList);
        return rv;
    }

    /* 7. save the list in the refer subscription object, this list will be copied
          to the new object that will be created on accepting the refer request.
          saving is done only if the list is not empty. */
    RvSipCommonListIsEmpty(hHeadersList, &bListEmpty);
    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferAnalyseSubsReferToHeader: subs 0x%p - final headers list bIsEmpty=%d",
            pSubs, bListEmpty));

    if(RV_FALSE == bListEmpty)
    {
        pSubs->pReferInfo->hReferToHeadersList = hHeadersList;
    }
    else
    {
        RvSipCommonListDestruct(hHeadersList);
    }
    return RV_OK;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SubsGetChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given change state reason.
 * Return Value: The string with the reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SubsReferGetNotifyReadyReasonName(
                               IN  RvSipSubsReferNotifyReadyReason  eReason)
{
    switch (eReason)
    {
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNDEFINED:
        return "Undefined";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_INITIAL_NOTIFY:
        return "Initial Notify";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_1XX_RESPONSE_MSG_RCVD:
        return "1xx Response Rcvd";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_FINAL_RESPONSE_MSG_RCVD:
        return "Final Response Rcvd";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_TIMEOUT:
        return "Timeout";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_ERROR_TERMINATION:
        return "Error Termination";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_OBJ_TERMINATED:
        return "Obj Terminated";
    case RVSIP_SUBS_REFER_NOTIFY_READY_REASON_UNSUPPORTED_AUTH_PARAMS:
        return "Unsupported Auth Params";
    default:
        return "--Unknown--";
    }
}

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
/*-----------------------------------------------------------------------
       S T A T I C     F U N C T I O N S
 ------------------------------------------------------------------------*/

/***************************************************************************
* InitSubsReferInfo
* ------------------------------------------------------------------------
* General: Allocate a ReferInfo on a subscription page, and set "refer" in
*          event header. it also set if this is the first refer subscription
*          or not.
*          The function does not set the event id,
* Return Value: RV_ERROR_OUTOFRESOURCES - Failed to initialize subscription due to a
*                                   resources problem.
*               RV_OK
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSubs - pointer to the subscription.
***************************************************************************/
static RvStatus RVCALLCONV InitSubsReferInfo(IN Subscription  *pSubs)
{
    RvStatus                   rv;
    RvInt32                    offset;
    RvBool                     bNotFirstRefer;

    if(NULL != pSubs->pReferInfo)
    {
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "InitSubsReferInfo - subscription 0x%p - referInfo was already allocated",
               pSubs));
        return RV_OK;
    }
    /* allocate the referInfo structure on the subscription page */
    pSubs->pReferInfo = (ReferInfo*)RPOOL_AlignAppend(
                                pSubs->pMgr->hGeneralPool,
                                pSubs->hPage,
                                sizeof(ReferInfo),
                                &offset);
    if(NULL == pSubs->pReferInfo)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "InitSubsReferInfo - subscription 0x%p - Failed to allocate referInfo",
               pSubs));
        return RV_ERROR_OUTOFRESOURCES;
    }

    memset(pSubs->pReferInfo, 0, sizeof(ReferInfo));
    pSubs->pReferInfo->eReferResultObjType = RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED;
    pSubs->pReferInfo->referFinalStatus = 0;

    /* set refer event in the refer structure */
    if(pSubs->hEvent == NULL)
    {
        rv = RvSipEventHeaderConstruct(pSubs->pMgr->hMsgMgr,
                                   pSubs->pMgr->hGeneralPool,
                                   pSubs->hPage,
                                   &(pSubs->hEvent));
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "InitSubsReferInfo - subscription 0x%p - Failed to construct Event header (rv=%d)",
                   pSubs,rv));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    rv = RvSipEventHeaderSetEventPackage(pSubs->hEvent, "refer");
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "InitSubsReferInfo - subscription 0x%p - failed to set Event header package (rv=%d)",
               pSubs,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* set if this is first refer */
    bNotFirstRefer = SipCallLegIsFirstReferExists(pSubs->hCallLeg);
    if(RV_TRUE == bNotFirstRefer)
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "InitSubsReferInfo - subscription 0x%p - the refer is not the first",
               pSubs));
        pSubs->pReferInfo->bFirstReferSubsInDialog = RV_FALSE;
    }
    else
    {
        RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "InitSubsReferInfo - subscription 0x%p - the refer is the first one",
               pSubs));
        pSubs->pReferInfo->bFirstReferSubsInDialog = RV_TRUE;
        rv = SipCallLegSetFirstReferExists(pSubs->hCallLeg);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "InitSubsReferInfo - subscription 0x%p - failure in SipCallLegSetFirstReferExists",
                pSubs));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
* SubsReferSetReferEventId
* ------------------------------------------------------------------------
* General: The function gets the transaction cseq value, and set is as refer
*          event id.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSubs    - pointer to the subscription.
*            hTransc  - handle to the refer transaction.
***************************************************************************/
static RvStatus SubsReferSetReferEventId(Subscription*     pSubs,
                                          RvSipTranscHandle hTransc)
{
    RvInt32 cseq;
    RvChar  referId[12];
    RvStatus rv;

    /* set the event id */
    rv = RvSipTransactionGetCSeqStep(hTransc, &cseq);
    if(RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferEventId - subscription 0x%p - Failed to get cseq from transc 0x%p (rv=%d)",
               pSubs,hTransc, rv));
        return rv;
    }

#ifdef RV_SIP_UNSIGNED_CSEQ
	RvSprintf(referId, "%u", cseq);
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
    RvSprintf(referId, "%d", cseq);
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

    rv = RvSipEventHeaderSetEventId(pSubs->hEvent, referId);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferSetReferEventId - subscription 0x%p - Failed to set Event id (rv=%d)",
               pSubs,rv));
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}

/***************************************************************************
* GetMethodFromReferToHeader
* ------------------------------------------------------------------------
* General: Gets the method parameter from subscription Refer-To URI.
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSubs    - pointer to the subscription.
* Output:   peMethod  - method.
***************************************************************************/
static RvStatus RVCALLCONV GetMethodFromReferToHeader(IN  Subscription*    pSubs,
                                                       OUT RvSipMethodType* peMethod)
{
    RvSipAddressHandle hAddr;
    RvSipAddressType   eAddrType;

    /* 1. take the method param from refer-To URI */
    hAddr = RvSipReferToHeaderGetAddrSpec(pSubs->pReferInfo->hReferTo);
    if(hAddr == NULL)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "GetMethodFromReferToHeader: subs 0x%p - no address in refer-to header",pSubs));
        return RV_ERROR_UNKNOWN;
    }

    eAddrType = RvSipAddrGetAddrType(hAddr);
    if(eAddrType != RVSIP_ADDRTYPE_URL)
    {
        /*
        RvLogWarning(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "GetMethodFromReferToHeader: subs 0x%p - refer-to address is not url",pSubs));
                return RV_ERROR_UNKNOWN;*/
        *peMethod = RVSIP_METHOD_INVITE;
        return RV_OK;
    }

    *peMethod = RvSipAddrUrlGetMethod(hAddr);
    return RV_OK;
}

/***************************************************************************
 * SubsReferCreateNewCallLegResultObj
 * ------------------------------------------------------------------------
 * General: The function create a new call-leg object, and associate it with
 *          the refer subscription..
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs   - Pointer to the subscription.
 *          hAppCallLeg - Application handle of the new call-leg.
 * Output:  phNewCallLeg - Handle the the new call-leg.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferCreateNewCallLegResultObj (
                                      IN  Subscription         *pSubs,
                                      IN  RvSipAppCallLegHandle hAppCallLeg,
                                      OUT RvSipCallLegHandle   *phNewCallLeg)
{
    RvStatus         rv;
    RvSipCallLegHandle hNewCallLeg = NULL;

    *phNewCallLeg = NULL;

    /* Create a new call-leg */
    rv = SipCallLegMgrCreateCallLeg(pSubs->pMgr->hCallLegMgr,
                                    RVSIP_CALL_LEG_DIRECTION_OUTGOING,
                                    RV_FALSE,
                                    &hNewCallLeg
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,RvSipCallLegGetCct(pSubs->hCallLeg),RvSipCallLegGetURIFlag(pSubs->hCallLeg)
#endif
//SPIRENT_END

                                    );
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewCallLegResultObj - Subs 0x%p - failed to create new call-leg",pSubs));
        return rv;
    }


    RvSipCallLegSetAppHandle(hNewCallLeg, hAppCallLeg);

    /* Initiate the new call-leg parameters (To, From, Call-Id, remote and local contact)*/
    rv = ReferSetParamsInNewObjDialog(pSubs, hNewCallLeg);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewCallLegResultObj - Subs 0x%p - failed to set dialog params in new call-leg 0x%p",pSubs, hNewCallLeg));
        SipCallLegDestruct(hNewCallLeg);
        return rv;
    }

    rv = SubsReferAttachResultCallLeg(pSubs, hNewCallLeg);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewCallLegResultObj - Subs 0x%p - failed to associate new call-leg 0x%p",pSubs, hNewCallLeg));
        SipCallLegDestruct(hNewCallLeg);
        return rv;
    }

    rv = SubsReferSetReplaces(pSubs,hNewCallLeg);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewCallLegResultObj - Subs 0x%p - failed set replaces in new call-leg 0x%p",pSubs, hNewCallLeg));
        SipCallLegDestruct(hNewCallLeg);
        return rv;
    }

    *phNewCallLeg = hNewCallLeg;
    return RV_OK;
}

/***************************************************************************
 * SubsReferCreateNewSubsObj
 * ------------------------------------------------------------------------
 * General: The function create a new subscription object, and attach it to
 *          the refer subscription generator.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs     - Pointer to the subscription.
 *          hAppSubs  - Application handle of the new call-leg.
 *          eNewSubsType  - Is this a refer subscription.
 * Output:  phNewSubs - Handle the the new call-leg.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferCreateNewSubsObj (
                                      IN  Subscription      *pSubs,
                                      IN  RvSipAppSubsHandle hAppSubs,
                                      OUT RvSipSubsHandle   *phNewSubs)
{
    RvStatus       rv;
    RvSipSubsHandle hNewSubs = NULL;
    Subscription*   pNewSubs = NULL;
    RvSipMethodType eMethod = RVSIP_METHOD_UNDEFINED;

    *phNewSubs = NULL;

    /* Create a new subscription */
    /* new subscription will always be created a hidden dialog. */
    rv = SubsMgrSubscriptionCreate((RvSipSubsMgrHandle)pSubs->pMgr,
                                       NULL, hAppSubs,
                                       RVSIP_SUBS_TYPE_SUBSCRIBER,
                                       RV_FALSE, &hNewSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
,RvSipCallLegGetCct(pSubs->hCallLeg),RvSipCallLegGetURIFlag(pSubs->hCallLeg)
#endif
//SPIRENT_END
                                       );
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewSubsObj - Subs 0x%p - failed to create new subscription object",pSubs));
        return rv;
    }

    pNewSubs = (Subscription*)hNewSubs;

    /* Initiate the new subscription dialog parameters (To, From, Call-Id, remote and local contact)*/
    rv = ReferSetParamsInNewObjDialog(pSubs, pNewSubs->hCallLeg);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferCreateNewSubsObj - Subs 0x%p - failed to set dialog params in new subs 0x%p",pSubs, hNewSubs));
        SubsFree(pNewSubs);
        return rv;
    }

    GetMethodFromReferToHeader(pSubs, &eMethod);

    if(RVSIP_METHOD_REFER == eMethod)
    {
        rv = InitSubsReferInfo(pNewSubs);
        if (rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "SubsReferCreateNewSubsObj - Subs 0x%p - failed to init refer info in new subs 0x%p",pSubs, hNewSubs));
            SubsFree(pNewSubs);
            return rv;
        }
    }
    rv = SubsReferAttachResultSubs(pSubs, hNewSubs, eMethod);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
             "SubsReferCreateNewSubsObj - Subs 0x%p - failed to attach new refer subs result 0x%p",pSubs, hNewSubs));
        SubsFree(pNewSubs);
        return rv;
    }

    *phNewSubs = hNewSubs;
    return RV_OK;
}

/***************************************************************************
 * ReferSetParamsInNewObjDialog
 * ------------------------------------------------------------------------
 * General: The function sets the dialog parameters in the dialog of a
 *          new object, that was created when accepting REFER request.
 *          The function sets the following parameters:
 *          remote contact - the address from the refer-to header.
 *          to header      - the address from the refer-to header.
 *          from header    - from the refer subscription dialog.
 *          local contact  - from the refer subscription dialog.
 *          call-id        - generate new call-id.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 *          hDialog    - Handle to the new dialog.
 ***************************************************************************/
static RvStatus RVCALLCONV ReferSetParamsInNewObjDialog(Subscription*      pSubs,
                                                         RvSipCallLegHandle hDialog)
{
    RvStatus              rv;
    RvSipPartyHeaderHandle hToHeader, hFromHeader, hTempParty;
    RvSipAddressHandle     hFromAddr;
    RvSipAddressType       eAddrType;
    RvSipAddressHandle     hReferToAddrSpec, hRemoteContact, hLocalContact, hTempAddr;
    RvSipCallLegDirection  eDirection;

    if ((NULL == pSubs->pReferInfo || NULL == pSubs->pReferInfo->hReferTo))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - no refer-to in subscription", pSubs));
        return RV_ERROR_UNKNOWN;
    }
    /* Set remote contact */
    hReferToAddrSpec = RvSipReferToHeaderGetAddrSpec(pSubs->pReferInfo->hReferTo);
    if (NULL == hReferToAddrSpec)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to get AddrSpec from Refer-To", pSubs));
        return RV_ERROR_UNKNOWN;
    }
    eAddrType = RvSipAddrGetAddrType(hReferToAddrSpec);

    /* construct new address on call-leg page */
    rv = RvSipCallLegGetNewAddressHandle(hDialog, eAddrType, &hRemoteContact);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to allocate new remote contact on new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }

    rv = RvSipAddrCopy(hRemoteContact, hReferToAddrSpec);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to copy address to remote contact on new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    /* Delete the Headers and method parameters of the contact Sip-Url */
    if(eAddrType == RVSIP_ADDRTYPE_URL)
    {
       RvSipAddrUrlSetHeaders(hRemoteContact, NULL);
       RvSipAddrUrlSetMethod(hRemoteContact,RVSIP_METHOD_UNDEFINED, NULL);
    }

    /* set the remote contact in the dialog */
    rv = RvSipCallLegSetRemoteContactAddress(hDialog, hRemoteContact);

    /* To header */
    /* construct party header on call-leg page */
    rv = RvSipCallLegGetNewPartyHeaderHandle(hDialog, &hToHeader);
    if ((RV_OK != rv) || (NULL == hToHeader))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to allocate To header for new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    rv = SipPartyHeaderSetType(hToHeader, RVSIP_HEADERTYPE_TO);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to set To header type for new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    rv = RvSipPartyHeaderSetAddrSpec(hToHeader, hReferToAddrSpec);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to set AddrSpec in To header of new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    SipPartyHeaderCorrectUrl(hToHeader);

    /* set to header to dialog */
    RvSipCallLegSetToHeader(hDialog, hToHeader);

    /* From header */
    RvSipCallLegGetDirection(pSubs->hCallLeg, &eDirection);
    if (RVSIP_CALL_LEG_DIRECTION_INCOMING == eDirection)
    {
        RvSipCallLegGetToHeader(pSubs->hCallLeg, &hTempParty);
        hFromAddr = RvSipPartyHeaderGetAddrSpec(hTempParty);
    }
    else
    {
        RvSipCallLegGetFromHeader(pSubs->hCallLeg, &hTempParty);
        hFromAddr = RvSipPartyHeaderGetAddrSpec(hTempParty);
    }
    /* construct new from party header on call-leg page */
    rv = RvSipCallLegGetNewPartyHeaderHandle(hDialog, &hFromHeader);
    if ((RV_OK != rv) || (NULL == hFromHeader))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to allocate From header of new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    rv = SipPartyHeaderSetType(hFromHeader, RVSIP_HEADERTYPE_FROM);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to set type of From header of new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    rv = RvSipPartyHeaderSetAddrSpec(hFromHeader, hFromAddr);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to set AddrSpec in From header of new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }
    SipPartyHeaderCorrectUrl(hFromHeader);

    /* set from header in call-leg */
    RvSipCallLegSetFromHeader(hDialog, hFromHeader);

    /* Set the subscription local-contact value to be also the call-leg local contact value */
    rv = RvSipCallLegGetLocalContactAddress(pSubs->hCallLeg, &hLocalContact);
    if(RV_OK == rv && hLocalContact != NULL)
    {
        rv = RvSipCallLegSetLocalContactAddress(hDialog, hLocalContact);
        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to set local contact of new dialog 0x%p",
                pSubs, hDialog));
            return rv;
        }
        /* Delete the method parameter of the local contact Sip-Url */
        rv = RvSipCallLegGetLocalContactAddress(hDialog, &hTempAddr);
        if(RvSipAddrGetAddrType(hTempAddr) == RVSIP_ADDRTYPE_URL)
        {
           RvSipAddrUrlSetMethod(hTempAddr,RVSIP_METHOD_UNDEFINED, NULL);
        }
    }

    /* Generate new Call-Id */
    rv = SipCallLegGenerateCallId(hDialog);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - Failed to generate new call-id of new dialog 0x%p",
            pSubs, hDialog));
        return rv;
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewObjDialog: subs 0x%p - parameters of new dialog 0x%p were set.",
            pSubs, hDialog));
    return RV_OK;
}

/***************************************************************************
 * SubsReferAttachResultCallLeg
 * ------------------------------------------------------------------------
 * General: 1. Set to the new call-leg the referred-by parameter of the refer
 *          subscription,
 *          2. attach the new call-leg to the refer subscription
 *          by setting the refer subscription to point to the new
 *          call-leg, and the new call-leg to point to the refer subscription.
 *          3. insert the new call-leg to the hash.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the refer subscription.
 *          hNewCallLeg - The new call-leg that created by the refer accept.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferAttachResultCallLeg (
                                      IN  Subscription*      pSubs,
                                      IN  RvSipCallLegHandle hNewCallLeg)
{
    RvStatus              rv;

    /* Set Referred-By header to call-leg.
       This header will be inserted to the INVITE request. */
    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultCallLeg: attach refer subs 0x%p to new call-leg 0x%p...",
               pSubs, hNewCallLeg));

    rv = SipCallLegSetReferredBy(hNewCallLeg, pSubs->pReferInfo->hReferredBy);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
           "SubsReferAttachResultCallLeg: subs 0x%p - Failed to set referred-by in new call-leg 0x%p (rv=%d)",
           pSubs, hNewCallLeg, rv));
        return rv;
    }

    if(pSubs->pReferInfo->hReferToHeadersList != NULL)
    {
        SipCallLegSetHeadersListToSetInInitialRequest(hNewCallLeg,
                                pSubs->pReferInfo->hReferToHeadersList);
        pSubs->pReferInfo->hReferToHeadersList  = NULL;
    }

    SubsReferAttachResultObjToReferSubsGenerator(pSubs,
                                      RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
                                      (void*)hNewCallLeg);

    /* Insert the new call-leg to the hash */
    rv = SipCallLegSubsInsertCallToHash(hNewCallLeg, RV_FALSE);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultCallLeg: subs 0x%p - Failed to insert new call-leg 0x%p to hash (rv=%d)",
               pSubs, hNewCallLeg, rv));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * SubsReferSetReplaces
 * ------------------------------------------------------------------------
 * General: Search for replaces header in the refer-to header url.
 *          if exists, set it to the new call-leg.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs       - Pointer to the refer subscription.
 *          hNewCallLeg - The new call-leg that created by the refer accept.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferSetReplaces (
                                      IN  Subscription*      pSubs,
                                      IN  RvSipCallLegHandle hNewCallLeg)
{
    RvStatus rv;
    RvSipReplacesHeaderHandle hReplacesHeader = NULL;

   /* If the REFER message contained a Replaces header in the Refer-To header save
       it in the new Call-Leg, and the new Call-Leg will add it to the INVITE */

    /* check if local party supports replaces */
    if(SipCallLegMgrGetReplacesStatus(pSubs->pMgr->hCallLegMgr) == RVSIP_CALL_LEG_REPLACES_UNDEFINED)
    {
        return RV_OK;
    }

    /* check if there is a replaces header in the refer-to header url. */
    hReplacesHeader = SipReferToHeaderGetReplacesHeader(pSubs->pReferInfo->hReferTo);
    if(hReplacesHeader == NULL)
    {
        /* no replaces header */
        return RV_OK;
    }

    if(SipReplacesHeaderGetToTag(hReplacesHeader) == UNDEFINED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferSetReplaces - subs 0x%p - Replaces header doesn't contain to-tag, and thus ignored", pSubs));
        return RV_ERROR_UNKNOWN;
    }
    else if(SipReplacesHeaderGetFromTag(hReplacesHeader) == UNDEFINED)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferSetReplaces - subs 0x%p - Replaces header doesn't contain from-tag, and thus ignored", pSubs));
        return RV_ERROR_UNKNOWN;
    }
    else
    {
        RvSipReplacesHeaderHandle hCallLegReplaces;
        rv = RvSipCallLegGetNewReplacesHeaderHandle(hNewCallLeg, &hCallLegReplaces);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                     "SubsReferSetReplaces - subs 0x%p - failed to construct Replaces header in call-leg 0x%p",
                     pSubs,hNewCallLeg));
            return rv;
        }
        rv = RvSipReplacesHeaderCopy(hCallLegReplaces, hReplacesHeader);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                     "SubsReferSetReplaces - subs 0x%p - failed to copy Replaces header to call-leg 0x%p",
                     pSubs,hNewCallLeg));
            return rv;
        }
        rv = RvSipCallLegSetReplacesHeader(hNewCallLeg, hCallLegReplaces);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                     "SubsReferSetReplaces - subs 0x%p - failed to set Replaces header to call-leg 0x%p",
                     pSubs,hNewCallLeg));
            return rv;
        }

    }
    return RV_OK;
}

/***************************************************************************
 * SubsReferAttachResultSubs
 * ------------------------------------------------------------------------
 * General: 1. Set to the new subs the referred-by parameter of the refer
 *             subscription,
 *          2. Take headers from the refer subscription, and set it to the
 *             new subscription (refer-to, event, expires will be set
 *             specifically, other will be kept in the list).
 *          3. attach the new subscription to the refer subscription
 *             by setting the refer subscription to point to the new
 *             subs, and the new subs to point to the refer subscription.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs    - Pointer to the refer subscription.
 *          hNewSubs - The new subs that created by the refer accept.
 *          eNewSubsType  - Is this a refer subscription or not.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferAttachResultSubs (
                                      IN  Subscription*   pSubs,
                                      IN  RvSipSubsHandle hNewSubs,
                                      IN  RvSipMethodType eMethod)
{
    RvStatus     rv = RV_OK;
    Subscription *pNewSubs;
    RvSipReferredByHeaderHandle hNewReferredBy;
    HRPOOL       hPool;

    RV_UNUSED_ARG(eMethod);
    hPool = pSubs->pMgr->hGeneralPool;

    pNewSubs = (Subscription*)hNewSubs;

    RvLogInfo(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultSubs: attach refer subs 0x%p to new subs 0x%p...",
               pSubs, hNewSubs));

    /* ------------------------------------------------------------------------
    1. Set to the new subs the referred-by parameter of the refer subscription
    ---------------------------------------------------------------------------*/
    if(NULL != pSubs->pReferInfo->hReferredBy)
    {
        rv = RvSipReferredByHeaderConstruct(pSubs->pMgr->hMsgMgr, hPool, pNewSubs->hPage, &hNewReferredBy);
        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultSubs: subs 0x%p - Failed to set referred-by in new subs 0x%p (rv=%d)",
               pSubs, hNewSubs, rv));
            return rv;
        }
        rv = RvSipReferredByHeaderCopy(hNewReferredBy, pSubs->pReferInfo->hReferredBy);
        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultSubs: subs 0x%p - Failed to copy referred-by to new subs 0x%p (rv=%d)",
               pSubs, hNewSubs, rv));
            return rv;
        }
        pNewSubs->hReferredByHeader = hNewReferredBy;
    }
    /* ------------------------------------------------------------------------
    2. Take headers from the refer subscription, and set it to the new subscription
       (refer-to, event, expires will be set specifically, other will be kept in the list).
    ---------------------------------------------------------------------------*/
    if(pSubs->pReferInfo->hReferToHeadersList != NULL)
    {
        RvSipHeaderType eHeaderType, eNextHeaderType;
        void           *pHeader, *pNextHeader;
        RvSipCommonListElemHandle hRelative, hNextRelative;
        RvBool         bListEmpty;

        rv = RvSipCommonListGetElem(pSubs->pReferInfo->hReferToHeadersList, RVSIP_FIRST_ELEMENT,
                                NULL, (RvInt*)&eHeaderType, &pHeader, &hRelative);
        if(rv != RV_OK)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "SubsReferAttachResultSubs: subs 0x%p - failed to get first header from list (rv=%d)",
               pSubs, rv));
            return rv;
        }

        while(pHeader != NULL)
        {
            /* take the next element in advance, because after removing the header, the next header
               won't be available (because RLIST_Remove reset the pNextElem) */
            rv = RvSipCommonListGetElem(pSubs->pReferInfo->hReferToHeadersList, RVSIP_NEXT_ELEMENT,
                                    hRelative, (RvInt*)&eNextHeaderType, &pNextHeader, &hNextRelative);
            if(rv != RV_OK)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsReferAttachResultSubs: subs 0x%p - failed to get next header from list (rv=%d)",
                   pSubs, rv));
                return rv;
            }
            switch(eHeaderType)
            {
            case RVSIP_HEADERTYPE_EVENT:
                RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsReferAttachResultSubs: subs 0x%p - found event header to be set in new subscription 0x%p",
                   pSubs, hNewSubs));
                rv = SubsSetEventHeader(pNewSubs, (RvSipEventHeaderHandle)pHeader);
                if(rv != RV_OK)
                {
                    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                       "SubsReferAttachResultSubs: subs 0x%p - failed to set event header to be set in new subscription 0x%p (rv=%d)",
                       pSubs, hNewSubs, rv));
                    return rv;
                }
                rv = RvSipCommonListRemoveElem(pSubs->pReferInfo->hReferToHeadersList, hRelative);
                if(rv != RV_OK)
                {
                    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                       "SubsReferAttachResultSubs: subs 0x%p - failed to remove header from list (rv=%d)",
                       pSubs, rv));
                    return rv;
                }
                break;
            case RVSIP_HEADERTYPE_EXPIRES:
                RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                   "SubsReferAttachResultSubs: subs 0x%p - found expires header to be set in new subscription 0x%p",
                   pSubs, hNewSubs));
                rv = RvSipExpiresHeaderGetDeltaSeconds((RvSipExpiresHeaderHandle)pHeader,
                                                   (RvUint32*)&(pNewSubs->expirationVal));
                if(rv != RV_OK)
                {
                    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                       "SubsReferAttachResultSubs: subs 0x%p - failed to get delta from expires header to be set in new subscription 0x%p (rv=%d)",
                       pSubs, hNewSubs, rv));
                    return rv;
                }
                rv = RvSipCommonListRemoveElem(pSubs->pReferInfo->hReferToHeadersList, hRelative);
                if(rv != RV_OK)
                {
                    RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                       "SubsReferAttachResultSubs: subs 0x%p - failed to remove header from list (rv=%d)",
                       pSubs, rv));
                    return rv;
                }
                break;
            case RVSIP_HEADERTYPE_REFER_TO:
                if(pNewSubs->pReferInfo != NULL) /* refer subscription */
                {
                    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                       "SubsReferAttachResultSubs: subs 0x%p - found refer-to header to be set in new subscription 0x%p",
                       pSubs, hNewSubs));
                    rv = SubsReferSetReferToHeader(pNewSubs, (RvSipReferToHeaderHandle)pHeader);
                    if(rv != RV_OK)
                    {
                        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                           "SubsReferAttachResultSubs: subs 0x%p - failed to set refer-to header to be set in new subscription 0x%p (rv=%d)",
                           pSubs, hNewSubs, rv));
                        return rv;
                    }
                    rv = RvSipCommonListRemoveElem(pSubs->pReferInfo->hReferToHeadersList, hRelative);
                    if(rv != RV_OK)
                    {
                        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                           "SubsReferAttachResultSubs: subs 0x%p - failed to remove header from list (rv=%d)",
                           pSubs, rv));
                        return rv;
                    }
                }
                break;
            default:
                break;
            }
            eHeaderType = eNextHeaderType;
            pHeader = pNextHeader;
            hRelative = hNextRelative;
        }

        RvSipCommonListIsEmpty(pSubs->pReferInfo->hReferToHeadersList, &bListEmpty);
        if(bListEmpty == RV_TRUE)
        {
            /* no headers left in list. destruct it */
            RvSipCommonListDestruct(pSubs->pReferInfo->hReferToHeadersList);
            pSubs->pReferInfo->hReferToHeadersList = NULL;
        }
        else
        {
            /* there are more headers, need to save the list in the new subscription.
               there is no need to copy the list, we can pass it as is, because it
               was allocated on a special page from the headers pool */
            pNewSubs->hHeadersListToSetInInitialRequest = pSubs->pReferInfo->hReferToHeadersList;
            pSubs->pReferInfo->hReferToHeadersList = NULL;
        }
    }
    /* ------------------------------------------------------------------------
    3. attach the new subscription to the refer subscription.
    ---------------------------------------------------------------------------*/

    /* Associate the refer subscription with the new subscription */
    SubsReferAttachResultObjToReferSubsGenerator(pSubs, RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION, (void*)hNewSubs);

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
       "SubsReferAttachResultSubs: subs 0x%p - Attached to new subs 0x%p successfully",
       pSubs, hNewSubs));

    return RV_OK;
}

/***************************************************************************
 * SubsReferCreateNewTranscObj
 * ------------------------------------------------------------------------
 * General: The function create a new transaction object, and fill it with
 *          method, to, from, and contacts parameters from the refer subscription.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs   - Pointer to the subscription.
 *          hAppTransc - Application handle of the new transaction.
 * Output:  phNewTransc - Handle the the new transaction.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferCreateNewTranscObj(
                                      IN  Subscription         *pSubs,
                                      IN  RvSipAppTranscHandle hAppTransc,
                                      OUT RvSipTranscHandle   *phNewTransc)
{
    RvStatus         rv;
    RvSipTranscHandle hNewTransc = NULL;

    *phNewTransc = NULL;

    /* Create a new transaction */
    rv = RvSipTranscMgrCreateTransaction(pSubs->pMgr->hTransactionMgr, (RvSipTranscOwnerHandle)hAppTransc, &hNewTransc);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewTranscObj - Subs 0x%p - failed to create new transaction",pSubs));
        return rv;
    }

    /* Initiate the new transaction parameters (To, From, Call-Id, remote and local contact)*/
    rv = ReferSetParamsInNewTransc(pSubs, hNewTransc);
    if (rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                  "SubsReferCreateNewTranscObj - Subs 0x%p - failed to set params in new transc 0x%p",pSubs, hNewTransc));
        SipTransactionTerminate(hNewTransc);
        /* this is an independent transaction. not related to the subscription transactions list */
        return rv;
    }

    if(pSubs->pReferInfo->hReferToHeadersList != NULL)
    {
        SipTranscSetHeadersListToSetInInitialRequest(hNewTransc,
                                                     pSubs->pReferInfo->hReferToHeadersList);
        pSubs->pReferInfo->hReferToHeadersList  = NULL;
    }

    *phNewTransc = hNewTransc;
    return RV_OK;
}

/***************************************************************************
 * ReferSetParamsInNewTransc
 * ------------------------------------------------------------------------
 * General: The function sets the transaction parameters in a new transaction,
 *          that was created when accepting REFER request.
 *          The function sets the following parameters:
 *          to header      - the address from the refer-to header.
 *          from header    - from the refer subscription dialog.
 *          call-id        - generate new call-id.
 *          request-URI    - the address from the refer-to header.
 *          method         - the method parameter from the refer-to url.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 *          hDialog    - Handle to the new dialog.
 ***************************************************************************/
static RvStatus RVCALLCONV ReferSetParamsInNewTransc(Subscription*     pSubs,
                                                      RvSipTranscHandle hTransc)
{
    RvStatus              rv;
    RvSipPartyHeaderHandle hToHeader, hFromHeader, hTempParty;
    RvSipAddressHandle     hFromAddr;
    RvSipAddressType       eAddrType;
    RvSipAddressHandle     hReferToAddrSpec, hReqUri;
    RvSipCallLegDirection  eDirection;
    RvSipMethodType        eMethod;

    if ((NULL == pSubs->pReferInfo || NULL == pSubs->pReferInfo->hReferTo))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - no refer-to in subscription", pSubs));
        return RV_ERROR_UNKNOWN;
    }

    /* Get the Refer-To address */
    hReferToAddrSpec = RvSipReferToHeaderGetAddrSpec(pSubs->pReferInfo->hReferTo);
    if (NULL == hReferToAddrSpec)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to get AddrSpec from Refer-To", pSubs));
        return RV_ERROR_UNKNOWN;
    }
    eAddrType = RvSipAddrGetAddrType(hReferToAddrSpec);

    /* transaction method */
    if(eAddrType == RVSIP_ADDRTYPE_URL)
    {
        RvChar  strMethod[50];
        RvChar  *pstrMethod = NULL;
        RvUint  methodBuffLen = 50;
        eMethod  = RvSipAddrUrlGetMethod(hReferToAddrSpec);
        if(eMethod == RVSIP_METHOD_OTHER)
        {
            rv = RvSipAddrUrlGetStrMethod(hReferToAddrSpec, (RvChar*)strMethod, methodBuffLen, &methodBuffLen);
            if(RV_OK != rv)
            {
                RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "ReferSetParamsInNewTransc: subs 0x%p - Failed to get method str from refer-to url(rv=%d:%s)",
                    pSubs, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            pstrMethod = (RvChar*)strMethod;
        }
        else if (eMethod == RVSIP_METHOD_UNDEFINED)
        {
            RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "ReferSetParamsInNewTransc: subs 0x%p - method in refer-to header is undefined 0x%p",
                pSubs, hTransc));
            return RV_ERROR_UNKNOWN;
        }
        rv = SipTransactionSetMethodStr(hTransc, eMethod, pstrMethod);
        if(RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                "ReferSetParamsInNewTransc: subs 0x%p - Failed to set method in new transc 0x%p",
                pSubs, hTransc));
            return rv;
        }
    }

    /* request-URI - the address from the refer-to header.*/
    /* construct new address on call-leg page */
    rv = SipTransactionGetNewAddressHandle(hTransc, eAddrType, &hReqUri);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to allocate new request-uri on new transc 0x%p",
            pSubs, hTransc));
        return rv;
    }

    rv = RvSipAddrCopy(hReqUri, hReferToAddrSpec);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to copy address to req-uri of new transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    /* Delete the Headers and method parameters of the request uri */
    if(eAddrType == RVSIP_ADDRTYPE_URL)
    {
       RvSipAddrUrlSetHeaders(hReqUri, NULL);
       RvSipAddrUrlSetMethod(hReqUri,RVSIP_METHOD_UNDEFINED, NULL);
    }

    /* set the remote contact in the dialog */
    rv = SipTransactionSetReqURI(hTransc, eAddrType, hReqUri);

    /* to header - the address from the refer-to header.*/
    /* construct party header on call-leg page */
    rv = SipTransactionGetNewPartyHeaderHandle(hTransc, &hToHeader);
    if ((RV_OK != rv) || (NULL == hToHeader))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to allocate To header for new transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    rv = SipPartyHeaderSetType(hToHeader, RVSIP_HEADERTYPE_TO);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to set To header type for new Transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    rv = RvSipPartyHeaderSetAddrSpec(hToHeader, hReferToAddrSpec);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to set AddrSpec in To header of new Transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    SipPartyHeaderCorrectUrl(hToHeader);

    /* set to header to hTransc */
    rv = RvSipTransactionSetToHeader(hTransc, hToHeader);

    /* From header */
    RvSipCallLegGetDirection(pSubs->hCallLeg, &eDirection);
    if (RVSIP_CALL_LEG_DIRECTION_INCOMING == eDirection)
    {
        RvSipCallLegGetToHeader(pSubs->hCallLeg, &hTempParty);
        hFromAddr = RvSipPartyHeaderGetAddrSpec(hTempParty);
    }
    else
    {
        RvSipCallLegGetFromHeader(pSubs->hCallLeg, &hTempParty);
        hFromAddr = RvSipPartyHeaderGetAddrSpec(hTempParty);
    }
    /* construct new from party header on call-leg page */
    rv = SipTransactionGetNewPartyHeaderHandle(hTransc, &hFromHeader);
    if ((RV_OK != rv) || (NULL == hFromHeader))
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to allocate From header of new Transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    rv = SipPartyHeaderSetType(hFromHeader, RVSIP_HEADERTYPE_FROM);
    if(rv != RV_OK)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to set type of From header of new Transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    rv = RvSipPartyHeaderSetAddrSpec(hFromHeader, hFromAddr);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - Failed to set AddrSpec in From header of new Transc 0x%p",
            pSubs, hTransc));
        return rv;
    }
    SipPartyHeaderCorrectUrl(hFromHeader);

    /* set from header in transaction */
    RvSipTransactionSetFromHeader(hTransc, hFromHeader);

    /* set the referred-by header */
    if (NULL != pSubs->pReferInfo->hReferredBy)
    {
        rv = SipTransactionConstructAndSetReferredByHeader(hTransc, pSubs->pReferInfo->hReferredBy);
        if (RV_OK != rv)
        {
            RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
               "ReferSetParamsInNewTransc: subs 0x%p - Failed to set referred-by in new transc 0x%p (rv=%d)",
               pSubs, hTransc, rv));
            return rv;
        }
    }

    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "ReferSetParamsInNewTransc: subs 0x%p - parameters of new Transc 0x%p were set.",
            pSubs, hTransc));
    return RV_OK;
}

/***************************************************************************
 * SubsReferUpdateReplacesHeader
 * ------------------------------------------------------------------------
 * General: The function goes over the headers list, and search for a replaces
 *          header.
 *          if found a replaces header, set it directly to the refer-to header,
 *          and remove it from the headers list (so it won't appear twice in
 *          the refer-to header).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSubs      - Pointer to the refer subscription.
 *          hReferToUrl  - Handle to the refer-to address object.
 *          hHeadersList - Handle of the headers list.
 ***************************************************************************/
static RvStatus RVCALLCONV SubsReferUpdateReplacesHeader(
                                                 Subscription         *pSubs,
                                                 RvSipAddressHandle    hReferToUrl,
                                                 RvSipCommonListHandle hHeadersList)
{
    RvStatus                  rv;
    RvSipHeaderType           eHeaderType;
    void*                     pHeader = NULL;
    RvSipCommonListElemHandle hRelative = NULL;

    RV_UNUSED_ARG(hReferToUrl);
    rv = RvSipCommonListGetElem(hHeadersList, RVSIP_FIRST_ELEMENT, hRelative,
                                (RvInt*)&eHeaderType, &pHeader, &hRelative);
    if (RV_OK != rv)
    {
        RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferUpdateReplacesHeader: subs 0x%p - failed to get first header from headers list (rv=%d)",
            pSubs, rv));
        return rv;
    }

    while(pHeader != NULL)
    {
        if(eHeaderType == RVSIP_HEADERTYPE_REPLACES)
        {
            /* found the replaces header, update the refer-to header, remove the
            replaces header from the headers list, and set the list back to the
            address parameter */
            break;
        }
        else
        {
            rv = RvSipCommonListGetElem(hHeadersList, RVSIP_NEXT_ELEMENT, hRelative,
                                    (RvInt*)&eHeaderType, &pHeader, &hRelative);
            if (RV_OK != rv)
            {
                RvLogExcep(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
                    "SubsReferUpdateReplacesHeader: subs 0x%p - failed to get next header from headers list (rv=%d)",
                    pSubs, rv));
                return rv;
            }
        }
    }
    if(NULL == pHeader)
    {
        return RV_OK;
    }

    /* if we reached this point, we found the replaces header */
    RvLogDebug(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferUpdateReplacesHeader: subs 0x%p - found replaces header in headers list ",pSubs));

    rv = SipReferToHeaderSetReplacesHeader(pSubs->pReferInfo->hReferTo,(RvSipReplacesHeaderHandle)pHeader);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferUpdateReplacesHeader: subs 0x%p - failed to set replaces header in refer-to header (rv=%d)",
            pSubs, rv));
        return rv;
    }

    rv = RvSipCommonListRemoveElem(hHeadersList, hRelative);
    if (RV_OK != rv)
    {
        RvLogError(pSubs->pMgr->pLogSrc,(pSubs->pMgr->pLogSrc,
            "SubsReferUpdateReplacesHeader: subs 0x%p - failed to remove replaces header from headers list (rv=%d)",
            pSubs, rv));
        return rv;
    }
    return RV_OK;

}

#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* ifndef RV_SIP_PRIMITIVES */


