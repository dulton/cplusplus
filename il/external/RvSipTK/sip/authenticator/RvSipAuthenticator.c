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
 *                              RvSipAuthenticator.c
 *
 * This c file contains functions that prepare digest string for
 * the authentication hash algorithm. The output of the algorithm will be the response
 * parameter of the authorization header. It also contains functions for building
 * the Authorization header from the Authentication header and function for registering
 * the algorithm callbacks.
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

#ifdef RV_SIP_AUTH_ON
#include "_SipCommonConversions.h"

#include "RvSipAuthenticator.h"
#include "RvSipMsgTypes.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "AuthenticatorObject.h"
#include "_SipAuthenticator.h"
#include "RvSipAuthenticationInfoHeader.h"
#include "_SipMsg.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



/************************************************************************************
 * RvSipAuthenticatorSetEvHandlers
 * ----------------------------------------------------------------------------------
 * General: Sets event handlers for the authentication events.
 * Return Value: RvStatus - RV_OK       - the event handlers were set successfully
 *                           RV_ERROR_INVALID_HANDLE - the authentication handle was invalid
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - handle to the authenticator module.
 *          evHandlers        - pointer to event handlers structure.
 *          evHandlersSize    - the size of the event handlers structure.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetEvHandlers(
                                   IN RvSipAuthenticatorHandle      hAuth,
                                   IN RvSipAuthenticatorEvHandlers  *evHandlers,
                                   IN RvUint32                      evHandlersSize)
{
    AuthenticatorInfo *pAuthMgr;

    pAuthMgr = (AuthenticatorInfo*)hAuth;
    if (pAuthMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvMutexLock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
    
	memset(&(pAuthMgr->evHandlers), 0, evHandlersSize);
    memcpy(&(pAuthMgr->evHandlers), evHandlers, evHandlersSize);
	
	RvMutexUnlock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
    
    RvLogInfo(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
              "RvSipAuthenticatorSetEvHandlers - the event handlers were set successfully"));

    return RV_OK;
}

/************************************************************************************
 * RvSipAuthenticatorSetAppHandle
 * ----------------------------------------------------------------------------------
 * General: Set the application authenticator handle to the SIP stack authenticator
 *          object. The application handle can allow you to identify information
 *          with the SIP stack authenticator in callbacks such as
 *          RvSipAuthenticatorGetSharedSecretEv().
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - Handle to the authenticator object.
 *          hAppAuth          - Handle to the application authenticator object.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetAppHandle(
                                   IN RvSipAuthenticatorHandle    hAuth,
                                   IN RvSipAppAuthenticatorHandle hAppAuth)
{
    AuthenticatorInfo *authInfo;

    authInfo = (AuthenticatorInfo*)hAuth;
    if (authInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    authInfo->hAppAuthHandle = hAppAuth;

    return RV_OK;
}

/************************************************************************************
 * RvSipAuthenticatorGetAppHandle
 * ----------------------------------------------------------------------------------
 * General: Get the application authenticator handle that is attached to the SIP stack 
 *          authenticator object. The application handle was attached via 
 *          RvSipAuthenticatorSetAppHandle.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - Handle to the authenticator object.        
 * Output:  phAppAuth         - The retrieved application authenticator object.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorGetAppHandle(
                                   IN  RvSipAuthenticatorHandle     hAuth,
                                   OUT RvSipAppAuthenticatorHandle *phAppAuth)
{
	AuthenticatorInfo *authInfo;

    authInfo = (AuthenticatorInfo*)hAuth;
    if (authInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (phAppAuth == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

    *phAppAuth = authInfo->hAppAuthHandle;

    return RV_OK;
}

/************************************************************************************
 * RvSipAuthenticatorSetActOnCallback
 * ----------------------------------------------------------------------------------
 * General: Sets the act on callback indications to the authenticator. Act on callback
 *          indicates for each callback whether the authenticator should consider this
 *			callback when deciding whether to apply default behavior. As default, the
 *			authenticator has these values set to RV_TRUE, indicating that if a callback
 *			was set by the application, then it overrides the need in automatic behavior.
 *			For example, if the application did not supply event handler for 
 *			UnsupportedChallengeEv(), the authenticator checks the validity of the
 *			Auth object and acts accordingly. If application event handler for 
 *			UnsupportedChallengeEv() was set, the authenticator treats the Auth object 
 *			as constantly valid. If you wish to treat the Auth object as constantly valid,
 *			and you did set an event handler for UnsupportedChallengeEv(), use
 *          RvSipAuthenticatorSetActOnCallback () to set bUnsupportedChallengeAuthentication 
 *          to RV_FALSE.
 * Return Value: RvStatus 
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - handle to the authenticator module.
 *          pActOnCallback    - pointer to act on callback structure.
 *          structSize        - the size of the act on callback structure.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetActOnCallback(
                                   IN RvSipAuthenticatorHandle          hAuth,
                                   IN RvSipAuthenticatorActOnCallback  *pActOnCallback,
                                   IN RvUint32                          structSize)
{
	AuthenticatorInfo *pAuthMgr;

    pAuthMgr = (AuthenticatorInfo*)hAuth;
    if (pAuthMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (pActOnCallback == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

    memset(&(pAuthMgr->actOnCallback), 0, structSize);
    memcpy(&(pAuthMgr->actOnCallback), pActOnCallback, structSize);
	
	RvLogInfo(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
              "RvSipAuthenticatorSetActOnCallback - act on callback indications were set successfully"));

    return RV_OK;
}

/************************************************************************************
 * RvSipAuthenticatorSetProxyAuthInfo
 * ----------------------------------------------------------------------------------
 * General: set global proxy authentication header. It is used for authentication process
 *          with the same outbound proxy server.
 * Return Value: RvStatus - RV_OK         - the global header was set successfully
 *                           RV_ERROR_INVALID_HANDLE   - failed setting the global header
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr          - handle to the authenticator module.
 *          hAuth             - handle to the authentication header.
 *          strUserName       - the user name string.
 *          strPassword       - the password string.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetProxyAuthInfo(
                                   IN RvSipAuthenticatorHandle         hAuthMgr,
                                   IN RvSipAuthenticationHeaderHandle  hAuth,
                                   IN RvChar                          *strUserName,
                                   IN RvChar                          *strPassword)
{
    AuthenticatorInfo *authInfo;
    RvStatus         status;
    RvInt32          realmOffset;
    RvInt32          nonceOffset;
    RvInt32          opaqueOffset;

    authInfo = (AuthenticatorInfo*)hAuthMgr;
    if (authInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /* checking that the authentication header is valid */
    status = SipAuthenticatorValidityChecking(hAuthMgr, hAuth);
    if (status != RV_OK)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "RvSipAuthenticatorSetProxyAuthInfo - the authentication header has unsupported parameters"));
        return status;
    }

    /* checking that all the needed parameters exist */
    /* realm */
    realmOffset = SipAuthenticationHeaderGetRealm(hAuth);
    if (realmOffset == UNDEFINED)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "RvSipAuthenticatorSetProxyAuthInfo - failed in getting the realm offset"));
        return RV_ERROR_UNKNOWN;
    }
    /*nonce */
    nonceOffset = SipAuthenticationHeaderGetNonce(hAuth);
    if (nonceOffset == UNDEFINED)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "RvSipAuthenticatorSetProxyAuthInfo - failed in getting the nonce offset"));
        return RV_ERROR_UNKNOWN;
    }
    /* opaque */
    opaqueOffset = SipAuthenticationHeaderGetOpaque(hAuth);
    if (opaqueOffset == UNDEFINED)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       /*
         opaque is optional; so, we shouldn't abort this procedure because of missing opaque value.
         definition - A string of data, specified by the server, which should be returned
         by the client unchanged in the Authorization header of subsequent
         requests with URIs in the same protection space. It is recommended
         that this string be base64 or hexadecimal data.
       */
        RvLogDebug(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "RvSipAuthenticatorSetProxyAuthInfo - failed in getting the opaque offset -> ignore"));
