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
 *                              <TransactionAuth.c>
 *
 *  Implements all server authentication functions
 *
 *    Author                         Date
 *    ------                        ------
 *   Ofra Wachsman                  2.02
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "TransactionAuth.h"
#include "TransactionControl.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipMsg.h"
#include "_SipAuthenticator.h"
#include "TransactionCallBacks.h"
#include "_SipCommonCore.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus AuthenticateHeader(
                           IN  Transaction*                   pTransc,
                           IN  RvSipAuthorizationHeaderHandle hAuthorization,
                           IN  RvChar                        *password,
                           IN  void*                          hObject,
                           IN  RvSipCommonStackObjectType     eObjectType,
                           IN  SipTripleLock*                 pObjTripleLock,
                           OUT RvBool*                        isCorrected);

static void FinishAuthentication(IN  Transaction* pTransc,
                                 IN  RvBool      bAuthSucceed);

static void NextAuthorizationFound(IN Transaction       *pTransc,
                              IN RLIST_ITEM_HANDLE hItem);

static RvStatus RVCALLCONV SetAuthenticationParams(
                                   IN  Transaction       *pTransc,
                                   IN  RvSipAuthenticationHeaderHandle hAuth,
                                   IN  RvUint16         responseCode,
                                   IN  RvChar           *realm,
                                   IN  RvChar           *domain,
                                   IN  RvChar           *nonce,
                                   IN  RvChar           *opaque,
                                   IN  RvBool           stale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop);

static RvStatus SetQuotationMarks(IN  HRPOOL hPool,
                                   IN  HPAGE  hPage,
                                   IN  RvChar* string,
                                   OUT RvInt32* pStrOffset);

static RvChar* makeStrMethod(Transaction* pTransc);
/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionAuthSetAuthorizationList
 * ------------------------------------------------------------------------
 * General: Go over the message authorization headers and add them
 *          to the authorization headers list in the transaction.
 *          We assume that the list was not set yet, because this function
 *          should be called only once, for every transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthSetAuthorizationList(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg)

{
    RvSipHeaderListElemHandle    msgListElem;
    RvSipAuthorizationHeaderHandle* phAuthoriz;
    void                        *pHeader;
    RvStatus                    rv = RV_OK;
    RvUint32                    safeCounter;
    RvUint32                    maxLoops;

    /*get the first authorization header*/
    pHeader = RvSipMsgGetHeaderByType(  hMsg,
                                        RVSIP_HEADERTYPE_AUTHORIZATION,
                                        RVSIP_FIRST_HEADER,
                                        &msgListElem);
    if(pHeader == NULL)
    {
        return RV_OK;
    }

    pTransc->pAuthorizationHeadersList = RLIST_RPoolListConstruct(
                                        pTransc->pMgr->hGeneralPool,
                                        pTransc->memoryPage, sizeof(void*),
                                        pTransc->pMgr->pLogSrc);
    if (NULL == pTransc->pAuthorizationHeadersList)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAuthSetAuthorizationList - Transaction 0x%p: fail to construct a new list.",
                  pTransc));
        return RV_ERROR_OUTOFRESOURCES;
    }

    safeCounter = 0;
    maxLoops    = 1000;

    /* each authorization we got from message, we insert to the transaction's list */
    while (pHeader != NULL)
    {
        rv = RLIST_InsertTail(NULL, pTransc->pAuthorizationHeadersList,
                              (RLIST_ITEM_HANDLE *)&phAuthoriz);

        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAuthSetAuthorizationList - Transaction 0x%p: fail insert tail to list.",
                  pTransc));
            break;
        }
        rv = RvSipAuthorizationHeaderConstruct((pTransc->pMgr)->hMsgMgr,
                                               (pTransc->pMgr)->hGeneralPool,
                                                pTransc->memoryPage,
                                                phAuthoriz);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAuthSetAuthorizationList - Transaction 0x%p: fail constructing authorization header(rv=%d:%s)",
                  pTransc, rv, SipCommonStatus2Str(rv)));
            break;
        }
        rv = RvSipAuthorizationHeaderCopy(*phAuthoriz,
                                          (RvSipAuthorizationHeaderHandle)pHeader);
        if (RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAuthSetAuthorizationList - Transaction 0x%p: failed to copy authorization header(rv=%d:%s)",
                  pTransc, rv, SipCommonStatus2Str(rv)));
            break;
        }

        /* get the next authorization header from message */
        pHeader = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_AUTHORIZATION,
                                          RVSIP_NEXT_HEADER, &msgListElem);
        if (safeCounter > maxLoops)
        {
            RvLogError((pTransc->pMgr)->pLogSrc,((pTransc->pMgr)->pLogSrc,
                      "TransactionAuthSetAuthorizationList - Error in loop - number of rounds is larger than number of headers"));
            break;
        }
        safeCounter++;

    }/* end of while loop */

    if (RV_OK == rv)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc  ,
                  "TransactionAuthSetAuthorizationList - Transaction 0x%p: The list of Authorization headers was successfully set.",
                  pTransc));
    }
    return rv;
}

