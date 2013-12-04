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
 *                              AuthenticatorObject.c
 *
 * This c file contains functions that prepare digest string for
 * the authentication hash algorithm. The output of the algorithm will be the response
 * parameter of the authorization header.
 *
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                      Jan. 2001
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifndef RV_SIP_LIGHT
#include "AuthenticatorObject.h"
#include "AuthenticatorCallbacks.h"
#include "_SipAuthenticator.h"

#include "_SipAddress.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthorizationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "RvSipAuthenticationInfoHeader.h"
#include "_SipAuthenticationInfoHeader.h"
#include "_SipCommonCore.h"
#include "rvansi.h"
/*
#include "AuthenticatorHighAvailability.h"*/

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

#define MAX_CNONCE_LEN 50

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_AUTH_ON
static RvStatus AuthenticatorAppendString(IN  AuthenticatorInfo  *pAuthInfo,
                                          IN  HRPOOL             destPool,
                                          IN  HPAGE              destPage,
                                          IN  HRPOOL             hPool,
                                          IN  HPAGE              hPage,
                                          IN  RvInt32           stringOffset,
                                           OUT RvUint32*         length);

static RvStatus AuthenticatorAppendExternalString(IN  AuthenticatorInfo  *pAuthInfo,
                                                  IN  HRPOOL             hPool,
                                                  IN  HPAGE              hPage,
                                                  IN  RvChar*            string,
                                                  OUT RvUint32*          length);



static RvStatus AuthenticatorAppendQuotedString(IN  AuthenticatorInfo  *pAuthInfo,
                                                IN  HRPOOL             destPool,
                                                IN  HPAGE              destPage,
                                                IN  HRPOOL             hPool,
                                                IN  HPAGE              hPage,
                                                IN  RvInt32           stringOffset,
                                                 OUT RvUint32*         length);

static RvStatus AuthenticatorGetStrRequestUri(
                                              IN     AuthenticatorInfo   *pAuthInfo,
                                              IN     HRPOOL              hPool,
                                              IN     HPAGE               hPage,
                                              IN     RvSipAddressHandle  hRequestUri,
                                              IN     RvInt32             origUriOffset,
                                              IN     RvBool              bAppendZero,
                                              IN OUT RvUint32            *length);

static RvStatus AuthenticatorGetParamsFromAuthorization(
                                                        IN AuthenticatorInfo                *pAuthInfo,
                                                        IN RvSipAuthorizationHeaderHandle   hAuthorization,
                                                        OUT RvSipAddressHandle              *hRequestUri,
                                                        OUT AuthenticatorParams             *pAuthParams);
static RvStatus AuthenticatorGetOtherParamsFromAuthorization(
                                             IN AuthenticatorParams              *pAuthParams,
                                             IN RvSipAuthorizationHeaderHandle   hAuthorization,
                                             OUT RvChar                          *strNonceCount,
                                             OUT RvChar                          *strCnonce,
                                             OUT RvChar                          **strQopVal);

static void AuthenticatorGetNewCnonce(OUT RvChar *pCNonce);

static RvStatus AuthenticatorGetSecret(IN    AuthenticatorInfo               *pAuthInfo,
                                       IN    RvSipAddressHandle               hRequestUri,
                                       IN    void*                            hObject,
                                       IN    RvSipCommonStackObjectType       eObjectType,
                                       IN    SipTripleLock                   *pObjTripleLock,
                                       INOUT AuthenticatorParams             *pAuthParams);

static RvStatus AuthenticatorGetSecretFromAuthenticationHeader(
                                       IN    AuthenticatorInfo               *pAuthInfo,
                                       IN    RvSipAuthenticationHeaderHandle  hAuthentication,
                                       IN    RvSipAddressHandle               hRequestUri,
                                       IN    void*                            hObject,
                                       IN    RvSipCommonStackObjectType       eObjectType,
                                       IN    SipTripleLock                   *pObjTripleLock,
                                       INOUT AuthenticatorParams             *pAuthParams);

static RvStatus AuthenticatorCreateCopyOfGlobalHeader(
                          IN  AuthenticatorInfo               *pAuthMgr,
                          OUT HPAGE                           *phTmpHeaderPage,
                          OUT RvSipAuthenticationHeaderHandle *phTmpHeader);

static RvStatus CreateResponseParamForAuthorization(IN  AuthenticatorInfo    *authInfo,
                                                    IN  AuthenticatorParams  *pAuthParams,
                                                    IN  RvChar               *strMethod,
                                                    IN  RvSipAddressHandle   hRequestUri,
                                                    IN  RvChar               *strNc,
                                                    IN  RvChar               *strCnonce,
                                                    IN  RvChar               *strQopVal,
                                                    IN  void*                hObject,
                                                    IN  RvSipCommonStackObjectType eObjectType,
                                                    IN  SipTripleLock        *pObjTripleLock,
                                                    IN  RvSipMsgHandle       hMsg,
                                                    IN  HPAGE                hResponsePage,
                                                    IN  HPAGE                hHelpPage,
                                                    OUT RPOOL_Ptr            *pRpoolResponse);

static RvStatus CreateResponseParamForAuthenticationInfo(IN  AuthenticatorInfo    *authInfo,
                                                         IN  AuthenticatorParams  *pAuthParams,
                                                         IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                                         IN  RvSipAddressHandle   hRequestUri,
                                                         IN  void*                hObject,
                                                         IN  RvSipCommonStackObjectType eObjectType,
                                                         IN  SipTripleLock        *pObjTripleLock,
                                                         IN  RvSipMsgHandle       hMsg,
                                                         IN  HPAGE                hResponsePage,
                                                         IN  HPAGE                hHelpPage,
                                                         OUT RPOOL_Ptr            *pRpoolResponse);

static RvStatus AuthenticatorSetParamsToAuthenticationInfo(IN AuthenticatorInfo    *authInfo,
                                                           IN AuthenticatorParams  *pAuthParams,
                                                           IN RPOOL_Ptr            pRpoolResponse,
                                                           IN RvSipAuthorizationHeaderHandle hAuthorization,
                                                           IN RvSipAuthenticationInfoHeaderHandle hHeader);

static void cleanAuthParamsStruct(AuthenticatorParams  *pAuthParams);

static RvStatus RVCALLCONV AuthenticatorCreateA1(
                            IN AuthenticatorInfo*    pAuthInfo,
                            IN AuthenticatorParams*  pAuthParams,
                            IN HRPOOL                hPool,
                            IN HPAGE                 hPage,
                            OUT RvUint32*            length);

static RvStatus RVCALLCONV AuthenticatorCreateA2(
                                IN AuthenticatorInfo          *pAuthInfo,
                                IN AuthenticatorParams        *pAuthParams,
                                IN HRPOOL                     hPool,
                                IN HPAGE                      hPage,
                                IN RvChar                     *strMethod,
                                IN RvSipAddressHandle         hRequestUri,
                                IN RvInt32                    origUriOffset,
                                IN void*                      hObject,
                                IN RvSipCommonStackObjectType eObjectType,
                                IN SipTripleLock              *pObjTripleLock,
                                IN RvSipMsgHandle             hMsg,
                                OUT RvUint32                  *length);

static RvStatus RVCALLCONV AuthenticatorCreateA2ForAuthInfo(
                                   IN AuthenticatorInfo          *pAuthInfo,
                                   IN AuthenticatorParams        *pAuthParams,
                                   IN HRPOOL                     hPool,
                                   IN HPAGE                      hPage,
                                   IN RvSipAddressHandle         hRequestUri,
                                   IN void*                      hObject,
                                   IN RvSipCommonStackObjectType eObjectType,
                                   IN SipTripleLock              *pObjTripleLock,
                                   IN RvSipMsgHandle             hMsg,
                                   OUT RvUint32                  *length);

static RvStatus RVCALLCONV AuthenticatorCreateDigestStr(
                                    IN AuthenticatorInfo                *pAuthInfo,
                                    IN AuthenticatorParams              *pAuthParams,
                                    IN HRPOOL                           hHelpPool,
                                    IN HPAGE                            hHelpPage,
                                    IN HRPOOL                           hResponsePool,
                                    IN HPAGE                            hResponsePage,
                                    IN RvUint32                        lengthA1,
                                    IN RvUint32                        lengthA2,
                                    IN RvChar                          *strNonceCount,
                                    IN RvChar                          *strCnonce,
                                    IN RvChar                          *strQopVal,
                                   OUT RPOOL_Ptr                       *pRpoolResponse);

static void AuthenticatorCreateOtherParams(
                                        IN  AuthenticatorInfo   *pAuthMgr,
                                        IN  AuthenticatorParams *pAuthParams,
                                        IN  RvInt32              nonceCount,
                                        OUT RvChar              *strNonceCount,
                                        OUT RvChar              *strCnonce,
                                        OUT RvChar             **strQopVal);

static RvStatus AuthenticatorGetParamsFromAuthentication(
                                 IN AuthenticatorInfo                  *pAuthInfo,
                                 IN RvSipAuthenticationHeaderHandle    hAuthentication,
                                 IN RvSipAddressHandle                 hRequestUri,
                                 IN  void                             *hObject,
                                 IN  RvSipCommonStackObjectType        eObjectType,
                                 IN  SipTripleLock                    *pObjTripleLock,
                                 OUT AuthenticatorParams              *pAuthParams);

static RvChar* AuthenticatorGetQopStr(IN RvSipAuthQopOption qop);

static RvStatus AuthenticatorSetParamsToAuthorization (
                            IN AuthenticatorInfo               *pAuthInfo,
                            IN AuthenticatorParams             *pAuthParams,
                            IN RvSipAddressHandle              hRequestUri,
                            IN RvInt32                         nonceCount,
                            IN RvChar                          *strCnonce,
                            IN RvChar                          *strQopVal,
                            IN RvSipAuthorizationHeaderHandle  hAuthorization);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * AuthenticatorCreateA1
 * ----------------------------------------------------------------------------------
 * General: create the A1 digest string (as specified in RFC 2617)which is
 *          a part of the response parameter of the authorization header.
 * Return Value: RvStatus -
 *          RV_OK     - the A1 string was created successfully
 *          RV_ERROR_UNKNOWN     - failed in creating the A1 string
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo      - the authentication info structure
 *          pAuthParams    - the structure that holds the parameters from the authentication header
 *          hPage          - handle to the memory page that is used for building the response
 * Output:  length         - the A1 string length.
 ***********************************************************************************/
static RvStatus RVCALLCONV AuthenticatorCreateA1(
                           IN AuthenticatorInfo               *pAuthInfo,
                           IN AuthenticatorParams             *pAuthParams,
                           IN HRPOOL                          hPool,
                           IN HPAGE                           hPage,
                           OUT RvUint32                      *length)
{
    RvStatus          rv;


    if (pAuthParams->eAuthAlgorithm == RVSIP_AUTH_ALGORITHM_MD5)
    {
        rv = AuthenticatorAppendString(pAuthInfo,
                                           hPool,
                                           hPage,
                                           pAuthParams->hPool,
                                           pAuthParams->hPage,
                                           pAuthParams->userNameOffset,
                                           length);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA1 - failed to append username"));
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hPool,
                                                   hPage,
                                                   ":",
                                                   length);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA1 - failed to append colon 1"));
            return rv;
        }
        rv = AuthenticatorAppendQuotedString(pAuthInfo,
                                                 hPool,
                                                 hPage,
                                                 pAuthParams->hPool,
                                                 pAuthParams->hPage,
                                                 pAuthParams->realmOffset,
                                                 length);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA1 - failed to append realm"));
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hPool,
                                                   hPage,
                                                   ":",
                                                   length);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA1 - failed to append colon 2"));
            return rv;
        }
        if (UNDEFINED != pAuthParams->passwordOffset)
        {
            rv = AuthenticatorAppendString(pAuthInfo,
                                           hPool,
                                           hPage,
                                           pAuthParams->hPool,
                                           pAuthParams->hPage,
                                           pAuthParams->passwordOffset,
                                           length);
            if(rv != RV_OK)
            {
                RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                    "AuthenticatorCreateA1 - failed to append password"));
                return rv;
            }
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hPool,
                                                   hPage,
                                                   "\0",
                                                   length);
    }
    else
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorCreateA1 - the authentication algorithm is not MD5"));
        return RV_ERROR_UNKNOWN;
    }

    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorCreateA1 - failed while appending strings"));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * AuthenticatorCreateA2
 * ----------------------------------------------------------------------------
 * General: create the A2 digest string (as specified in RFC 2617)which is
 *          a part of the response parameter of the authorization header.
 *
 *          If the "qop" directive's value is "auth" or is unspecified, then A2 is:
 *          A2       = Method ":" digest-uri-value
 *          If the "qop" value is "auth-int", then A2 is:
 *          A2       = Method ":" digest-uri-value ":" H(entity-body)
 *
 * Return Value: RvStatus -
 *          RV_OK     - the A2 string was created successfully
 *          RV_ERROR_UNKNOWN     - failed in creating the A2 string
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo      - the authentication inf structure
 *          pAuthParams    - the structure that holds the parameters
 *                           from the authentication header
 *          hPage          - handle to the memory page that is used for
 *                           building the response
 *          strMethod      - the method type of the request.
 *          hRequestUri    - handle to the request uri.
 *          origUriOffset  - when building from Authorization we want to use the excat uri.
 *          hObject        - handle to the Object, that is served
 *                           by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType   - pointer to the variable, that stores type of
 *                           the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *          hObjectLock    - Lock of the object. It has to be unlocked before
 *                           code control passes to the Application
 *          hMsg           - handle to the message object, for which
 *                           the Authenticator header is being prepared
 * Output:  strA2Digest    - the A2 string.
 *****************************************************************************/
