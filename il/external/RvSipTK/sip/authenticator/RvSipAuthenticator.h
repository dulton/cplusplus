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
 *                              RvSipAuthenticator.h
 *
 *  This file provides headers for the functions that handle the digest response creating,
 *  the authorization header building.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                      Jan. 2001
 *********************************************************************************/

#ifndef _RV_SIP_AUTHENTICATOR_H
#define _RV_SIP_AUTHENTICATOR_H


#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "RvSipCommonTypes.h"
#include "RvSipMsgTypes.h"
#include "rpool_API.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* declaring the handle to the Authenticator Manager object */
RV_DECLARE_HANDLE(RvSipAuthenticatorHandle);

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
/* declaring an application handle for Authentication process */
DECLARE_VOID_POINTER(RvSipAppAuthenticationHandle);
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */

/* declaring the application handle to the Authenticator object */
RV_DECLARE_HANDLE(RvSipAppAuthenticatorHandle);

/* declaring the handle to an auth-object.
   (auth object represent a single authentication header kept in a sip stack object) */
RV_DECLARE_HANDLE(RvSipAuthObjHandle);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipAuthenticatorMD5Ev (DEPRECATED - use RvSipAuthenticatorMD5ExEv)
 * ------------------------------------------------------------------------
 * General:  Notifies that there is a need to use the MD5 algorithm.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pRpoolMD5Input - Rpool pointer to the MD5 input
 *          length         - length of the string inside the page
 * Output:     strMd5Output   - The output of the hash algorithm
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipAuthenticatorMD5Ev)(
                                   IN  RPOOL_Ptr                     *pRpoolMD5Input,
                                   IN  RvUint32                     length,
                                   OUT RPOOL_Ptr                     *pRpoolMD5Output);

/***************************************************************************
 * RvSipAuthenticatorMD5ExEv
 * ------------------------------------------------------------------------
 * General:  Notifies that there is a need to use the MD5 algorithm.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hAuthenticator    - Handle to the authenticator object.
 *          hAppAuthenticator - Handle to the application authenticator handle.
 *          pRpoolMD5Input - Rpool pointer to the MD5 input
 *          length         - length of the string inside the page
 * Output:     strMd5Output   - The output of the hash algorithm
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipAuthenticatorMD5ExEv)(
                                   IN  RvSipAuthenticatorHandle       hAuthenticator,
                                   IN  RvSipAppAuthenticatorHandle    hAppAuthenticator,
                                   IN  RPOOL_Ptr                     *pRpoolMD5Input,
                                   IN  RvUint32                     length,
                                   OUT RPOOL_Ptr                     *pRpoolMD5Output);


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
/***************************************************************************
 * RvSipAuthenticatorGetAppAuthHandle
 * ------------------------------------------------------------------------
 * General:  Figures out the application authentication handle
 *           given an application handle for CallLeg or Register-Client.
 * Return Value: application handle for Authentication process.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	appHandle   - application handle for CallLeg or Register-Client
 *          isRegClient - RV_TRUE means hAppRegClient is given,
 *                        RV_FALSE - hAppCallLeg
 ***************************************************************************/
typedef RvSipAppAuthenticationHandle
		(RVCALLCONV * RvSipAuthenticatorGetAppAuthHandle)(
                                   IN  void                          *appHandle,
                                   IN  RV_BOOL                       isRegClient);
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipAuthenticatorSharedSecretEv
 * ------------------------------------------------------------------------
 * General:  Notifies that there is a need for the user-name and password.
 *           This callback function is for client side authentication.
 *           Note: This function is obsolete and was deprecated in
 *           RvSipAuthenticatorGetSharedSecretEv().
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hRequestUri      - handle to the request uri
 *          pRpoolRealm      - the realm string in RPool_ptr format
 * Output:     pRpoolUserName   - the user-name string in RPool_ptr format
 *          pRpoolPassword   - the password string in RPool_ptr format
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipAuthenticatorSharedSecretEv)(
                                   IN  RvSipAddressHandle            hRequestUri,
                                   IN  RPOOL_Ptr                     *pRpoolRealm,
                                   OUT RPOOL_Ptr                     *pRpoolUserName,
                                   OUT RPOOL_Ptr                     *pRpoolPassword);