#else
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "RvSipAuthenticatorSetProxyAuthInfo - failed in getting the opaque offset"));
        return RV_ERROR_UNKNOWN;
#endif
/* SPIRENT_END */

    }

    RvMutexLock(&authInfo->lock,authInfo->pLogMgr);

    /* Check, if the new 'nonce' is equal to the previous.
       In this case 'nonce-count' should be reset */
    if (NULL != authInfo->hGlobalAuth)
    {
        RvInt32 prevNonceOffset;
        HPAGE  newPage = SipAuthenticationHeaderGetPage(hAuth);
        HRPOOL newPool = SipAuthenticationHeaderGetPool(hAuth);

        prevNonceOffset = SipAuthenticationHeaderGetNonce(authInfo->hGlobalAuth);
        if (RV_FALSE == RPOOL_Strcmp(newPool, newPage, nonceOffset,
                                     authInfo->hGeneralPool, authInfo->hGlobalHeaderPage,prevNonceOffset))
        {
            authInfo->nonceCount = 0;
        }
    }

    /* getting new page for the global authentication header */
    if (authInfo->hGlobalHeaderPage != NULL_PAGE)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; 
        authInfo->hGlobalAuth = NULL;
    }
    status = RPOOL_GetPage(authInfo->hGeneralPool, 0, &(authInfo->hGlobalHeaderPage));
    if (RV_OK != status)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "RvSipAuthenticatorSetProxyAuthInfo - failed to get page for header. Pool=0x%p, hAuthMgr=0x%p, hAuth=0x%p",
            authInfo->hGeneralPool,hAuthMgr,hAuth));
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        return RV_ERROR_UNKNOWN;
    }

    status = RvSipAuthenticationHeaderConstruct(authInfo->hMsgMgr,
                                                authInfo->hGeneralPool,
                                                authInfo->hGlobalHeaderPage,
                                                &(authInfo->hGlobalAuth));
    if (status != RV_OK)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; authInfo->hGlobalAuth = NULL;
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "RvSipAuthenticatorSetProxyAuthInfo - failed in constructing the header. hAuthMgr=0x%p, hAuth=0x%p, hPool=0x%p, hPage=0x%px",
            hAuthMgr,hAuth,authInfo->hGeneralPool,authInfo->hGlobalHeaderPage));
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        return status;
    }
    status = RvSipAuthenticationHeaderCopy(authInfo->hGlobalAuth, hAuth);
    if (status != RV_OK)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; authInfo->hGlobalAuth = NULL;
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "RvSipAuthenticatorSetProxyAuthInfo - failed in copying the header 0x%p into header 0x%p. hAuthMgr=0x%p, hAuth=0x%p",
            hAuth,authInfo->hGlobalAuth,hAuthMgr,hAuth));
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        return status;
    }
    status = SipAuthenticationHeaderSetUserName(authInfo->hGlobalAuth, strUserName, NULL, NULL_PAGE, UNDEFINED);
    if (status != RV_OK)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; authInfo->hGlobalAuth = NULL;
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "RvSipAuthenticatorSetProxyAuthInfo - failed to set username %s into header 0x%p. hAuthMgr=0x%p, hAuth=0x%p",
            strUserName, authInfo->hGlobalAuth, hAuthMgr, hAuth));
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        return status;
    }
    status = SipAuthenticationHeaderSetPassword(authInfo->hGlobalAuth, strPassword, NULL, NULL_PAGE, UNDEFINED);
    if (status != RV_OK)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; authInfo->hGlobalAuth = NULL;
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "RvSipAuthenticatorSetProxyAuthInfo - failed to set password %s into header 0x%p. hAuthMgr=0x%p, hAuth=0x%p",
            strPassword, authInfo->hGlobalAuth, hAuthMgr, hAuth));
        RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);
        return status;
    }

    RvMutexUnlock(&authInfo->lock,authInfo->pLogMgr);

    RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
        "RvSipAuthenticatorSetProxyAuthInfo - the global authentication header was set successfully. "));

    return RV_OK;
}