static RvStatus RVCALLCONV AuthenticatorCreateA2(
                                   IN AuthenticatorInfo          *pAuthInfo,
                                   IN AuthenticatorParams        *pAuthParams,
                                   IN HRPOOL                     hPool,
                                   IN HPAGE                      hPage,
                                   IN RvChar                     *strMethod,
                                   IN RvSipAddressHandle         hRequestUri,
                                   IN RvInt32                    origUriOffset,
                                   IN void*                      hObject,
                                   IN RvSipCommonStackObjectType eObjectType,
                                   IN SipTripleLock              *pObjTripleLock,
                                   IN RvSipMsgHandle             hMsg,
                                   OUT RvUint32                  *length)
{
    RvStatus rv;
    RvBool   bAuthInt = 
        RVSIP_AUTH_QOP_AUTHINT_ONLY==pAuthParams->eQopOption ||
        RVSIP_AUTH_QOP_AUTH_AND_AUTHINT==pAuthParams->eQopOption;

    switch(pAuthParams->eQopOption)
    {
        case RVSIP_AUTH_QOP_AUTH_ONLY:
        case RVSIP_AUTH_QOP_AUTH_AND_AUTHINT:
        case RVSIP_AUTH_QOP_OTHER:
        case RVSIP_AUTH_QOP_UNDEFINED:
        case RVSIP_AUTH_QOP_AUTHINT_ONLY:
            {
                /* Append SIP Method */
                rv = AuthenticatorAppendExternalString(
                        pAuthInfo,hPool, hPage, strMethod, length);
                if(rv != RV_OK)
                {
                    RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                        "AuthenticatorCreateA2 - Object 0x%p - Failed in AuthenticatorAppendExternalString() 1 (rv=%d:%s)",
                        hObject,rv,SipCommonStatus2Str(rv)));
                    return rv;
                }
                /* Append ":" */
                rv = AuthenticatorAppendExternalString(
                        pAuthInfo, hPool, hPage, ":", length);
                if(rv != RV_OK)
                {
                    RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                        "AuthenticatorCreateA2 - Object 0x%p - Failed in AuthenticatorAppendExternalString() 2 (rv=%d:%s)",
                        hObject,rv,SipCommonStatus2Str(rv)));
                    return rv;
                }
                /* Append Request:URI + "\0" if needed */
                rv = AuthenticatorGetStrRequestUri(pAuthInfo, hPool, hPage, hRequestUri, origUriOffset,
                    (RV_TRUE!=bAuthInt)/*bAppendZero*/,length);
                if (rv != RV_OK)
                {
                    RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                              "AuthenticatorCreateA2 - Object 0x%p - Failed in AuthenticatorGetStrRequestUri() (rv=%d:%s)",
                              hObject,rv,SipCommonStatus2Str(rv)));
                    return rv;
                }
                /* Append H(entity-body), if the QualityOfProtection (QoP) is 'auth-int' */
                if (RV_TRUE==bAuthInt)
                {
                    RvUint32   sizeOfMD5EntityBody = 0;
                    RPOOL_Ptr pRpoolPtr;
                    pRpoolPtr.hPool =hPool;
                    pRpoolPtr.hPage =hPage;
                    pRpoolPtr.offset =*length;

                /* Append ":" */
                    rv = AuthenticatorAppendExternalString(
                                        pAuthInfo, hPool, hPage, ":", length);
                    if(rv != RV_OK)
                    {
                        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                            "AuthenticatorCreateA2 - Object 0x%p - Failed in AuthenticatorAppendExternalString() 3 (rv=%d:%s)",
                            hObject,rv,SipCommonStatus2Str(rv)));
                        return rv;
                    }
                /* Append H(entity-body) */
                    AuthenticatorCallbackAddMD5EntityBody(pAuthInfo, hObject,
                        eObjectType, pObjTripleLock, hMsg, &pRpoolPtr,
                        &sizeOfMD5EntityBody);
                    *length += sizeOfMD5EntityBody;
                /* Append "\0" */
                    rv = AuthenticatorAppendExternalString(
                                        pAuthInfo, hPool, hPage, "\0", length);
                    if (rv != RV_OK)
                    {
                        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                            "AuthenticatorCreateA2 - Object 0x%p - Failed in AuthenticatorAppendExternalString() 4 (rv=%d:%s)",
                            hObject,rv,SipCommonStatus2Str(rv)));
                        return rv;
                    }
                } /* END OF: if (RV_TRUE==bAuthInt) */
            }
            break;

        default:
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA2 - Object 0x%p - unsupported qop - %d",
                hObject, pAuthParams->eQopOption));
            return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}


/******************************************************************************
 * AuthenticatorCreateA2ForAuthInfo
 * ----------------------------------------------------------------------------
 * General:    A2       = ":" digest-uri-value
 *             and if "qop=auth-int", then A2 is
 *             A2       = ":" digest-uri-value ":" H(entity-body)
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo      - the authentication inf structure
 *          pAuthParams    - the structure that holds the parameters
 *                           from the authentication header
 *          hPage          - handle to the memory page that is used for
 *                           building the response
 *          strMethod      - the method type of the request.
 *          hRequestUri    - handle to the request uri.
 *          hObject        - handle to the Object, that is served
 *                           by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType   - pointer to the variable, that stores type of
 *                           the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *          hObjectLock    - Lock of the object. It has to be unlocked before
 *                           code control passes to the Application
 *          hMsg           - handle to the message object, for which
 *                           the Authenticator header is being prepared
 * Output:  strA2Digest    - the A2 string.
 *****************************************************************************/
static RvStatus RVCALLCONV AuthenticatorCreateA2ForAuthInfo(
                                   IN AuthenticatorInfo          *pAuthInfo,
                                   IN AuthenticatorParams        *pAuthParams,
                                   IN HRPOOL                     hPool,
                                   IN HPAGE                      hPage,
                                   IN RvSipAddressHandle         hRequestUri,
                                   IN void*                      hObject,
                                   IN RvSipCommonStackObjectType eObjectType,
                                   IN SipTripleLock              *pObjTripleLock,
                                   IN RvSipMsgHandle             hMsg,
                                   OUT RvUint32                  *length)
{
    RvStatus rv;
    RvBool   bAuthInt = RV_FALSE;

    if(RVSIP_AUTH_QOP_AUTHINT_ONLY==pAuthParams->eQopOption ||
       RVSIP_AUTH_QOP_AUTH_AND_AUTHINT==pAuthParams->eQopOption)
    {
        bAuthInt = RV_TRUE;
    }
    

    /* Append Request:URI + "\0" if needed */
    rv = AuthenticatorGetStrRequestUri(pAuthInfo, hPool, hPage, hRequestUri, UNDEFINED,
                                       (RV_TRUE!=bAuthInt)/*bAppendZero*/,length);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
              "AuthenticatorCreateA2ForAuthInfo - Object 0x%p - Failed in AuthenticatorGetStrRequestUri() (rv=%d:%s)",
              hObject,rv,SipCommonStatus2Str(rv)));
        return rv;
    }
    /* Append H(entity-body), if the QualityOfProtection (QoP) is 'auth-int' */
    if (RV_TRUE == bAuthInt)
    {
        RvUint32   sizeOfMD5EntityBody = 0;
        RPOOL_Ptr pRpoolPtr;
        pRpoolPtr.hPool =hPool;
        pRpoolPtr.hPage =hPage;
        pRpoolPtr.offset =*length;

        /* Append ":" */
        rv = AuthenticatorAppendExternalString(
                            pAuthInfo, hPool, hPage, ":", length);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA2ForAuthInfo - Object 0x%p - Failed in AuthenticatorAppendExternalString() 3 (rv=%d:%s)",
                hObject,rv,SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Append H(entity-body) */
        AuthenticatorCallbackAddMD5EntityBody(pAuthInfo, hObject,
                                        eObjectType, pObjTripleLock, hMsg, &pRpoolPtr,
                                        &sizeOfMD5EntityBody);
        *length += sizeOfMD5EntityBody;
        
        /* Append "\0" */
        rv = AuthenticatorAppendExternalString(
                            pAuthInfo, hPool, hPage, "\0", length);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorCreateA2ForAuthInfo - Object 0x%p - Failed in AuthenticatorAppendExternalString() 4 (rv=%d:%s)",
                hObject,rv,SipCommonStatus2Str(rv)));
            return rv;
        }
    } /* END OF: if (RV_TRUE==bAuthInt) */

    return RV_OK;
}

/************************************************************************************
 * AuthenticatorCreateDigestStr
 * ----------------------------------------------------------------------------------
 * General: create the digest string (as specified in RFC 2617)which will
 *          be the response parameter of the authorization header after the
 *          hash algorithm is applied.
 * Return Value: RvStatus -
 *          RV_OK     - the digest string was created successfully
 *          RV_ERROR_UNKNOWN     - failed in creating the digest string
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo      - authentication info structure
 *          pAuthParams    - the structure that holds the parameters of the authentication header
 *          hhelpPage      - handle to the helper memory page
 *          hResponsePage  - handle to the memory page that is used for building the response
 *          lengthA1       - length of A1 string
 *          lengthA2       - length of A2 string
 *          strA2Digest    - a2 string
 *          strNonceCount  - how many times did we use that nonce
 *          strCnonce      - the cnonce string (only if the qop is present)
 *          strNonce       - the nonce string that was got from the authentication header
 *          strQopVal      - the qop value that is going to be set
 * Output:  pRpoolResponse - the response string in rpool_ptr format.
 ***********************************************************************************/
static RvStatus RVCALLCONV AuthenticatorCreateDigestStr(
                                   IN AuthenticatorInfo                *pAuthInfo,
                                   IN AuthenticatorParams              *pAuthParams,
                                   IN HRPOOL                           hHelpPool,
                                   IN HPAGE                            hHelpPage,
                                   IN HRPOOL                           hResponsePool,
                                   IN HPAGE                            hResponsePage,
                                   IN RvUint32                        lengthA1,
                                   IN RvUint32                        lengthA2,
                                   IN RvChar                          *strNonceCount,
                                   IN RvChar                          *strCnonce,
                                   IN RvChar                          *strQopVal,
                                   OUT RPOOL_Ptr                       *pRpoolResponse)
{
    RvStatus                     rv;
    RvUint32                     length;
    RPOOL_Ptr                     pRpoolPtr;
    RPOOL_Ptr                     pRpoolMD5Output;
    RvInt32                      a1MD5OutputOffset;
    RvInt32                      a2MD5OutputOffset;

    /* initialize the rpool ptr to the md5 output */
    pRpoolMD5Output.hPool  = hHelpPool;
    pRpoolMD5Output.hPage  = hHelpPage;
    pRpoolMD5Output.offset = 0;

    /* notifying the application that there is a need to use the hash algorithm */
    if (RV_TRUE == AuthenticatorActOnCallback(pAuthInfo, AUTH_CALLBACK_MD5))
    {
        pRpoolPtr.hPool  = hHelpPool;
        pRpoolPtr.hPage  = hHelpPage;
        pRpoolPtr.offset = 0;
        AuthenticatorCallbackMD5Calculate(pAuthInfo, &pRpoolPtr, lengthA1, &pRpoolMD5Output);
        a1MD5OutputOffset = pRpoolMD5Output.offset;

        pRpoolPtr.offset = lengthA1;
        pRpoolMD5Output.offset = 0;
        AuthenticatorCallbackMD5Calculate(pAuthInfo, &pRpoolPtr, lengthA2, &pRpoolMD5Output);
        a2MD5OutputOffset = pRpoolMD5Output.offset;
    }
    else
    {
        /* the md5 handler was not set */
        RvLogExcep(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorCreateDigestStr - the md5 handler was not set"));
        return RV_ERROR_UNKNOWN;
    }
   length = 0;
    /* building the final string */
   if (strQopVal != NULL)
   {
       /* calculating the final string */
        rv = AuthenticatorAppendString(pAuthInfo,
                                          hResponsePool,
                                          hResponsePage,
                                          hHelpPool,
                                          hHelpPage,
                                          a1MD5OutputOffset,
                                          &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendQuotedString(pAuthInfo,
                                                 hResponsePool,
                                                 hResponsePage,
                                                 pAuthParams->hPool,
                                                 pAuthParams->hPage,
                                                 pAuthParams->nonceOffset,
                                                 &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   strNonceCount,
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   strCnonce,
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   strQopVal,
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendString(pAuthInfo,
                                           hResponsePool,
                                           hResponsePage,
                                           hHelpPool,
                                           hHelpPage,
                                           a2MD5OutputOffset,
                                           &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   "\0",
                                                   &length);
    }
    else
    {   /* the qop value is not present */
        /* calculating the final string */
        rv = AuthenticatorAppendString(pAuthInfo,
                                          hResponsePool,
                                          hResponsePage,
                                          hHelpPool,
                                          hHelpPage,
                                          a1MD5OutputOffset,
                                          &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendQuotedString(pAuthInfo,
                                                 hResponsePool,
                                                 hResponsePage,
                                                 hHelpPool,
                                                 pAuthParams->hPage,
                                                 pAuthParams->nonceOffset,
                                                 &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   ":",
                                                   &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendString(pAuthInfo,
                                           hResponsePool,
                                           hResponsePage,
                                           hHelpPool,
                                           hHelpPage,
                                           a2MD5OutputOffset,
                                           &length);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   "\0",
                                                   &length);
    }

    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorCreateDigestStr - failed while appending strings"));
        return rv;
    }

    /* inserting the first quotation mark for the response string */
    rv = AuthenticatorAppendExternalString(pAuthInfo,
                                               hResponsePool,
                                               hResponsePage,
                                               "\"",
                                               &length);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorCreateDigestStr - failed while appending strings"));
        return rv;
    }
    /* notifying the application that there is a need to use the hash algorithm */
    if (RV_TRUE == AuthenticatorActOnCallback(pAuthInfo, AUTH_CALLBACK_MD5))
    {
        pRpoolPtr.hPool  = hResponsePool;
        pRpoolPtr.hPage  = hResponsePage;
        pRpoolPtr.offset = 0;

        pRpoolMD5Output.hPool = hResponsePool;
        pRpoolMD5Output.hPage = hResponsePage;
        pRpoolMD5Output.offset = 0;

        AuthenticatorCallbackMD5Calculate(pAuthInfo, &pRpoolPtr, length, &pRpoolMD5Output);

        /* checking if the offset from the application is valid */
        if (pRpoolMD5Output.offset == UNDEFINED)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorCreateDigestStr - failed in getting the response offset from the application"));
            return RV_ERROR_UNKNOWN;
        }
        /* inserting the last quotation mark for the response string */
        rv = RPOOL_CopyFromExternal(hResponsePool,
                                        hResponsePage,
                                        pRpoolMD5Output.offset + 32,
                                        (void*)"\"",
                                        1);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorCreateDigestStr - failed while appending strings"));
            return rv;
        }
        rv = AuthenticatorAppendExternalString(pAuthInfo,
                                                   hResponsePool,
                                                   hResponsePage,
                                                   "\0",
                                                   &length);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorCreateDigestStr - failed while appending strings"));
            return rv;
        }
        pRpoolResponse->hPool  = hResponsePool;
        pRpoolResponse->hPage  = hResponsePage;
        pRpoolResponse->offset = pRpoolMD5Output.offset - 1;
    }


    return RV_OK;

}