/***************************************************************************
 * RvSipAuthenticatorGetSharedSecretEv
 * ------------------------------------------------------------------------
 * General:  Notifies that there is a need for the user-name and password.
 *           This callback function may be called in 2 cases:
 *        1. Client authentication - when the stack builds the Authorization
 *           header, using the received WWW-Authenticate header.
 *           In this case, the client should supply the user-name and password.
 *        2. Server authentication - when application uses the stack to build
 *           an Authentication-info header. in this case the stack already has
 *           the user-name, from the Authorization header, and application
 *           should only supply the password.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthenticator    - Handle to the authenticator object.
 *          hAppAuthenticator - Handle to the application authenticator.
 *          hObject           - Handle to the Object, that is served
 *                              by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType      - pointer to the variable, that stores the
 *                              type of the hObject.
 *                              Use following code to get the type:
 *                              RvSipCommonStackObjectType eObjType = *peObjectType;
 *          pRpoolRealm       - the realm string in RPool_ptr format
 * Output:  pRpoolUserName    - the user-name string in RPool_ptr format.
 *                              (this is an OUT parameter for client, and IN
 *                              parameter for server).
 *          pRpoolPassword    - the password string in RPool_ptr format
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipAuthenticatorGetSharedSecretEv)(
                                   IN    RvSipAuthenticatorHandle       hAuthenticator,
                                   IN    RvSipAppAuthenticatorHandle    hAppAuthenticator,
                                   IN    void*                          hObject,
                                   IN    void*                          peObjectType,
                                   IN    RPOOL_Ptr                     *pRpoolRealm,
                                   INOUT RPOOL_Ptr                     *pRpoolUserName,
                                   OUT   RPOOL_Ptr                     *pRpoolPassword);

/***************************************************************************
 * RvSipAuthenticatorMD5EntityBodyEv
 * ------------------------------------------------------------------------
 * General:  This callback function notifies the application that
 *           it should supply the hashing result performed on the message body
 *           MD5(entity-body). Message body hash value is needed when
 *           the required quality of protection (qop) is 'auth-int'.
 *
 *           Two notes regarding the message parameter supplied in this callback:
 *           1) On client side: Note that this callback is called before the 
 *           msgToSend callback of the stack object is called. Therefore, if 
 *           your code adds the message body in the msgToSend callback, the body 
 *           will not be available on this callback.
 *           In order to use the message body at this stage, you must use the
 *           outbound message mechanism to add the body.
 *           2) On server side: Note that the message is supplied only if the
 *           authentication procedure was started synchronic (within the
 *           change-state callback). in a-synchronic mode, the parameter is NULL.
 *           In order to perform auth-int authentication in an a-synchronic 
 *           mode, the application should save the incoming request in the msg-rcvd
 *           callback, and use it in this function.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthenticator    - handle to the Authenticator object
 *          hAppAuthenticator - Handle to the application authenticator.
 *          hObject           - Handle to the Object, that is served
 *                              by the Authenticator (e.g. CallLeg, RegClient)
 *          peObjectType      - Pointer to the variable, that stores the
 *                              type of the hObject.
 *                              Use following code to get the type:
 *                              RvSipCommonStackObjectType eObjType = *peObjectType;
 *          hMsg              - Handle to the message that is now being sent
 *                              and that will include the user credentials.
 * Output:  pRpoolMD5Output   - The MD5 of the message body in RPOOL_Ptr format.
 *          pLength           - length of the string after MD5 result
 *                              concatenation.
 ***************************************************************************/
typedef void
        (RVCALLCONV * RvSipAuthenticatorMD5EntityBodyEv)(
                       IN     RvSipAuthenticatorHandle      hAuthenticator,
                       IN     RvSipAppAuthenticatorHandle   hAppAuthenticator,
                       IN     void*                         hObject,
                       IN     void*                         peObjectType,
                       IN     RvSipMsgHandle                hMsg,
                       OUT    RPOOL_Ptr                     *pRpoolMD5Output,
                       OUT    RvUint32                      *pLength);

