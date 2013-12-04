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
 *                          AuthenticatorObject.h
 *
 *  This file provides definition for the authentication object.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                      Jan. 2001
 *********************************************************************************/

#ifndef _AUTHENTICATOR_OBJECT_H
#define _AUTHENTICATOR_OBJECT_H


#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "_SipCommonUtils.h"
#include "RvSipAuthenticator.h"
#include "_SipAuthenticator.h"
#include "RvSipAuthenticationHeader.h"

#include "AdsRlist.h"
#include "rvlog.h"
#include "rvmutex.h"

#ifdef RV_SIP_AUTH_ON
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/******************************************************************************
 * Definition of AuthenticatorInfo:
 * evHandlers          - event handlers structure.
 * actOnCallback       - act on callbacks structure
 * hLog                - handle to the log.
 * regId               - register number for the log.
 * hGeneralPool        - handle to the general pool
 * hGlobalHeaderPage   - page for the global authentication header
 * hGlobalAuth         - handle to the global proxy authentication header
 * hStack              - A pointer to the stack
 * nonceCount          - global Nonce-Count. If was set, will be used by
 *                       Authenticator for all messages instead of Nonce Count,
 *                       managed by the Object (RegClient, CallLeg)
 * lock                - protection for 'hGlobalAuth' and 'nonceCount' fields
 *****************************************************************************/
typedef struct {
    RvLogMgr*                       pLogMgr;
    RvSipAuthenticatorEvHandlers    evHandlers;
    RvSipAuthenticatorActOnCallback actOnCallback;
	RvLogSource*                    pLogSrc;
    HRPOOL                          hGeneralPool;
    HRPOOL                          hElementPool;
    HPAGE                           hGlobalHeaderPage;
    RvSipAuthenticationHeaderHandle hGlobalAuth;
    RvSipMsgMgrHandle               hMsgMgr;
    RvSipAppAuthenticatorHandle     hAppAuthHandle;
    void*                           hStack;
    RvInt32                         nonceCount;
    RvMutex                         lock;
}AuthenticatorInfo;


/********************************************************************************************
 * Definition of AuthenticatorParams:
 * hPool                   - handle to the pool
 * hPage                   - handle to the memory page
 * userNameOffset          - event handlers structure.
 * passwordOffset          - handle to the log.
 * realmOffset             - register number for the log.
 * opaqueOffset            - handle to the general pool
 * nonceOffset             - page that is used for preparing the authorization header
 * eAuthAlgorithm          - page that is used for preparing the A1 string
 * eQopOption              - page that is used for preparing the A2 string
 * eAuthHeaderType         - the authentication header type
 * akaVersion              - for IMS only.
 ********************************************************************************************/

typedef struct {
    HRPOOL                            hPool;
    HPAGE                             hPage;
    RvInt32                          userNameOffset;
    RvInt32                          passwordOffset;
    RvInt32                          realmOffset;
    RvInt32                          opaqueOffset;
    RvInt32                          nonceOffset;
    RvSipAuthAlgorithm                eAuthAlgorithm;
    RvSipAuthQopOption                eQopOption;
    RvSipAuthenticationHeaderType     eAuthHeaderType;
#ifdef RV_SIP_IMS_ON
    RvInt32                         akaVersion;
#endif
}AuthenticatorParams;


/*  AuthObj
 * -------------------
 *  An object that represent an incoming authentication header.
 *  The structure contains the authentication header and additional parameters, 
 *  for credentials calculation.
 *
 *  bValid     - RV_TRUE, if the Authentication header contains supported
 *               by the Stack credentials. RV_FALSE - otherwise.
 *  hAuthObjPage - The page of the auth obj. (allocated from the element pool)
 *  hListElement - place in the list (for remove function).
 *  hHeader    - handle to the Authentication header, containing challenge.
 *  nonceCount - NONCE usage counter.
 *               The counter counts times, the credentials for the challenge
 *               were calculated. This number equal to the times, the NONCE
 *               was used, since NONCE, contained in the Authentication header
 *               can't be changed.
 * pContext    - place to save application context.
 * pListInfo   - pointer to the list info struct, containing the lock, and lock functions.
 */
typedef struct  {
    AuthenticatorInfo*   pAuthMgr;
    RvBool          bValid;
    HPAGE           hAuthObjPage;
    RLIST_ITEM_HANDLE   hListElement;
    RvSipAuthenticationHeaderHandle hHeader;
    RvInt32          nonceCount;
    void*            pContext;
    AuthObjListInfo* pListInfo;
#ifdef RV_SIP_IMS_ON
    /* params: AUTS, shared secret */
    RvInt32         autsParamOffset;
#endif /*RV_SIP_IMS_ON*/
} AuthObj;

/*  AuthCallbacks
 * -------------------
 *  An enumeration listing the callbacks of the authenticator module
 *  AUTH_CALLBACK_GET_SHARED_SECRET - RvSipAuthenticatorSharedSecretEv or RvSipAuthenticatorGetSharedSecretEv
 *  AUTH_CALLBACK_MD5 - RvSipAuthenticatorMD5Ev or RvSipAuthenticatorMD5ExEv
 *  AUTH_CALLBACK_MD5_ENTITY_BODY - RvSipAuthenticatorMD5EntityBodyEv
 *  AUTH_CALLBACK_UNSUPPORTED_CHALLENGE - RvSipAuthenticatorUnsupportedChallengeEv
 *  AUTH_CALLBACK_NONCE_COUNT_USAGE - RvSipAuthenticatorNonceCountUsageEv
 *  AUTH_CALLBACK_AUTHORIZATION_READY - RvSipAuthenticatorAuthorizationReadyEv
 */
typedef enum {
    AUTH_CALLBACK_UNDEFINED = -1,
	AUTH_CALLBACK_GET_SHARED_SECRET,
	AUTH_CALLBACK_MD5,
	AUTH_CALLBACK_MD5_ENTITY_BODY,
	AUTH_CALLBACK_UNSUPPORTED_CHALLENGE,
	AUTH_CALLBACK_NONCE_COUNT_USAGE,
	AUTH_CALLBACK_AUTHORIZATION_READY	

} AuthCallbacks;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************************
 *
 * General functions
 *
 ***************************************************************************************/
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
                            OUT RvBool*                         pbIsCorrect);

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
                IN  HPAGE                           hPage);

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
                            OUT RvSipAuthorizationHeaderHandle hAuthorization);

/******************************************************************************
 * AuthenticatorPrepareAuthorizationHeader
 * ----------------------------------------------------------------------------
 * General: create the Authorization header according to the credentials in the
 *          Authentication header. this functions also creates the digest string
 *          as specified in RFC 2617.
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
                           OUT RvSipAuthorizationHeaderHandle  hAuthorization);

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
                                          IN RvSipAddressHandle         hRequestUri);

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
                             IN OUT  RvSipAuthenticationInfoHeaderHandle  hAuthenticationInfo);

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
                   OUT AuthObj**            ppAuthObj);

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
                   IN  RvBool               bHeaderValid);

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
								IN  RvChar*							strHeader);

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
                                         INOUT RLIST_ITEM_HANDLE* phListElement);

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
                   IN  AuthObj*             pAuthObj);

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
                   IN  AuthObj*             pAuthObj);

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
                   IN  AuthObj*             pAuthObj);

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
						IN  AuthCallbacks        eCallbackType);

#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _AUTHENTICATOR_OBJECT_H*/