/***************************************************************************
 * AuthenticatorSetParamsToAuthorization
 * ------------------------------------------------------------------------
 * General: The function sets parameters from the AuthenticatorPrams struct to
 *          the authorization header.
 * Return Value: RvStatus - RV_OK - the parameters were set successfully
 *                           RV_ERROR_UNKNOWN - failed in setting the parameters
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         pAuthParams     - the structure that holds the offset and values of the
 *                           authentication parameters.
 *         hRequestUri     - handle to the requestUri
 *         nonceCount      - the number of times we used the current nonce
 *         strCnonce       - the cnonce string
 *         strQopVal       - the qop option in string format
 * Output: hAuthorization  - handle to the authentication header
 ***************************************************************************/
static RvStatus AuthenticatorSetParamsToAuthorization (
                            IN AuthenticatorInfo               *pAuthInfo,
                            IN AuthenticatorParams             *pAuthParams,
                            IN RvSipAddressHandle              hRequestUri,
                            IN RvInt32                         nonceCount,
                            IN RvChar                          *strCnonce,
                            IN RvChar                          *strQopVal,
                            IN RvSipAuthorizationHeaderHandle  hAuthorization)
{
    RvStatus rv;
    HPAGE     hPage;
    RvChar   tempCnonce[12];
    RvUint32 length = 0;

    /* settings for the Authorization header */

    /* header type */
    if (pAuthParams->eAuthHeaderType == RVSIP_AUTH_WWW_AUTHENTICATION_HEADER)
    {
        rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorization,
                                                       RVSIP_AUTH_AUTHORIZATION_HEADER);
    }
    else
    {
        rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorization,
                                                       RVSIP_AUTH_PROXY_AUTHORIZATION_HEADER);
    }
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the header type"));
        return rv;
    }

    /* authentication scheme */
    rv = RvSipAuthorizationHeaderSetAuthScheme(hAuthorization,
                                                   RVSIP_AUTH_SCHEME_DIGEST,
                                                   "Digest");
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the authentication scheme"));
        return rv;
    }

    /* realm */
    rv = SipAuthorizationHeaderSetRealm(hAuthorization,
                                            NULL,
                                            pAuthParams->hPool,
                                            pAuthParams->hPage,
                                            pAuthParams->realmOffset);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the realm"));
        return rv;
    }
    /*nonce */
    rv = SipAuthorizationHeaderSetNonce(hAuthorization,
                                            NULL,
                                            pAuthParams->hPool,
                                            pAuthParams->hPage,
                                            pAuthParams->nonceOffset);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the nonce"));
        return rv;
    }
    /* opaque */
    rv = SipAuthorizationHeaderSetOpaque(hAuthorization,
                                             NULL,
                                             pAuthParams->hPool,
                                             pAuthParams->hPage,
                                             pAuthParams->opaqueOffset);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the opaque"));
        return rv;
    }
    /* request uri */
    rv = RvSipAuthorizationHeaderSetDigestUri(hAuthorization, hRequestUri);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the request uri"));
        return rv;
    }

    /* username */
    /* getting new help page for adding quotation marks to the user name */
    rv = RPOOL_GetPage(pAuthInfo->hGeneralPool, 0, &hPage);
	if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
			"AuthenticatorSetParamsToAuthorization - to allocate new page from pool 0x%p,  rv=%d", pAuthInfo->hGeneralPool, rv));
        return rv;
    }
    rv = AuthenticatorAppendExternalString(pAuthInfo,
                                               pAuthInfo->hGeneralPool,
                                               hPage,
                                               "\"",
                                               &length);
	if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in preparing the quoted username"));
        RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);
        return rv;
    }
    rv = AuthenticatorAppendString(pAuthInfo,
                                       pAuthInfo->hGeneralPool,
                                       hPage,
                                       pAuthParams->hPool,
                                       pAuthParams->hPage,
                                       pAuthParams->userNameOffset,
                                       &length);
	if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in preparing the quoted username"));
        RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);
        return rv;
    }
	
    rv = AuthenticatorAppendExternalString(pAuthInfo,
                                               pAuthInfo->hGeneralPool,
                                               hPage,
                                               "\"",
                                               &length);
	if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in preparing the quoted username"));
        RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);
        return rv;
    }
    rv = AuthenticatorAppendExternalString(pAuthInfo,
                                               pAuthInfo->hGeneralPool,
                                               hPage,
                                               "\0",
                                               &length);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in preparing the quoted username"));
        RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);
        return rv;
    }

    rv = SipAuthorizationHeaderSetUserName(hAuthorization,
                                               NULL,
                                               pAuthInfo->hGeneralPool,
                                               hPage,
                                               0);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the username"));
        RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);
        return rv;
    }
    RPOOL_FreePage(pAuthInfo->hGeneralPool, hPage);

    /* qop, nonceCount and cnonce */
    if (pAuthParams->eQopOption != RVSIP_AUTH_QOP_UNDEFINED)
    {
        if (strQopVal != NULL)
        {
            RvSipAuthQopOption eQop;

            if (strcmp(strQopVal, "auth") == 0)
            {
                eQop = RVSIP_AUTH_QOP_AUTH_ONLY;
            }
            else
            if (strcmp(strQopVal, "auth-int") == 0)
            {
                eQop = RVSIP_AUTH_QOP_AUTHINT_ONLY;
            }
            else
            {
                RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                    "AuthenticatorSetParamsToAuthorization - only quality of protection - auth is supported"));
                return RV_ERROR_UNKNOWN;
            }
            rv = RvSipAuthorizationHeaderSetQopOption(hAuthorization, eQop);
            if (rv != RV_OK)
            {
                RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                          "AuthenticatorSetParamsToAuthorization - failed in setting the qop options"));
                return rv;
            }
        }

        rv = RvSipAuthorizationHeaderSetNonceCount(hAuthorization, nonceCount);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorSetParamsToAuthorization - failed in setting the nonce count"));
            return rv;

        }
        /* cnonce */
        RvSprintf(tempCnonce, "\"%s\"", strCnonce);
        rv = RvSipAuthorizationHeaderSetCNonce(hAuthorization, tempCnonce);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorSetParamsToAuthorization - failed in setting the cnonce"));
            return rv;

        }
    }
    rv = RvSipAuthorizationHeaderSetAuthAlgorithm(hAuthorization,
                                                  pAuthParams->eAuthAlgorithm,
                                                  NULL);
    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorSetParamsToAuthorization - failed in setting the algorithm"));
        return rv;
    }
#ifdef RV_SIP_IMS_ON
    rv = RvSipAuthorizationHeaderSetAKAVersion(hAuthorization, pAuthParams->akaVersion);
#endif

    return RV_OK;

}

/***************************************************************************
 * AuthenticatorGetQopStr
 * ------------------------------------------------------------------------
 * General: The function gets the qop in a string format.
 * Return Value: RvChar* - the qop string or NULL if the qop options are not present.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  qop - the qop options of the server
 * Output: none
 ***************************************************************************/
static RvChar* AuthenticatorGetQopStr(IN RvSipAuthQopOption qop)
{
    if (qop == RVSIP_AUTH_QOP_AUTH_ONLY)
    {
        return "auth";
    }
    else if (qop == RVSIP_AUTH_QOP_AUTHINT_ONLY ||
             qop == RVSIP_AUTH_QOP_AUTH_AND_AUTHINT)
    {
        return "auth-int";
    }

    return NULL;
}

/******************************************************************************
 * AuthenticatorAuthorizationHeaderListAddHeader
 * ----------------------------------------------------------------------------
 * General: copies the Authorization header, and adds it to the list of
 *          Authorization Headers.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr   - Pointer to the Authentication manager object.
 *         hListAuthorizationHeaders - Handle of the List, to which the new
 *                      element should be added. 
 *         hHeader    - Handle of the header to be added to the list.
 *         hPool      - Pool, on page of which the header copy will be
 *                      constructed.
 *         hPage      - Page, on which the header copy will be constructed.
 * Output: none.
 *****************************************************************************/
RvStatus AuthenticatorAuthorizationHeaderListAddHeader(
                IN  AuthenticatorInfo*              pAuthMgr,
                IN  RLIST_HANDLE                    hListAuthorizationHeaders,
                IN  RvSipAuthorizationHeaderHandle  hHeader,
                IN  HRPOOL                          hPool,
                IN  HPAGE                           hPage)
{
    RvStatus rv;
    RLIST_ITEM_HANDLE              hListElement;
    RvSipAuthorizationHeaderHandle hHeaderCopy;
    
    rv = RLIST_InsertTail(NULL/*hPoolList*/, hListAuthorizationHeaders, &hListElement);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorAuthorizationHeaderListAddHeader - failed to add element to list 0x%p (rv=%d:%s)",
            hListAuthorizationHeaders, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = RvSipAuthorizationHeaderConstruct(pAuthMgr->hMsgMgr, hPool, hPage,
        &hHeaderCopy);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorAuthorizationHeaderListAddHeader - failed to construct header (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
		/*Since we already allocated element in the list and we didn't managed to populate it we need to 
		  deallocate it from the list*/
		RLIST_Remove(NULL /*hPoolList*/, hListAuthorizationHeaders, hListElement);
        return rv;
    }
    rv = RvSipAuthorizationHeaderCopy(hHeaderCopy, hHeader);
    if (rv != RV_OK)
    {
		RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorAuthorizationHeaderListAddHeader - failed to copy header 0x%p into header 0x%p (rv=%d:%s)",
            hHeader, hHeaderCopy, rv, SipCommonStatus2Str(rv)));
		/*Since we already allocated element in the list and we didn't managed to populate it we need to 
		  deallocate it from the list*/
		RLIST_Remove(NULL /*hPoolList*/, hListAuthorizationHeaders, hListElement);
        return rv;
    }
    
    *((RvSipAuthorizationHeaderHandle*)hListElement) = hHeaderCopy;
    return RV_OK;
}

/***************************************************************************
 * AuthenticatorCreateOtherParams
 * ------------------------------------------------------------------------
 * General: The function gets new cnonce string. this is done by using the
 *          current time in milliSeconds.
 *          note: the output buffer must be allocated before.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr      - Autheticator Manager
 *         pAuthParams   - the structure that holds the parameters from the authentication header
 *         nonceCount    - the number of times we used the current nonce
 * Output: strNonceCount - the nonce count in string format
 *         strCnonce     - the cnonce string
 *         strQopVal     - the qop option in string format
 ***************************************************************************/
static void AuthenticatorCreateOtherParams(
                                        IN  AuthenticatorInfo   *pAuthMgr,
                                        IN  AuthenticatorParams *pAuthParams,
                                        IN  RvInt32              nonceCount,
                                        OUT RvChar              *strNonceCount,
                                        OUT RvChar              *strCnonce,
                                        OUT RvChar             **strQopVal)
{
    /* qop */
    if (pAuthParams->eQopOption != RVSIP_AUTH_QOP_UNDEFINED)
    {
        /* Choose one of QoPs, proposed by Server in Authentication header */
        if (RVSIP_AUTH_QOP_AUTH_AND_AUTHINT == pAuthParams->eQopOption)
        {
            /* If Application supports AUTHINT, choose it as a more strong */
            if (RV_TRUE == AuthenticatorActOnCallback(pAuthMgr, AUTH_CALLBACK_MD5_ENTITY_BODY))
            {
                pAuthParams->eQopOption = RVSIP_AUTH_QOP_AUTHINT_ONLY;
            }
            else
            {
                pAuthParams->eQopOption = RVSIP_AUTH_QOP_AUTH_ONLY;
            }
        }
        *strQopVal = AuthenticatorGetQopStr(pAuthParams->eQopOption);
    }
    /* creating nonceCount in hexadecimal form */
    RvSprintf(strNonceCount, "%08x", nonceCount);

    /* creating cnonce */
    AuthenticatorGetNewCnonce(strCnonce);
}


/******************************************************************************
 * AuthenticatorVerifyCredentials
 * ----------------------------------------------------------------------------
 * General: This function is for a server side authentication.
 *          The user gives the password belongs to username and realm in
 *          authorization header, and wants to know if the authorization header
 *          is correct, for this username.
 *          This function creates the digest string as specified in RFC 2617,
 *          and compares it to the digest string inside the given authorization
 *          header. If it is equal, the header is correct.
 * Return Value: RvStatus -
 *          RV_OK     - the check ended successfully
 *          RV_ERROR_UNKNOWN     - failed during the check.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthorization  - handle to the authorization header.
 *          strMethod       - the method type of the request.
 *          nonceCount      - how many times did we use that nonce
 *          hObject         - handle to the object to be authenticated.
 *          eObjectType     - type of the object to be authenticated.
 *          pObjTripleLock  - lock of the object. Can be NULL. If it is
 *                            supplied, it will be unlocked before callbacks
 *                            to application and locked again after them.
 *          hMsg            - message object, to which hAuthorization belongs.
 * Output:  pbIsCorrect     - boolean, TRUE if correct, False if not.
 *****************************************************************************/