/************************************************************************************
 * RvSipAuthenticatorFreeAuthGlobalPage
 * ----------------------------------------------------------------------------------
 * General: Free a global Page (authPage) that is allocated when the application 
 *          creates authentication header in advance. 
 *         
 *          This is an internal function. Should not be used.
 *
 * Return Value: RvStatus - RV_OK  - returned if the global page was free successfully
 *                          RV_ERROR_INVALID_HANDLE - returned if hAuthMgr is NULL
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr - handle to the authenticator module.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorFreeAuthGlobalPage(
                                                IN RvSipAuthenticatorHandle  hAuthMgr)
{
    AuthenticatorInfo *authInfo;
    RvStatus           status = RV_OK;
    
    authInfo = (AuthenticatorInfo*)hAuthMgr;
    if (authInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (authInfo->hGlobalHeaderPage != NULL_PAGE)
    {
        RPOOL_FreePage(authInfo->hGeneralPool, authInfo->hGlobalHeaderPage);
        authInfo->hGlobalHeaderPage = NULL; 
        authInfo->hGlobalAuth = NULL;
    }
    return status;


}
/************************************************************************************
 * RvSipAuthenticatorVerifyCredentials
 * ----------------------------------------------------------------------------------
 * General: This function is for a server side authentication.
 *          The user gives the password belongs to username and realm in authorization
 *          header, and wants to know if the authorization header is correct, for this
 *          username.
 *          This function creates the digest string as specified in RFC 2617, and compare
 *          it to the digest string inside the given authorization header.
 *          If it is equal, the header is correct.
 * Return Value: RvStatus -
 *          RV_OK     - the check ended successfully
 *          RV_ERROR_UNKNOWN     - failed during the check.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthorization  - handle to the authorization header.
 *          strMethod       - the method type of the request.
 *          nonceCount      - how many times did we use that nonce
 * Output:  isCorrect       - boolean, TRUE if correct, False if not.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorVerifyCredentials(
                                   IN RvSipAuthenticatorHandle         hAuth,
                                   IN RvSipAuthorizationHeaderHandle   hAuthorization,
                                   IN RvChar                          *password,
                                   IN RvChar                          *strMethod,
                                   OUT RvBool                         *isCorrect)
{
    return SipAuthenticatorVerifyCredentials(hAuth, hAuthorization, password,
        strMethod, NULL/*hObject*/, RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED,
        NULL/*hObjectLock*/, NULL/*hMsg*/, isCorrect);
}