/***************************************************************************
 * TransactionAuthBegin
 * ------------------------------------------------------------------------
 * General: Start the server authentication proccess.
 *          if there are authorization headers in the transaction list,
 *          we will supply it to application, so it can call to authenticateHeader
 *          function.
 *          if there are none, we can call to the authCompletedEv, notifing that
 *          the authentication procedure had finished with failure.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthBegin(IN RvSipTranscHandle hTransc)
{
    Transaction        *pTransc;
    RLIST_ITEM_HANDLE   hItem;

    pTransc = (Transaction *)hTransc;

    if(RV_FALSE == pTransc->pMgr->enableServerAuth)
    {
        RvLogWarning(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthBegin - enableServerAuth = false. trans 0x%p", hTransc));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pTransc->pAuthorizationHeadersList == NULL)
    {
        /* no authorization headers, authentication procedure fail. */
        RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthBegin - AuthCompleted with Failure.(no authorization headers). trans 0x%p",
                   hTransc));

        FinishAuthentication(pTransc, RV_FALSE);
        return RV_OK;
    }

    /* the list is not empty, take the first header, and give it to user */
    RLIST_GetHead(NULL, pTransc->pAuthorizationHeadersList,
                   &hItem);
    if(hItem == NULL)
    {
        /* the list is empty - error */
        RvLogExcep(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthBegin - Fail in getting first authorization from list. trans 0x%p",
                   hTransc));
        return RV_ERROR_UNKNOWN;
    }

    NextAuthorizationFound(pTransc, hItem);
    return RV_OK;
}