RvStatus RVCALLCONV AuthenticatorVerifyCredentials(
                            IN  RvSipAuthenticatorHandle        hAuth,
                            IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                            IN  RvChar*                         password,
                            IN  RvChar*                         strMethod,
                            IN  void*                           hObject,
                            IN  RvSipCommonStackObjectType      eObjectType,
                            IN  SipTripleLock*                  pObjTripleLock,
                            IN  RvSipMsgHandle                  hMsg,
                            OUT RvBool*                         pbIsCorrect)
{
    RvStatus                      rv;
    HPAGE                         hResponsePage;
    HPAGE                         hHelpPage;
    AuthenticatorInfo            *authInfo;
    AuthenticatorParams           authParams;
    RPOOL_Ptr                     pRpoolResponse;
    RvUint32                      lengthA1;
    RvUint32                      lengthA2;
    RvChar                        strNc[11];
    RvChar                        strCnonce[MAX_CNONCE_LEN];
    RvChar                       *strQopVal = NULL;
    RvInt32                       responseOffset;
    RvUint32                      length;
    RvSipAddressHandle            hRequestUri;
    RvInt32                     origUriOffset = UNDEFINED;


    authInfo = (AuthenticatorInfo*)hAuth;
    lengthA1 = 0;
    lengthA2 = 0;

    /* getting parameters from the authorization header
    (realm, nonce, opaque, username, qop, algorithm )*/
    rv = AuthenticatorGetParamsFromAuthorization(authInfo,
                                                      hAuthorization,
                                                      &hRequestUri,
                                                      &authParams);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting parameters from authentication header"));
        return rv;
    }

    /*password */
    if (0 < strlen(password))
    {
        rv = RPOOL_AppendFromExternalToPage(authParams.hPool,
                                            authParams.hPage,
                                            (void*)password,
                                            (RvInt)strlen(password),
                                            &authParams.passwordOffset);
        if (rv != RV_OK)
        {
            RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                "AuthenticatorVerifyCredentials - failed in set password on params page"));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        authParams.passwordOffset = UNDEFINED;
    }
    rv = AuthenticatorAppendExternalString(authInfo,
                                               authParams.hPool,
                                               authParams.hPage,
                                               "\0",
                                               &length);

    /* getting other parameters that are used for building the authorization header
    (NonceCount, CNonce, strQopVal )*/
    rv = AuthenticatorGetOtherParamsFromAuthorization(&authParams,
                                                         hAuthorization,
                                                         strNc,
                                                         strCnonce,
                                                         &strQopVal);
    if(rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting nc, cnonce and qop from authorization header"));
        return rv;
    }

    /* getting helper page for strings A1, A2 */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hHelpPage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting a page"));
        return rv;
    }
    /* A1 string */
    rv = AuthenticatorCreateA1(authInfo,
                                   &authParams,
                                   authInfo->hGeneralPool,
                                   hHelpPage,
                                   &lengthA1);

    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting the A1 string"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        return rv;

    }
    /* A2 string */
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    origUriOffset = SipAuthorizationHeaderGetOrigUri(hAuthorization);
/*#endif*/
    rv = AuthenticatorCreateA2(authInfo, &authParams, authInfo->hGeneralPool,
            hHelpPage, strMethod, hRequestUri, origUriOffset, hObject, eObjectType,
            pObjTripleLock, hMsg, &lengthA2);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting the A2 string"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        return rv;
    }

    /* getting the page that is used for building the response */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hResponsePage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in getting a page"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        return rv;
    }

    /* response */
    rv = AuthenticatorCreateDigestStr(authInfo,
                                          &authParams,
                                          authInfo->hGeneralPool,
                                          hHelpPage,
                                          authInfo->hGeneralPool,
                                          hResponsePage,
                                          lengthA1,
                                          lengthA2,
                                          strNc,
                                          strCnonce,
                                          strQopVal,
                                          &pRpoolResponse);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorVerifyCredentials - failed in the response string"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
        return rv;
    }

    /* Free the helper page */
    /* Override password, stored on the helper page withing A1 with zeroes.
       This is done in order to prevent password exposure by memory dump. */
    RPOOL_ResetPage(authInfo->hGeneralPool, hHelpPage,
                    0 /*A1 is located at the begining of the helper page*/,
                    lengthA1);
    /* Free the page */
    RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);

    /* Override password, stored on the authParams page with zeroes.
       This is done in order to prevent password exposure by memory dump. */

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(authParams.passwordOffset != UNDEFINED) 
       // we only attempt to reset the password if it exists
#endif
/* SPIRENT_END */
    RPOOL_ResetPage(authParams.hPool, authParams.hPage,
                    authParams.passwordOffset, (RvInt32)strlen(password));

    responseOffset = SipAuthorizationHeaderGetResponse(hAuthorization);

    /* check if the digest string is correct */
    *pbIsCorrect = RPOOL_Strcmp(authParams.hPool,
                              authParams.hPage,
                              responseOffset,
                              pRpoolResponse.hPool,
                              pRpoolResponse.hPage,
                              pRpoolResponse.offset);

    RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
    return RV_OK;

}


/******************************************************************************
 * AuthenticatorPrepareAuthorizationHeader
 * ----------------------------------------------------------------------------
 * General: create the Authorization header according to the credentials in the
 *          Authentication header. this functions also creates the digest string
 *          as specified in RFC 2617.
 *          Note - in case of AKA-MD5, if the auth obj contains the auts parameter,
 *          the response should be computed with an empty password "".
 * Return Value: RvStatus -
 *          RV_OK     - the Authorization header created successfully
 *          RV_ERROR_UNKNOWN     - failed in creating the Authorization header
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthentication - handle to the authentication header.
 *                            if NULL then the global header will be used.
 *          strMethod       - the method type of the request.
 *          hRequestUri     - the requestUri of the request.
 *          nonceCount      - how many times did we use that nonce
 *          hObject         - handle to the Object, that is served
 *                            by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType    - pointer to the variable, that stores type of
 *                            the Object. Use following code to get the type:
 *                            RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                            code control passes to the Application
 *          hMsg            - handle to the message object Authorization header
 *                            for which is being built
 * Output:  hAuthorization  - handle to the authorization header.
 *                            The authorization header must be constructed.
 ***********************************************************************************/
RvStatus RVCALLCONV AuthenticatorPrepareAuthorizationHeader(
                           IN  RvSipAuthenticatorHandle        hAuth,
                           IN  RvSipAuthenticationHeaderHandle hAuthentication,
                           IN  RvChar                          *strMethod,
                           IN  RvSipAddressHandle              hRequestUri,
                           IN  RvInt32                         nonceCount,
                           IN  void*                           hObject,
                           IN  RvSipCommonStackObjectType      eObjectType,
                           IN  SipTripleLock                   *pObjTripleLock,
                           IN  RvSipMsgHandle                  hMsg,
                           OUT RvSipAuthorizationHeaderHandle  hAuthorization)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus             rv;
    HPAGE                hResponsePage;
    HPAGE                hHelpPage;
    AuthenticatorInfo    *authInfo;
    AuthenticatorParams  authParams;
    RPOOL_Ptr            pRpoolResponse;
    RvChar               strNc[9];
    RvChar               strCnonce[10];
    RvChar               *strQopVal = NULL;

    authInfo = (AuthenticatorInfo*)hAuth;
    
    /* initiate the auth-params structure */
    cleanAuthParamsStruct(&authParams);

    /* getting parameters from the authentication header */
    rv = AuthenticatorGetParamsFromAuthentication(authInfo,
                                                  hAuthentication,
                                                  hRequestUri,
                                                  hObject,
                                                  eObjectType,
                                                  pObjTripleLock,
                                                  &authParams);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthorizationHeader - failed in getting parameters from authentication header"));
        return rv;
    }

    /* getting other parameters that are used for building the authorization header */
    AuthenticatorCreateOtherParams(authInfo, &authParams, nonceCount, strNc,
                                   strCnonce, &strQopVal);

    /* getting helper page */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hHelpPage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthorizationHeader - failed in getting a help page"));
        return rv;
    }
    /* getting the page that is used for building the response */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hResponsePage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "AuthenticatorPrepareAuthorizationHeader - failed in getting a page for the response"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        return rv;
    }
    rv = CreateResponseParamForAuthorization (authInfo, &authParams, strMethod, hRequestUri,
                                              strNc, strCnonce, strQopVal,hObject, eObjectType, pObjTripleLock, hMsg, 
                                              hResponsePage, hHelpPage, &pRpoolResponse);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "AuthenticatorPrepareAuthorizationHeader - failed to create the response string"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
        return rv;
    }
    /* free the helper page */
    RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);

    /* setting parameters to the authorization header */
    rv = AuthenticatorSetParamsToAuthorization(authInfo,
                                                   &authParams,
                                                   hRequestUri,
                                                   nonceCount,
                                                   strCnonce,
                                                   strQopVal,
                                                   hAuthorization);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthorizationHeader - failed in setting parameters to authorization header"));
        RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
        return rv;
    }

    rv = SipAuthorizationHeaderSetResponse(hAuthorization,
                                           NULL,
                                           pRpoolResponse.hPool,
                                           pRpoolResponse.hPage,
                                           pRpoolResponse.offset);
    /* free the page of the response */
    RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
        
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthorizationHeader - failed in setting the response"));
        return rv;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth);
    RV_UNUSED_ARG(hAuthentication);
    RV_UNUSED_ARG(strMethod);
    RV_UNUSED_ARG(hRequestUri);
    RV_UNUSED_ARG(nonceCount);
    RV_UNUSED_ARG(hObject);
    RV_UNUSED_ARG(eObjectType);
    RV_UNUSED_ARG(pObjTripleLock);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(hAuthorization);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */

}


/******************************************************************************
 * AuthenticatorPrepareAuthenticationInfoHeader
 * ----------------------------------------------------------------------------
 * General: This function builds an authentication-info header.
 *          1. It generates and set the response-auth string.  
 *             it is being calculated very similarly to the "request-digest" 
 *             in the Authorization header
 *          2. It sets the "message-qop", "cnonce", and "nonce-count" 
 *             parameters if needed.
 *             
 *          The function receives an authorization header (that was received in the
 *          request, and was authorized well), and uses it's parameters in order to
 *          build the authentication-info header.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth       - Handle to the Authenticator object.
 *          hAuthorization - Handle to the authorization header.
 *          nextNonce   - value for the next-nonce parameter. optional.
 *          hObject     - Handle to the application object that will be 
 *                        supplied to the application as a callback parameter
 *                        during authorization header preparing.
 *                        Can be NULL.
 *          hMsg        - Handle to the Message, which will contain the
 *                        prepared Authentication-info header. 
 *          hAuthenticationInfo - Handle to the prepared header.
 *                        Can be NULL.
 *                        If NULL, 'hMsg' parameter should be provided.
 *                        In last case new header will be constructed,
 *                        while using the Message's page.
 ***********************************************************************************/
RvStatus RVCALLCONV AuthenticatorPrepareAuthenticationInfoHeader(
                             IN      RvSipAuthenticatorHandle        hAuth,
                             IN      RvSipAuthorizationHeaderHandle  hAuthorization,
                             IN      void*                           hObject,
                             IN      RvSipCommonStackObjectType      eObjectType,
                             IN      SipTripleLock                   *pObjTripleLock,
                             IN      RvSipMsgHandle                  hMsg,
                             IN OUT  RvSipAuthenticationInfoHeaderHandle  hAuthenticationInfo)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus             rv;
    HPAGE                hResponsePage;
    HPAGE                hHelpPage;
    AuthenticatorInfo    *authInfo;
    AuthenticatorParams  authParams;
    RPOOL_Ptr            pRpoolResponse;
    RvSipAddressHandle   hRequestUri;

    authInfo = (AuthenticatorInfo*)hAuth;
    
    cleanAuthParamsStruct(&authParams);

    /* getting parameters from the authorization header
    (pool, page, username, request-uri, realm, nonce, opaque, qop, algorithm)*/
    rv = AuthenticatorGetParamsFromAuthorization(authInfo,
                                                  hAuthorization,
                                                  &hRequestUri,
                                                  &authParams);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthenticationInfoHeader - failed in getting parameters from authentication header"));
        return rv;
    }

    /* get the relevant password for this user name from application*/
    rv = AuthenticatorGetSecret(authInfo,
                                hRequestUri,
                                hObject,
                                eObjectType,
                                pObjTripleLock,
                                &authParams);

    /* getting helper page */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hHelpPage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthenticationInfoHeader - failed in getting a page"));
        return rv;
    }

    /* getting the page that is used for building the response */
    rv = RPOOL_GetPage(authInfo->hGeneralPool, 0, &hResponsePage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthenticationInfoHeader - failed in getting a page"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        return rv;
    }

    rv = CreateResponseParamForAuthenticationInfo(authInfo, &authParams,hAuthorization, hRequestUri, hObject,
                                                  eObjectType, pObjTripleLock, hMsg, 
                                                  hResponsePage, hHelpPage, &pRpoolResponse);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthenticationInfoHeader - failed to build the response string"));
        RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);
        RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
        return rv;
    }

    /* free the helper page */
    RPOOL_FreePage(authInfo->hGeneralPool, hHelpPage);

    /* setting parameters to the authorization header */
    /*It sets the "message-qop", "cnonce", and "nonce-count" 
 *             parameters if needed.*/
    rv = AuthenticatorSetParamsToAuthenticationInfo(authInfo, &authParams, pRpoolResponse,
                                                   hAuthorization, hAuthenticationInfo);
    /* free the page of the response */
    RPOOL_FreePage(authInfo->hGeneralPool, hResponsePage);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareAuthenticationInfoHeader - failed in setting parameters to the header"));
        return rv;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth);
    RV_UNUSED_ARG(hAuthentication);
    RV_UNUSED_ARG(strMethod);
    RV_UNUSED_ARG(hRequestUri);
    RV_UNUSED_ARG(nonceCount);
    RV_UNUSED_ARG(hObject);
    RV_UNUSED_ARG(eObjectType);
    RV_UNUSED_ARG(pObjTripleLock);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(hAuthorization);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */

}