/******************************************************************************
 * RvSipAuthenticatorVerifyCredentialsExt
 * ----------------------------------------------------------------------------
 * General: This function is for a server side authentication.
 *          Using this function the application can verify the credentials
 *          received in an incoming request message. The application supplies
 *          the password that belong to the user name and realm found
 *          in the authorization header, and wishes to know
 *          if the authorization header is correct for this username.
 *          This function creates the digest string as specified in RFC 2617,
 *          and compares it to the digest string inside the given authorization
 *          header.
 *          If it is equal, the header is correct.
 *          In comparison to RvSipAuthenticatorVerifyCredentials() function
 *          this function support auth-int Quality-of-Protection.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthorization  - handle to the authorization header.
 *          password        - password of the user in hAuthorization header.
 *          strMethod       - the method type of the request.
 *          hObject         - handle to the object to be authenticated.
 *          peObjectType    - type of the object to be authenticated.
 *          hMsg            - message object, to which hAuthorization belongs.
 * Output:  pbIsCorrect     - RV_TRUE if correct, RV_FALSE otherwise.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorVerifyCredentialsExt(
                            IN  RvSipAuthenticatorHandle        hAuth,
                            IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                            IN  RvChar*                         password,
                            IN  RvChar*                         strMethod,
                            IN  void*                           hObject,
                            IN  void*                           peObjectType,
                            IN  RvSipMsgHandle                  hMsg,
                            OUT RvBool*                         pbIsCorrect)
{
    return SipAuthenticatorVerifyCredentials(hAuth, hAuthorization, password,
              strMethod, hObject, *((RvSipCommonStackObjectType*)peObjectType),
              NULL /*hObjectLock*/, hMsg, pbIsCorrect);
}

/***************************************************************************
 * RvSipAuthenticatorCredentialsSupported
 * ------------------------------------------------------------------------
 * General: The function checks parameters in the authorization header like
 *          qop, algorithm and scheme and decides if we can authenticate it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hAuthMgr - handle to the authenticator manager.
 *        hAuth    - handle to the authorization header.
 * Output:  bIsSupported - RV_TRUE if supported.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorCredentialsSupported(
                                        IN  RvSipAuthenticatorHandle         hAuthMgr,
                                        IN  RvSipAuthorizationHeaderHandle   hAuthoriz,
                                        OUT RvBool*                         bIsSupported)
{
    return SipAuthenticatorCredentialsSupported(hAuthMgr, hAuthoriz, bIsSupported);
}

/***************************************************************************
 * RvSipAuthenticatorGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this
 *          authenticator belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hAuthMgr     - Handle to the stack authentication manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorGetStackInstance(
                                   IN   RvSipAuthenticatorHandle  hAuthMgr,
                                   OUT  void                    **phStackInstance)
{
    AuthenticatorInfo* pAuthInfo = (AuthenticatorInfo*)hAuthMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;
    if(pAuthInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phStackInstance = pAuthInfo->hStack;

    return RV_OK;
}

/******************************************************************************
 * RvSipAuthenticatorSetNonceCount
 * ----------------------------------------------------------------------------
 * General: Set initial value of 'nonce-count' parameter, which will be used
 *          for 'Authorization'/'Proxy-Authorization' header for all outgoing
 *          requests independently of the request owner(e.g. CallLeg,RegClient)
 *          'nonce-count' value is incremented by the Authenticator after each
 *          header calculation in according to RFC 2617.
 *          If the value is not set, a 'nonce-count' value, managed by each
 *          owner and not by authenticator is used for the headers.
 *          Range of the legal values for the 'nonce-count' is [0,MAX_INT32].
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr   - Handle to the stack authentication manager.
 *          nonceCount - value to be set.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetNonceCount(
                                   IN RvSipAuthenticatorHandle  hAuthMgr,
                                   IN RvInt32                   nonceCount)
{
    AuthenticatorInfo *pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    if (NULL == pAuthMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    RvMutexLock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
    pAuthMgr->nonceCount = nonceCount;
    RvMutexUnlock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
    RvLogInfo(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
        "RvSipAuthenticatorSetNonceCount - Authenticator 'nonce-count' was set to %d",nonceCount));
    return RV_OK;
}

/******************************************************************************
 * RvSipAuthenticatorAddProxyAuthorizationToMsg
 * ----------------------------------------------------------------------------
 * General: Adds a Proxy-Authorization header to the supplied message object.
 *          You can use this function only if you set a Proxy-Authenticate
 *          header to the authenticator using the RvSipAuthenticatorSetProxyAuthInfo()
 *          API function. The authenticator uses the challenge found in the
 *          Proxy-Authenticate header to build the correct Proxy-Authorization()
 *          header.
 *          You should use this function if you want to add credentials to
 *          outgoing requests sent by stand alone transactions. The function
 *          should be called from the message to send callback of the transaction.
 *          For other stack objects, the process of adding the authorization
 *          header is automatic.
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth      - Handle to the Authenticator object.
 *          hMsg       - Handle to the message, to which the Proxy-Authorization
 *                       header should be added.
 *          nonceCount - 'nonce-count' value, that should be used for the MD5 signature
 *                       generation. If set to UNDEFINED, internal nonce-count,
 *                       managed by the Authenticator, will be used. If no
 *                       internal 'nonce-value' was set (with the
 *                       RvSipAuthenticatorSetNonceCount() function), '1' will be used.
 *          hObject    - Handle to the object this message belongs to.
 *                       The handle will be supplied back to the application in
 *                       some of the authenticator callback functions.
 *                       You can supply NULL value.
 *          eObjectType- The type of the object this message belongs to.
 *                       The type will be supplied back to the application in
 *                       some of the authenticator callback functions.
 *                       You can set this value to RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorAddProxyAuthorizationToMsg(
                            IN RvSipAuthenticatorHandle        hAuth,
                            IN RvSipMsgHandle                  hMsg,
                            IN RvInt32                         nonceCount,
                            IN void*                           hObject,
                            IN RvSipCommonStackObjectType      eObjectType)
{
    RvStatus rv;
    RvChar             strMethod[SIP_METHOD_LEN] = {'\0'};
    RvSipAddressHandle hRequestUri;
    RvSipAuthorizationHeaderHandle hAuthorization;
    AuthenticatorInfo *pAuthMgr = (AuthenticatorInfo*)hAuth;
    if (NULL == pAuthMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Construct Authorization header */
    rv = RvSipAuthorizationHeaderConstructInMsg(
                          hMsg, RV_FALSE/*pushHeaderAtHead*/, &hAuthorization);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RvSipAuthenticatorAddProxyAuthorizationToMsg - failed to construct Authorization header, hMsg=0x%p (rv=%d:%s)",
            hMsg,rv,SipCommonStatus2Str(rv)));
        return rv;
    }
    /* Set necessary data into the header */
    /* If the 'nonceCount' parameter should be ignored, overwrite it with
    internal one */
    if (UNDEFINED == nonceCount)
    {
        RvMutexLock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
        ++(pAuthMgr->nonceCount);
        nonceCount = (RvInt32)pAuthMgr->nonceCount;
        RvMutexUnlock(&pAuthMgr->lock,pAuthMgr->pLogMgr);
    }
    SipAuthenticatorMsgGetMethodStr(hMsg, strMethod);
    hRequestUri = RvSipMsgGetRequestUri(hMsg);
    rv = AuthenticatorPrepareProxyAuthorization(hAuth, strMethod,
        hRequestUri, nonceCount, hObject, eObjectType, NULL, hMsg,
        hAuthorization);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RvSipAuthenticatorAddProxyAuthorizationToMsg - failed in SipAuthenticatorPrepareProxyAuthorization, hMsg=0x%p (rv=%d:%s)",
            hMsg,rv,SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}
