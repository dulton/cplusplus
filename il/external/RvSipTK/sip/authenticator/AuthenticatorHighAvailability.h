/******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************


 ******************************************************************************
 *                       <AuthenticatorHighAvailability.h>
 *
 *    Author                         Date
 *    ------                        ------
 *
 ******************************************************************************
*/

#ifndef AUTHENTICATOR_HIGH_AVAILABILITY_H
#define AUTHENTICATOR_HIGH_AVAILABILITY_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*    INCLUDE HEADER FILES                                                   */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef  RV_SIP_HIGHAVAL_ON
#ifdef  RV_SIP_AUTH_ON

/*---------------------------------------------------------------------------*/
/*    FUNCTIONS HEADERS                                                      */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * AuthenticatorHighAvailStoreAuthData
 * ----------------------------------------------------------------------------
 * General: goes through list of Authentication Data structures and stores each
 *          strcuture into string buffer in the High Availability record format
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthenticationData - Autheticatin Data structure list to be stored.
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
RvStatus AuthenticatorHighAvailStoreAuthData(
                          IN    AuthenticatorInfo*  pAuthMgr,
                          IN    RLIST_HANDLE        hListAuthenticationData,
                          IN    RvUint32            maxBuffLen,
                          INOUT RvChar              **ppCurrPos,
                          INOUT RvUint32            *pCurrLen);

/******************************************************************************
 * AuthenticatorHighAvailRestoreAuthData
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
RvStatus AuthenticatorHighAvailRestoreAuthData(
                                               IN    AuthenticatorInfo*  pAuthMgr,
                                               IN    RvChar*             buffer,
                                               IN    RvUint32            buffLen,
                                               IN    HPAGE               hPage,
                                               IN    void               *pParentObj,
                                               IN    ObjLockFunc         pfnLockAPI,
                                               IN    ObjUnLockFunc       pfnUnLockAPI,
                                               INOUT AuthObjListInfo*   *ppAuthListInfo,
                                               INOUT RLIST_HANDLE*       phListAuthObj);

#ifdef  RV_SIP_HIGH_AVAL_3_0
/******************************************************************************
 * AuthenticatorHighAvailRestoreAuthData3_0
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
 *                          in the record.
 *         nonceCount401  - nonceCount, last used for WWW-Authentica...
 *         proxyAuthHeaderOffset - offset of the Authenticate header
 *                          in the record.
 *         nonceCount407  - nonceCount, last used for Proxy Authenti...
 *         hPage          - The page if the object holding the authentication list.
 *         pTripleLock    - pointer to the lock of the object that is holding this list.
 *         pfnLockAPI     - function to lock the object that is holding this list.
 *         pfnUnLockAPI   - function to unlock the object that is holding this list.
 *         phListAuthObj  - list to be used for restoring.
 * Output: ppAuthListInfo - pointer to the list info struct, constructed on the given page.
 *         phListAuthObj  - list, used for restoring.
 *****************************************************************************/
RvStatus RVCALLCONV AuthenticatorHighAvailRestoreAuthData3_0(
                        IN    AuthenticatorInfo*    pAuthMgr,
                        IN    RvChar*               buffer,
                        IN    RvInt32               wwwAuthHeaderOffset,
                        IN    RvInt32               nonceCount401,
                        IN    RvInt32               proxyAuthHeaderOffset,
                        IN    RvInt32               nonceCount407,
                        IN    HPAGE                 hPage,
                        IN    SipTripleLock         *pTripleLock,
                        IN    ObjLockFunc           pfnLockAPI,
                        IN    ObjUnLockFunc         pfnUnLockAPI,
                        INOUT AuthObjListInfo*   *ppAuthListInfo,
                        INOUT RLIST_HANDLE*         phListAuthObj);
#endif /* #ifdef  RV_SIP_HIGH_AVAL_3_0 */

/******************************************************************************
 * AuthenticatorHighAvailGetAuthDataStorageSize
 * ----------------------------------------------------------------------------
 * General: calculates the size of buffer consumed by the list of
 *          Authentication Data structures in High Availability record format.
 *          For details, see High Availability module documentation.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthenticationData - Handle of the list.
 * Output: pStorageSize        - Requested size.
 *****************************************************************************/
RvStatus AuthenticatorHighAvailGetAuthDataStorageSize(
                          IN  AuthenticatorInfo*  pAuthMgr,
                          IN  RLIST_HANDLE        hListAuthenticationData,
                          OUT RvInt32             *pStorageSize);

#endif /* #ifdef  RV_SIP_AUTH_ON */
#endif /* #ifdef  RV_SIP_HIGHAVAL_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef AUTHENTICATOR_HIGH_AVAILABILITY_H*/