/******************************************************************************
 * RvSipAuthenticatorUnsupportedChallengeEv
 * ----------------------------------------------------------------------------
 * General: This callback function notifies the application about challenge,
 *          credentials for which can't be prepared by the Stack.
 *          The Application can calculate the credentials by itself,
 *          build the Authorization Header in the message,
 *          using RvSipAuthorizationHeaderConstructInMsg() and set
 *          the calculated credential into it.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthenticator        - Handle to the Authenticator object
 *          hAppAuthenticator     - Application handle stored in Authenticator.
 *          hObject               - Handle to the Object, that is served
 *                                  by the Authenticator(e.g.CallLeg,RegClient)
 *          peObjectType          - Pointer to the variable that stores
 *                                  the type of the hObject.
 *                                  Use following code to get the type:
 *                                  RvSipCommonStackObjectType eObjType = *peObjectType;
 *          hAuthenticationHeader - Handle to the Authentication header,
 *                                  containing unsupported challenge.
 *          hMsg                  - Handle to the message where the Application
 *                                  should set the credentials, if it wants.
 * Output:  none.
 ***************************************************************************/
typedef void
(RVCALLCONV * RvSipAuthenticatorUnsupportedChallengeEv)(
                    IN RvSipAuthenticatorHandle         hAuthenticator,
                    IN RvSipAppAuthenticatorHandle      hAppAuthenticator,
                    IN void*                            hObject,
                    IN void*                            peObjectType,
                    IN RvSipAuthenticationHeaderHandle  hAuthenticationHeader,
                    IN RvSipMsgHandle                   hMsg);

/******************************************************************************
 * RvSipAuthenticatorNonceCountUsageEv
 * ----------------------------------------------------------------------------
 * General: This callback function notifies the application about the value
 *          of the nonceCount parameter, which the Stack is going to use,
 *          while calculating credentials.
 *          The Application can change this value in order to fit more
 *          precious management of the nonceCount. The Stack doesn't check
 *          the uniquity of the used NONCE in the realm or by the different
 *          object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthenticator        - Handle to the Authenticator object
 *          hAppAuthenticator     - Application handle stored in Authenticator.
 *          hObject               - Handle to the Object, that is served
 *                                  by the Authenticator(e.g.CallLeg,RegClient)
 *          peObjectType          - Pointer to the variable that stores
 *                                  the type of the hObject.
 *                                  Use following code to get the type:
 *                                  RvSipCommonStackObjectType eObjType = *peObjectType;
 *          hAuthenticationHeader - Handle to the Authentication header,
 *                                  containing challenge, credentials for which
 *                                  are being prepared.
 *          pNonceCount           - pointer to the nonceCount value, managed by
 *                                  the Stack per Challenge.
 * Output:  pNonceCount           - pointer to the nonceCount value, set by
 *                                  the Application in order to be used by
 *                                  the Stack for Credentials calculation.
 ***************************************************************************/
typedef void
(RVCALLCONV * RvSipAuthenticatorNonceCountUsageEv)(
                   IN    RvSipAuthenticatorHandle        hAuthenticator,
                   IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
                   IN    void*                           hObject,
                   IN    void*                           peObjectType,
                   IN    RvSipAuthenticationHeaderHandle hAuthenticationHeader,
                   INOUT RvInt32*                        pNonceCount);



/******************************************************************************
 * RvSipAuthenticatorAuthorizationReadyEv
 * ----------------------------------------------------------------------------
 * General: This callback function notifies the application that an authorization
 *          header was fully built in the message.
 *          In this callback application may add any additional information
 *          to the given authorization header.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthenticator        - Handle to the Authenticator object
 *          hAppAuthenticator     - Application handle stored in Authenticator.
 *          hObject               - Handle to the Object, that is served
 *                                  by the Authenticator(e.g.CallLeg,RegClient)
 *          peObjectType          - Pointer to the variable that stores
 *                                  the type of the hObject.
 *                                  Use following code to get the type:
 *                                  RvSipCommonStackObjectType eObjType = *peObjectType;
 *          RvSipAuthObjHandle    - Handle to the auth-object, related to this
 *                                  new authorization header.
 *          pAuthObjContext       - Context of application of the related auth-object.
 *          hAuthorizationHeader  - The already filled authorization header.
 * Output:  none.
 ***************************************************************************/
typedef void
(RVCALLCONV * RvSipAuthenticatorAuthorizationReadyEv)(
                   IN    RvSipAuthenticatorHandle        hAuthenticator,
                   IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
                   IN    void*                           hObject,
                   IN    void*                           peObjectType,
                   IN    RvSipAuthObjHandle              hAuthObj,
                   IN    void*                           pAuthObjContext,
                   IN    RvSipAuthorizationHeaderHandle  hAuthorizationHeader);