/***************************************************************************
 * TransactionAuthProceed
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed in authentication procedure.
 *          actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password.
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct. (will cause AuthCompletedEv to be called, with status success)..
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop that searchs for authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure).
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the authentication procedure
 *            with next header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc -        The transaction handle.
 *          action -         In which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          hObject        - handle of the object to be authenticated.
 *          eObjectType    - type of the object to be authenticated.
 *          pObjTripleLock - lock of the object. Can be NULL. If it is
 *                           supplied, it will be unlocked before callbacks
 *                           to application and locked again after them.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthProceed(
                            IN  RvSipTranscHandle              hTransc,
                            IN  RvSipTransactionAuthAction     action,
                            IN  RvSipAuthorizationHeaderHandle hAuthorization,
                            IN  RvChar*                        password,
                            IN  void*                          hObject,
                            IN  RvSipCommonStackObjectType     eObjectType,
                            IN  SipTripleLock*                 pObjTripleLock)
{
    Transaction        *pTransc;
    RLIST_ITEM_HANDLE   hItem;
    RvBool            headerResult;
    RvStatus          stat;

    pTransc = (Transaction *)hTransc;

    switch(action)
    {
    case RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD:
        /* we need to check authentication with the given password */
        if(NULL == hAuthorization)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "TransactionAuthProceed: transc 0x%p - Can't USE_PW without Authorization header", pTransc));
            return RV_ERROR_INVALID_HANDLE;
        }
        if(password == NULL)
        {
            RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "TransactionAuthProceed: transc 0x%p - Can't USE_PW without password", pTransc));
            return RV_ERROR_NULLPTR;
        }

        stat = AuthenticateHeader(pTransc, hAuthorization, password, hObject,
                                  eObjectType, pObjTripleLock, &headerResult);
        if(stat != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthProceed - Failed to check the given header. trans 0x%p",
                   hTransc));
            return RV_ERROR_UNKNOWN;
        }
        if(headerResult == RV_TRUE)
        {
            FinishAuthentication(pTransc, RV_TRUE);
            return RV_OK;
        }
        break;
    case RVSIP_TRANSC_AUTH_ACTION_SUCCESS:
        FinishAuthentication(pTransc, RV_TRUE);
        return RV_OK;
    case RVSIP_TRANSC_AUTH_ACTION_FAILURE:
        FinishAuthentication(pTransc, RV_FALSE);
        return RV_OK;
    case RVSIP_TRANSC_AUTH_ACTION_SKIP:
        break;
    default:
        RvLogExcep(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthProceed - unknown action. trans 0x%p",
                   hTransc));
        return RV_ERROR_UNKNOWN;
    }

    /* if we reached this point, we continue the loop with next header */

    if(NULL == pTransc->lastAuthorizGotFromList)
    {
        /* lastAuthorizGotFromList is NULL here, only if application called the authProceed
           function, without calling to authBegin before. */
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                   "TransactionAuthProceed - Auth procedure was not beginned correctly. trans 0x%p",
                   hTransc));
        return RV_ERROR_ILLEGAL_ACTION;

    }
    RLIST_GetNext(NULL,
                  pTransc->pAuthorizationHeadersList,
                  pTransc->lastAuthorizGotFromList,
                  &hItem);
    if(hItem == NULL)
    {
        /* no more authorization headers to check */
        FinishAuthentication(pTransc, RV_FALSE);
    }
    else
    {
        NextAuthorizationFound(pTransc, hItem);
    }
    return RV_OK;
}

