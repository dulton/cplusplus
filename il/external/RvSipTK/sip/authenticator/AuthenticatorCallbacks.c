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
 *                              <AuthenticatorCallbacks.c>
 *
 *
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "AuthenticatorCallbacks.h"

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AuthenticatorCallbackMD5Calculate
 * ------------------------------------------------------------------------
 * General: notifies the application of MD5 callbacks.
 *          Tries the new call back. If it is not implemented,
 *          sends the old callback
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthInfo        - authenticator manager
 *         pRpoolMD5Input   - pointer to md5 input
 *         length           - length of the string inside the page
 * Output  pRpoolMD5Output  - The output of the hash algorithm
 ***************************************************************************/
void AuthenticatorCallbackMD5Calculate(
                        IN  AuthenticatorInfo  *pAuthInfo,
                        IN  RPOOL_Ptr          *pRpoolMD5Input,
                        IN  RvUint32           length,
                        OUT RPOOL_Ptr          *pRpoolMD5Output)
{
    RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
        "AuthenticatorCallbackMD5Calculate - Before callback"));

    if (NULL != pAuthInfo->evHandlers.pfnMD5AuthenticationExHandler)
    {
        pAuthInfo->evHandlers.pfnMD5AuthenticationExHandler((RvSipAuthenticatorHandle)pAuthInfo,
                                                            pAuthInfo->hAppAuthHandle,
                                                            pRpoolMD5Input,
                                                            length,
                                                            pRpoolMD5Output);
    }
    else
    {
        pAuthInfo->evHandlers.pfnMD5AuthenticationHandler(pRpoolMD5Input,
                                                          length,
                                                          pRpoolMD5Output);
    }
    RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
               "AuthenticatorCallbackMD5Calculate - After callback"));
}

/***************************************************************************
 * AuthenticatorCallbackGetSharedSecret
 * ------------------------------------------------------------------------
 * General:  Notifies that there is a need for the username and password.
 *           This callback function is for client side authentication.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo         - pointer to the authenticator object.
 *          hObject           - Handle to the Object, that is served
 *                              by the Authenticator (e.g. CallLeg, RegClient)
 *          eObjectType       - type of the Object. Use following code to get the type:
 *                              RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock    - handle to object triple lock, which should be
 *                              unlocked before pass code flow to application
 *          hRequestUri       - handle of the Request URI header
 *          pRpoolRealm       - the realm string in RPool_ptr format
 * Output:  pRpoolUserName    - the username string in RPool_ptr format
 *          pRpoolPassword    - the password string in RPool_ptr format
 *          pRpoolRealm       - the realm string in RPool_ptr format
 ***************************************************************************/
