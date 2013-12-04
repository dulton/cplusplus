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
 *                              SipAuthentication.h
 *
 *  This file provides headers for the functions that create digest string
 *  to the hash algorithm.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                      Jan. 2001
 *********************************************************************************/

#ifndef _SIP_AUTHENTICATION_H
#define _SIP_AUTHENTICATION_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipCommonTypes.h"
#include "RvSipMsgTypes.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipAuthenticator.h"
#include "_SipCommonUtils.h"
#include "AdsRpool.h"
#include "AdsRlist.h"
#include "rvlog.h"
#include "rvmutex.h"

#define HIGHAVAIL_AUTHDATA_ID             "AuthDataStruct"
#define HIGHAVAIL_AUTHDATA_HEADER_ID      "AuthDataHeader"
#define HIGHAVAIL_AUTHDATA_VALIDITY_ID    "AuthDataValidity"
#define HIGHAVAIL_AUTHDATA_NONCECOUNT_ID  "AuthDataNonceCount"
#ifdef RV_SIP_IMS_ON
#define HIGHAVAIL_AUTHDATA_AUTS_ID        "AuthDataAuts"
#endif /* RV_SIP_IMS_ON */
    
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * LockFunc
 * ----------------------------------------------------------------------------------
 * General: function that is used to lock the object (usually XXXLockAPI())
 *
 * Return Value:(-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj     - A pointer to stack object
 ***********************************************************************************/
typedef RvStatus (RVCALLCONV * ObjLockFunc)(IN  void* pObj);
/************************************************************************************
 * UnLockFunc
 * ----------------------------------------------------------------------------------
 * General: function that is used to unlock the object (usually XXXUnLockAPI())
 *
 * Return Value:(-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj     - A pointer to stack object
 ***********************************************************************************/
typedef void (RVCALLCONV * ObjUnLockFunc)(IN  void* pObj);



/*  AuthObjListInfo
 * -------------------
 *  An object that saves information for all the list objects.
 *
 *  bListIsValid - RV_FALSE, if one of the headers is not valid.
 *  bListInitiated - indicates if the authentication list was already constructed.
 *                  (if SipAuthenticatorAuthListDestruct was called, this parameter is false).
 *  hCurrProcessedAuthObj - Handle to the auth-obj in the list that is currently being
 *                  processed.
 *  pTripleLock - lock of the object that holds this list.
 *  pfnLockAPI  - function to lock the triple lock.
 *  pfnUnLockAPI - function to unlock the triple lock.
 *  pParentObj   - pointer to the parent object (call-leg/regclient) to use in the lock function.
 */
typedef struct  {
    RvBool          bListIsValid;
    RvBool          bListInitiated;
    RvSipAuthObjHandle hCurrProcessedAuthObj;
    
    /* lock and unlock functions */
    ObjLockFunc        pfnLockAPI;
    ObjUnLockFunc      pfnUnLockAPI;
    void*              pParentObj;
} AuthObjListInfo;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************************
 *
 * General functions
 *
 ***************************************************************************************/

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
                                               OUT RvSipAuthenticatorHandle *hAuth);

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
RvStatus RVCALLCONV SipAuthenticatorDestruct(IN RvSipAuthenticatorHandle hAuth);


/***************************************************************************
 * SipAuthenticatorValidityChecking
 * ------------------------------------------------------------------------
 * General: The function checks parameters in the authentication header like
 *          QoP, algorithm and scheme and decides if the authorization header
 *          can be built.
 * Return Value: RV_OK          - If valid.
 *               RV_ERROR_UNKNOWN - invalid parameters, so the authorization
 *                                  can not be built.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hAuthMgr - handle to the authenticator manager.
 *        hAuth    - handle to the authentication header.
 * Output:  none
 ***************************************************************************/
RvStatus SipAuthenticatorValidityChecking(
                                        IN  RvSipAuthenticatorHandle         hAuthMgr,
                                        IN  RvSipAuthenticationHeaderHandle  hAuth);

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
                                           RvInt32              akaVersion);

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
                            OUT RvBool*                         pbIsCorrect);


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
                            IN  RvSipAuthenticatorHandle        hAuthMgr,
                            IN  RvSipAuthorizationHeaderHandle  hAuthoriz,
                            OUT RvBool*                         bIsSupported);

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
                                     OUT RvChar*        strMethod);

/******************************************************************************
 * SipAuthenticatorAuthListDestruct
 * ----------------------------------------------------------------------------
 * General: destruct list, allocated on the General Pool page. The list
 *          elements (headers) are lost as a result of this operation.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hList  - Handle to the list to be destructed.
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV SipAuthenticatorAuthListDestruct(IN RvSipAuthenticatorHandle  hAuth,
                                                 IN AuthObjListInfo*   pAuthListInfo,
                                                 IN RLIST_HANDLE  hList);

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
void RVCALLCONV SipAuthenticatorAuthListResetPasswords(IN RLIST_HANDLE  hList);

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
 *          phListAuthObj - Pointer to the list to be updated with
 *                          new Authentication Data structure. Can point to NULL.
 * Output:  pAuthListInfo - pointer to the auth list info structure.
 *          phListAuthObj - Pointer to the created list handle.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorUpdateAuthObjListFromMsg(
                    IN    RvSipAuthenticatorHandle  hAuth,
                    IN    RvSipMsgHandle            hMsg,
                    IN    HPAGE                     hPage,
                    IN    void*                     pParentObj,
                    IN    ObjLockFunc               pfnLockAPI,
                    IN    ObjUnLockFunc             pfnUnLockAPI,
                    INOUT AuthObjListInfo*          *pAuthListInfo,
                    INOUT RLIST_HANDLE              *phListAuthObj);

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
					IN    RvInt32                           akaVersion);
#endif /* RV_SIP_IMS_ON */

