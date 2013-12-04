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
 *                              SipAuthenticator.c
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

#include "_SipAuthenticator.h"
#include "AuthenticatorCallbacks.h"
#include "AuthenticatorHighAvailability.h"
#include "RvSipMsgTypes.h"
#include "RvSipAuthenticationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "AuthenticatorObject.h"
#include "RvSipAuthenticator.h"
#include "_SipAuthenticationHeader.h"
#include "_SipAuthenticator.h"
#include "RvSipAddress.h"
#include "AdsRpool.h"
#include "rvmemory.h"
#include <stdio.h>
#include <string.h>
#include "_SipMsg.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_AUTH_ON
static void RVCALLCONV InitiateActOnCallbackStructure(IN  AuthenticatorInfo    *pAuthenticator);
#endif /*#ifdef RV_SIP_AUTH_ON*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/* --------- A U T H E N T I C A T O R   M A N A G E R   F U N C T I O N S  -----*/

/************************************************************************************
 * SipAuthenticationConstruct
 * ----------------------------------------------------------------------------------
 * General: construct and initiate the authentication module.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   regId         - log registration number, used for log printing
 *          hLog          - handle to the log
 *          hGeneralPool  - handle to the general pool.
 *          hElementPool  - handle to the element pool.
 *          hStack        - pointer to the stack
 * Output:  hAuth         - handle to the authentication module.
 ***********************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorConstruct(IN RvSipMsgMgrHandle         hMsgMgr,
                                               IN RvLogMgr*                 pLogMgr,
                                               IN RvLogSource*              regId,
                                               IN HRPOOL                    hGeneralPool,
                                               IN HRPOOL                    hElementPool,
                                               IN void*                     hStack,
                                               OUT RvSipAuthenticatorHandle *hAuth)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus rv;
    AuthenticatorInfo *authInfo;

    *hAuth = NULL;
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(AuthenticatorInfo), pLogMgr, (void*)&authInfo))
    {
        return RV_ERROR_UNKNOWN;
    }
    memset(authInfo, 0, sizeof(AuthenticatorInfo));
    authInfo->pLogMgr = pLogMgr;
    authInfo->pLogSrc = regId;
    authInfo->hGlobalHeaderPage = NULL_PAGE;
    authInfo->hGeneralPool = hGeneralPool;
    authInfo->hElementPool = hElementPool;
    authInfo->hMsgMgr = hMsgMgr;
    authInfo->hStack = hStack;
	InitiateActOnCallbackStructure(authInfo);
	 
    rv = RvMutexConstruct(authInfo->pLogMgr, &authInfo->lock);
    if(RV_OK != rv)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "SipAuthenticatorConstruct - Failed initializing mutex"));
        RvMemoryFree(authInfo, authInfo->pLogMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }

    *hAuth = (RvSipAuthenticatorHandle)authInfo;
    RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
              "SipAuthenticatorConstruct - The authentication module was constructed successfully"));
    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hMsgMgr);
    RV_UNUSED_ARG(pLogMgr);
    RV_UNUSED_ARG(regId);
    RV_UNUSED_ARG(hGeneralPool || hElementPool);
    RV_UNUSED_ARG(hStack);

    *hAuth = NULL;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/************************************************************************************
 * SipAuthenticatorDestruct
 * ----------------------------------------------------------------------------------
 * General: Destruct the Authentication module and release the used space.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth          - Handle to authentication module
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorDestruct(IN RvSipAuthenticatorHandle hAuth)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo *authInfo;
    RvLogSource*      pLogSrc;
    RvStatus          rv;

    authInfo = (AuthenticatorInfo*)hAuth;
    if (authInfo == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    pLogSrc = authInfo->pLogSrc;

    rv = RvMutexDestruct(&authInfo->lock, authInfo->pLogMgr);
    if(RV_OK != rv)
    {
        RvLogError(authInfo->pLogSrc,(authInfo->pLogSrc,
            "SipAuthenticatorDestruct - Failed destructing mutex"));
    }

    RvMemoryFree(authInfo, authInfo->pLogMgr);
    RvLogInfo(pLogSrc,(pLogSrc,
              "SipAuthenticatorDestruct - the authentication module was destructed successfully"));

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/* --------- C L I E N T   A U T H   F U N C T I O N S ---------------*/
/***************************************************************************
 * SipAuthenticatorValidityChecking
 * ------------------------------------------------------------------------
 * General: The function checks parameters in the authentication header like
 *          qop, algorithm and scheme and decides if the authorization header can be
 *          built.
 * Return Value: RV_OK          - If valid.
 *               RV_ERROR_UNKNOWN          - invalid parameters, so the authorization can not
 *                                     be built.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hAuthMgr - handle to the authenticator manager.
 *        hAuth    - handle to the authentication header.
 * Output:  none
 ***************************************************************************/
RvStatus SipAuthenticatorValidityChecking(
                                        IN  RvSipAuthenticatorHandle         hAuthMgr,
                                        IN  RvSipAuthenticationHeaderHandle  hAuth)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo    *authInfo;
    RvSipAuthAlgorithm   algorithm;
    RvSipAuthQopOption   qopOptions;
    RvSipAuthScheme      authScheme;
    RvInt32              akaVersion;

    authInfo = (AuthenticatorInfo*)hAuthMgr;
    if (authInfo == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if(hAuth == NULL)
        return RV_ERROR_UNKNOWN;

    /* we first check if user registered on authenticator callbacks.
        If he did - then we can move on, and let him know about this specific response
        (401 or 407), else we return RV_ERROR_UNKNOWN, and it will be treated as a regular
        reject */
    if(RV_FALSE == AuthenticatorActOnCallback(authInfo, AUTH_CALLBACK_MD5) ||
	   RV_FALSE == AuthenticatorActOnCallback(authInfo, AUTH_CALLBACK_GET_SHARED_SECRET))
    {
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorValidityChecking - user did not set the authenticator events"));
        return RV_ERROR_UNKNOWN;
    }

    authScheme = RvSipAuthenticationHeaderGetAuthScheme(hAuth);

    algorithm  = RvSipAuthenticationHeaderGetAuthAlgorithm(hAuth);

    qopOptions = RvSipAuthenticationHeaderGetQopOption(hAuth);

    akaVersion = RvSipAuthenticationHeaderGetAKAVersion(hAuth);

    return SipAuthenticatorIsHeaderSupported(hAuthMgr,
                                             algorithm,
                                             qopOptions,
                                             authScheme,
                                             akaVersion);
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth);
    RV_UNUSED_ARG(hAuthMgr);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}



/***************************************************************************
 * SipAuthenticatorIsHeaderSupported
 * ------------------------------------------------------------------------
 * General: The function checks parameters in the authorization header like
 *          qop, algorithm and scheme and decides if we can authenticate it.
 * Return Value: RV_OK          - If valid.
 *               RV_ERROR_UNKNOWN          - invalid parameters, so the authorization can not
 *                                     be built.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hAuthMgr - handle to the authenticator manager.
 *        hAuth    - handle to the authentication header.
 * Output:  none
 ***************************************************************************/
RvStatus SipAuthenticatorIsHeaderSupported(RvSipAuthenticatorHandle hAuthMgr,
                                           RvSipAuthAlgorithm   algorithm,
                                           RvSipAuthQopOption   qopOptions,
                                           RvSipAuthScheme      authScheme,
                                           RvInt32              akaVersion)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo* authInfo = (AuthenticatorInfo*)hAuthMgr;

    if (authScheme != RVSIP_AUTH_SCHEME_DIGEST)
    {
         RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorIsHeaderSupported - only Digest Scheme is supported"));
         return RV_ERROR_BADPARAM;
    }
    if (algorithm != RVSIP_AUTH_ALGORITHM_MD5 && algorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
         RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorIsHeaderSupported - only MD5 algorithm is supported."));
         return RV_ERROR_BADPARAM;
    }
    if (qopOptions == RVSIP_AUTH_QOP_OTHER)
    {
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorIsHeaderSupported - the qop is 'other' which is not supported"));
        return RV_ERROR_BADPARAM;
    }
    if(akaVersion > 0)
    {
#ifndef RV_SIP_IMS_ON
        /* AKA-MD5 is not supported without IMS compilation */
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
            "SipAuthenticatorIsHeaderSupported - AKAv1-MD5 algorithm is not supported without IMS add-on."));
        return RV_ERROR_BADPARAM;