/******************************************************************************
 * RvSipAuthenticatorPrepareAuthorizationHeader
 * ----------------------------------------------------------------------------
 * General: Prepares 'Authorization'/'Proxy-Authorization' header based
 *          on the 'WWW-Authenticate'/'Authenticate' header and other data,
 *          supplied as a parameters.
 *
 * Return Value: RvStatus: RV_OK on success, error code otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth       - Handle to the Authenticator object.
 *          hAuthenticationHeader - Handle to the 'WWW-Authenticate' / 
 *                        'Authenticate' header.
 *          strMethod   - string representing SIP method name.
 *                        Can be NULL.
 *                        If NULL, the 'hMsg' parameter should be supplied,
 *                        when it handles the Message object with set method.
 *          hRequestUri - Handle to the Address object representing the address
 *                        to which the message with prepared 'Authorization' /
 *                        'Proxy-Authorization' header will be sent.
 *          nonceCount  - nonce count, as it is defined in RFC 3261.
 *                        If UNDEFINED, global Nonce Count, set by means of 
 *                        RvSipAuthenticatorSetNonceCount(), will be used.
 *                        If the last is not set, an error will occur.
 *                        Note that no check for matching of the global Nonce
 *                        value, set by RvSipAuthenticatorSetProxyAuthInfo(),
 *                        to the value, contained in the 'WWW-Authenticate' / 
 *                        'Authenticate' header, is done.
 *          hObject     - Handle to the application object that will be 
 *                        supplied to the application as a callback parameter
 *                        during authorization header preparing.
 *                        Can be NULL.
 *          hMsg        - Handle to the Message, which will contain
 *                        prepared 'Authorization'/'Proxy-Authorization' header
 *          hAuthorizationHeader - Handle to the prepared header.
 *                        Can be NULL.
 *                        If NULL, 'hMsg' parameter should be provided.
 *                        In last case new header will be constructed,
 *                        while using the Message's page.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorPrepareAuthorizationHeader(
                 IN      RvSipAuthenticatorHandle        hAuth,
                 IN      RvSipAuthenticationHeaderHandle hAuthenticationHeader,
                 IN      RvChar*                         strMethod,
                 IN      RvSipAddressHandle              hRequestUri,
                 IN      RvInt32                         nonceCount,
                 IN      void*                           hObject,
                 IN      RvSipMsgHandle                  hMsg,
                 IN OUT  RvSipAuthorizationHeaderHandle  hAuthorizationHeader)
{
    AuthenticatorInfo*  pAuthenticator = (AuthenticatorInfo*)hAuth;
    RvStatus            rv;

    /* Check parameter validity */
    if (NULL == hAuth)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == hAuthenticationHeader)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthorizationHeader - hAuthenticationHeader is NULL"));
        return RV_ERROR_INVALID_HANDLE;
    }
    /* If strMethod was not provided, try to get it from the Message */
    if (strMethod==NULL)
    {
        RvChar   strMethodBuf[SIP_METHOD_LEN]; /* MUSTN'T be static if we support Symbian */
        RvSipMethodType eMethodType;

        if (NULL==hMsg)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthorizationHeader - neither strMethod nor hMsg is provided"));
            return RV_ERROR_INVALID_HANDLE;
        }
        eMethodType = RvSipMsgGetRequestMethod(hMsg);
        if (RVSIP_METHOD_OTHER == eMethodType)
        {
            RvUint  strMethodLen = 0;
            rv = RvSipMsgGetStrRequestMethod(hMsg, strMethodBuf, SIP_METHOD_LEN,
                                        &strMethodLen);
            strMethodBuf[strMethodLen] = '\0';
            strMethod = strMethodBuf;
            if (RV_OK!=rv)
            {
                RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                    "RvSipAuthenticatorPrepareAuthorizationHeader - Method can't be extracted from hMsg 0x%p",hMsg));
                return RV_ERROR_INVALID_HANDLE;
            }
        }
        else
        {
            strMethod = SipCommonConvertEnumToStrMethodType(eMethodType);
        }
    }
    if (hRequestUri==NULL)
    {
        if (NULL==hMsg)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthorizationHeader - neither hRequestUri nor hMsg is provided"));
            return RV_ERROR_INVALID_HANDLE;
        }
        hRequestUri = RvSipMsgGetRequestUri(hMsg);
        if (NULL == hRequestUri)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthorizationHeader - hRequestUri can't be extracted from the Message 0x%p",hMsg));
            return RV_ERROR_INVALID_HANDLE;
        }
    }
    if (NULL == hAuthorizationHeader && NULL==hMsg)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthorizationHeader - hAuthorizationHeader is NULL. hMsg is NULL"));
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Get data, necessary for an authorization header preparation */
    if (0 > nonceCount)
    {
        pAuthenticator->nonceCount++;
        nonceCount = pAuthenticator->nonceCount;
    }
    if (NULL == hAuthorizationHeader)
    {
        rv = RvSipAuthorizationHeaderConstructInMsg(hMsg,
                          RV_FALSE/*pushHeaderAtHead*/, &hAuthorizationHeader);
        if(rv != RV_OK)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthorizationHeader - failed to construct hAuthorizationHeader (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Prepare the authorization header */
    rv = AuthenticatorPrepareAuthorizationHeader(hAuth, 
        hAuthenticationHeader, strMethod, hRequestUri, nonceCount, hObject,
        RVSIP_COMMON_STACK_OBJECT_TYPE_APP_OBJECT, NULL/*pObjectLock*/, hMsg,
        hAuthorizationHeader);
    if (RV_OK != rv)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthorizationHeader - failed to prepare authorization header 0x%p from authentication header 0x%p (rv=%d:%s)",
            hAuthorizationHeader, hAuthenticationHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogInfo(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
        "RvSipAuthenticatorPrepareAuthorizationHeader - authentication header 0x%p was prepared from authentication header 0x%p successfully",
        hAuthorizationHeader, hAuthenticationHeader));
    return RV_OK;
}