/******************************************************************************
 * SipAuthenticatorBuildAuthorizationListInMsg
 * ----------------------------------------------------------------------------
 * General: goes through the list of Authentication Data structures and
 *          for each strcuture prepares credentials. The credentials are set
 *          into the Authorization header, built in the message.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuthMgr - Handle to the Authentication Manager object.
 *          hMsg     - Handle to the Message, to be filled with the headers.
 *          hObject     - handle to the  object, required authentication.
 *          eObjectType - type of the object (CallLeg / RegClient).
 *          pObjTripleLock - object's triple lock.
 *          hListAuthObj - list of challenges, used for credential
 *                        calculation.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorBuildAuthorizationListInMsg(
                        IN RvSipAuthenticatorHandle   hAuth,
                        IN RvSipMsgHandle             hMsg,
                        IN void                       *hObject,
                        IN RvSipCommonStackObjectType eObjectType,
                        IN SipTripleLock              *pObjTripleLock,
                        IN AuthObjListInfo*           pAuthListInfo,
                        IN RLIST_HANDLE               hListAuthObj);


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
                        IN RLIST_HANDLE             hListAuthorizationHeaders);

/******************************************************************************
 * SipAuthenticatorInviteMoveAuthorizationListIntoMsg
 * ----------------------------------------------------------------------------
 * General: goes through list of Authorization headers and copy them into
 *          the message. Destructs list after this.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or in rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hAuth  - Handle to the Authentication Manager object.
 *          hMsg   - Handle to the Message to be searched for the headers.
 *          hListAuthorizationHeaders - List of Authorization headers.
 * Output:  none.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorInviteMoveAuthorizationListIntoMsg(
                         IN RvSipAuthenticatorHandle hAuth,
                         IN RvSipMsgHandle           hMsg,
                         IN RLIST_HANDLE             hListAuthorizationHeaders);

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
                    INOUT RLIST_HANDLE            *phListAuthorizationHeaders);

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
                                        OUT RvBool                   *pbValid);

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
                                        INOUT RLIST_HANDLE              *phAuthList);

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
 *          hPage        - The page if the object holding the authentication list.
 *          hListSrc     - Handle to the source list
 *          pParentObj   - pointer to the object that is holding this list.
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
                                        OUT RLIST_HANDLE             *phList);

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
			                                       OUT  RvSipAuthObjHandle*    phAuthObj);

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
                                                      IN   RvSipAuthObjHandle     hAuthObj);
#endif /*#ifdef RV_SIP_AUTH_ON*/


#ifdef RV_SIP_HIGHAVAL_ON
/******************************************************************************
 * SipAuthenticatorHighAvailStoreAuthObj
 * ----------------------------------------------------------------------------
 * General: goes through list of Authentication Data structures and stores each
 *          strcuture into string buffer in the High Availability record format
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthObj - Autheticatin Data structure list to be stored.
 *         maxBuffLen          - Size of the buffer.
 *         ppCurrPos           - Pointer to the place in buffer,
 *                               where the structure should be stored.
 *         pCurrLen            - Length of the record, currently stored
 *                               in the buffer.
 * Output: ppCurrPos           - Pointer to the end of the Authentication Data
 *                               list record in the buffer.
 *         pCurrLen            - Length of the record, currently stored in,
 *                               the buffer including currently stored list.
 *****************************************************************************/
RvStatus RVCALLCONV SipAuthenticatorHighAvailStoreAuthObj(
                            IN    RvSipAuthenticatorHandle hAuthMgr,
                            IN    RLIST_HANDLE             hAuthObj,
                            IN    RvUint32                 maxBuffLen,
                            INOUT RvChar                   **ppCurrPos,
                            INOUT RvUint32                 *pCurrLen);


/******************************************************************************
 * SipAuthenticatorHighAvailRestoreAuthObj
 * ----------------------------------------------------------------------------
 * General: restores the list of Authentication Data structures
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
                        IN    ObjLockFunc         pfnLockAPI,
                        IN    ObjUnLockFunc       pfnUnLockAPI,
                        INOUT AuthObjListInfo*   *ppAuthListInfo,
                        INOUT RLIST_HANDLE*       phListAuthObj);

#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 


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
                        INOUT RLIST_HANDLE             *phAuthObj);
#endif /* #if defined(RV_SIP_HIGH_AVAL_3_0) && defined(RV_SIP_HIGHAVAL_ON) */


#ifdef RV_SIP_HIGHAVAL_ON
/******************************************************************************
 * SipAuthenticatorHighAvailGetAuthObjStorageSize
 * ----------------------------------------------------------------------------
 * General: calculates the size of buffer consumed by the list of
 *          Authentication Data structures in High Availability record format.
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
                            OUT RvInt32                  *pStorageSize);


#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 

#endif /*#ifndef RV_SIP_LIGHT */
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
                                        IN  RV_BOOL                          isRegClient);

#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _SIP_AUTHENTICATION_H*/