#endif
    }
    if (RVSIP_AUTH_QOP_AUTHINT_ONLY == qopOptions &&
        RV_FALSE == AuthenticatorActOnCallback(authInfo, AUTH_CALLBACK_MD5_ENTITY_BODY))
    {
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
            "SipAuthenticatorIsHeaderSupported - the qop is 'auth-int', but MD5EntityBodyAuthentication callback is not set"));
        return RV_ERROR_BADPARAM;
    }

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(algorithm);
    RV_UNUSED_ARG(qopOptions);
    RV_UNUSED_ARG(authScheme || akaVersion);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/******************************************************************************
 * SipAuthenticatorMsgGetMethodStr
 * ----------------------------------------------------------------------------
 * General: extracts RvSipMethodType from the message and converts it to string
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg      - handle to the message.
 * Output:  strMethod - buffer, where the function will store the string
 ***************************************************************************/
void SipAuthenticatorMsgGetMethodStr(IN  RvSipMsgHandle hMsg,
                                     OUT RvChar*        strMethod)
{
#ifdef RV_SIP_AUTH_ON
    RvSipMethodType eMethodType;

    /* Extract method from the Message */
    eMethodType = RvSipMsgGetRequestMethod(hMsg);
    /* Convert the method into string */
    switch(eMethodType)
    {
    case RVSIP_METHOD_INVITE:   strcpy(strMethod, "INVITE");
        break;
    case RVSIP_METHOD_BYE:      strcpy(strMethod, "BYE");
        break;
    case RVSIP_METHOD_ACK:      strcpy(strMethod, "ACK");
        break;
    case RVSIP_METHOD_REFER:    strcpy(strMethod, "REFER");
        break;
    case RVSIP_METHOD_NOTIFY:   strcpy(strMethod, "NOTIFY");
        break;
    case RVSIP_METHOD_PRACK:    strcpy(strMethod, "PRACK");
        break;
    case RVSIP_METHOD_CANCEL:   strcpy(strMethod, "CANCEL");
        break;
    case RVSIP_METHOD_SUBSCRIBE:strcpy(strMethod, "SUBSCRIBE");
        break;
    case RVSIP_METHOD_REGISTER: strcpy(strMethod, "REGISTER");
        break;
    case RVSIP_METHOD_UNDEFINED: strcpy(strMethod, "UNDEFINED");
        break;
    case RVSIP_METHOD_OTHER:
        {
            RvChar *pStrMethod;
            pStrMethod = (RvChar*)RPOOL_GetPtr(SipMsgGetPool(hMsg),
                SipMsgGetPage(hMsg),
                SipMsgGetStrRequestMethod(hMsg));
            strcpy(strMethod, pStrMethod);
        }
        break;
    }
    return;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hMsg)
	RV_UNUSED_ARG(strMethod);
    return;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/* -------------  S E R V E R   A U T H E N T I C A T I O N ---------------- */
/******************************************************************************
 * SipAuthenticatorVerifyCredentials
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
RvStatus RVCALLCONV SipAuthenticatorVerifyCredentials(
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
#ifdef RV_SIP_AUTH_ON
    return AuthenticatorVerifyCredentials(hAuth, hAuthorization, password, strMethod, 
                                           hObject, eObjectType, pObjTripleLock, hMsg, pbIsCorrect);
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hMsg); 
    RV_UNUSED_ARG(pObjTripleLock);
    RV_UNUSED_ARG(eObjectType);
    RV_UNUSED_ARG(hObject);
    RV_UNUSED_ARG(hAuth);
    RV_UNUSED_ARG(hAuthorization);
    RV_UNUSED_ARG(password);
    RV_UNUSED_ARG(strMethod);
    *pbIsCorrect = RV_FALSE;
    
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */

}

/***************************************************************************
 * SipAuthenticatorCredentialsSupported
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
RvStatus RVCALLCONV SipAuthenticatorCredentialsSupported(
                                        IN  RvSipAuthenticatorHandle         hAuthMgr,
                                        IN  RvSipAuthorizationHeaderHandle   hAuthoriz,
                                        OUT RvBool*                          bIsSupported)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo   *authInfo;
    RvSipAuthAlgorithm   algorithm;
    RvSipAuthQopOption   qopOptions;
    RvSipAuthScheme      authScheme;
    RvInt32              akaVersion;
    RvStatus tempStat;

    authInfo = (AuthenticatorInfo*)hAuthMgr;
    *bIsSupported = RV_FALSE;

    if (authInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(hAuthoriz == NULL)
    {
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorCredentialsSupported - no authorization header was supplied"));
        return RV_ERROR_INVALID_HANDLE;
    }

    /* we first check if user registered on authenticator callbacks.
        If he did - then we can move on, and let him know about this specific response
        (401 or 407), else we return RV_ERROR_UNKNOWN, and it will be treated as a regular
        reject */
    if(RV_FALSE == AuthenticatorActOnCallback(authInfo, AUTH_CALLBACK_MD5))
    {
        RvLogInfo(authInfo->pLogSrc,(authInfo->pLogSrc,
                  "SipAuthenticatorCredentialsSupported - user did not set the authenticator events"));
        return RV_OK;
    }

    authScheme = RvSipAuthorizationHeaderGetAuthScheme(hAuthoriz);
    qopOptions = RvSipAuthorizationHeaderGetQopOption(hAuthoriz);
    algorithm  = RvSipAuthorizationHeaderGetAuthAlgorithm(hAuthoriz);
    akaVersion = RvSipAuthorizationHeaderGetAKAVersion(hAuthoriz);

    tempStat = SipAuthenticatorIsHeaderSupported(hAuthMgr,
                                                algorithm,
                                                qopOptions,
                                                authScheme,
                                                akaVersion);
    if(tempStat == RV_OK)
    {
        *bIsSupported = RV_TRUE;
        return RV_OK;
    }
    else if(tempStat == RV_ERROR_BADPARAM)
    {
        *bIsSupported = RV_FALSE;
        return RV_OK;
    }

    return tempStat;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(hAuthoriz);
    *bIsSupported = RV_FALSE;

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}


/* --------- A U T H    O B J   L I S T    F U N C T I O N S -----------------*/

/******************************************************************************
 * SipAuthenticatorAuthListDestruct
 * ----------------------------------------------------------------------------
 * General: destruct list, allocated on the General Pool page. The list
 *          elements (headers) are lost as a result of this operation.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthList  - Handle to the list to be destructed.
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV SipAuthenticatorAuthListDestruct(IN RvSipAuthenticatorHandle  hAuth,
                                                 IN AuthObjListInfo*   pAuthListInfo,
                                                 IN RLIST_HANDLE  hAuthList)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuth;
    RLIST_ITEM_HANDLE hListElement = NULL;
    AuthObj* pAuthObj, *pNextAuthObj = NULL;
    
    /* the list is on the object page - can not destruct it.
       have to go over the list members and free all of them */
    pAuthObj = AuthenticatorGetAuthObjFromList(hAuthList, RVSIP_FIRST_ELEMENT, &hListElement);
    while(pAuthObj != NULL)
    {
        pNextAuthObj = AuthenticatorGetAuthObjFromList(hAuthList, RVSIP_NEXT_ELEMENT, &hListElement);
        AuthObjDestructAndRemoveFromList(pAuthMgr, hAuthList, pAuthObj);
        pAuthObj = pNextAuthObj;
    }
    if(pAuthListInfo != NULL)
    {
        pAuthListInfo->bListInitiated = RV_FALSE;
    }