RvStatus AuthenticatorCallbackGetSharedSecret(
                     IN     AuthenticatorInfo           *pAuthInfo,
                     IN     void*                       hObject,
                     IN     RvSipCommonStackObjectType  eObjectType,
                     IN     SipTripleLock               *pObjTripleLock,
                     IN     RvSipAddressHandle          hRequestUri,
                     IN OUT RPOOL_Ptr                   *ppRpoolPtrUserName,
                     IN OUT RPOOL_Ptr                   *ppRpoolPtrPassword,
                     IN OUT RPOOL_Ptr                   *ppRpoolRealm)
{
    if ((pAuthInfo->evHandlers).pfnGetSharedSecretAuthenticationHandler != NULL)
    {
        RvInt32     nestedLock = 0;
        RvThreadId  currThreadId = 0;
        RvInt32     threadIdLockCount = 0;

        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - before callback"));

        if (NULL != pObjTripleLock)
        {
            SipCommonUnlockBeforeCallback(pAuthInfo->pLogMgr,pAuthInfo->pLogSrc,
                pObjTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }
        (pAuthInfo->evHandlers).pfnGetSharedSecretAuthenticationHandler(
                                        (RvSipAuthenticatorHandle)pAuthInfo,
                                        pAuthInfo->hAppAuthHandle,
                                        hObject,
                                        (void*)&eObjectType,
                                        ppRpoolRealm,
                                        ppRpoolPtrUserName,
                                        ppRpoolPtrPassword);
        if (NULL != pObjTripleLock)
        {
            SipCommonLockAfterCallback(pAuthInfo->pLogMgr,pObjTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }

        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - after callback"));

        RvLogInfo(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - UserName and password were supplied for 0x%p (ObjType=%d)",
            hObject,eObjectType));
    }
    else
    if ((pAuthInfo->evHandlers).pfnSharedSecretAuthenticationHandler != NULL)
    {
        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - before callback"));
        (pAuthInfo->evHandlers).pfnSharedSecretAuthenticationHandler(
                                                           hRequestUri,
                                                           ppRpoolRealm,
                                                           ppRpoolPtrUserName,
                                                           ppRpoolPtrPassword);
        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - after callback"));

    }
    else
    {
        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackGetSharedSecret - callback is not set"));
        return RV_ERROR_NOT_FOUND;
    }

    return RV_OK;
}

/***************************************************************************
 * AuthenticatorCallbackAddMD5EntityBody
 * ------------------------------------------------------------------------
 * General:  Asks the Application to concatenate to the A2 string prefix
 *           an MD5(entity-body) string. For details see RFC 2617.
 *           This callback function is implemented meanwhile for client side
 *           authentication only.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthInfo         - pointer to the Authenticator object
 *          hObject           - handle to the Object, that is served
 *                              by the Authenticator (e.g. CallLeg, RegClient)
 *          eObjectType       - type of the Object. Use following code to get the type:
 *                              RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock    - Lock, which should be unlocked before pass code
 *                              control to the application.
 *          hMsg              - handle to the message, Authorization header for
 *                              which is being prepared
 *          pRpoolMD5Output   - string in RPOOL_Ptr format, to which an MD5
 *                              result should be concatenated.
 *          pLength           - length of the appended string.
 * Output:  pRpoolMD5Output   - string in RPOOL_Ptr format, representing A2.
 *          pLength           - length of the string after MD5 result
 *                              concatenation.
 ***************************************************************************/
void AuthenticatorCallbackAddMD5EntityBody(
                    IN     AuthenticatorInfo          *pAuthInfo,
                    IN     void*                      hObject,
                    IN     RvSipCommonStackObjectType eObjectType,
                    IN     SipTripleLock              *pObjTripleLock,
                    IN     RvSipMsgHandle             hMsg,
                    IN OUT RPOOL_Ptr                  *pRpoolMD5Output,
                    OUT    RvUint32                   *pLength)
{
    if (NULL != pAuthInfo->evHandlers.pfnMD5EntityBodyAuthenticationHandler)
    {
        RvInt32     nestedLock = 0;
        RvThreadId  currThreadId = 0;
        RvInt32     threadIdLockCount = 0;

        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackAddMD5EntityBody - Before callback"));

        if (NULL != pObjTripleLock)
        {
            SipCommonUnlockBeforeCallback(pAuthInfo->pLogMgr,pAuthInfo->pLogSrc,
                pObjTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        pAuthInfo->evHandlers.pfnMD5EntityBodyAuthenticationHandler(
            (RvSipAuthenticatorHandle)pAuthInfo, pAuthInfo->hAppAuthHandle,
            hObject, &eObjectType, hMsg, pRpoolMD5Output, pLength);

        if (NULL != pObjTripleLock)
        {
            SipCommonLockAfterCallback(pAuthInfo->pLogMgr,pObjTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }

        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackAddMD5EntityBody - After callback"));
    }
    else
    {
        RvLogDebug(pAuthInfo->pLogSrc,(pAuthInfo->pLogSrc,
            "AuthenticatorCallbackAddMD5EntityBody - callback is not set"));
    }
}

/******************************************************************************
 * AuthenticatorCallbackUnsupportedChallenge
 * ----------------------------------------------------------------------------
 * General: Exposes WWW-Authenticate/Proxy-Authenticate, conatinig 
 *          not supported challenge, to the Application. The Application should
 *          prepare matching cerdentials, build correspondent Authorization
 *          header and set it into the message.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthMgr       - pointer to the Authenticator object.
 *          hObject        - handle to the Object, that is served.
 *                           by the Authenticator (e.g. CallLeg, RegClient).
 *          eObjectType    - type of the Object. Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock - Lock, which should be unlocked before pass code
 *                           control to the application.
 *          hHeader        - handle to the Authentication header, containig
 *                           not supported challenges.
 *          hMsg           - handle to the message, where the Authorization
 *                           header, built by the Application, should be set.
 * Output:  none.
 *****************************************************************************/
void AuthenticatorCallbackUnsupportedChallenge(
                    IN  AuthenticatorInfo               *pAuthMgr,
                    IN  void*                           hObject,
                    IN  RvSipCommonStackObjectType      eObjectType,
                    IN  SipTripleLock                   *pObjTripleLock,
                    IN  RvSipAuthenticationHeaderHandle hHeader,
                    IN  RvSipMsgHandle                  hMsg)
{
    if (NULL != pAuthMgr->evHandlers.pfnUnsupportedChallengeAuthenticationHandler)
    {
        RvInt32     nestedLock = 0;
        RvThreadId  currThreadId = 0;
        RvInt32     threadIdLockCount = 0;
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackUnsupportedChallenge - Before callback"));
        
        if (NULL != pObjTripleLock)
        {
            SipCommonUnlockBeforeCallback(pAuthMgr->pLogMgr,pAuthMgr->pLogSrc,
                pObjTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }
        
        pAuthMgr->evHandlers.pfnUnsupportedChallengeAuthenticationHandler(
            (RvSipAuthenticatorHandle)pAuthMgr, pAuthMgr->hAppAuthHandle,
            hObject, &eObjectType, hHeader, hMsg);
        
        if (NULL != pObjTripleLock)
        {
            SipCommonLockAfterCallback(pAuthMgr->pLogMgr,pObjTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackUnsupportedChallenge - After callback"));
    }
    else
    {
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackUnsupportedChallenge - callback is not set"));
    }
}

/******************************************************************************
 * AuthenticatorCallbackNonceCountUsage
 * ----------------------------------------------------------------------------
 * General: Notifies the application about value of the nonceCount parameter,
 *          which the Stack is going to use, while calculating credentials.
 *          The Application can change this value in order to fit more
 *          precious management of the nonceCount. The Stack doesn't check
 *          the uniquity of the used NONCE in the realm or by the different
 *          object.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthMgr       - Pointer to the Authenticator object
 *          hObject        - Handle to the Object, that is served
 *                           by the Authenticator(e.g.CallLeg,RegClient)
 *          peObjectType   - Pointer to the variable that stores the type
 *                           of the hObject.
 *                           Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock - Pointer to the triple lock, which should be
 *                           unlocked before control passes to the application.
 *          hHeader        - Handle to the Authentication header,
 *                           containing challenge, credentials for which
 *                           are being prepared.
 *          pNonceCount    - pointer to the nonceCount value, managed by
 *                           the Stack per Challenge.
 * Output:  pNonceCount    - pointer to the nonceCount value, set by
 *                           the Application in order to be used by the Stack
 *                           for Credentials calculation.
 ***************************************************************************/
void AuthenticatorCallbackNonceCountUsage(
                    IN  AuthenticatorInfo               *pAuthMgr,
                    IN  void*                           hObject,
                    IN  RvSipCommonStackObjectType      eObjectType,
                    IN  SipTripleLock                   *pObjTripleLock,
                    IN  RvSipAuthenticationHeaderHandle hHeader,
                    OUT RvInt32                         *pNonceCount)
{
    if (NULL != pAuthMgr->evHandlers.pfnNonceCountUsageAuthenticationHandler)
    {
        RvInt32     nestedLock = 0;
        RvThreadId  currThreadId = 0;
        RvInt32     threadIdLockCount = 0;
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackNonceCountUsage - Before callback. nonceCount=%d", *pNonceCount));
        
        if (NULL != pObjTripleLock)
        {
            SipCommonUnlockBeforeCallback(pAuthMgr->pLogMgr,pAuthMgr->pLogSrc,
                pObjTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        pAuthMgr->evHandlers.pfnNonceCountUsageAuthenticationHandler(
            (RvSipAuthenticatorHandle)pAuthMgr, pAuthMgr->hAppAuthHandle,
            hObject, &eObjectType, hHeader, pNonceCount);
        
        if (NULL != pObjTripleLock)
        {
            SipCommonLockAfterCallback(pAuthMgr->pLogMgr,pObjTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackNonceCountUsage - After callback. nonceCount=%d", *pNonceCount));
    }
    else
    {
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackNonceCountUsage - callback is not set"));
    }
}

/******************************************************************************
 * AuthenticatorCallbackAuthorizationReady
 * ----------------------------------------------------------------------------
 * General: Notifies the application about value of the nonceCount parameter,
 *          which the Stack is going to use, while calculating credentials.
 *          The Application can change this value in order to fit more
 *          precious management of the nonceCount. The Stack doesn't check
 *          the uniquity of the used NONCE in the realm or by the different
 *          object.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pAuthMgr       - Pointer to the Authenticator object
 *          hObject        - Handle to the Object, that is served
 *                           by the Authenticator(e.g.CallLeg,RegClient)
 *          peObjectType   - Pointer to the variable that stores the type
 *                           of the hObject.
 *                           Use following code to get the type:
 *                           RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pObjTripleLock - Pointer to the triple lock, which should be
 *                           unlocked before control passes to the application.
 *          hHeader        - Handle to the Authentication header,
 *                           containing challenge, credentials for which
 *                           are being prepared.
 *          pNonceCount    - pointer to the nonceCount value, managed by
 *                           the Stack per Challenge.
 * Output:  pNonceCount    - pointer to the nonceCount value, set by
 *                           the Application in order to be used by the Stack
 *                           for Credentials calculation.
 ***************************************************************************/
void AuthenticatorCallbackAuthorizationReady(
                    IN  AuthenticatorInfo               *pAuthMgr,
                    IN  void*                           hObject,
                    IN  RvSipCommonStackObjectType      eObjectType,
                    IN  SipTripleLock                   *pObjTripleLock,
                    IN  AuthObj                         *pAuthObj,
                    IN  RvSipAuthorizationHeaderHandle  hHeader)
{
    if (NULL != pAuthMgr->evHandlers.pfnAuthorizationReadyAuthenticationHandler)
    {
        RvInt32     nestedLock = 0;
        RvThreadId  currThreadId = 0;
        RvInt32     threadIdLockCount = 0;
        void*       pContext = pAuthObj->pContext;
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackAuthorizationReady - Before callback. authObj=0x%p header=0x%p", 
            pAuthObj, hHeader));
        
        if (NULL != pObjTripleLock)
        {
            SipCommonUnlockBeforeCallback(pAuthMgr->pLogMgr,pAuthMgr->pLogSrc,
                pObjTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        }

        pAuthMgr->evHandlers.pfnAuthorizationReadyAuthenticationHandler(
                                (RvSipAuthenticatorHandle)pAuthMgr, pAuthMgr->hAppAuthHandle,
                                hObject, &eObjectType, 
                                (RvSipAuthObjHandle)pAuthObj, 
                                pContext,
                                hHeader);
        
        if (NULL != pObjTripleLock)
        {
            SipCommonLockAfterCallback(pAuthMgr->pLogMgr,pObjTripleLock,nestedLock,currThreadId,threadIdLockCount);
        }
        
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackAuthorizationReady - After callback. authObj=0x%p header=0x%p", 
            pAuthObj, hHeader));
    }
    else
    {
        RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorCallbackAuthorizationReady - callback is not set"));
    }
}

#endif /* #ifdef RV_SIP_AUTH_ON */