/* --------- C L I E N T   A U T H   F U N C T I O N S ---------------*/
/******************************************************************************
 * AuthenticatorPrepareProxyAuthorization
 * ----------------------------------------------------------------------------
 * General: prepare the Authorization header according to the credentials
 *          in the global proxy Authentication header.
 *          This functions also creates the digest string as specified in RFC 2617.
 * Return Value: RvStatus -
 *          RV_OK  - the Proxy-Authorization header was prepared successfully
 *          RV_ERROR_UNKNOWN - failed in preparing the Proxy-Authorization header
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          strMethod       - the method type of the request.
 *          hRequestUri     - the requestUri of the request.
 *          nonceCount      - how many times did we use that nonce
 *          hObject         - handle to the Object, that is served
 *                            by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType    - pointer to the variable, that stores type of
 *                            the Object. Use following code to get the type:
 *                            RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                            code control passes to the Application
 *          hMsg            - handle to the message object, for which
 *                            the Proxy-Authorization header is being prepared.
 * Output:  hAuthorization  - handle to the authorization header.
 *                            The authorization header must be constructed.
 *****************************************************************************/
RvStatus RVCALLCONV AuthenticatorPrepareProxyAuthorization(
                            IN RvSipAuthenticatorHandle        hAuth,
                            IN RvChar                          *strMethod,
                            IN RvSipAddressHandle              hRequestUri,
                            IN RvInt32                         nonceCount,
                            IN void*                           hObject,
                            IN RvSipCommonStackObjectType      eObjectType,
                            IN  SipTripleLock                  *pObjTripleLock,
                            IN RvSipMsgHandle                  hMsg,
                            OUT RvSipAuthorizationHeaderHandle hAuthorization)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo *authInfo;
    RvStatus          rv;
    HPAGE             hTmpHeaderPage = NULL; /* Page used by temporary header hTmpHeader. Is allocated from the Authenticator pool and is freed before function returns */
    RvSipAuthenticationHeaderHandle hTmpHeader; /* Copy of global Proxy-Authentication header set in Authenticator */

    authInfo = (AuthenticatorInfo*)hAuth;

    /* In order to be thread safe, create a copy of the global Authentication
    header and use the copy for further processing */
    RvMutexLock(&authInfo->lock,authInfo->pLogMgr);

    /* Copy a header */
    rv = AuthenticatorCreateCopyOfGlobalHeader(authInfo,&hTmpHeaderPage,&hTmpHeader);
    if (RV_OK != rv)
    {
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "AuthenticatorPrepareProxyAuthorization - Object 0x%p failed in AuthenticatorCreateCopyOfGlobalHeader (rv=%d:%s)",
            hObject, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    /* Overwrite a Nonce-Count parameter */
    if (UNDEFINED == nonceCount)
    {
        authInfo->nonceCount++;
        nonceCount = authInfo->nonceCount;
    }
    RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);

    rv = AuthenticatorPrepareAuthorizationHeader(hAuth,
                                                    hTmpHeader,
                                                    strMethod,
                                                    hRequestUri,
                                                    nonceCount,
                                                    hObject,
                                                    eObjectType,
                                                    pObjTripleLock,
                                                    hMsg,
                                                    hAuthorization);
    if (rv != RV_OK)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, hTmpHeaderPage);
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "AuthenticatorPrepareProxyAuthorization - failed to prepare authorization header from global proxy header"));
        return rv;
    }

    RPOOL_FreePage(authInfo->hGeneralPool, hTmpHeaderPage);

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth);
    RV_UNUSED_ARG(strMethod);
    RV_UNUSED_ARG(hRequestUri);
    RV_UNUSED_ARG(nonceCount);
    RV_UNUSED_ARG(hObject);
    RV_UNUSED_ARG(eObjectType);
    RV_UNUSED_ARG(pObjTripleLock);
    RV_UNUSED_ARG(hMsg);
    RV_UNUSED_ARG(hAuthorization);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/******************************************************************************
 * AuthenticatorBuildAuthorizationForAuthObj
 * ----------------------------------------------------------------------------
 * General: The function builds the authorization header that related to the
 *          given authentication object.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr - Handle to the Authentication Manager object.
 *          hMsg     - Handle to the Message, where the headers will be set.
 *          hObject     - handle to the  object, required authentication.
 *          eObjectType - type of the object (CallLeg / RegClient).
 *          pObjTripleLock - object's triple lock.
 *          hAuthList - list of challenges, used for credential
 *                        calculation.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV AuthenticatorBuildAuthorizationForAuthObj(
                                          IN AuthenticatorInfo          *pAuthMgr,
                                          IN AuthObj                    *pAuthObj,
                                          IN RvSipMsgHandle             hMsg,
                                          IN void                       *hObject,
                                          IN RvSipCommonStackObjectType eObjectType,
                                          IN SipTripleLock              *pObjTripleLock,
                                          IN RvChar*                    strMethod,
                                          IN RvSipAddressHandle         hRequestUri)
{
    RvSipAuthorizationHeaderHandle  hAuthorizationHeader;
    RvStatus rv;
    
    /* Build the authorization header (calculate the response digest) */
    /* If the challenge is not valid, ask the Application to do the job*/
    if (RV_FALSE == pAuthObj->bValid)
    {
        AuthenticatorCallbackUnsupportedChallenge(pAuthMgr,
                                                  hObject, eObjectType, pObjTripleLock, 
                                                  pAuthObj->hHeader, hMsg);
        
        return RV_OK;
    }
    
    /* Construct the authorization header in the message */
    rv = RvSipAuthorizationHeaderConstructInMsg(hMsg, RV_FALSE, &hAuthorizationHeader);
    if(rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorBuildAuthorizationForAuthObj - hMsg 0x%p - Failed to construct authorization header in Msg(rv=%d:%s)",
            hMsg, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    /* Update nonceCount */
    ++(pAuthObj->nonceCount);

    AuthenticatorCallbackNonceCountUsage(pAuthMgr, hObject, eObjectType, pObjTripleLock, 
                                         pAuthObj->hHeader,&pAuthObj->nonceCount);
    
    /* Build the credentials */
    rv = AuthenticatorPrepareAuthorizationHeader((RvSipAuthenticatorHandle)pAuthMgr,
                                            pAuthObj->hHeader, strMethod, hRequestUri,
                                            pAuthObj->nonceCount, hObject, eObjectType,
                                            pObjTripleLock, hMsg, hAuthorizationHeader);
    if(rv != RV_OK)
    {
        --(pAuthObj->nonceCount);
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorBuildAuthorizationForAuthObj - hMsg 0x%p, hAuthenticationHeader 0x%p - Failed to prepare authorization header 0x%p (rv=%d:%s)",
            hMsg, pAuthObj->hHeader, hAuthorizationHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
#ifdef RV_SIP_IMS_ON
    if(pAuthObj->autsParamOffset != UNDEFINED)
    {
        /* add the auts parameter to the authorization header */
        rv = SipAuthorizationHeaderSetStrAuts(hAuthorizationHeader, NULL, 
                                              pAuthMgr->hElementPool, 
                                              pAuthObj->hAuthObjPage, 
                                              pAuthObj->autsParamOffset);
        if(rv != RV_OK)
        {
            --(pAuthObj->nonceCount);
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorBuildAuthorizationForAuthObj - hMsg 0x%p, hAuthenticationHeader 0x%p - Failed to insert AUTS param to authorization header 0x%p (rv=%d:%s)",
                hMsg, pAuthObj->hHeader, hAuthorizationHeader, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        
    }
#endif /*#ifdef RV_SIP_IMS_ON*/
    /* header is ready - call the Authorization ready callback */
    AuthenticatorCallbackAuthorizationReady(pAuthMgr, hObject, eObjectType, pObjTripleLock, 
                                            pAuthObj, hAuthorizationHeader);

    return RV_OK;
}

/* --------- A U T H    O B J   L I S T    F U N C T I O N S -----------------*/
/******************************************************************************
 * AuthenticatorGetAuthObjFromList
 * ----------------------------------------------------------------------------
 * General: return a pointer to the next/first AuthObj in the given list.
 * Return Value: pointer to the auth obj.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthList  - Handle to the list to be destructed.
 *          eLocation  - location in the list (first/last/next/prev)
 * Output:  phListElement - pointer to the relative place in the list.
 *****************************************************************************/
AuthObj* AuthenticatorGetAuthObjFromList(IN    RLIST_HANDLE         hAuthList,
                                         IN    RvSipListLocation      eLocation,
                                         INOUT RLIST_ITEM_HANDLE* phListElement)
{
    AuthObj* pAuthObj = NULL;
    RLIST_ITEM_HANDLE hTempListItem;
    switch(eLocation)
    {
    case RVSIP_FIRST_ELEMENT:
        RLIST_GetHead(NULL/*hPoolList*/, hAuthList, phListElement);
        break;
    case RVSIP_NEXT_ELEMENT:
        RLIST_GetNext(NULL/*hPoolList*/, hAuthList, *phListElement, phListElement);
        break;
    case RVSIP_PREV_ELEMENT:
        RLIST_GetPrev(NULL/*hPoolList*/, hAuthList, *phListElement, phListElement);
        break;
    case RVSIP_LAST_ELEMENT:
        RLIST_GetTail(NULL/*hPoolList*/, hAuthList, phListElement);
        break;
    default:
        return NULL;
    }
    
    if (NULL == *phListElement)
    {
        return NULL;
    }
    hTempListItem = *phListElement;
    pAuthObj = *(AuthObj**)hTempListItem;
    return pAuthObj;
}

/***************************************************************************
 * AuthenticatorGetParamsFromAuthentication
 * ------------------------------------------------------------------------
 * General: The function gets parameters from the authentication header.
 * Return Value: RvStatus - RV_OK - the parameters were got successfully
 *                           RV_ERROR_UNKNOWN - failed in getting the parameters
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         hAuthentication - handle to the authentication header
 *         hRequestUri     - handle to the requestUri
 *         hObject         - handle to the Object, that is served
 *                           by the Authenticator (e.g. CallLeg, RegClient)
 *         peObjectType    - pointer to the variable, that stores type of
 *                           the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *         pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                           code control passes to the Application
 * Output: pAuthParams     - the structure that holds the offset and values of the
 *                           authentication parameters.
 ***************************************************************************/
static RvStatus AuthenticatorGetParamsFromAuthentication(
                                 IN AuthenticatorInfo                  *pAuthInfo,
                                 IN RvSipAuthenticationHeaderHandle    hAuthentication,
                                 IN RvSipAddressHandle                 hRequestUri,
                                 IN  void                             *hObject,
                                 IN  RvSipCommonStackObjectType        eObjectType,
                                 IN  SipTripleLock                    *pObjTripleLock,
                                 OUT AuthenticatorParams              *pAuthParams)
{
    RvStatus rv;

    /* getting the pool and page from the authentication header */
    pAuthParams->hPool = SipAuthenticationHeaderGetPool(hAuthentication);
    pAuthParams->hPage = SipAuthenticationHeaderGetPage(hAuthentication);

    /* header type */
    pAuthParams->eAuthHeaderType = RvSipAuthenticationHeaderGetHeaderType(hAuthentication);

    /* realm */
    pAuthParams->realmOffset = SipAuthenticationHeaderGetRealm(hAuthentication);
    if (pAuthParams->realmOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthentication - failed in getting the realm offset"));
        return RV_ERROR_UNKNOWN;
    }
    /*nonce */
    pAuthParams->nonceOffset = SipAuthenticationHeaderGetNonce(hAuthentication);
    if (pAuthParams->nonceOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthentication - failed in getting the nonce offset"));
        return RV_ERROR_UNKNOWN;
    }
    /* opaque */
    pAuthParams->opaqueOffset = SipAuthenticationHeaderGetOpaque(hAuthentication);
    /* username and password */
    rv = AuthenticatorGetSecretFromAuthenticationHeader(
                                    pAuthInfo,
                                    hAuthentication,
                                    hRequestUri,
                                    hObject,
                                    eObjectType,
                                    pObjTripleLock,
                                    pAuthParams);

    if (rv != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthentication - failed in getting the username and password offsets"));
        return RV_ERROR_UNKNOWN;
    }

    if (pAuthParams->userNameOffset == UNDEFINED ||
        pAuthParams->passwordOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthentication - failed in getting user name and password"));
        return RV_ERROR_UNKNOWN;
    }


    /* qop option */
    pAuthParams->eQopOption = RvSipAuthenticationHeaderGetQopOption(hAuthentication);

    /* algorithm */
    pAuthParams->eAuthAlgorithm = RvSipAuthenticationHeaderGetAuthAlgorithm(hAuthentication);
    if (pAuthParams->eAuthAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        pAuthParams->eAuthAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
    }
#ifdef RV_SIP_IMS_ON
    pAuthParams->akaVersion = RvSipAuthenticationHeaderGetAKAVersion(hAuthentication);
#endif

    return RV_OK;
}

/* --------- A U T H    O B J   L I S T    F U N C T I O N S -----------------*/

/******************************************************************************
 * AuthObjCreateInList
 * ----------------------------------------------------------------------------
 * General: allocates AuthObj element in the list.
 *          each object has a different page from the element pool.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr   - Pointer to the Authentication manager object.
 *         pAuthListInfo - list info.
 *         hAuthList - Handle of the List, to which the new
 *                      element should be added. 
 * Output: ppAuthObj  - Pointer to the AuthObj structure, added to the list.
 *****************************************************************************/
RvStatus RVCALLCONV AuthObjCreateInList(
                   IN  AuthenticatorInfo*   pAuthMgr,
                   IN  AuthObjListInfo*     pAuthListInfo,
                   IN  RLIST_HANDLE         hAuthList,
                   OUT AuthObj**            ppAuthObj)
{
    RvStatus rv;
    RLIST_ITEM_HANDLE   hListElement;
    AuthObj* pAuthObj;
    AuthObj** pAuthListObj;
    HPAGE    newObjPage;
    RvInt32  offset;
    
    rv = RLIST_InsertTail(NULL/*hPoolList*/, hAuthList, &hListElement);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthObjCreateInList - failed to add element to list 0x%p (rv=%d:%s)",
            hAuthList, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = RPOOL_GetPage(pAuthMgr->hElementPool, 0, &newObjPage);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthObjCreateInList - failed to allocate new page from element pool 0x%p (rv=%d:%s)",
            pAuthMgr->hElementPool, rv, SipCommonStatus2Str(rv)));
		/*Since we already allocated element in the list and we didn't managed to populate it we need to 
		  deallocate it from the list*/
		RLIST_Remove(NULL, hAuthList, hListElement);
        return rv;
    }

    pAuthObj = (AuthObj*)RPOOL_AlignAppend(pAuthMgr->hElementPool, newObjPage, sizeof(AuthObj), &offset);
    if (NULL == pAuthObj)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthObjCreateInList - failed to allocate AuthObj on page 0x%p (rv=%d:%s)",
            newObjPage, rv, SipCommonStatus2Str(rv)));
        RPOOL_FreePage(pAuthMgr->hElementPool, newObjPage);
		/*Since we already allocated element in the list and we didn't managed to populate it we need to 
		  deallocate it from the list*/
		RLIST_Remove(NULL, hAuthList, hListElement);
        return rv;
    }
    pAuthObj->hAuthObjPage = newObjPage;
    pAuthObj->hListElement = hListElement;
    pAuthObj->pListInfo = pAuthListInfo;
    pAuthObj->pAuthMgr = pAuthMgr;
       
    pAuthListObj = (AuthObj**)hListElement;
    *pAuthListObj = pAuthObj;
    
    RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
        "AuthObjCreateInList - new authObj element 0x%p was allocated (on page 0x%p) in list 0x%p ",
         pAuthObj, newObjPage ,hAuthList));
    
    *ppAuthObj = pAuthObj;
    return RV_OK;
}