#else
    RV_UNUSED_ARG(hAuthList||pAuthListInfo||hAuth);
#endif /*#ifdef RV_SIP_AUTH_ON*/
    return;
}

/******************************************************************************
* SipAuthenticatorAuthListResetPasswords
* ----------------------------------------------------------------------------
* General: goes through list of Authentication headers and reset password to
*          zeroes in each element. The function is used before list
*          destruction in order to prevent password hijacking by memory dump.
* Return Value: none.
* ----------------------------------------------------------------------------
* Arguments:
* Input:   hList  - Handle to the list to be processed.
* Output:  none.
*****************************************************************************/
void RVCALLCONV SipAuthenticatorAuthListResetPasswords(IN RLIST_HANDLE  hList)
{
#ifdef RV_SIP_AUTH_ON
    RLIST_ITEM_HANDLE   hListElement = NULL;
    AuthObj*            pAuthObj;
    
    /* Go through source list and copy its elements (headers) */
    RLIST_GetHead(NULL/*hPoolList*/, hList, &hListElement);
    while (NULL != hListElement)
    {
        pAuthObj = *((AuthObj**)hListElement);
        if (NULL != pAuthObj->hHeader)
        {
            SipAuthenticationHeaderClearPassword(pAuthObj->hHeader);
        }
        /* Get the next Authentication Data structure */
        RLIST_GetNext(NULL/*hPoolList*/, hList, hListElement, &hListElement);
    }
#else
    RV_UNUSED_ARG(hList);
#endif /*#ifdef RV_SIP_AUTH_ON*/
    return;
}

#ifdef RV_SIP_AUTH_ON
/******************************************************************************
 * SipAuthenticatorAuthListGetObj
 * ----------------------------------------------------------------------------
 * General: return a pointer to the next/first AuthObj in the given list.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth     - Handle to the authenticator object.
 *          hAuthList - Handle to the list of the auth objects.
 *          eLocation - location in the list (first/last/next/prev)
 *          hRelative - relative object (in case of NEXT location)
 * Output:  phAuthObj - the required auth-obj.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorAuthListGetObj(IN   RvSipAuthenticatorHandle  hAuth,
                                                   IN   RLIST_HANDLE           hAuthList,
                                                   IN   RvSipListLocation      eLocation, 
                                                   IN   RvSipAuthObjHandle     hRelative,
			                                       OUT  RvSipAuthObjHandle*    phAuthObj)
{
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuth;
    AuthObj* pRelative = (AuthObj*)hRelative;
    RLIST_ITEM_HANDLE hElem = NULL;
	
	if(pAuthMgr == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

    if((eLocation == RVSIP_NEXT_ELEMENT || eLocation == RVSIP_PREV_ELEMENT) &&
        hRelative == NULL)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorAuthListGetObj - can't get next object. no relative object supplied"));
        return RV_ERROR_BADPARAM;
    }

    if(pRelative != NULL)
    {
        hElem = pRelative->hListElement;
    }
    
    *phAuthObj = (RvSipAuthObjHandle)AuthenticatorGetAuthObjFromList(hAuthList, eLocation, &hElem);
    
    
    return RV_OK;
}

/******************************************************************************
 * SipAuthenticatorAuthListRemoveObj
 * ----------------------------------------------------------------------------
 * General: remove one AuthObj in the given list.
 * Return Value: pointer to the auth obj.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthList  - Handle to the list to be destructed.
 *          eLocation  - location in the list (first/last/next/prev)
 * Output:  phListElement - pointer to the relative place in the list.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorAuthListRemoveObj(IN   RvSipAuthenticatorHandle  hAuth,
                                                      IN   RLIST_HANDLE           hAuthList,
                                                      IN   RvSipAuthObjHandle     hAuthObj)
{
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuth;
    AuthObj* pObj = (AuthObj*)hAuthObj;
    
    RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
        "SipAuthenticatorAuthListRemoveObj - remove obj 0x%p from list 0x%p", hAuthObj, hAuthList));

    AuthObjDestructAndRemoveFromList(pAuthMgr, hAuthList, pObj);
    return RV_OK;
}

#endif /*#ifdef RV_SIP_AUTH_ON*/