/***************************************************************************
 * TransactionAuthRespondUnauthenticated
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given authentication header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 * output:   (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthRespondUnauthenticated(
                                   IN  Transaction*         pTransc,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader,
                                   IN  RvChar              *realm,
                                   IN  RvChar              *domain,
                                   IN  RvChar              *nonce,
                                   IN  RvChar              *opaque,
                                   IN  RvBool              stale,
                                   IN  RvSipAuthAlgorithm   eAlgorithm,
                                   IN  RvChar              *strAlgorithm,
                                   IN  RvSipAuthQopOption   eQop,
                                   IN  RvChar              *strQop)
{
    RvStatus    rv;
    RvSipTransactionState eInitialState = pTransc->eState;

    RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
              "TransactionAuthRespondUnauthenticated - Transaction 0x%p: about to send %d response",
              pTransc,responseCode));

    /*generate a new empty message if the user did not send one */
    if (pTransc->hOutboundMsg == NULL)
    {
        rv = RvSipMsgConstruct(pTransc->pMgr->hMsgMgr, pTransc->pMgr->hMessagePool,&pTransc->hOutboundMsg);
        if(rv != RV_OK)
        {
            RvLogInfo(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to create new message", pTransc));
            return rv;
        }
    }

    /* add the authenticate (or other) header to the constructed message */
    if(hHeader != NULL)
    {
        RvSipHeaderListElemHandle hElem;
        void*                     hNewHeader;

        /* set the other header in the message */
        rv = RvSipMsgPushHeader(pTransc->hOutboundMsg,
                               RVSIP_FIRST_HEADER,
                               hHeader,
                               headerType,
                               &hElem,
                               &hNewHeader);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to push otherHeader",
                pTransc));
            return rv;
        }
        if(headerType == RVSIP_HEADERTYPE_OTHER)
        {
            rv = RvSipOtherHeaderCopy((RvSipOtherHeaderHandle)hNewHeader,
                                      (RvSipOtherHeaderHandle)hHeader);
            if(RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to copy otherHeader 0x%p",
                    pTransc, hHeader));
                return rv;
            }
        }
        if(headerType == RVSIP_HEADERTYPE_AUTHENTICATION)
        {
            rv = RvSipAuthenticationHeaderCopy((RvSipAuthenticationHeaderHandle)hNewHeader,
                                               (RvSipAuthenticationHeaderHandle)hHeader);
            if(RV_OK != rv)
            {
                RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                    "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to copy authenticationHeader 0x%p",
                    pTransc, hHeader));
                return rv;
            }
        }
    }
    else
    {
        /* construct and set an authenticate header */
        RvSipAuthenticationHeaderHandle hAuth;

        if(realm == NULL || (strcmp(realm, "")==0))
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionAuthRespondUnauthenticated - Transaction 0x%p: no realm parameter was given",
                pTransc));
            return RV_ERROR_BADPARAM;
        }

        rv = RvSipAuthenticationHeaderConstructInMsg(pTransc->hOutboundMsg,
                                                    RV_TRUE,
                                                    &hAuth);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to construct authentication header",
                pTransc));
            return rv;
        }
        rv = SetAuthenticationParams(pTransc, hAuth, responseCode, realm, domain, nonce,
                                    opaque, stale, eAlgorithm, strAlgorithm, eQop, strQop);
        if(RV_OK != rv)
        {
            RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                "TransactionAuthRespondUnauthenticated - Transaction 0x%p: Failed to set authentication params",
                pTransc));
            return rv;
        }
    }


    pTransc->retransmissionsCount=0;
    rv = TransactionControlGenerateAndSendResponse(pTransc, responseCode,
                                                   strReasonPhrase, RV_FALSE);

    if(pTransc->hOutboundMsg != NULL)
    {
        RvSipMsgDestruct(pTransc->hOutboundMsg);
        pTransc->hOutboundMsg = NULL;
    }
    if(rv != RV_OK)
    {
        return rv;
    }

    rv = TransactionControlChangeToNextSrvState(pTransc,eInitialState, responseCode,
                                         RV_FALSE, RVSIP_TRANSC_REASON_USER_COMMAND);

    if (RV_OK != rv)
    {
        RvLogError(pTransc->pMgr->pLogSrc ,(pTransc->pMgr->pLogSrc ,
                  "TransactionAuthRespondUnauthenticated - Transaction 0x%p: changing to next server state (rv=%d).",
                  pTransc,rv));
        return rv;
    }
    return RV_OK;

}