/******************************************************************************
 * AuthObjSetParams
 * ----------------------------------------------------------------------------
 * General: Set parameters in the AuthObj element.
 *          
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr   - Pointer to the Authentication manager object.
 *         hList      - The list of auth objects.
 *         pAuthObj   - Pointer to the AuthObj structure, to fill.
 *         hHeader    - Pointer to the authentication header.
 *         strHeader  - String with the authentication header value. (if hHeader is NULL)
 *         bHeaderValid - is the header credentials supported or not.
 *****************************************************************************/
RvStatus RVCALLCONV AuthObjSetParams(
                   IN  AuthenticatorInfo*   pAuthMgr,
                   IN  RLIST_HANDLE         hList,
                   IN  AuthObj*             pAuthObj,
                   IN  RvSipAuthenticationHeaderHandle hHeader,
                   IN  RvChar*              strHeader,
                   IN  RvBool               bHeaderValid)
{
    RvStatus rv;

    rv = AuthObjSetHeader(pAuthMgr, hList, pAuthObj, hHeader, strHeader);
	if (RV_OK != rv)
	{
		RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
					"AuthObjSetParams - Auth obj 0x%p - Failed to set header (rv=%d:%s)",
				   pAuthObj, rv, SipCommonStatus2Str(rv)));
		return rv;
	}

    pAuthObj->bValid = bHeaderValid;
    
    RV_UNUSED_ARG(hList)
    return RV_OK;
}

/******************************************************************************
 * AuthObjSetHeader
 * ----------------------------------------------------------------------------
 * General: Sets all parameters required by an Auth object for an Authentication
 *          header
 *          
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr   - Pointer to the Authentication manager object.
 *         hList      - The list of auth objects.
 *         pAuthObj   - Pointer to the AuthObj structure, to fill.
 *         hHeader    - Pointer to the authentication header.
 *         strHeader  - String with the authentication header value. (if hHeader is NULL)
 *****************************************************************************/
RvStatus RVCALLCONV AuthObjSetHeader(
								IN  AuthenticatorInfo*				pAuthMgr,
								IN  RLIST_HANDLE					hList,
								IN  AuthObj*						pAuthObj,
								IN  RvSipAuthenticationHeaderHandle hHeader,
								IN  RvChar*							strHeader)
{
    RvStatus rv;

    rv = RvSipAuthenticationHeaderConstruct(pAuthMgr->hMsgMgr, 
                                            pAuthMgr->hElementPool, 
                                            pAuthObj->hAuthObjPage,
                                            &pAuthObj->hHeader);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
					"AuthObjSetHeader - failed to construct Authentication header (rv=%d:%s)",
					rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    if(hHeader != NULL)
    {
        rv = RvSipAuthenticationHeaderCopy(pAuthObj->hHeader, hHeader);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
						"AuthObjSetHeader - failed to copy header 0x%p into header 0x%p (rv=%d:%s)",
						hHeader, pAuthObj->hHeader, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    else if(strHeader != NULL)
    {
        rv = RvSipAuthenticationHeaderParse(pAuthObj->hHeader, strHeader);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
						"AuthObjSetHeader - failed to parse given header into header 0x%p (rv=%d:%s)",
						pAuthObj->hHeader, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    else
    {
        RvLogExcep(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
					"AuthObjSetHeader - no header was given"));
        return RV_ERROR_UNKNOWN;
    }
	pAuthObj->nonceCount      = 0;
    pAuthObj->pContext        = NULL;
#ifdef RV_SIP_IMS_ON
	pAuthObj->autsParamOffset = UNDEFINED;
#endif /*RV_SIP_IMS_ON*/

    RV_UNUSED_ARG(hList)

    return RV_OK;
}

/******************************************************************************
 * AuthObjDestructAndRemoveFromList
 * ----------------------------------------------------------------------------
 * General: remove AuthObj element from the list, and free it's page.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr  - Pointer to the Authentication manager object.
 *         hAuthList - Handle of the List, holding the object. 
 *         pAuthObj  - Pointer to the AuthObj structure to be remove.
 *****************************************************************************/
void RVCALLCONV AuthObjDestructAndRemoveFromList(
                   IN  AuthenticatorInfo*   pAuthMgr,
                   IN  RLIST_HANDLE         hAuthList,
                   IN  AuthObj*             pAuthObj)
{
    HPAGE    hObjPage;
    
    hObjPage = pAuthObj->hAuthObjPage;
    
    RLIST_Remove(NULL/*hPoolList*/, hAuthList, pAuthObj->hListElement);
    RPOOL_FreePage(pAuthMgr->hElementPool, hObjPage);
    
    RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthObjDestructAndRemoveFromList - object 0x%p (page 0x%p) on list 0x%p was removed",
            pAuthObj, hObjPage, hAuthList));
    
}

/******************************************************************************
 * AuthObjLockAPI
 * ----------------------------------------------------------------------------
 * General: lock the auth obj.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthObj  - Pointer to the AuthObj structure to be remove.
 *****************************************************************************/
RvStatus RVCALLCONV AuthObjLockAPI(
                   IN  AuthObj*             pAuthObj)
{
    AuthObjListInfo* pAuthListInfo = pAuthObj->pListInfo;
    
    if (NULL == pAuthListInfo->pfnLockAPI)
    {
        return RV_OK;
    }

    RvLogSync(pAuthObj->pAuthMgr->pLogSrc,(pAuthObj->pAuthMgr->pLogSrc,
            "AuthObjLockAPI - object 0x%p  - owner 0x%p - LOCK",
            pAuthObj, pAuthListInfo->pParentObj));
    return pAuthListInfo->pfnLockAPI(pAuthListInfo->pParentObj);
}

/******************************************************************************
 * AuthObjUnLockAPI
 * ----------------------------------------------------------------------------
 * General: unlock the auth obj.
 * Return Value:
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthObj  - Pointer to the AuthObj structure to be remove.
 *****************************************************************************/
void RVCALLCONV AuthObjUnLockAPI(
                   IN  AuthObj*             pAuthObj)
{
    AuthObjListInfo* pAuthListInfo = pAuthObj->pListInfo;

    if (NULL != pAuthListInfo->pfnUnLockAPI)
    {
        RvLogSync(pAuthObj->pAuthMgr->pLogSrc,(pAuthObj->pAuthMgr->pLogSrc,
            "AuthObjUnLockAPI - object 0x%p  - owner 0x%p - UNLOCK",
            pAuthObj, pAuthListInfo->pParentObj));
        pAuthListInfo->pfnUnLockAPI(pAuthListInfo->pParentObj);
    }
}

/* ----- A U T H    O B J   A C T  O N  C A L L B A C K    F U N C T I O N S ----*/

/******************************************************************************
 * AuthenticatorActOnCallback
 * ----------------------------------------------------------------------------
 * General: Indicates whether the specified callback is set to the authenticator,
 *          and whether it can be relied on by the authenticator.
 * Return Value: Boolean indication to act on callback.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr      - Pointer to the Authentication manager object.
 *         eCallbackType - Enumeration type of the requested callback
 *****************************************************************************/
RvBool RVCALLCONV AuthenticatorActOnCallback(
						IN  AuthenticatorInfo*   pAuthMgr,
						IN  AuthCallbacks        eCallbackType)
{
	switch (eCallbackType)
	{
	case AUTH_CALLBACK_GET_SHARED_SECRET:
		if (pAuthMgr->evHandlers.pfnGetSharedSecretAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bGetSharedSecretAuthentication == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		if (pAuthMgr->evHandlers.pfnSharedSecretAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bSharedSecretAuthentication == RV_TRUE)
		{
			/* The old callback is used. The callback was set and also the act on 
			   callback indication is set to true */
			return RV_TRUE;
		}
		break;
	case AUTH_CALLBACK_MD5:
		if (pAuthMgr->evHandlers.pfnMD5AuthenticationExHandler != NULL &&
			pAuthMgr->actOnCallback.bMD5AuthenticationEx == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		if (pAuthMgr->evHandlers.pfnMD5AuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bMD5Authentication == RV_TRUE)
		{
			/* The old callback is used. The callback was set and also the act on 
			   callback indication is set to true */
			return RV_TRUE;
		}
		break;
	case AUTH_CALLBACK_MD5_ENTITY_BODY:
		if (pAuthMgr->evHandlers.pfnMD5EntityBodyAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bMD5EntityBodyAuthentication == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		break;
	case AUTH_CALLBACK_UNSUPPORTED_CHALLENGE:
		if (pAuthMgr->evHandlers.pfnUnsupportedChallengeAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bUnsupportedChallengeAuthentication == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		break;
	case AUTH_CALLBACK_NONCE_COUNT_USAGE:
		if (pAuthMgr->evHandlers.pfnNonceCountUsageAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bNonceCountUsageAuthentication == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		break;
	case AUTH_CALLBACK_AUTHORIZATION_READY:
		if (pAuthMgr->evHandlers.pfnAuthorizationReadyAuthenticationHandler != NULL &&
			pAuthMgr->actOnCallback.bAuthorizationReadyAuthentication == RV_TRUE)
		{
			/* The callback was set and also the act on callback indication is set to true */
			return RV_TRUE;
		}
		break;
	default:
		break;
	}

	return RV_FALSE;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * AuthenticatorAppendString
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end of it.
 *          The given string is from another rpool page (pool+page+offset).
 *          The setting os done by asking for memory on the dest page (RPOOL_Append)
 *          and setting the string (with RPOOL_CopyFrom).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo   - the authenticator info structure
 *         destPool    - Handle of the destination pool
 *         destPage    - Handle of the destination page
 *         hPool       - Handle of the pool of the given string.
 *         hPage       - Handle of the pool page of the given string.
 *           stringOffet - The offset of the string to encode.
 * Output: length      - The length that was given + the new encoded string length.
 ***************************************************************************/
static RvStatus AuthenticatorAppendString(IN  AuthenticatorInfo  *pAuthInfo,
                                           IN  HRPOOL             destPool,
                                           IN  HPAGE              destPage,
                                           IN  HRPOOL             hPool,
                                           IN  HPAGE              hPage,
                                           IN  RvInt32           stringOffset,
                                           OUT RvUint32*         length)
{
    RvInt32         offset;
    RvStatus         stat;
    RvInt32          strLen;

    strLen = RPOOL_Strlen(hPool, hPage, stringOffset);

    *length += strLen;

    stat = RPOOL_Append(destPool, destPage, strLen, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return stat;
    }

    stat = RPOOL_CopyFrom(destPool,
                          destPage,
                          offset,
                          hPool,
                          hPage,
                          stringOffset,
                          strLen);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, destPool 0x%p, destPage 0x%p, RvStatus: %d",
            hPool, hPage, destPool, destPage, stat));
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pAuthInfo);
#endif
    return stat;
}


/***************************************************************************
 * AuthenticatorAppendQuotedString
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end of it.
 *          The given string is from another rpool page (pool+page+offset).
 *          The setting os done by asking for memory on the dest page (RPOOL_Append)
 *          and setting the string (with RPOOL_CopyFrom).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length. The string is being appended without the quatation
 *          marks.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo   - the authenticator info structure
 *         destPool    - Handle of the destination pool
 *         destPage    - Handle of the destination page
 *         hPool       - Handle of the pool of the given string.
 *         hPage       - Handle of the pool page of the given string.
 *           stringOffet - The offset of the string to encode without the first quotation mark.
 * Output: length      - The length that was given + the new encoded string length without the
 *                       quotation marks.
 ***************************************************************************/
static RvStatus AuthenticatorAppendQuotedString(IN  AuthenticatorInfo  *pAuthInfo,
                                                 IN  HRPOOL             destPool,
                                                 IN  HPAGE              destPage,
                                                 IN  HRPOOL             hPool,
                                                 IN  HPAGE              hPage,
                                                 IN  RvInt32           stringOffset,
                                                 OUT RvUint32*         length)
{
    RvInt32         offset;
    RvStatus         stat;
    RvInt32          strLen;
    RvInt32          unquotedStrLen;

    strLen = RPOOL_Strlen(hPool, hPage, stringOffset);

    /* calculating the length of the string without the quotation marks */
    unquotedStrLen = strLen - 2;
    if (unquotedStrLen < 0)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendQuotedString - Failed: string of length less than 2. hPool %p, hPage %p",
            destPool, destPage));
        return RV_ERROR_BADPARAM;
    }

    *length += unquotedStrLen;

    stat = RPOOL_Append(destPool, destPage, unquotedStrLen, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendQuotedString - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            destPool, destPage, stat));
        return stat;
    }

    stat = RPOOL_CopyFrom(destPool,
                          destPage,
                          offset,
                          hPool,
                          hPage,
                          stringOffset + 1,
                          unquotedStrLen);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendQuotedString - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, destPool 0x%p, destPage 0x%p, RvStatus: %d",
            hPool, hPage, destPool, destPage, stat));
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pAuthInfo);
#endif
    return stat;
}