/******************************************************************************
 * SipAuthenticatorUpdateAuthObjListFromMsg
 * ----------------------------------------------------------------------------
 * General: goes through Authentication headers in the message, prepare
 *          the correspondent Authentication Data structure, and add it into
 *          the List.
 *          If List is not provided, the function creates a new one. 
 *          The list is constructed on the given object page.
 *          each object in the list is constructed on a special page from the
 *          element pool.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth  - Handle to the Authentication Manager object.
 *          hMsg   - Handle to the Message to be searched for the headers.
 *          hPage  - The page if the object holding the authentication list.
 *          pTripleLock - pointer to the lock of the object that is holding this list.
 *          pfnLockAPI  - function to lock the object that is holding this list.
 *          pfnUnLockAPI - function to unlock the object that is holding this list.
 *          phAuthList - Pointer to the list to be updated with
 *                          new Authentication Data structure. Can point to NULL.
 * Output:  ppAuthListInfo - pointer to the auth list info structure.
 *          phAuthList - Pointer to the created list handle.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorUpdateAuthObjListFromMsg(
                    IN    RvSipAuthenticatorHandle  hAuth,
                    IN    RvSipMsgHandle            hMsg,
                    IN    HPAGE                     hPage,
                    IN    void*                     pParentObj,
                    IN    ObjLockFunc               pfnLockAPI,
                    IN    ObjUnLockFunc             pfnUnLockAPI,
                    INOUT AuthObjListInfo*          *ppAuthListInfo,
                    INOUT RLIST_HANDLE              *phAuthList)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus    rv;
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuth;
    RvSipAuthenticationHeaderHandle hHeaderMsg;
    RvSipHeaderListElemHandle       hListElementMsg;
    AuthObj              *pAuthObj;
    
    /* If there is at least one Authentication header in the message,
       construct the the list, if it was not constructed yet */
    hHeaderMsg = (RvSipAuthenticationHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_AUTHENTICATION, RVSIP_FIRST_HEADER, &hListElementMsg);
    if (NULL == hHeaderMsg)
    {
        /* if there is no authentication header in the message */
        return RV_OK;
    }

    /* build a list, if needed, and update the list */
    if (NULL==*phAuthList)
    {
        rv = SipAuthenticatorConstructAuthObjList(hAuth, hPage, pParentObj, pfnLockAPI, pfnUnLockAPI,ppAuthListInfo, phAuthList);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorUpdateAuthObjListFromMsg - hMsg 0x%p - failed to construct list(rv=%d:%s)",
                hMsg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }   
    }

    /* Go through message's authentication headers and add them to the list */
    while (NULL != hHeaderMsg)
    {
        /* before adding a new object to the list, we check if it a valid header.
        if it is not valid, and the application did not registered on the 'unsupported' callback,
        we won't create an object, to prevent increasing of the page size.
        if it not valid, the list will be marked as not valid anyway... */
        RvBool bCreateObj = RV_TRUE;
        RvBool bHeaderValid = RV_TRUE;

        if (RV_OK != SipAuthenticatorValidityChecking((RvSipAuthenticatorHandle)pAuthMgr, hHeaderMsg))
        {
            bHeaderValid = RV_FALSE;
            (*ppAuthListInfo)->bListIsValid = RV_FALSE;
        }
        
        if( RV_FALSE == bHeaderValid &&
            RV_FALSE == AuthenticatorActOnCallback(pAuthMgr, AUTH_CALLBACK_UNSUPPORTED_CHALLENGE))
        {
            bCreateObj = RV_FALSE;
        }
        
        if(bCreateObj == RV_TRUE)
        {
            rv = AuthObjCreateInList(pAuthMgr, *ppAuthListInfo, *phAuthList, &pAuthObj);
            if (rv != RV_OK)
            {
                RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                    "SipAuthenticatorUpdateAuthObjListFromMsg - hMsg 0x%p - failed to create authObj for authentication header 0x%p (rv=%d:%s)",
                    hMsg, hHeaderMsg, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            rv = AuthObjSetParams(pAuthMgr, *phAuthList, pAuthObj, hHeaderMsg, NULL, bHeaderValid);
            if (rv != RV_OK)
            {
                RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                    "SipAuthenticatorUpdateAuthObjListFromMsg - hMsg 0x%p - failed to set params in authObj 0x%p (rv=%d:%s)",
                    hMsg, pAuthObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
        }
        else
        {
            RvLogInfo(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                    "SipAuthenticatorUpdateAuthObjListFromMsg - hMsg 0x%p - header 0x%p is invalid. mark the list as invalid. do not create auth obj",
                    hMsg, hHeaderMsg));
        }

        
        /* get the next authentication header from message */
        hHeaderMsg = (RvSipAuthenticationHeaderHandle)RvSipMsgGetHeaderByType(
                    hMsg, RVSIP_HEADERTYPE_AUTHENTICATION, RVSIP_NEXT_HEADER,
                    &hListElementMsg);
    } /* ENDOF: while (NULL != hHeaderMsg) */

    return RV_OK;

#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth)
	RV_UNUSED_ARG(hMsg || hPage || pParentObj)
    RV_UNUSED_ARG(phAuthList || ppAuthListInfo || pfnUnLockAPI || pfnLockAPI)
    
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipAuthenticatorUpdateAuthObjListFromHeader
 * ----------------------------------------------------------------------------
 * General: Receives a single Authentication header and prepares the correspondent 
 *          Authentication Data structure, and adds it into the List.
 *          If qop or algorithm are received, they run over the given header.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth         - Handle to the Authentication Manager object.
 *          pAuthListInfo - Pointer to the Auth list info structure.
 *          hAuthList     - Pointer to the Auth list handle.
 *          hHeader       - Pointer to the header
 *          eQop          - Qop enumeration for overriding the existing one
 *          pQop          - Qop string for override the existing one
 *          eAlgorithm    - Algorithm enumeration to override the existing one
 *          pAlgorithm    - Algorithm string to override the existing one
 *          akaVersion    - Aka version to ovveride the ecisting one
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorUpdateAuthObjListFromHeader(
                    IN    RvSipAuthenticatorHandle          hAuth,
                    IN    AuthObjListInfo                  *pAuthListInfo,
                    IN    RLIST_HANDLE                      hAuthList,
					IN    RvSipAuthenticationHeaderHandle   hHeader,
					IN    RvSipAuthQopOption                eQop,
					IN    RPOOL_Ptr                        *pQop,
					IN    RvSipAuthAlgorithm                eAlgorithm,
					IN    RPOOL_Ptr                        *pAlgorithm,
					IN    RvInt32                           akaVersion)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus            rv;
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuth;
    AuthObj            *pAuthObj;
	
	/* Create Auth-obj */
	rv = AuthObjCreateInList(pAuthMgr, pAuthListInfo, hAuthList, &pAuthObj);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
					"SipAuthenticatorUpdateAuthObjListFromHeader - Auth-list 0x%p - Failed to create authObj for authentication header 0x%p (rv=%d:%s)",
					hAuthList, hHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
       
	/* Set header parameters */
	rv = AuthObjSetHeader(pAuthMgr, hAuthList, pAuthObj, hHeader, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
					"SipAuthenticatorUpdateAuthObjListFromHeader - Auth-list 0x%p - Failed to set header 0x%p (rv=%d:%s)",
				   hAuthList, hHeader, rv, SipCommonStatus2Str(rv)));
		return rv;
	}

	/* Run over qop */
	if (eQop != RVSIP_AUTH_QOP_UNDEFINED)
	{
		rv = RvSipAuthenticationHeaderSetQopOption(pAuthObj->hHeader, eQop);
		if (RV_OK == rv && RVSIP_AUTH_QOP_OTHER == eQop)
		{
			rv = SipAuthenticationHeaderSetStrQop(pAuthObj->hHeader, NULL, pQop->hPool, pQop->hPage, pQop->offset);
		}
		if (RV_OK != rv)
		{
			RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
						"SipAuthenticatorUpdateAuthObjListFromHeader - Auth-obj 0x%p - Failed to set header qop option (rv=%d:%s)",
					   pAuthObj, rv, SipCommonStatus2Str(rv)));
			return rv;
		}
	}

	/* Run over algorithm */
	if (eAlgorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
	{
		rv = SipAuthenticationHeaderSetAuthAlgorithm(pAuthObj->hHeader, eAlgorithm, NULL, pAlgorithm->hPool, pAlgorithm->hPage, pAlgorithm->offset);
		if (RV_OK != rv)
		{
			RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
						"SipAuthenticatorUpdateAuthObjListFromHeader - Auth-obj 0x%p - Failed to set header algorithm (rv=%d:%s)",
					   pAuthObj, rv, SipCommonStatus2Str(rv)));
			return rv;
		}

		rv = RvSipAuthenticationHeaderSetAKAVersion(pAuthObj->hHeader, akaVersion);
		if (RV_OK != rv)
		{
			RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
						"SipAuthenticatorUpdateAuthObjListFromHeader - Auth-obj 0x%p - Failed to set header Aka version (rv=%d:%s)",
					   pAuthObj, rv, SipCommonStatus2Str(rv)));
			return rv;
		}
	}

	/* Check validity */
    rv = SipAuthenticatorValidityChecking((RvSipAuthenticatorHandle)pAuthMgr, pAuthObj->hHeader);
    if (RV_OK == rv)
    {
        pAuthObj->bValid = RV_TRUE;
    }
    else
    {
        pAuthObj->bValid = RV_FALSE;
        /* Reset validity for whole list */
        pAuthListInfo->bListIsValid = RV_FALSE;
    }
	
	return RV_OK;

#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuth)
	RV_UNUSED_ARG(pAuthListInfo)
    RV_UNUSED_ARG(hHeader)
	RV_UNUSED_ARG(eQop)
	RV_UNUSED_ARG(pQop)
	RV_UNUSED_ARG(eAlgorithm)
	RV_UNUSED_ARG(pAlgorithm)
    
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #ifdef RV_SIP_AUTH_ON */
}
#endif /*# ifdef RV_SIP_IMS_ON */