/* RvSipAuthenticatorEvHandlers
 * ------------------------------------------------------------------------
 * Structure with function pointers to the hash algorithm call-backs.
 * This structure is used to set the application call-backs in the function
 * RvSipAuthenticationSetEventHandlers();
 * pfnMD5AuthenticationHandler          - Deprecated
 * pfnSharedSecretAuthenticationHandler - Deprecated
 * pfnGetSharedSecretAuthenticationHandler - notify the application that
 *          there is a need for the user-name and password.
 * pfnMD5AuthenticationExHandler - notify that MD5 algorithm should be used.
 * pfnMD5EntityBodyAuthenticationHandler - asks an application for
 *          H(entity-body) string, necessary for A2 token calculation in case
 *          "auth-int" Quality-of-Protection authentication.
 * pfnUnsupportedChallengeAuthenticationHandler - notifies the application
 *          about challenge, credentials for which can't be prepared
 *          by the Stack. The Application can calculate the credentials
 *          by itself, build the Authorization Header in the message,
 *          using RvSipAuthorizationHeaderConstructInMsg() and set
 *          the calculated credential into it.
 * pfnNonceCountUsageAuthenticationHandler - notifies the application about
 *          nonce count that will be used while preparing credentials.
 *          The application can change the nonce count value.
 * pfnAuthorizationReadyAuthenticationHandler - notifies the application that an
 *          Authorization header was built, and is now ready for sending.
 *          The application can set any additional information to the header in this cb.
 */
typedef struct
{
    RvSipAuthenticatorMD5Ev                  pfnMD5AuthenticationHandler;
    RvSipAuthenticatorSharedSecretEv         pfnSharedSecretAuthenticationHandler;
    RvSipAuthenticatorGetSharedSecretEv      pfnGetSharedSecretAuthenticationHandler;
    RvSipAuthenticatorMD5ExEv                pfnMD5AuthenticationExHandler;
    RvSipAuthenticatorMD5EntityBodyEv        pfnMD5EntityBodyAuthenticationHandler;
    RvSipAuthenticatorUnsupportedChallengeEv pfnUnsupportedChallengeAuthenticationHandler;
    RvSipAuthenticatorNonceCountUsageEv      pfnNonceCountUsageAuthenticationHandler;
    RvSipAuthenticatorAuthorizationReadyEv   pfnAuthorizationReadyAuthenticationHandler;
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    RvSipAuthenticatorGetAppAuthHandle  pfnGetAppAuthenticationHandle;
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */
} RvSipAuthenticatorEvHandlers;

/*-----------------------------------------------------------------------*/
/*               AUTOMATIC BEHAVIOR FOR NON REGISTERD CALLBACKS          */
/*-----------------------------------------------------------------------*/

/* RvSipAuthenticatorActOnCallback
 * ------------------------------------------------------------------------
 * Indicates for each callback whether the authenticator should consider this
 * callback when deciding whether to apply default behavior. As default, the
 * authenticator has these values set to RV_TRUE, indicating that if a callback
 * was set by the application, then it overrides the need in automatic behavior.
 * For example, if the application did not supply event handler for 
 * UnsupportedChallengeEv(), the authenticator checks the validity of the
 * Auth object and acts accordingly. If application event handler for 
 * UnsupportedChallengeEv() was set, the authenticator treats the Auth object 
 * as constantly valid. If you wish to treat the Auth object as constantly valid,
 * and you did set an event handler for UnsupportedChallengeEv(), set 
 * bUnsupportedChallengeAuthentication to RV_FALSE.
 * 
 * bMD5Authentication - A boolean referring to the MD5AuthenticationEv() event handler
 * bSharedSecretAuthentication - A boolean referring to the SharedSecretAuthenticationEv() event handler
 * bGetSharedSecretAuthentication - A boolean referring to the GetSharedSecretAuthenticationEv() event handler
 * bMD5AuthenticationEx - A boolean referring to the MD5AuthenticationExEv() event handler
 * bMD5EntityBodyAuthentication - A boolean referring to the MD5EntityBodyAuthenticationEv() event handler
 * bUnsupportedChallengeAuthentication - A boolean referring to the UnsupportedChallengeAuthenticationEv() event handler
 * bNonceCountUsageAuthentication - A boolean referring to the NonceCountUsageAuthenticationEv() event handler
 * bAuthorizationReadyAuthentication - A boolean referring to the AuthorizationReadyAuthenticationEv() event handler
 */