/***************************************************************************
 * AuthenticatorAppendExternalString
 * ------------------------------------------------------------------------
 * General: The function set a string in the rpool page, by asking for memory
 *          and setting the string (with RPOOL_AppendFromExternal).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pAuthInfo - the structure that holds the authenticator info
 *        hPage     - Handle of the rpool page.
 *          string    - the string to set.
 * Output: encoded  - Pointer to the beginning of the encoded string in the page.
 *         length   - The length that was given + the new encoded string length.
 ***************************************************************************/
static RvStatus AuthenticatorAppendExternalString(IN  AuthenticatorInfo  *pAuthInfo,
                                                  IN  HRPOOL             hPool,
                                                  IN  HPAGE              hPage,
                                                  IN  RvChar*            string,
                                                  OUT RvUint32*          length)
{
    RvInt32         offset;
    RvInt32          size;
    RvStatus         stat;
    RPOOL_ITEM_HANDLE encoded;

    if(string == NULL)
    {
        return RV_OK;
    }

    if (string[0] == '\0')
    {
        size = 1;
        ++(*length);
    }
    else
    {
        *length += (RvUint32)strlen(string);
        size = (RvInt32)strlen(string);
    }

    stat = RPOOL_AppendFromExternal(hPool,
                                    hPage,
                                    (void*)string,
                                    size,
                                    RV_FALSE,
                                    &offset,
                                    &encoded);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorAppendExternalString - Failed, AppendFromExternal failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            hPool, hPage, stat));
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pAuthInfo);
#endif
    return stat;
}

/***************************************************************************
 * AuthenticatorGetStrRequestUri
 * ------------------------------------------------------------------------
 * General: The function gets request uri string from the RvSipAddressHandle,
 *          after encoding it.
 * Return Value: RvStatus - RV_OK or RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         hPool           - handle to the pool, to which the page belongs
 *         hPage           - handle to the memory page that is used for building the response
 *         hRequestUri     - handle to the address header
 *         bAppendZero     - if RV_TRUE, '\0' will be added at the URI end
 *         length          - length of the string, to which URI should be
 *                           concatenated
 * Output: length          - length of the string with concatenated URI
 ***************************************************************************/
static RvStatus AuthenticatorGetStrRequestUri(
                                 IN     AuthenticatorInfo   *pAuthInfo,
                                 IN     HRPOOL              hPool,
                                 IN     HPAGE               hPage,
                                 IN     RvSipAddressHandle  hRequestUri,
                                 IN     RvInt32             origUriOffset,
                                 IN     RvBool              bAppendZero,
                                 IN OUT RvUint32            *length)
{
    RvStatus rv;
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
    if(origUriOffset > UNDEFINED)
    {   
        RvInt      strLen;
        RvInt32    offset;
        
        strLen = RPOOL_Strlen(SipAddrGetPool(hRequestUri), SipAddrGetPage(hRequestUri), origUriOffset);
        rv = RPOOL_Append(hPool, hPage, strLen, RV_FALSE, &offset);
        if(rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorGetStrRequestUri - Failed, RPOOL_Append failed. hPool 0x%p, hPage %d, RvStatus: %d",
                hPool, hPage, rv));
            return UNDEFINED;
        }
        rv = RPOOL_CopyFrom(hPool, hPage, offset,SipAddrGetPool(hRequestUri),
                            SipAddrGetPage(hRequestUri),origUriOffset,strLen);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorGetStrRequestUri - failed in copy orig uri"));    
        }
        *length += strLen;
    }
    else
    {
        rv = SipAddrEncode(hRequestUri, hPool, hPage, RV_FALSE, length);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                      "AuthenticatorGetStrRequestUri - failed in getting or copying the request uri"));
            return rv;

        }
    }

    if (RV_TRUE==bAppendZero)
    {
        rv = AuthenticatorAppendExternalString(
            pAuthInfo, hPool, hPage, "\0", length);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                "AuthenticatorGetStrRequestUri - failed to append '\0'"));
            return rv;
        }
    }
	
    return RV_OK;
}

/***************************************************************************
 * AuthenticatorGetParamsFromAuthentication
 * ------------------------------------------------------------------------
 * General: The function gets parameters from the authentication header.
 * Return Value: RvStatus - RV_OK - the parameters were got successfully
 *                           RV_ERROR_UNKNOWN - failed in getting the parameters
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         hAuthentication - handle to the authentication header
 *         hRequestUri     - handle to the requestUri
 * Output: pAuthParams     - the structure that holds the offset and values of the
 *                           authentication parameters.
 ***************************************************************************/
static RvStatus AuthenticatorGetParamsFromAuthorization(
								 IN AuthenticatorInfo                  *pAuthInfo,
                                 IN RvSipAuthorizationHeaderHandle     hAuthorization,
                                 OUT RvSipAddressHandle                *hRequestUri,
                                 OUT AuthenticatorParams               *pAuthParams)
{
    RvInt32 tempOffset;
    RvInt32 tempLen;
    RvStatus stat;

    /* getting the pool and page from the authentication header */
    pAuthParams->hPool = SipAuthorizationHeaderGetPool(hAuthorization);
    pAuthParams->hPage = SipAuthorizationHeaderGetPage(hAuthorization);

    *hRequestUri = RvSipAuthorizationHeaderGetDigestUri(hAuthorization);
    /* username - need to be without "" */
    tempOffset = SipAuthorizationHeaderGetUserName(hAuthorization);
    if (tempOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in getting user name"));
        return RV_ERROR_UNKNOWN;
    }
    tempLen = RPOOL_Strlen(pAuthParams->hPool, pAuthParams->hPage, tempOffset);
    /* append space on the header page */
    stat = RPOOL_Append(pAuthParams->hPool,
                        pAuthParams->hPage,
                        tempLen-1, /*-1, because we want to omit 2 '"' and to add one NULL */
                        RV_FALSE,
                        &(pAuthParams->userNameOffset));
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in appending mem for user name"));
        return stat;
    }

    /* copy the string without ""*/
    stat = RPOOL_CopyFrom(pAuthParams->hPool,
                          pAuthParams->hPage,
                          pAuthParams->userNameOffset,
                          pAuthParams->hPool,
                          pAuthParams->hPage,
                          tempOffset+1, /* +1 to skip the first " */
                          tempLen-2); /* -2 because there are 2 " chars less */
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in copy user name without \"\""));
        return stat;
    }
    /* set NULL at the end of the username string */
    stat = RPOOL_CopyFromExternal(pAuthParams->hPool,
                          pAuthParams->hPage,
                          pAuthParams->userNameOffset + tempLen-2,
                          "", 1);
    if(stat != RV_OK)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in set NULL at end of user name"));
        return stat;
    }

    /* set more parameters */

    /* realm */
    pAuthParams->realmOffset = SipAuthorizationHeaderGetRealm(hAuthorization);
    if (pAuthParams->realmOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in getting the realm offset"));
        return RV_ERROR_UNKNOWN;
    }
    /*nonce */
    pAuthParams->nonceOffset = SipAuthorizationHeaderGetNonce(hAuthorization);
    if (pAuthParams->nonceOffset == UNDEFINED)
    {
        RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                  "AuthenticatorGetParamsFromAuthorization - failed in getting the nonce offset"));
        return RV_ERROR_UNKNOWN;
    }
    /* opaque */
    pAuthParams->opaqueOffset = SipAuthorizationHeaderGetOpaque(hAuthorization);


    /* qop option */
    pAuthParams->eQopOption = RvSipAuthorizationHeaderGetQopOption(hAuthorization);

    /* algorithm */
    pAuthParams->eAuthAlgorithm = RvSipAuthorizationHeaderGetAuthAlgorithm(hAuthorization);
    if (pAuthParams->eAuthAlgorithm == RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        pAuthParams->eAuthAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
    }
#ifdef RV_SIP_IMS_ON
    pAuthParams->akaVersion = RvSipAuthorizationHeaderGetAKAVersion(hAuthorization);
#endif
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pAuthInfo);
#endif

    return RV_OK;
}

/***************************************************************************
 * AuthenticatorGetOtherParamsFromAuthorization
 * ------------------------------------------------------------------------
 * General: The function gets new cnonce string. this is done by using the
 *          current time in milliSeconds.
 *          note: the output buffer must be allocated before.
 *                we assume that strCnonce size is 12.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthParams   - the structure that holds the parameters from the authentication header
 * Output: strNonceCount - the nonce count in string format
 *         strCnonce     - the cnonce string
 *         strQopVal     - the qop option in string format
 ***************************************************************************/
static RvStatus AuthenticatorGetOtherParamsFromAuthorization(
                                   IN AuthenticatorParams               *pAuthParams,
                                   IN RvSipAuthorizationHeaderHandle     hAuthorization,
                                   OUT RvChar                          *strNonceCount,
                                   OUT RvChar                          *strCnonce,
                                   OUT RvChar                          **strQopVal)
{
    RvInt32    nonceCount;
    RvUint     len;
    RvChar     tempBuff[MAX_CNONCE_LEN+2]; /* remote server might send a long cnonce.
                                          we can't limit it to 10 characters. (50 might be
                                          too short as well) */
    RvStatus   rv;

    /* nonceCount */
    nonceCount = RvSipAuthorizationHeaderGetNonceCount(hAuthorization);
    /* should I use the %08x? or should I use %x? */
    if(nonceCount == UNDEFINED)
    {
        strNonceCount = NULL;
    }
    else
    {
        RvSprintf(strNonceCount, "%08x", nonceCount);
    }

    /* cnonce */
    rv = RvSipAuthorizationHeaderGetCNonce(hAuthorization,
                                         tempBuff,
                                         MAX_CNONCE_LEN+2,
                                         &len);
    if(rv == RV_OK)
    {
        /* copy cnonce without "" */
        strcpy(strCnonce, tempBuff+1);
        strCnonce[len-3] = '\0';
    }
    else if(rv == RV_ERROR_NOT_FOUND)
    {
        strCnonce = NULL;
    }
    else
    {
        return rv;
    }

    /* qop */
    pAuthParams->eQopOption = RvSipAuthorizationHeaderGetQopOption(hAuthorization);
    if (pAuthParams->eQopOption != RVSIP_AUTH_QOP_UNDEFINED)
    {
        *strQopVal = AuthenticatorGetQopStr(pAuthParams->eQopOption);
    }
    return RV_OK;
}

/***************************************************************************
 * AuthenticatorGetNewCnonce
 * ------------------------------------------------------------------------
 * General: The function gets new cnonce string. this is done by using the
 *          current time in milliSeconds.
 *          note: the output buffer must be allocated before.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCNonce - the buffer to be filled with the new cnonce.
 * Output: none
 ***************************************************************************/
static void AuthenticatorGetNewCnonce(OUT RvChar *pCNonce)
{
    RvChar tempCnonce[10];
    RvUint32 timeInMsec = SipCommonCoreRvTimestampGet(IN_MSEC);

    RvSprintf(tempCnonce, "%x", timeInMsec);
    strcpy(pCNonce, tempCnonce);
}

/***************************************************************************
 * AuthenticatorGetSecret
 * ------------------------------------------------------------------------
 * General: The function gets username and password from the application.
 *
 * Return Value: RvStatus - RV_OK - the secret was got successfully
 *                           RV_ERROR_UNKNOWN - failed in getting the secret
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         hRequestUri     - handle to the request uri
 *         hObject         - handle to the Object, that is served
 *                           by the Authenticator (e.g. CallLeg, RegClient)
 *         peObjectType    - pointer to the variable, that stores type of
 *                           the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *         pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                           code control passes to the Application
 * Output: pAuthParams     - the structure that holds the parameters from the authentication header
 ***************************************************************************/
static RvStatus AuthenticatorGetSecret(IN    AuthenticatorInfo               *pAuthInfo,
                                       IN    RvSipAddressHandle               hRequestUri,
                                       IN    void*                            hObject,
                                       IN    RvSipCommonStackObjectType       eObjectType,
                                       IN    SipTripleLock                   *pObjTripleLock,
                                       INOUT AuthenticatorParams             *pAuthParams)
{
    RvStatus  rv;
    RPOOL_Ptr pRpoolPtrUserName;
    RPOOL_Ptr pRpoolPtrPassword;
    RPOOL_Ptr pRpoolRealm;

    /* getting username and password from  application */
    pRpoolPtrUserName.hPool     = pAuthParams->hPool;
    pRpoolPtrUserName.hPage     = pAuthParams->hPage;
    if(pAuthParams->userNameOffset > UNDEFINED)
    {
        /* when building the response for authentication-info header, 
           er already have the user-name from the Authorization header */
        pRpoolPtrUserName.offset = pAuthParams->userNameOffset;
    }
    else
    {
        pRpoolPtrUserName.offset    = 0;
    }
    
    pRpoolPtrPassword.hPool  = pAuthParams->hPool;
    pRpoolPtrPassword.hPage  = pAuthParams->hPage;
    pRpoolPtrPassword.offset = 0;

    pRpoolRealm.hPool = pAuthParams->hPool;
    pRpoolRealm.hPage = pAuthParams->hPage;
    pRpoolRealm.offset = pAuthParams->realmOffset;

    rv = AuthenticatorCallbackGetSharedSecret(pAuthInfo, hObject, eObjectType,
        pObjTripleLock, hRequestUri, &pRpoolPtrUserName,&pRpoolPtrPassword,&pRpoolRealm);

    if (RV_ERROR_NOT_FOUND == rv)
    {
        return RV_OK;
    }
    pAuthParams->userNameOffset = pRpoolPtrUserName.offset;
    pAuthParams->passwordOffset = pRpoolPtrPassword.offset;
   
    return RV_OK;
}