/******************************************************************************
 * SipAuthenticatorBuildAuthorizationListInMsg
 * ----------------------------------------------------------------------------
 * General: goes through the list of Auth Objects and
 *          for each structure prepares credentials. The credentials are set
 *          into the Authorization header, built in the message.
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
RvStatus RVCALLCONV SipAuthenticatorBuildAuthorizationListInMsg(
                        IN RvSipAuthenticatorHandle   hAuthMgr,
                        IN RvSipMsgHandle             hMsg,
                        IN void                       *hObject,
                        IN RvSipCommonStackObjectType eObjectType,
                        IN SipTripleLock              *pObjTripleLock,
                        IN AuthObjListInfo*           pAuthListInfo,
                        IN RLIST_HANDLE               hAuthList)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus rv;
    AuthenticatorInfo               *pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    RvChar                          strMethod[SIP_METHOD_LEN] = {'\0'};
    RLIST_ITEM_HANDLE               hListElement = NULL;
    AuthObj                         *pAuthObj;
    RvSipAddressHandle              hRequestUri;
    RvSipAuthorizationHeaderHandle  hAuthorizationHeader;
    

    /*getting the request-URI from the message*/
    hRequestUri = RvSipMsgGetRequestUri(hMsg);
    if (NULL == hRequestUri)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorBuildAuthorizationListInMsg - hMsg 0x%p, hAuthList 0x%p - RvSipMsgGetRequestUri() failed",
            hMsg, hAuthList));
        return RV_ERROR_UNKNOWN;
    }
    /*getting the method from the message*/
    SipAuthenticatorMsgGetMethodStr(hMsg, strMethod);
    if(0 == strlen(strMethod))
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorBuildAuthorizationListInMsg - hMsg 0x%p, hAuthList 0x%p - MsgGetMethodStr() failed",
            hMsg, hAuthList));
        return RV_ERROR_UNKNOWN;
    }

    /* Set In-Advance Proxy Authentication, if it was set by Application */
    if (NULL != pAuthMgr->hGlobalAuth)
    {
        /* Construct Authorization header */
        rv = RvSipAuthorizationHeaderConstructInMsg(hMsg, RV_FALSE, &hAuthorizationHeader);
        if(rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorBuildAuthorizationListInMsg - hMsg 0x%p - Failed to construct proxy-authorization (rv=%d:%s)",
                hMsg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Prepare the global Proxy-Authorization header */
        rv=AuthenticatorPrepareProxyAuthorization(hAuthMgr,
                                    strMethod, hRequestUri, UNDEFINED/*nonceCount*/, hObject,
                                    eObjectType, pObjTripleLock, hMsg, hAuthorizationHeader);
        if(rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorBuildAuthorizationListInMsg - hMsg 0x%p - Failed to prepare proxy-authorization header 0x%p (rv=%d:%s)",
                hMsg, hAuthorizationHeader, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    
    /* Goes through Call-leg/reg-client list of Authentication headers, build the matching
    Authorization headers, using the message page. */
    if (NULL == hAuthList)
    {
        return RV_OK;
    }

    pAuthObj = AuthenticatorGetAuthObjFromList(hAuthList, RVSIP_FIRST_ELEMENT, &hListElement);
    while (NULL != pAuthObj)
    {
        pAuthListInfo->hCurrProcessedAuthObj = (RvSipAuthObjHandle)pAuthObj;
    
        rv = AuthenticatorBuildAuthorizationForAuthObj(pAuthMgr, pAuthObj, hMsg, hObject, eObjectType, pObjTripleLock, strMethod, hRequestUri);
        if(rv != RV_OK)
        {
            pAuthListInfo->hCurrProcessedAuthObj = NULL;
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorBuildAuthorizationListInMsg - hMsg 0x%p - Failed to build authorization  for auth obj 0x%p (rv=%d:%s)",
                hMsg, pAuthObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Get the next Authentication header */
        pAuthObj = AuthenticatorGetAuthObjFromList(hAuthList, RVSIP_NEXT_ELEMENT, &hListElement);
    } /* ENDOF: while (NULL!=hListElement)*/

    pAuthListInfo->hCurrProcessedAuthObj = NULL;

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
	RV_UNUSED_ARG(hAuthMgr)
	RV_UNUSED_ARG(hMsg)
	RV_UNUSED_ARG(hObject)
	RV_UNUSED_ARG(eObjectType)
	RV_UNUSED_ARG(pObjTripleLock)
	RV_UNUSED_ARG(hAuthList || pAuthListInfo)

	return RV_OK;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/******************************************************************************
 * SipAuthenticatorCheckValidityOfAuthList
 * ----------------------------------------------------------------------------
 * General: returns whether the list of Auth Objects is valid.
 *          the actual validity check per object is done on the construction
 *          of each object (SipAuthenticatorUpdateAuthObjListFromMsg function)
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr - Handle to the Authentication Manager object.
 *          hList    - Handle to the Message to be searched for the headers.
 * Output:  pbValid  - RV_FALSE, if not supported by the Stack data was found,
 *                     and application didn't register to 
 *                     pfnUnsupportedChallengeAuthenticationHandler callback.
 *                     RV_TRUE otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorCheckValidityOfAuthList(
                                        IN  RvSipAuthenticatorHandle hAuthMgr,
                                        IN  AuthObjListInfo          *pAuthListInfo,
                                        IN  RLIST_HANDLE             hList,
                                        OUT RvBool                   *pbValid)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo  *pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    
    *pbValid = RV_FALSE;

    /* If Application will handle unsupported challenges, approve validity */
    if (RV_TRUE == AuthenticatorActOnCallback(pAuthMgr, AUTH_CALLBACK_UNSUPPORTED_CHALLENGE))
    {
        *pbValid = RV_TRUE;
        return RV_OK;
    }
    
    if (NULL == hList)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorCheckValidityOfAuthList - bad parameter - hList is NULL"));
        return RV_ERROR_BADPARAM;
    }

    *pbValid = pAuthListInfo->bListIsValid;

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(hList || pAuthListInfo);
    RV_UNUSED_ARG(pbValid);
    return RV_ERROR_NOTSUPPORTED;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/******************************************************************************
 * SipAuthenticatorConstructAuthObjList
 * ----------------------------------------------------------------------------
 * General: constructs list of the Authentication objects, using the given
 *          page of General Pool.
 * Return Value: RV_OK - on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth          - Handle to authentication module.
 *          hPage          - The page if the object holding the authentication list.
 *          pTripleLock    - pointer to the lock of the object that is holding this list.
 *          pfnLockAPI     - function to lock the object that is holding this list.
 *          pfnUnLockAPI   - function to unlock the object that is holding this list.
 * Output:  ppAuthListInfo - pointer to the list info struct, constructed on the given page.
 *          phAuthList     - Handle to the constructed list
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorConstructAuthObjList(
                                        IN    RvSipAuthenticatorHandle  hAuth,
                                        IN    HPAGE                     hPage,
                                        IN    void*                     pParentObj,
                                        IN    ObjLockFunc               pfnLockAPI,
                                        IN    ObjUnLockFunc             pfnUnLockAPI,
                                        INOUT AuthObjListInfo*          *ppAuthListInfo,
                                        INOUT RLIST_HANDLE              *phAuthList)
{
#ifdef RV_SIP_AUTH_ON
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuth;
    RLIST_HANDLE        hListNew;
    RvInt32             offset;
    
    /* Create the AuthObjListInfo structure on the given page */
    if(*ppAuthListInfo == NULL) /* list info was not constructed yet */
    {
        *ppAuthListInfo = (AuthObjListInfo*)RPOOL_AlignAppend(pAuthMgr->hGeneralPool, hPage,
                                                sizeof(AuthObjListInfo), &offset);
        if (NULL == *ppAuthListInfo)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorConstructAuthObjList - failed to allocate AuthObjListInfo on the page 0x%p, pool 0x%p",
                hPage, pAuthMgr->hGeneralPool));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }
    (*ppAuthListInfo)->bListIsValid = RV_TRUE;
    (*ppAuthListInfo)->hCurrProcessedAuthObj = NULL;
    (*ppAuthListInfo)->pParentObj   = pParentObj;
    (*ppAuthListInfo)->pfnLockAPI   = pfnLockAPI;
    (*ppAuthListInfo)->pfnUnLockAPI = pfnUnLockAPI;

    /* Create list on the given page. the list holds only pointer to the object that is allocated
       on a different page.*/
    hListNew = RLIST_RPoolListConstruct(pAuthMgr->hGeneralPool, hPage,
                                        sizeof(AuthObj*), pAuthMgr->pLogSrc);
    if (NULL == hListNew)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorConstructAuthObjList - failed to construct list on page 0x%p", hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }
    *phAuthList = hListNew;
    (*ppAuthListInfo)->bListInitiated = RV_TRUE;
    
    return RV_OK;
#else
    RV_UNUSED_ARG(hPage)
    RV_UNUSED_ARG(ppAuthListInfo || pParentObj)
    RV_UNUSED_ARG(hAuth)
    RV_UNUSED_ARG(phAuthList)
    RV_UNUSED_ARG(pfnUnLockAPI)
    RV_UNUSED_ARG(pfnLockAPI)
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}