/***************************************************************************
 * SetAuthenticationParams
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add the authentication header to the response message, with the
 *          given arguments in it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          realm -          mandatory.
 *          domain -         optional string. may be NULL.
 *          nonce -          optional string. may be NULL.
 *          opaque -         optional string. may be NULL.
 *          stale -          TRUE or FALSE
 *          eAlgorithm -     enumuration of algorithm. if OTHER we will take the
 *                           algorithm value from next argument.
 *          strAlgorithm -   string of algorithm. will be set only if eAlgorithm
 *                           argument is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           enumuration of qop. if OTHER we will take the
 *                           qop value from next argument.
 *          strQop -         string of qop. will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
static RvStatus RVCALLCONV SetAuthenticationParams(
                               IN  Transaction       *pTransc,
                               IN  RvSipAuthenticationHeaderHandle hAuth,
                               IN  RvUint16         responseCode,
                               IN  RvChar           *strRealm,
                               IN  RvChar           *strDomain,
                               IN  RvChar           *strNonce,
                               IN  RvChar           *strOpaque,
                               IN  RvBool           bStale,
                               IN  RvSipAuthAlgorithm eAlgorithm,
                               IN  RvChar            *strAlgorithm,
                               IN  RvSipAuthQopOption eQop,
                               IN  RvChar            *strQop)
{
    RvStatus      rv = RV_OK;
    RvSipAuthStale eStale;
    HPAGE          page = SipAuthenticationHeaderGetPage(hAuth);
    HRPOOL         pool = SipAuthenticationHeaderGetPool(hAuth);
    RvInt32       strOffset;

    /* check all mandatory parameters */
    if(strRealm == NULL)
    {
        RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. no realm was given. mandatory parameter!",
            pTransc));
        return RV_ERROR_BADPARAM;
    }

    /* set header type */
    if(responseCode == 401)
    {
        rv = RvSipAuthenticationHeaderSetHeaderType(hAuth, RVSIP_AUTH_WWW_AUTHENTICATION_HEADER);
    }
    else if (responseCode == 407)
    {
        rv = RvSipAuthenticationHeaderSetHeaderType(hAuth, RVSIP_AUTH_PROXY_AUTHENTICATION_HEADER);
    }
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set authentication headerType",
            pTransc));
        return rv;
    }

    /* scheme */
    rv = RvSipAuthenticationHeaderSetAuthScheme(hAuth,RVSIP_AUTH_SCHEME_DIGEST,NULL);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set digest scheam in authentication",
            pTransc));
        return rv;
    }

    /* set realm - need to be in "" */
    rv = SetQuotationMarks(pool, page, strRealm, &strOffset);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set realm in quotation marks in authentication header",
            pTransc));
        return rv;
    }
    rv = SipAuthenticationHeaderSetRealm(hAuth, NULL, pool, page, strOffset);
    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set realm in authentication header",
            pTransc));
        return rv;
    }

    /* set domain - optional - need to be in ""*/
    if(strDomain != NULL)
    {
        rv = SetQuotationMarks(pool, page, strDomain, &strOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set domain in quotation marks in authentication header",
                pTransc));
            return rv;
        }
        rv = SipAuthenticationHeaderSetDomain(hAuth, NULL, pool, page, strOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set domain in authentication header",
                pTransc));
            return rv;
        }
    }

    /* nonce - if NULL, we generate one  - need to be in "".*/
    if(strNonce != NULL)
    {
        rv = SetQuotationMarks(pool, page, strNonce, &strOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set nonce in quotation marks in authentication header",
                pTransc));
            return rv;
        }
        rv = SipAuthenticationHeaderSetNonce(hAuth, NULL, pool, page, strOffset);
    }
    else
    {
        /* generate nonce fom hTransc + hAuth + timestamp */
        RvChar   newNonce[128];
        RvUint32 timer     = SipCommonCoreRvTimestampGet(IN_MSEC);

        newNonce[0] = '\"';
        RvSprintf(newNonce+1,"%lx%lx%x",(RvUlong)RvPtrToUlong(pTransc),
										(RvUlong)RvPtrToUlong(hAuth), 
										timer);

        newNonce[13] = '\"';
        newNonce[14] = '\0';

        rv = RvSipAuthenticationHeaderSetNonce(hAuth, newNonce);
    }
    if(rv != RV_OK)
    {
        RvLogDebug(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set nonce in authentication header",
            pTransc));
        return rv;
    }

    /* opaque - optional - need to be in "" */
    if(strOpaque != NULL)
    {
        rv = SetQuotationMarks(pool, page, strOpaque, &strOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set opaque in quotation marks in authentication header",
                pTransc));
            return rv;
        }
        rv = SipAuthenticationHeaderSetOpaque(hAuth, NULL, pool, page, strOffset);
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set opaque in authentication header",
                pTransc));
            return rv;
        }
    }

    /* stale */
    if(bStale == RV_TRUE)
    {
        eStale = RVSIP_AUTH_STALE_TRUE;
    }
    else
    {
        eStale = RVSIP_AUTH_STALE_FALSE;
    }
    rv = RvSipAuthenticationHeaderSetStale(hAuth, eStale);

    /* algorithm */
    if(eAlgorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        rv = RvSipAuthenticationHeaderSetAuthAlgorithm(hAuth, eAlgorithm, strAlgorithm);
    }

    if(rv != RV_OK)
    {
        RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
            "SetAuthenticationParams - Transaction 0x%p. fail to set algorithm in authentication header",
            pTransc));
        return rv;
    }

    /* qop - optional */
    if(eQop != RVSIP_AUTH_QOP_UNDEFINED)
    {
        if(eQop != RVSIP_AUTH_QOP_OTHER)
        {
            rv = RvSipAuthenticationHeaderSetQopOption(hAuth, eQop);
        }
        else
        {
            rv = RvSipAuthenticationHeaderSetStrQop(hAuth, strQop);
        }
        if(rv != RV_OK)
        {
            RvLogError(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc,
                "SetAuthenticationParams - Transaction 0x%p. fail to set Qop in authentication header",
                pTransc));
            return rv;
        }
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AuthenticateHeader
 * ------------------------------------------------------------------------
 * General: Try to authenticate the given authorization header.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc        - Pointer to the transaction structure.
 *          hAuthorization - The authorization header
 *          password       - password of user and realm in given header.
 *          hObject        - handle of the object to be authenticated.
 *          eObjectType    - type of the object to be authenticated.
 *          pObjTripleLock - lock of the object. Can be NULL. If it is
 *                           supplied, it will be unlocked before callbacks
 *                           to application and locked again after them.
 * Output:  isCorrect      - The result. RV_TRUE if correct. RV_FALSE if not.
 ***************************************************************************/