/***************************************************************************
 * AuthenticatorGetSecretFromAuthenticationHeader
 * ------------------------------------------------------------------------
 * General: The function gets user-name and password from the authentication header
 *          or from the application.
 * Return Value: RvStatus - RV_OK - the secret was got successfully
 *                           RV_ERROR_UNKNOWN - failed in getting the secret
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo       - the authentication info structure
 *         hAuthentication - handle to the authentication header
 *         hRequestUri     - handle to the request uri
 *         hObject         - handle to the Object, that is served
 *                           by the Authenticator (e.g. CallLeg, RegClient)
 *         peObjectType    - pointer to the variable, that stores type of
 *                           the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *         pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                           code control passes to the Application
 * Output: pAuthParams     - the structure that holds the parameters from the authentication header
 ***************************************************************************/
static RvStatus AuthenticatorGetSecretFromAuthenticationHeader(
                                       IN    AuthenticatorInfo               *pAuthInfo,
                                       IN    RvSipAuthenticationHeaderHandle  hAuthentication,
                                       IN    RvSipAddressHandle               hRequestUri,
                                       IN    void*                            hObject,
                                       IN    RvSipCommonStackObjectType       eObjectType,
                                       IN    SipTripleLock                   *pObjTripleLock,
                                       INOUT AuthenticatorParams             *pAuthParams)
{
    RvStatus  rv;
    
    /* getting username and password from the authentication header or
       from the application */
    pAuthParams->userNameOffset = SipAuthenticationHeaderGetUserName(hAuthentication);

    pAuthParams->passwordOffset = SipAuthenticationHeaderGetPassword(hAuthentication);

    /* notify the application that there is a need for username and password */   
    if (pAuthParams->userNameOffset == UNDEFINED ||
        pAuthParams->passwordOffset == UNDEFINED)
    {
        rv = AuthenticatorGetSecret(pAuthInfo, hRequestUri, hObject, eObjectType, pObjTripleLock, pAuthParams);
        if (RV_ERROR_NOT_FOUND == rv)
        {
            return RV_OK;
        }

        /* setting the username and password from application to the authentication header */
        rv = SipAuthenticationHeaderSetUserName(hAuthentication,
                                                    NULL,
                                                    pAuthParams->hPool,
                                                    pAuthParams->hPage,
                                                    pAuthParams->userNameOffset);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                       "AuthenticatorGetSecretFromAuthenticationHeader - failed in setting the username"));
            return rv;
        }
        rv = SipAuthenticationHeaderSetPassword(hAuthentication,
                                                    NULL,
                                                    pAuthParams->hPool,
                                                    pAuthParams->hPage,
                                                    pAuthParams->passwordOffset);
        if (rv != RV_OK)
        {
            RvLogError(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
                       "AuthenticatorGetSecretFromAuthenticationHeader - failed in setting the password"));
            return rv;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * AuthenticatorCreateCopyOfGlobalHeader
 * ------------------------------------------------------------------------
 * General: The function allocates page, constructs the Authentication header,
 *          and copies into it the Global Authentication Header.
 *          Note: the function caller should free page after the copied header
 *                was used.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthParams   - the structure that holds the parameters from the authentication header
 * Output: strNonceCount - the nonce count in string format
 *         strCnonce     - the cnonce string
 *         strQopVal     - the qop option in string format
 ***************************************************************************/
static RvStatus AuthenticatorCreateCopyOfGlobalHeader(
                            IN  AuthenticatorInfo               *pAuthMgr,
                            OUT HPAGE                           *phTmpHeaderPage,
                            OUT RvSipAuthenticationHeaderHandle *phTmpHeader)
{
    RvStatus rv;

    *phTmpHeader = NULL;
    RPOOL_GetPage(pAuthMgr->hGeneralPool, 0, phTmpHeaderPage);
    if (NULL == *phTmpHeaderPage)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCreateCopyOfGlobalHeader - failed to allocate new page. Pool 0x%p",
            pAuthMgr->hGeneralPool));
        return RV_ERROR_UNKNOWN;
    }
    rv = RvSipAuthenticationHeaderConstruct(pAuthMgr->hMsgMgr,
                        pAuthMgr->hGeneralPool, *phTmpHeaderPage, phTmpHeader);
    if (rv != RV_OK)
    {
        RPOOL_FreePage(pAuthMgr->hGeneralPool, *phTmpHeaderPage);
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCreateCopyOfGlobalHeader - failed to construct new header (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = RvSipAuthenticationHeaderCopy(*phTmpHeader ,pAuthMgr->hGlobalAuth);
    if (RV_OK != rv)
    {
        RPOOL_FreePage(pAuthMgr->hGeneralPool, *phTmpHeaderPage);
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCreateCopyOfGlobalHeader - failed to copy header (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
}
/******************************************************************************
 * CreateResponseParamForAuthorization
 * ----------------------------------------------------------------------------
 * General: create the response parameter value for an Authorization header. 
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthentication - handle to the authentication header.
 *                            if NULL then the global header will be used.
 *          strMethod       - the method type of the request.
 *          hRequestUri     - the requestUri of the request.
 *          nonceCount      - how many times did we use that nonce
 *          hObject         - handle to the Object, that is served
 *                            by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType    - pointer to the variable, that stores type of
 *                            the Object. Use following code to get the type:
 *                            RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                            code control passes to the Application
 *          hMsg            - handle to the message object Authorization header
 *                            for which is being built
 * Output:  hAuthorization  - handle to the authorization header.
 *                            The authorization header must be constructed.
 ***********************************************************************************/
static RvStatus CreateResponseParamForAuthorization(IN  AuthenticatorInfo    *authInfo,
                                                    IN  AuthenticatorParams  *pAuthParams,
                                                    IN  RvChar               *strMethod,
                                                    IN  RvSipAddressHandle   hRequestUri,
                                                    IN  RvChar               *strNc,
                                                    IN  RvChar               *strCnonce,
                                                    IN  RvChar               *strQopVal,
                                                    IN  void*                hObject,
                                                    IN  RvSipCommonStackObjectType eObjectType,
                                                    IN  SipTripleLock        *pObjTripleLock,
                                                    IN  RvSipMsgHandle       hMsg,
                                                    IN  HPAGE                hResponsePage,
                                                    IN  HPAGE                hHelpPage,
                                                    OUT RPOOL_Ptr            *pRpoolResponse)
{
    RvStatus             rv;
    RvUint32             lengthA1;
    RvUint32             lengthA2;
    
    lengthA1 = 0;
    lengthA2 = 0;

    /* A1 string */
    rv = AuthenticatorCreateA1(authInfo, pAuthParams, authInfo->hGeneralPool, hHelpPage, &lengthA1);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthorization - failed in getting the A1 string"));
        return rv;
    }
    /* A2 string */
    rv = AuthenticatorCreateA2(authInfo, pAuthParams, authInfo->hGeneralPool,
                               hHelpPage, strMethod, hRequestUri, UNDEFINED, hObject, eObjectType,
                               pObjTripleLock, hMsg, &lengthA2);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthorization - failed in getting the A2 string"));
        return rv;
    }
    
    /* response */
    rv = AuthenticatorCreateDigestStr(authInfo, pAuthParams, authInfo->hGeneralPool, hHelpPage,
                                      authInfo->hGeneralPool, hResponsePage,
                                      lengthA1, lengthA2, strNc, strCnonce, strQopVal, pRpoolResponse);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthorization - failed in the response string"));
        return rv;
    }
    return RV_OK;
}

/******************************************************************************
 * CreateResponseParamForAuthenticationInfo
 * ----------------------------------------------------------------------------
 * General: create the response parameter value for an Authorization header. 
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthentication - handle to the authentication header.
 *                            if NULL then the global header will be used.
 *          strMethod       - the method type of the request.
 *          hRequestUri     - the requestUri of the request.
 *          nonceCount      - how many times did we use that nonce
 *          hObject         - handle to the Object, that is served
 *                            by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType    - pointer to the variable, that stores type of
 *                            the Object. Use following code to get the type:
 *                            RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock  - Triple Lock of the object. It has to be unlocked before
 *                            code control passes to the Application
 *          hMsg            - handle to the message object Authorization header
 *                            for which is being built
 * Output:  hAuthorization  - handle to the authorization header.
 *                            The authorization header must be constructed.
 ***********************************************************************************/
static RvStatus CreateResponseParamForAuthenticationInfo(IN  AuthenticatorInfo    *authInfo,
                                                    IN  AuthenticatorParams  *pAuthParams,
                                                    IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                                    IN  RvSipAddressHandle   hRequestUri,
                                                    IN  void*                hObject,
                                                    IN  RvSipCommonStackObjectType eObjectType,
                                                    IN  SipTripleLock        *pObjTripleLock,
                                                    IN  RvSipMsgHandle       hMsg,
                                                    IN  HPAGE                hResponsePage,
                                                    IN  HPAGE                hHelpPage,
                                                    OUT RPOOL_Ptr            *pRpoolResponse)
{
    RvStatus        rv;
    RvUint32        lengthA1;
    RvUint32        lengthA2;
    RvChar          strCnonce[10];
    RvChar          strNc[10];
    RvChar         *strQopVal = NULL;
    
    lengthA1 = 0;
    lengthA2 = 0;

    rv = AuthenticatorGetOtherParamsFromAuthorization(pAuthParams, hAuthorization, 
                                                       strNc, strCnonce, &strQopVal);

    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthenticationInfo - failed to get other params from authorization"));
        return rv;
    }

    /* A1 string */
    rv = AuthenticatorCreateA1(authInfo, pAuthParams, authInfo->hGeneralPool, hHelpPage, &lengthA1);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthenticationInfo - failed in getting the A1 string"));
        return rv;
    }
    /* A2 string */
    rv = AuthenticatorCreateA2ForAuthInfo(authInfo, pAuthParams, authInfo->hGeneralPool,
                                       hHelpPage, hRequestUri, hObject, eObjectType,
                                       pObjTripleLock, hMsg, &lengthA2);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthenticationInfo - failed in getting the A2 string"));
        return rv;
    }
    
    /* response */
    rv = AuthenticatorCreateDigestStr(authInfo, pAuthParams, authInfo->hGeneralPool, hHelpPage,
                                      authInfo->hGeneralPool, hResponsePage,
                                      lengthA1, lengthA2, strNc, strCnonce, strQopVal, pRpoolResponse);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "CreateResponseParamForAuthenticationInfo - failed in the response string"));
        return rv;
    }
    return RV_OK;
}

/******************************************************************************
 * AuthenticatorSetParamsToAuthenticationInfo
 * ----------------------------------------------------------------------------
 * General: Takes the needed parameters from the authorization header, and set 
 *          it to the authentication-info header.
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   authInfo        - pointer to the authentication module.
 *          pAuthParams     - the params we already got from the authorization header.
 *          hAuthorization  - the authorization header.
 *          hHeader         - the authentication-info header.
 *****************************************************************************/
static RvStatus AuthenticatorSetParamsToAuthenticationInfo(IN AuthenticatorInfo    *authInfo,
                                                           IN AuthenticatorParams  *pAuthParams,
                                                           IN RPOOL_Ptr            pRpoolResponse,
                                                           IN RvSipAuthorizationHeaderHandle hAuthorization,
                                                           IN RvSipAuthenticationInfoHeaderHandle hHeader)
{
    RvChar  strCnonce[10];
    RvUint  len;
    RvStatus rv = RV_OK;

    /*It sets the "message-qop", "cnonce", and "nonce-count" parameters if needed.*/
    /* RFC 2617 (page 16): "The server SHOULD use the same value for the message-
     qop directive in the response as was sent by the client in the
     corresponding request."*/
    rv = RvSipAuthenticationInfoHeaderSetQopOption(hHeader, pAuthParams->eQopOption, NULL);
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "AuthenticatorSetParamsToAuthenticationInfo - failed to set the Qop"));
        return rv;
    }
    
    /*RFC 2617 (page 17): "The "cnonce-value" and "nc-value" MUST be the ones for the client request 
      to which this message is the response. The "response-auth", "cnonce", and "nonce-count"
      directives MUST BE present if "qop=auth" or "qop=auth-int" is  specified.
*/
    rv = RvSipAuthenticationInfoHeaderSetNonceCount(hHeader, 
                                                    RvSipAuthorizationHeaderGetNonceCount(hAuthorization));
    if (rv != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "AuthenticatorSetParamsToAuthenticationInfo - failed to set the Nonce-Count"));
        return rv;
    }
    rv = RvSipAuthorizationHeaderGetCNonce(hAuthorization, strCnonce, 10, &len);
    if (rv == RV_OK)
    {
        rv = RvSipAuthenticationInfoHeaderSetCNonce(hHeader, strCnonce);
        if (rv != RV_OK)
        {
            RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                "AuthenticatorSetParamsToAuthenticationInfo - failed to set the cnonce"));
            return rv;
        }
    }

    rv = SipAuthenticationInfoHeaderSetResponseAuth((RvSipAuthenticationInfoHeaderHandle)hHeader, NULL, 
                                                    pRpoolResponse.hPool,
                                                    pRpoolResponse.hPage,
                                                    pRpoolResponse.offset);
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(authInfo);
#endif
    return rv;
}
/******************************************************************************
 * cleanAuthParamsStruct
 * ----------------------------------------------------------------------------
 * General: initiate all members in an AuthenticatorParams structure. 
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthParams  - pointer to an AuthenticatorParams struct.
 ***********************************************************************************/
static void cleanAuthParamsStruct(AuthenticatorParams  *pAuthParams)
{
    memset(pAuthParams, 0, sizeof(AuthenticatorParams));
    pAuthParams->eAuthAlgorithm = RVSIP_AUTH_ALGORITHM_UNDEFINED;
    pAuthParams->eAuthHeaderType = RVSIP_AUTH_UNDEFINED_AUTHENTICATION_HEADER;
    pAuthParams->eQopOption = RVSIP_AUTH_QOP_UNDEFINED;
    pAuthParams->userNameOffset = UNDEFINED;
    pAuthParams->passwordOffset = UNDEFINED;
    pAuthParams->realmOffset = UNDEFINED;
    pAuthParams->opaqueOffset = UNDEFINED;
    pAuthParams->nonceOffset = UNDEFINED;
#ifdef RV_SIP_IMS_ON
    pAuthParams->akaVersion = UNDEFINED;
#endif
}

#endif /*#ifdef RV_SIP_AUTH_ON*/

#endif /* #ifndef RV_SIP_LIGHT */