/******************************************************************************
 * SipAuthenticatorConstructAndCopyAuthObjList
 * ----------------------------------------------------------------------------
 * General: Constructs list of the Authentication objects, using the given
 *          page of General Pool.
 *          Copies into it the elements of the source list.
 * Return Value: RV_OK - on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth        - Handle to authentication module
 *          hPage          - The page if the object holding the authentication list.
 *          hListSrc     - Handle to the source list
 *          pTripleLock  - pointer to the lock of the object that is holding this list.
 *          pfnLockAPI   - function to lock the object that is holding this list.
 *          pfnUnLockAPI - function to unlock the object that is holding this list.
 * Output:  ppAuthListInfo - pointer to the list info struct, constructed on the given page.
 *          phList - Handle to the constructed list
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorConstructAndCopyAuthObjList(
                                        IN  RvSipAuthenticatorHandle  hAuth,
                                        IN  HPAGE                     hPage,
                                        IN  RLIST_HANDLE              hListSrc,
                                        IN  AuthObjListInfo*         pAuthListInfoSrc,
                                        IN  void*                    pParentObj,
                                        IN  ObjLockFunc              pfnLockAPI,
                                        IN  ObjUnLockFunc            pfnUnLockAPI,
                                        INOUT AuthObjListInfo*       *ppAuthListInfo,
                                        OUT RLIST_HANDLE             *phList)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus            rv;
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuth;
    RLIST_HANDLE        hListNew;
    RLIST_ITEM_HANDLE   hListElement;
    AuthObj*            pAuthObjSrc;
    AuthObj*            pAuthObjDst;
    
    if (NULL == hListSrc)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorConstructAndCopyAuthObjList - NULL source list"));
        return RV_ERROR_UNKNOWN;
    }

    rv = SipAuthenticatorConstructAuthObjList(hAuth, hPage, pParentObj, pfnLockAPI, pfnUnLockAPI, 
                                              ppAuthListInfo, &hListNew);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorConstructAndCopyAuthObjList - failed to construct list (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    (*ppAuthListInfo)->bListIsValid = pAuthListInfoSrc->bListIsValid;

    /* Go through source list and copy its elements (headers) */
    pAuthObjSrc = AuthenticatorGetAuthObjFromList(hListSrc, RVSIP_FIRST_ELEMENT, &hListElement);
    while (NULL != pAuthObjSrc)
    {
        rv = AuthObjCreateInList(pAuthMgr, *ppAuthListInfo, hListNew, &pAuthObjDst);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorConstructAndCopyAuthObjList - hListSrc 0x%p - failed to create new authOnj for header 0x%p to list 0x%p (rv=%d:%s)",
                hListSrc, pAuthObjSrc->hHeader, hListNew, rv, SipCommonStatus2Str(rv)));
            SipAuthenticatorAuthListDestruct(hAuth, *ppAuthListInfo, hListNew);
            return rv;
        }
        rv = AuthObjSetParams(pAuthMgr, hListNew, pAuthObjDst, pAuthObjSrc->hHeader, NULL, pAuthObjSrc->bValid);

        pAuthObjDst->nonceCount= pAuthObjSrc->nonceCount;
        
        /* Get the next Authentication Data structure */
        pAuthObjSrc = AuthenticatorGetAuthObjFromList(hListSrc, RVSIP_NEXT_ELEMENT, &hListElement);
    }
    *phList = hListNew;

    return RV_OK;
#else
    RV_UNUSED_ARG(hAuth || ppAuthListInfo || pfnUnLockAPI || pfnLockAPI);
    RV_UNUSED_ARG(hListSrc || hPage || pParentObj);
    RV_UNUSED_ARG(phList || pAuthListInfoSrc);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}

/* ------- FUNCTIONS FOR SENDING CORRECT AUTHORIZATION LIST IN ACK REQUEST
           NOT THE SAME LIST AS THE AUTH-OBJ LIST ----------------------------*/
/******************************************************************************
 * SipAuthenticatorInviteDestructAuthorizationList
 * ----------------------------------------------------------------------------
 * General: destruct the list - just free the page 
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth  - Handle to the Authentication Manager object.
 *          hListAuthorizationHeaders - List of Authorization headers.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorInviteDestructAuthorizationList(
                        IN RvSipAuthenticatorHandle hAuth,
                        IN RLIST_HANDLE             hListAuthorizationHeaders)
{
#ifdef RV_SIP_AUTH_ON	
    HRPOOL hPool;
    HPAGE  hPage;
    AuthenticatorInfo* pAuthMgr = (AuthenticatorInfo*)hAuth;

	if(pAuthMgr == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
    RLIST_RPoolListGetPoolAndPage(hListAuthorizationHeaders, &hPool, &hPage);
    
    RvLogDebug(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
        "SipAuthenticatorInviteDestructAuthorizationList - destruct list 0x%p - free page 0x%p",
        hListAuthorizationHeaders, hPage));
        
    RPOOL_FreePage(hPool, hPage);
#else
	RV_UNUSED_ARG(hAuth || hListAuthorizationHeaders)
#endif /*#ifdef RV_SIP_AUTH_ON	*/
    return RV_OK;
}
/******************************************************************************
 * SipAuthenticatorInviteMoveAuthorizationListIntoMsg
 * ----------------------------------------------------------------------------
 * General: goes through list of Authorization headers and copy them into
 *          the message. Destructs list (free the page) after this.
 *          (The function is only relevant for invite object, that push the list
 *          to the ACK message)
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth  - Handle to the Authentication Manager object.
 *          hMsg   - Handle to the Message to be filled with the headers.
 *          hListAuthorizationHeaders - List of Authorization headers.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorInviteMoveAuthorizationListIntoMsg(
                        IN RvSipAuthenticatorHandle hAuth,
                        IN RvSipMsgHandle           hMsg,
                        IN RLIST_HANDLE             hListAuthorizationHeaders)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus                       rv;
    RvSipAuthorizationHeaderHandle hAuthorizationHeader;
    RLIST_ITEM_HANDLE              hListElement;
    AuthenticatorInfo*             pAuthMgr = (AuthenticatorInfo*)hAuth;

	if(pAuthMgr == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
    /* Goes through CallLeg list of Authorization headers, build their copies
    into the Message object. */
    RLIST_GetHead(NULL/*hPoolList*/, hListAuthorizationHeaders, &hListElement);
    while (NULL != hListElement)
    {
        /* Construct the authorization header in the message*/
        rv = RvSipAuthorizationHeaderConstructInMsg(hMsg,
                        RV_FALSE/*pushHeaderAtHead*/, &hAuthorizationHeader);
        if(rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorInviteMoveAuthorizationListIntoMsg - hMsg 0x%p - Failed to construct header in the message(rv=%d:%s)",
                hMsg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Copy the header, stored in CallLeg object into to the Message obj */
        rv = RvSipAuthorizationHeaderCopy(hAuthorizationHeader,
            *((RvSipAuthorizationHeaderHandle*)hListElement));
        if(rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorInviteMoveAuthorizationListIntoMsg - hMsg 0x%p - Failed to copy header 0x%p into header 0x%p (rv=%d:%s)",
                hMsg, hListElement, hAuthorizationHeader, rv,
                SipCommonStatus2Str(rv)));
            return rv;
        }

        /* Get the next Authentication header */
        RLIST_GetNext(NULL/*hPoolList*/, hListAuthorizationHeaders,
            hListElement, &hListElement);
    }

    /* Free the list of authorization headers */
    SipAuthenticatorInviteDestructAuthorizationList(hAuth, hListAuthorizationHeaders);

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
	RV_UNUSED_ARG(hAuth)
	RV_UNUSED_ARG(hMsg)
	RV_UNUSED_ARG(hListAuthorizationHeaders)

	return RV_OK;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