static RvStatus AuthenticateHeader(
                            IN  Transaction*                   pTransc,
                            IN  RvSipAuthorizationHeaderHandle hAuthorization,
                            IN  RvChar                        *password,
                            IN  void*                          hObject,
                            IN  RvSipCommonStackObjectType     eObjectType,
                            IN  SipTripleLock*                 pObjTripleLock,
                            OUT RvBool*                        isCorrect)
{
    RvStatus stat;
    RvBool tempResult;
    RvSipAuthQopOption qopOptions;

    *isCorrect = RV_FALSE;

    qopOptions = RvSipAuthorizationHeaderGetQopOption(hAuthorization);

    /* if this is an 'empty' authorization header (response="")
       we do not need to authenticate this header */
#ifdef RV_SIP_IMS_ON
    {
        RvUint responseLen;
        responseLen = RvSipAuthorizationHeaderGetStringLength(hAuthorization, RVSIP_AUTHORIZATION_RESPONSE);
        if(responseLen <= 3) /* len of ""+NULL */
        {
            RvLogInfo(pTransc->pMgr->pLogSrc,(pTransc->pMgr->pLogSrc, 
                "AuthenticateHeader: transc 0x%p - empty authorization header 0x%p - skip authentication procedure",
                pTransc, hAuthorization));
            return RV_OK;
        }
    }
#endif /*RV_SIP_IMS_ON*/
    stat = SipAuthenticatorCredentialsSupported(pTransc->pMgr->hAuthenticator,
                                                hAuthorization,
                                                &tempResult);
    if(RV_OK != stat || RV_FALSE == tempResult ||
       (qopOptions!=RVSIP_AUTH_QOP_AUTH_ONLY && 
        qopOptions!=RVSIP_AUTH_QOP_AUTHINT_ONLY &&
        qopOptions!=RVSIP_AUTH_QOP_AUTH_AND_AUTHINT))
    {
       return RV_ERROR_UNKNOWN;
    }

    stat = SipAuthenticatorVerifyCredentials(pTransc->pMgr->hAuthenticator,
                                             hAuthorization,
                                             password,
                                             makeStrMethod(pTransc),
                                             hObject,
                                             eObjectType,
                                             pObjTripleLock,
                                             TransactionGetReceivedMsg(pTransc),
                                             isCorrect);
    return stat;
}