/******************************************************************************
 * RvSipAuthenticatorPrepareAuthenticationInfoHeader
 * ----------------------------------------------------------------------------
 * General: This function builds an authentication-info header.
 *          1. It generates and set the response-auth string, very similarly to   
 *             the "request-digest" in the Authorization header.
 *             (Note that the MD5 and shared-secret callback will be called
 *              during the response-auth generation).
 *          2. It sets the "message-qop", "cnonce", and "nonce-count" 
 *             parameters if needed.
 *          3. It sets the given next-nonce parameter to the header.
 *             
 *          The function receives an authorization header (that was received in the
 *          request, and was authorized well), and uses it's parameters in order to
 *          build the authentication-info header.
 *          you may use this function to set only the next-nonce parameter, by giving
 *          NULL in the Authorization header argument.
 *
 * Return Value: RvStatus: RV_OK on success, error code otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth       - Handle to the Authenticator object.
 *          hAuthorization - Handle to the authorization header.
 *          nextNonce   - value for the next-nonce parameter. optional.
 *          eObjType    - The type of the following hObj parameter.
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
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorPrepareAuthenticationInfoHeader(
                 IN      RvSipAuthenticatorHandle        hAuth,
                 IN      RvSipAuthorizationHeaderHandle  hAuthorization,
                 IN      RvChar*                         nextNonce,
                 IN      RvSipCommonStackObjectType      eObjType,
                 IN      void*                           hObject,
                 IN      RvSipMsgHandle                  hMsg,
                 IN OUT  RvSipAuthenticationInfoHeaderHandle  hAuthenticationInfo)
{
    AuthenticatorInfo*  pAuthenticator = (AuthenticatorInfo*)hAuth;
    RvStatus            rv;

    /* Check parameter validity */
    if (NULL == pAuthenticator)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == hAuthorization)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthenticationInfoHeader - no authorization header is given."));
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == hAuthenticationInfo && NULL == hMsg)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthenticationInfoHeader - hAuthenticationInfo is NULL. hMsg is NULL"));
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == hAuthenticationInfo)
    {
        rv = RvSipAuthenticationInfoHeaderConstructInMsg(hMsg,
                          RV_FALSE/*pushHeaderAtHead*/, &hAuthenticationInfo);
        if(rv != RV_OK)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthenticationInfoHeader - failed to construct hAuthenticationInfo (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* from RFC 2617:
       The server SHOULD use the same value for the message-qop 
       directive in the response as was sent by the client in the
       corresponding request.
       The "cnonce-value" and "nc-value" MUST be the ones for the client 
       request to which this message is the response. 
       The "response-auth", "cnonce", and "nonce-count" directives MUST BE present
       if "qop=auth" or "qop=auth-int" is specified.*/

    /* Prepare the response-auth parameter */
    rv = AuthenticatorPrepareAuthenticationInfoHeader(hAuth, hAuthorization,
            hObject, eObjType, NULL/*pObjectLock*/, hMsg, hAuthenticationInfo);
    if (RV_OK != rv)
    {
        RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
            "RvSipAuthenticatorPrepareAuthenticationInfoHeader - failed to prepare authentication-info header 0x%p from authorization header 0x%p (rv=%d:%s)",
            hAuthenticationInfo, hAuthorization, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    if(nextNonce != NULL)
    {
        rv = RvSipAuthenticationInfoHeaderSetNextNonce(hAuthenticationInfo, nextNonce);
        if (RV_OK != rv)
        {
            RvLogError(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
                "RvSipAuthenticatorPrepareAuthenticationInfoHeader - failed to set nextNonce in authentication-info header 0x%p (rv=%d:%s)",
                hAuthenticationInfo, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    RvLogInfo(pAuthenticator->pLogSrc,(pAuthenticator->pLogSrc,
        "RvSipAuthenticatorPrepareAuthenticationInfoHeader - authentication-info header 0x%p was prepared from authorization header 0x%p successfully",
        hAuthenticationInfo, hAuthorization));
    return RV_OK;
}

/* ------------------------- AUTH  OBJ  FUNCTIONS -----------------------------*/

/******************************************************************************
 * RvSipAuthObjGetAuthenticationHeader
 * ----------------------------------------------------------------------------
 * General: return the authentication header kept in an auth-object.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 * Output:  phHeader  - Handle to the Authentication header in the object.
 *          pbIsValid - Is the authentication header valid or not.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjGetAuthenticationHeader(
                                           IN   RvSipAuthObjHandle        hAuthObj,
			                               OUT  RvSipAuthenticationHeaderHandle* phHeader,
                                           OUT  RvBool                   *pbIsValid)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv;
    
    if(hAuthObj == NULL || phHeader == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *phHeader = pAuthObj->hHeader;
    if(pbIsValid != NULL)
    {
        if(*phHeader == NULL)
        {
            *pbIsValid = RV_FALSE;
            rv = RV_ERROR_NOT_FOUND;
        }
        else
        {
            *pbIsValid = pAuthObj->bValid; 
        }
    }
    AuthObjUnLockAPI(pAuthObj);    
    return rv;
}

/******************************************************************************
 * RvSipAuthObjSetUserInfo
 * ----------------------------------------------------------------------------
 * General: Set the user-name and password in the authentication object.
 *          Application may use this function to set the user information before 
 *          sending the request. In this case, the RvSipAuthenticatorSharedSecretEv 
 *          callback function won't be called.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 *          pstrUserName - Pointer to the user-name string. If NULL,
 *                      will erase the exists parameter in the auth-obj.
 *          pstrUserPw - Pointer to the password string. If NULL,
 *                      will erase the exists parameter in the auth-obj.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjSetUserInfo(
                               IN   RvSipAuthObjHandle        hAuthObj,
                               IN   RvChar*                   pstrUserName,
                               IN   RvChar*                   pstrUserPw)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv = RV_OK, rv2 = RV_OK;
    
    if(hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pstrUserName == NULL || pstrUserPw == NULL)
    {
        RvLogError(pAuthObj->pAuthMgr->pLogSrc,(pAuthObj->pAuthMgr->pLogSrc,
            "RvSipAuthObjSetUserInfo - auth-obj 0x%p - must have usernane and pw!!!",
            pAuthObj));
        AuthObjUnLockAPI(pAuthObj);  
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    /* we set the user info directly to the authentication header, to keep the 
       old authentication behavior */
    rv = SipAuthenticationHeaderSetUserName(pAuthObj->hHeader, pstrUserName, NULL, NULL, UNDEFINED);
    if(rv != RV_OK)
    {
        RvLogError(pAuthObj->pAuthMgr->pLogSrc,(pAuthObj->pAuthMgr->pLogSrc,
            "RvSipAuthObjSetUserInfo - auth-obj 0x%p - failed to set user name (rv=%d:%s)",
            pAuthObj, rv, SipCommonStatus2Str(rv)));
    }
    rv2= SipAuthenticationHeaderSetPassword(pAuthObj->hHeader, pstrUserPw, NULL, NULL, UNDEFINED);
    if(rv2 != RV_OK)
    {
        RvLogError(pAuthObj->pAuthMgr->pLogSrc,(pAuthObj->pAuthMgr->pLogSrc,
            "RvSipAuthObjSetUserInfo - auth-obj 0x%p - failed to set pw (rv=%d:%s)",
            pAuthObj, rv2, SipCommonStatus2Str(rv2)));
    }
    

    AuthObjUnLockAPI(pAuthObj);    
    if(rv != RV_OK) return rv;
    if(rv2!= RV_OK) return rv2;
    return RV_OK;
}

/******************************************************************************
 * RvSipAuthObjSetAppContext
 * ----------------------------------------------------------------------------
 * General: Set a pointer to application context in the authentication object.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 *          pContext  - Pointer to the application context. If NULL,
 *                      will erase the exists parameter in the auth-obj.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjSetAppContext(
                               IN   RvSipAuthObjHandle        hAuthObj,
                               IN   void*                     pContext)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv = RV_OK;
    
    if(hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pAuthObj->pContext = pContext;
    
    AuthObjUnLockAPI(pAuthObj);    
    return rv;
}

/******************************************************************************
 * RvSipAuthObjGetAppContext
 * ----------------------------------------------------------------------------
 * General: Get the pointer of application context in the authentication object.
 *          (the context was set by application in RvSipAuthObjSetAppContext()).
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 * Output:  ppContext - Pointer to the application context. 
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjGetAppContext(
                               IN   RvSipAuthObjHandle        hAuthObj,
                               OUT  void*                     *ppContext)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv = RV_OK;
    
    if(hAuthObj == NULL || ppContext == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *ppContext = pAuthObj->pContext;
    
    AuthObjUnLockAPI(pAuthObj);    
    return rv;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipAuthObjSetAutsParam
 * ----------------------------------------------------------------------------
 * General: Set the AUTS parameter string in the authentication object.
 *          The AKA AUTS parameter of the Authorization header is a base64 encoded string,  
 *          used to re-synchronize the server side SQN.  
 *          Example: auts="CjkyMzRfOiwg5CfkJ2UK="
 *
 *          Application may use this function to set the AUTS parameter to the 
 *          authentication object in advanced, so the authorization header will be built
 *          with this parameter.
 *          (Application may also set it directly to the Authorization header, in the
 *          RvSipAuthenticatorAuthorizationReadyEv callback).
 *
 *          If the AUTS is present, the client should supply an empty password ("") 
 *          when calculating its credentials.  
 *          
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 *          pstrAuts  - Pointer to the AUTS string (already 64 encoded).
 *                      if NULL - will erase the exists parameter in the auth-obj.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjSetAutsParam(
                              IN   RvSipAuthObjHandle        hAuthObj,
                               IN   RvChar*                   pstrAuts)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv = RV_OK;
    
    if(hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pstrAuts == NULL)
    {
        pAuthObj->autsParamOffset = UNDEFINED;
    }
    else
    {
        rv = RPOOL_AppendFromExternalToPage(pAuthObj->pAuthMgr->hElementPool,
                                       pAuthObj->hAuthObjPage, 
                                       pstrAuts, (RvInt)strlen(pstrAuts)+1, /* +1 for the NULL termination */
                                       &pAuthObj->autsParamOffset);
    }

    AuthObjUnLockAPI(pAuthObj);    
    return rv;
}

/******************************************************************************
 * RvSipAuthObjGetAutsParam
 * ----------------------------------------------------------------------------
 * General: Get the AUTS parameter from the authentication object.
 *          This function retrieves only an AUTS parameter that was set to the
 *          authentication object with RvSipAuthObjSetAutsParam().
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthObj  - Handle to the authentication object.
 *          strBuffer - buffer to fill with the AUTS string.
 *                      if NULL - retrieves only the actual length of the string.
 *          bufferLen - length of the given buffer.
 * Output:  actualLen - the actual length of the string. (0 if not exists).
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjGetAutsParam(
                               IN   RvSipAuthObjHandle        hAuthObj,
                               IN   RvChar*                   strBuffer,
                               IN   RvUint                    bufferLen,
                               OUT  RvUint*                   actualLen)
{
    AuthObj* pAuthObj = (AuthObj*)hAuthObj;
    RvStatus rv = RV_OK;
    
    if( hAuthObj == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    
    *actualLen = 0;
    
    rv = AuthObjLockAPI(pAuthObj);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pAuthObj->autsParamOffset == UNDEFINED)
    {
        *actualLen = 0;
        rv = RV_ERROR_NOT_FOUND;
    }
    else
    {
        *actualLen = (RPOOL_Strlen(pAuthObj->pAuthMgr->hElementPool, 
                                pAuthObj->hAuthObjPage, 
                                pAuthObj->autsParamOffset) + 1);
        if(strBuffer != NULL)
        {
            /* fill the buffer */
            if(bufferLen < *actualLen)
                rv = RV_ERROR_INSUFFICIENT_BUFFER;
            else
            {
                rv = RPOOL_CopyToExternal(  pAuthObj->pAuthMgr->hElementPool, 
                                            pAuthObj->hAuthObjPage, 
                                            pAuthObj->autsParamOffset, 
                                            strBuffer, *actualLen);
            }
        }
    }

    AuthObjUnLockAPI(pAuthObj);    
    return rv;
}

#endif /* RV_SIP_IMS_ON */

#endif /* #ifdef RV_SIP_AUTH_ON */