/******************************************************************************
 * SipAuthenticatorInviteLoadAuthorizationListFromMsg
 * ----------------------------------------------------------------------------
 * General: goes through Authorization headers in the message and add them
 *          to the newly created list of Authorization headers.
 *          The function is relevant only for invite object.
 *          The page is allocated on a dedicated page, and all it's headers
 *          are allocated on the same page.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth  - Handle to the Authentication Manager object.
 *          hMsg   - Handle to the Message to be searched for the headers.
 *          phListAuthorizationHeaders - List of Authorization headers to be
 *                   destructed.
 * Output:  phListAuthorizationHeaders - Created List of Authorization headers.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorInviteLoadAuthorizationListFromMsg(
                    IN RvSipAuthenticatorHandle   hAuth,
                    IN RvSipMsgHandle             hMsg,
                    INOUT RLIST_HANDLE            *phListAuthorizationHeaders)
{
#ifdef RV_SIP_AUTH_ON
    HRPOOL hPool;
    HPAGE  hPage;
    RvSipAuthorizationHeaderHandle  hHeaderMsg;
    RvSipHeaderListElemHandle       hListElementMsg;
    AuthenticatorInfo *pAuthMgr = (AuthenticatorInfo*)hAuth;
    RvStatus rv;

    /* O.W. in this function we build an Authorization list, which is different than the
       auth-obj list.
       this list is constructed on a dedicated page, and is used only by an invite object,
       that needs to set the same authorization list to the ACK as for the INIVTE */ 

    /* Destroy list, if exists */
    if (NULL != *phListAuthorizationHeaders)
    {
        /* Get Pool and Page, on which the headers will be constructed */
        rv = RLIST_RPoolListGetPoolAndPage(*phListAuthorizationHeaders, &hPool, &hPage);
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorInviteLoadAuthorizationListFromMsg - hMsg 0x%p - failed to get List 0x%p pool and page (rv=%d:%s)",
                hMsg, *phListAuthorizationHeaders, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        RPOOL_FreePage(hPool, hPage);
    }

    hHeaderMsg = (RvSipAuthorizationHeaderHandle)RvSipMsgGetHeaderByType(
            hMsg, RVSIP_HEADERTYPE_AUTHORIZATION, RVSIP_FIRST_HEADER, &hListElementMsg);
    
    /* O.W. build the authorization headers list, only if there are authorization
       headers in the invite request. */
    if(hHeaderMsg != NULL)
    {
        /* Create / recreate list */
        rv = RPOOL_GetPage(pAuthMgr->hGeneralPool,0/*chained page num*/,&hPage);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorInviteLoadAuthorizationListFromMsg - hMsg 0x%p - failed to get a page from the Authenticator General pool (rv=%d:%s)",
                hMsg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        *phListAuthorizationHeaders = RLIST_RPoolListConstruct(
                                    pAuthMgr->hGeneralPool, hPage, sizeof(void*), pAuthMgr->pLogSrc);
        if (NULL == *phListAuthorizationHeaders)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "SipAuthenticatorInviteLoadAuthorizationListFromMsg - hMsg 0x%p - failed to construct list",
                hMsg));
            RPOOL_FreePage(pAuthMgr->hGeneralPool, hPage);
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* Go through message's authorization headers and add them to the list */
        while (NULL != hHeaderMsg)
        {
            rv = AuthenticatorAuthorizationHeaderListAddHeader(pAuthMgr,
                *phListAuthorizationHeaders, hHeaderMsg, pAuthMgr->hGeneralPool, hPage);
            if (rv != RV_OK)
            {
                RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                    "SipAuthenticatorInviteLoadAuthorizationListFromMsg - hMsg 0x%p - failed to add header 0x%p to list 0x%p (rv=%d:%s)",
                    hMsg, hHeaderMsg, *phListAuthorizationHeaders, rv,
                    SipCommonStatus2Str(rv)));
                RPOOL_FreePage(pAuthMgr->hGeneralPool, hPage);
                return rv;
            }
            
            hHeaderMsg = (RvSipAuthorizationHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_AUTHORIZATION, RVSIP_NEXT_HEADER,
                &hListElementMsg);
        } /* ENDOF: while (NULL != hHeaderMsg) */

    }

    return RV_OK;
#else /* #ifdef RV_SIP_AUTH_ON */
	RV_UNUSED_ARG(hAuth)
	RV_UNUSED_ARG(hMsg)
	RV_UNUSED_ARG(phListAuthorizationHeaders)

	return RV_OK;
#endif /* #ifdef RV_SIP_AUTH_ON */
}