/***************************************************************************
 * FinishAuthentication
 * ------------------------------------------------------------------------
 * General: Calls to RvSipTransactionAuthCompletedEv.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc   - Pointer to the transaction structure.
 *          finfishResult  - Status to use when calling to the event.
 ***************************************************************************/
static void FinishAuthentication(IN  Transaction* pTransc,
                                 IN  RvBool      bAuthSucceed)
{
    TransactionCallbackAuthCompletedEv(pTransc,bAuthSucceed);
}

/***************************************************************************
 * NextAuthorizationFound
 * ------------------------------------------------------------------------
 * General: Calls to RvSipTransactionAuthCredentialsFoundEv.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc - Pointer to the transaction structure.
 *          hItem        - Position in the list of the found header.
 ***************************************************************************/
static void NextAuthorizationFound(IN Transaction       *pTransc,
                                   IN RLIST_ITEM_HANDLE hItem)
{
    RvSipAuthorizationHeaderHandle hAuthoriz;
    RvBool                        isSupported;

    pTransc->lastAuthorizGotFromList = hItem;

    hAuthoriz = *(RvSipAuthorizationHeaderHandle*)hItem;

    SipAuthenticatorCredentialsSupported(pTransc->pMgr->hAuthenticator,
                                           hAuthoriz,
                                           &isSupported);

    TransactionCallbackAuthCredentialsFoundEv(pTransc,hAuthoriz,isSupported);
}

/***************************************************************************
 * SetQuotationMarks
 * ------------------------------------------------------------------------
 * General: Set the given string to be inside quotation marks, and null terminated,
 *          On a given page. (Probobly a temp page that will be copied later)
 *          the resulted string will be: "string"\0
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hPool       - Handle to the pool.
 *          hPage       - Handle to the page.
 *          string      - The string to be set in quotation marks.
 * Output:  pStrOffset  - Offset of the beginning of the new string.
 ***************************************************************************/
static RvStatus SetQuotationMarks(IN  HRPOOL hPool,
                                   IN  HPAGE  hPage,
                                   IN  RvChar* string,
                                   OUT RvInt32* pStrOffset)
{
    RvInt32            offset;
    RvChar*            firstMark = "\"";
    RvChar*            secondMark = "\"";

    /* first "" */
    if(RPOOL_AppendFromExternal(hPool,
                                hPage,
                                (void*)firstMark,
                                (RvInt)strlen(firstMark),
                                RV_FALSE,
                                &offset,
                                NULL) != RV_OK)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    *pStrOffset = offset;

    /* string */
    if(RPOOL_AppendFromExternal(hPool,
                                hPage,
                                (void*)string,
                                (RvInt)strlen(string),
                                RV_FALSE,
                                &offset,
                                NULL) != RV_OK)
    {

        return RV_ERROR_OUTOFRESOURCES;
    }
    /* second "" */
    if(RPOOL_AppendFromExternal(hPool,
                                hPage,
                                (void*)secondMark,
                                (RvInt)strlen(secondMark)+1, /* +1 for null termination */
                                RV_FALSE,
                                &offset,
                                NULL) != RV_OK)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}

/***************************************************************************
 * makeStrMethod
 * ------------------------------------------------------------------------
 * General: Convert the method enumeration + string that kept in the transaction
 *          structure, to a string.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc - Pointer to the transaction structure.
 ***************************************************************************/
static RvChar* makeStrMethod(Transaction* pTransc)
{
    if(pTransc->eMethod == SIP_TRANSACTION_METHOD_OTHER)
    {
        return pTransc->strMethod;
    }

    return TransactionGetStringMethodFromEnum(pTransc->eMethod);
}

#endif /* #ifdef RV_SIP_AUTH_ON */