typedef struct
{
    RvBool      bMD5Authentication;
    RvBool      bSharedSecretAuthentication;
    RvBool		bGetSharedSecretAuthentication;
    RvBool      bMD5AuthenticationEx;
    RvBool      bMD5EntityBodyAuthentication;
    RvBool      bUnsupportedChallengeAuthentication;
    RvBool      bNonceCountUsageAuthentication;
    RvBool      bAuthorizationReadyAuthentication;
} RvSipAuthenticatorActOnCallback;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************************
 *
 * General functions
 *
 ***************************************************************************************/

/************************************************************************************
 * RvSipAuthenticatorSetEvHandlers
 * ----------------------------------------------------------------------------------
 * General: Sets event handlers for the authenticator events.
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
                                   IN RvUint32                      evHandlersSize);

/******************************************************************************
 * RvSipAuthenticatorSetProxyAuthInfo
 * ----------------------------------------------------------------------------
 * General: Sets a Proxy-Authenticate header to the authenticator object.
 *          This header will be used to authenticate requests in advance.
 *          The authenticator object will use this header to build a
 *          Proxy-Authorization header that will be placed automatically
 *          in all requests sent by Call-Legs, Register-Clients and Subscriptions.
 *          An application should use this function if it knows the challenge
 *          requested by its proxy. By setting the Proxy-Authorization header
 *          in initial requests the application might avoid the first message exchange
 *          of a request and its 407 response.
 *          Note1: If a stack object received 407 response despite of the
 *          existence of the Proxy-Authorization header in the request,
 *          the object will use the new received challenge to create correct
 *          credentials.
 *          Note2: This function can be used only for setting Proxy-Authenticate
 *          header. You cannot set a WWW-Authenticate header here.
 *
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - handle to the authenticator object.
 *          hAuthHeader       - handle to the authentication header.
 *          strUserName       - the user name string.
 *          strPassword       - the password string.
 * Output:  none
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetProxyAuthInfo(
                              IN RvSipAuthenticatorHandle        hAuth,
                              IN RvSipAuthenticationHeaderHandle hAuthHeader,
                              IN RvChar                          *strUserName,
                              IN RvChar                          *strPassword);


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
                                                IN RvSipAuthenticatorHandle  hAuthMgr);

/************************************************************************************
 * RvSipAuthenticatorSetAppHandle
 * ----------------------------------------------------------------------------------
 * General: Set the application authenticator handle to the SIP stack authenticator
 *          object. This handle is returned in some of the authenticator callback
 *          functions.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth             - Handle to the authenticator object.
 *          hAppAuth          - Handle to the application authenticator object.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetAppHandle(
                                   IN RvSipAuthenticatorHandle    hAuth,
                                   IN RvSipAppAuthenticatorHandle hAppAuth);

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
                                   OUT RvSipAppAuthenticatorHandle *phAppAuth);

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
                                   IN RvUint32                          structSize);

/***************************************************************************
 * RvSipAuthenticatorCredentialsSupported
 * ------------------------------------------------------------------------
 * General: The function checks parameters in the authorization header like
 *          qop, algorithm and scheme and determines if the header can be
 *          authenticated by SIP Stack or not.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hAuth - handle to the authenticator manager.
 *        hAuthorization    - handle to the authorization header.
 * Output:  bIsSupported - RV_TRUE if supported.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorCredentialsSupported(
                                        IN  RvSipAuthenticatorHandle         hAuth,
                                        IN  RvSipAuthorizationHeaderHandle   hAuthorization,
                                        OUT RvBool*                         bIsSupported);

/************************************************************************************
 * RvSipAuthenticatorVerifyCredentials
 * ----------------------------------------------------------------------------------
 * General: This function is for a server side authentication.
 *          Using this function the application can verify the credentials received
 *          in an incoming request message. The application
 *          supplies the password that belong to the user name and realm found
 *          in the authorization header, and wishes to know if the authorization
 *          header is correct for this user-name.
 *          This function creates the digest string as specified in RFC 2617, and compare
 *          it to the digest string inside the given authorization header.
 *          If it is equal, the header is correct.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth           - handle to the authentication module.
 *          hAuthorization  - handle to the authorization header.
 *          password        - password of the user indicated in the Authorization header.
 *          strMethod       - the method type of the request.
 * Output:  isCorrect       - boolean, TRUE if correct, False if not.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorVerifyCredentials(
                                   IN RvSipAuthenticatorHandle         hAuth,
                                   IN RvSipAuthorizationHeaderHandle   hAuthorization,
                                   IN RvChar                          *password,
                                   IN RvChar                          *strMethod,
                                   OUT RvBool                         *isCorrect);

/******************************************************************************
 * RvSipAuthenticatorVerifyCredentialsExt
 * ----------------------------------------------------------------------------
 * General: This function is for a server side authentication.
 *          Using this function the application can verify the credentials
 *          received in an incoming request message. The application supplies
 *          the password that belong to the user name and realm found
 *          in the authorization header, and wishes to know
 *          if the authorization header is correct for this user-name.
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
 * Output:  isCorrect       - RV_TRUE if correct, RV_FALSE otherwise.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorVerifyCredentialsExt(
                            IN  RvSipAuthenticatorHandle        hAuth,
                            IN  RvSipAuthorizationHeaderHandle  hAuthorization,
                            IN  RvChar*                         password,
                            IN  RvChar*                         strMethod,
                            IN  void*                           hObject,
                            IN  void*                           peObjectType,
                            IN  RvSipMsgHandle                  hMsg,
                            OUT RvBool*                         isCorrect);

/***************************************************************************
 * RvSipAuthenticatorGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the Stack instance to which this
 *          authenticator belongs.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hAuth     - Handle to the Authenticator.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorGetStackInstance(
                                   IN   RvSipAuthenticatorHandle  hAuth,
                                   OUT  void*       *phStackInstance);

/******************************************************************************
 * RvSipAuthenticatorSetNonceCount
 * ----------------------------------------------------------------------------
 * General: Sets the initial value of the nonce-count parameter, that will be used
 *          when creating the Proxy-Authorization header placed in outgoing
 *          requests.
 *          This nonce count is used with the challenge supplied by calling
 *          the RvSipAuthenticatorSetProxyAuthInfo() for authenticating
 *          messages in advance.
 *          The nonce-count value is incremented by the authenticator after each
 *          header calculation according to RFC 2617.
 *          Note1: Range of legal values for the nonce-count is [0,MAX_INT32].
 *          Note2: The value of the nonce count is reset each time the application
 *          calls the RvSipAuthenticatorSetProxyAuthInfo() with a new nonce field.
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth      - Handle to the authenticator object.
 *          nonceCount - The nonce count value to use.
 * Output: none.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthenticatorSetNonceCount(
                                   IN RvSipAuthenticatorHandle  hAuth,
                                   IN RvInt32                   nonceCount);

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
                            IN RvSipCommonStackObjectType      eObjectType);

/******************************************************************************
 * RvSipAuthenticatorPrepareAuthorizationHeader
 * ----------------------------------------------------------------------------
 * General: Prepares 'Authorization'/'Proxy-Authorization' header based
 *          on the 'WWW-Authenticate'/'Authenticate' header and other data,
 *          supplied as a parameters.
 *          A stand alone client transaction can receive a 401 or
 *          407 response. The response will include a challenge, and the application
 *          may wish to send a new request with the required credentials.
 *          For that the application needs to create a new transaction. The application
 *          can use this function in order to add the credentials to the new outgoing
 *          request. In the msgToSend callback, or using the outbound message mechanism
 *          the application should supply this function with the outbound message together
 *          with the Authentication header taken from the 401 or 407 response. The function
 *          will add the required Authorization header to the message.
 *          Note: it is the applications responsibility to add to the new request all 
 *          applicative information that was added to the original request.
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
                 INOUT   RvSipAuthorizationHeaderHandle  hAuthorizationHeader);

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
                 IN OUT  RvSipAuthenticationInfoHeaderHandle  hAuthenticationInfo);

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
                                           OUT  RvBool                   *pbIsValid);

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
                               IN   RvChar*                   pstrUserPw);

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
                               IN   void*                     pContext);

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
                               OUT  void*                     *ppContext);

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
                               IN   RvChar*                   pstrAuts);

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
 *                      if NULL - retrieves only the string actual length.
 *          bufferLen - length of the given buffer.
 * Output:  actualLen - the actual length of the string. (0 if not exists).
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAuthObjGetAutsParam(
                               IN   RvSipAuthObjHandle        hAuthObj,
                               IN   RvChar*                   strBuffer,
                               IN   RvUint                    bufferLen,
                               OUT  RvUint*                   actualLen);
#endif /*RV_SIP_IMS_ON */

#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _RV_SIP_AUTHENTICATOR_H*/