#ifdef RV_SIP_HIGHAVAL_ON
/******************************************************************************
 * SipAuthenticatorHighAvailStoreAuthObj
 * ----------------------------------------------------------------------------
 * General: goes through list of Auth Objects and stores each
 *          structure into string buffer in the High Availability record format
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr - Pointer to the Authenticator.
 *         hAuthObj - Auth obj list to be stored.
 *         maxBuffLen - Size of the buffer.
 *         ppCurrPos  - Pointer to the place in buffer,
 *                    where the structure should be stored.
 *         pCurrLen   - Length of the record, currently stored
 *                    in the buffer.
 * Output: ppCurrPos  - Pointer to the end of the Authentication Data
 *                    list record in the buffer.
 *         pCurrLen   - Length of the record, currently stored in,
 *                    the buffer including currently stored list.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorHighAvailStoreAuthObj(
                            IN    RvSipAuthenticatorHandle hAuthMgr,
                            IN    RLIST_HANDLE             hAuthObj,
                            IN    RvUint32                 maxBuffLen,
                            INOUT RvChar                   **ppCurrPos,
                            INOUT RvUint32                 *pCurrLen)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus            rv;
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuthMgr;

    rv = AuthenticatorHighAvailStoreAuthData(pAuthMgr, hAuthObj,
        maxBuffLen, ppCurrPos, pCurrLen);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorHighAvailStoreAuthObj - hAuthObj 0x%p - failed to store data (rv=%d:%s)",
            hAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
#else
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(hAuthObj);
    RV_UNUSED_ARG(maxBuffLen);
    RV_UNUSED_ARG(ppCurrPos);
    RV_UNUSED_ARG(pCurrLen);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}

/******************************************************************************
 * SipAuthenticatorHighAvailRestoreAuthObj
 * ----------------------------------------------------------------------------
 * General: restores the list of Auth Objects
 *          from the High Availability record buffer into the provided list.
 *          If no list was provided, construct it, using General Pool.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr - Pointer to the Authenticator.
 *         buffer   - Buffer, containing record to be restored.
 *         buffLen  - Length of the record.
 *         hPage          - The page if the object holding the authentication list.
 *         pTripleLock    - pointer to the lock of the object that is holding this list.
 *         pfnLockAPI     - function to lock the object that is holding this list.
 *         pfnUnLockAPI   - function to unlock the object that is holding this list.
 *         phListAuthObj - list to be used for restoring.
 * Output: ppAuthListInfo - pointer to the list info struct, constructed on the given page.
 *         phListAuthObj - list, used for restoring.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorHighAvailRestoreAuthObj(
                        IN    RvSipAuthenticatorHandle hAuthMgr,
                        IN    RvChar*                  buffer,
                        IN    RvUint32                 buffLen,
                        IN    HPAGE                    hPage,
                        IN    void                    *pParentObj,
                        IN    ObjLockFunc              pfnLockAPI,
                        IN    ObjUnLockFunc            pfnUnLockAPI,
                        INOUT AuthObjListInfo*        *ppAuthListInfo,
                        INOUT RLIST_HANDLE*            phListAuthObj)
{  
#ifdef RV_SIP_AUTH_ON
    RvStatus            rv;
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    
    rv = AuthenticatorHighAvailRestoreAuthData(pAuthMgr, buffer, buffLen, hPage,
                                               pParentObj, pfnLockAPI, pfnUnLockAPI,
                                               ppAuthListInfo ,phListAuthObj);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorHighAvailRestoreAuthObj - failed to restore (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
#else
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(buffer);
    RV_UNUSED_ARG(buffLen || hPage || pParentObj);
    RV_UNUSED_ARG(phListAuthObj || ppAuthListInfo || pfnUnLockAPI || pfnLockAPI);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 

#ifndef RV_SIP_PRIMITIVES
#if defined(RV_SIP_HIGH_AVAL_3_0) && defined(RV_SIP_HIGHAVAL_ON)
/******************************************************************************
 * SipAuthenticatorHighAvailRestoreAuthObj3_0
 * ----------------------------------------------------------------------------
 * General: restores the authentication data from the High Availability record
 *          buffer prepared by the SipTK v.3.0, into the provided list.
 *          If no list was provided, construct it, using General Pool.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr - Pointer to the Authenticator.
 *         buffer   - Buffer, containing record to be restored.
 *         wwwAuthHeaderOffset   - offset of the WWW-Authenticate header
 *                                 in the record.
 *         nonceCount401         - nonceCount, last used for WWW-Authentica...
 *         proxyAuthHeaderOffset - offset of the Authenticate header
 *                                 in the record.
 *         nonceCount407         - nonceCount, last used for Proxy Authenti...
 *         hPage          - The page if the object holding the authentication list.
 *         pTripleLock    - pointer to the lock of the object that is holding this list.
 *         pfnLockAPI     - function to lock the object that is holding this list.
 *         pfnUnLockAPI   - function to unlock the object that is holding this list.
 *         phListAuthObj  - list to be used for restoring.
 * Output: ppAuthListInfo - pointer to the list info struct, constructed on the given page.
 *         phListAuthObj  - list, used for restoring.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorHighAvailRestoreAuthObj3_0(
                        IN    RvSipAuthenticatorHandle hAuthMgr,
                        IN    RvChar*                  buffer,
                        IN    RvInt32                  wwwAuthHeaderOffset,
                        IN    RvInt32                  nonceCount401,
                        IN    RvInt32                  proxyAuthHeaderOffset,
                        IN    RvInt32                  nonceCount407,
                        IN    HPAGE                    hPage,
                        IN    void*                    pParentObj,
                        IN    ObjLockFunc               pfnLockAPI,
                        IN    ObjUnLockFunc             pfnUnLockAPI,
                        INOUT AuthObjListInfo*        *ppAuthListInfo,
                        INOUT RLIST_HANDLE             *phAuthObj)
{
#if defined(RV_SIP_AUTH_ON)
    RvStatus            rv;
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    
    rv = AuthenticatorHighAvailRestoreAuthData3_0(pAuthMgr, buffer,
        wwwAuthHeaderOffset, nonceCount401, proxyAuthHeaderOffset,
        nonceCount407,hPage, pParentObj, pfnLockAPI, pfnUnLockAPI ,ppAuthListInfo,phAuthObj);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorHighAvailRestoreAuthObj3_0 - failed to restore (rv=%d:%s)",
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
#else
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(buffer);
    RV_UNUSED_ARG(wwwAuthHeaderOffset);
    RV_UNUSED_ARG(nonceCount401);
    RV_UNUSED_ARG(proxyAuthHeaderOffset);
    RV_UNUSED_ARG(nonceCount407);
    RV_UNUSED_ARG(phAuthObj);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}
#endif /* #ifdef  RV_SIP_HIGH_AVAL_3_0 && RV_SIP_HIGHAVAL_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES*/




#ifdef RV_SIP_HIGHAVAL_ON
/******************************************************************************
 * SipAuthenticatorHighAvailGetAuthObjStorageSize
 * ----------------------------------------------------------------------------
 * General: calculates the size of buffer consumed by the list of
 *          Auth Objects in High Availability record format.
 *          For details, see High Availability module documentation.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthObj - Handle of the list.
 * Output: pStorageSize        - Requested size.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorHighAvailGetAuthObjStorageSize(
                            IN  RvSipAuthenticatorHandle hAuthMgr,
                            IN  RLIST_HANDLE             hAuthObj,
                            OUT RvInt32                  *pStorageSize)
{
#ifdef RV_SIP_AUTH_ON
    RvStatus            rv;
    AuthenticatorInfo*  pAuthMgr = (AuthenticatorInfo*)hAuthMgr;
    
    rv = AuthenticatorHighAvailGetAuthDataStorageSize(
        pAuthMgr, hAuthObj, pStorageSize);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "SipAuthenticatorHighAvailGetAuthObjStorageSize - hAuthData 0x%p - failed to get storage size (rv=%d:%s)",
            hAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    return RV_OK;
#else
    RV_UNUSED_ARG(hAuthMgr);
    RV_UNUSED_ARG(hAuthObj);
    RV_UNUSED_ARG(pStorageSize);
    return RV_ERROR_NOTSUPPORTED;
#endif /*#ifdef RV_SIP_AUTH_ON*/
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 


#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/******************************************************************************
 * InitiateActOnCallbackStructure
 * ----------------------------------------------------------------------------
 * General: Indicates for each callback whether the authenticator should consider this
 *			callback when deciding whether to apply default behavior. As default, the
 *			authenticator has these values set to RV_TRUE, indicating that if a callback
 *			was set by the application, then it overrides the need in automatic behavior.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthenticator - Pointer to the Authenticator
 * Output: none
 *****************************************************************************/
static void RVCALLCONV InitiateActOnCallbackStructure(IN  AuthenticatorInfo    *pAuthenticator)
{
	pAuthenticator->actOnCallback.bAuthorizationReadyAuthentication	  = RV_TRUE;
	pAuthenticator->actOnCallback.bGetSharedSecretAuthentication	  = RV_TRUE;
	pAuthenticator->actOnCallback.bMD5Authentication				  = RV_TRUE;
	pAuthenticator->actOnCallback.bMD5AuthenticationEx				  = RV_TRUE;
	pAuthenticator->actOnCallback.bMD5EntityBodyAuthentication		  = RV_TRUE;
	pAuthenticator->actOnCallback.bNonceCountUsageAuthentication	  = RV_TRUE;
	pAuthenticator->actOnCallback.bSharedSecretAuthentication		  = RV_TRUE;
	pAuthenticator->actOnCallback.bUnsupportedChallengeAuthentication = RV_TRUE;
}

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
/***************************************************************************
 * SipAuthenticatorGetAppAuthHandle
 * ------------------------------------------------------------------------
 * General:  Figures out the application authentication handle
 *           given an application handle for CallLeg or Register-Client
 *           by using the application callback function.
 * Return Value: application handle for Authentication process.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr    - handle to the authenticator manager
 *          appHandle   - application handle for CallLeg or Register-Client
 *          isRegClient - RV_TRUE means hAppRegClient is given,
 *                        RV_FALSE - hAppCallLeg
 ***************************************************************************/
RvSipAppAuthenticationHandle
          SipAuthenticatorGetAppAuthHandle(
                                        IN  RvSipAuthenticatorHandle         hAuthMgr,
                                        IN  void                             *appHandle,
                                        IN  RV_BOOL                          isRegClient)
{
    AuthenticatorInfo *pAuthInfo;
    pAuthInfo = (AuthenticatorInfo *)hAuthMgr;

    if (pAuthInfo->evHandlers.pfnGetAppAuthenticationHandle != NULL &&
                                                     appHandle != NULL)
    {
       return (pAuthInfo->evHandlers.pfnGetAppAuthenticationHandle(appHandle,
                                                                   isRegClient));
    }
    else
       return (RvSipAppAuthenticationHandle)NULL;
}

#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */

#endif /* #ifdef RV_SIP_AUTH_ON */

#endif /* #ifndef RV_SIP_LIGHT */


