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
 *                              <AuthenticatorHighAvailability.c>
 *
 *  The following functions are for High Availability use. It means that user can get all
 *  the nessecary parameters of a connected call, (which is not in the middle of
 *  a refer or re-invite proccess), and copy it to another list of calls - of another
 *  call manager. The format of each stored parameter is:
 *  <total param len><internal sep><param ID><internal sep><param data><external sep>
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 November 2001
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifndef RV_SIP_PRIMITIVES
#ifdef  RV_SIP_HIGHAVAL_ON
#ifdef  RV_SIP_AUTH_ON

#include "AuthenticatorObject.h"
#include "_SipAuthenticator.h"
#include "AdsRlist.h"
#include "_SipCommonHighAval.h"
#include "RvSipAuthenticationHeader.h"

/*---------------------------------------------------------------------------*/
/*               MACRO DEFINITIONS                                           */
/*---------------------------------------------------------------------------*/

/* Number of Authentication Data parameters stored for High Availability: */
/*      Authentication header, Authentication nonceCount */
#define MAX_AUTHDATA_PARAM_IDS_NUM (4)

/*---------------------------------------------------------------------------*/
/*               TYPE DEFINITIONS                                            */
/*---------------------------------------------------------------------------*/

/* RestoreInfo
 * ----------------------------------------------------------------------------
 * Auxiliary structure, unifying data, required for restoring of Authentication
 * Data from the string buffer
 * ----------------------------------------------------------------------------
 * pAuthMgr            - Pointer to the Authenticator.
 * pAuthObj - Pointer to the Auth object, into
 *                       which the Authentication Data should be restored.
 */
typedef struct
{
    AuthenticatorInfo*  pAuthMgr;
    AuthObj*            pAuthObj;
} RestoreInfo;

/*---------------------------------------------------------------------------*/
/*               STATIC FUNCTIONS PROTOTYPES                                 */
/*---------------------------------------------------------------------------*/
static RvStatus GetADStructStorageSize(
                                IN  AuthenticatorInfo*  pAuthMgr,
                                IN  AuthObj* pAuthObj,
                                OUT RvInt32*            pADStructStorageSize);

static RvStatus StoreADStruct(  IN    AuthenticatorInfo*  pAuthMgr,
                                IN    AuthObj* pAuthObj,
                                IN    RvUint32            maxBuffLen,
                                INOUT RvChar**            ppCurrBuffPos,
                                INOUT RvUint32*           pCurrLen);

static RvStatus RVCALLCONV RestoreADStructHeader(
                                IN    RvChar    *pHeaderID,
                                IN    RvUint32  headerIDLen,
                                IN    RvChar    *pStoredHeader,
                                IN    RvUint32  storedHeaderLen,
                                IN    void      *pObj);

#ifdef RV_SIP_IMS_ON
static RvStatus RVCALLCONV RestoreADStructAutsStr(
                                      IN    RvChar   *pHeaderID,
                                      IN    RvUint32  headerIDLen,
                                      IN    RvChar   *pStoredStr,
                                      IN    RvUint32  storedStrLen,
                                      IN    void     *pObj);
#endif /*RV_SIP_IMS_ON*/

/*---------------------------------------------------------------------------*/
/*               MODULE FUNCTIONS                                            */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * AuthenticatorHighAvailStoreAuthData
 * ----------------------------------------------------------------------------
 * General: goes through list of Auth objects and stores each
 *          structure into string buffer in the High Availability record format
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr     - Pointer to the Authenticator.
 *         hAuthObj     - Auth object list to be stored.
 *         maxBuffLen   - Size of the buffer.
 *         ppCurrPos    - Pointer to the place in buffer,
 *                        where the structure should be stored.
 *         pCurrLen     - Length of the record, currently stored
 *                        in the buffer.
 * Output: ppCurrPos    - Pointer to the end of the Authentication Data
 *                        list record in the buffer.
 *         pCurrLen     - Length of the record, currently stored in,
 *                        the buffer including currently stored list.
 *****************************************************************************/
RvStatus AuthenticatorHighAvailStoreAuthData(
                          IN    AuthenticatorInfo*  pAuthMgr,
                          IN    RLIST_HANDLE        hListAuthObj,
                          IN    RvUint32            maxBuffLen,
                          INOUT RvChar              **ppCurrPos,
                          INOUT RvUint32            *pCurrLen)
{
    RvStatus rv;
    RLIST_ITEM_HANDLE   hListElement = NULL;
    AuthObj  *pAuthObj;
    RvInt32             currADStructStorageSize;

    /* Goes through the list of data structures and store them one by one */
    pAuthObj = AuthenticatorGetAuthObjFromList(hListAuthObj, RVSIP_FIRST_ELEMENT, &hListElement);
    while (NULL != pAuthObj)
    {
        /*Calculate the size, which the structure will consume in the storage*/
        rv = GetADStructStorageSize(pAuthMgr, pAuthObj,
                                    &currADStructStorageSize);
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailStoreAuthData - hListAuthData 0x%p - Failed to get storage size (rv=%d:%s)",
                hListAuthObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }

        /* Store the prefix of the AD (Authentication Data) Structure */
        rv = SipCommonHighAvalStoreSingleParamPrefix(HIGHAVAIL_AUTHDATA_ID,
            maxBuffLen, currADStructStorageSize, ppCurrPos, pCurrLen);
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailStoreAuthData - hListAuthData 0x%p - Failed to store prefix %s (rv=%d:%s)",
                hListAuthObj, HIGHAVAIL_AUTHDATA_ID,
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }

        /* Store the AD Structure itself */
        rv = StoreADStruct(pAuthMgr, pAuthObj, maxBuffLen,
                           ppCurrPos, pCurrLen);
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailStoreAuthData - hListAuthData 0x%p - Failed to store AuthData structure (rv=%d:%s)",
                hListAuthObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }

        /* Store the suffix of the AD (Authentication Data) Structure.
        Is necessary,if the SipCommonHighAvalStoreSingleParamPrefix was used*/
        rv = SipCommonHighAvalStoreSingleParamSuffix(
                                            maxBuffLen, ppCurrPos, pCurrLen) ;
        if (RV_OK != rv)
        {
            return rv;
        }
        
        /* Get the next Auth object */
        pAuthObj = AuthenticatorGetAuthObjFromList(hListAuthObj, RVSIP_NEXT_ELEMENT, &hListElement);
    }

    return RV_OK;
}

/******************************************************************************
 * AuthenticatorHighAvailRestoreAuthData
 * ----------------------------------------------------------------------------
 * General: restores the list of Auth objects
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
                            INOUT RLIST_HANDLE*       phListAuthObj)
{
    RvStatus                   rv;
    SipCommonHighAvalParameter AuthObjIDsArray[MAX_AUTHDATA_PARAM_IDS_NUM];
    SipCommonHighAvalParameter *pCurrAuthDataID = AuthObjIDsArray;
    RLIST_HANDLE               hNewList = *phListAuthObj;       
    AuthObj*                   pAuthObj;
    RestoreInfo                restoreInfo;
    
    if (0 == buffLen)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorHighAvailRestoreAuthData - buffLen=0"));
        return RV_ERROR_UNKNOWN;
    }

    /* Create list, if was not created yet */
    if (NULL==*phListAuthObj)
    {
        rv = SipAuthenticatorConstructAuthObjList((RvSipAuthenticatorHandle)pAuthMgr, hPage,
                                pParentObj, pfnLockAPI, pfnUnLockAPI, ppAuthListInfo, &hNewList);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailRestoreAuthData - failed to construct list(rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        *phListAuthObj = hNewList;
    }

    /* Create new Auth object */
    rv = AuthObjCreateInList(pAuthMgr, *ppAuthListInfo, hNewList, &pAuthObj);
    if (RV_OK != rv)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorHighAvailRestoreAuthData - failed to add element to list 0x%p (rv=%d:%s)",
            *phListAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    
    pCurrAuthDataID->strParamID       = HIGHAVAIL_AUTHDATA_VALIDITY_ID;
    pCurrAuthDataID->pParamObj        = &pAuthObj->bValid;
    pCurrAuthDataID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrAuthDataID += 1; /* Promote the pointer to next Subs ID*/

    pCurrAuthDataID->strParamID       = HIGHAVAIL_AUTHDATA_NONCECOUNT_ID;
    pCurrAuthDataID->pParamObj        = &pAuthObj->nonceCount;
    pCurrAuthDataID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrAuthDataID += 1; /* Promote the pointer to next Subs ID*/

    /* the restoreInfo struct wraps the auth-obj and auth-mgr, 
       and is given as the obj to the restore functions of the 
       authentication header, and the AUTS string */
    restoreInfo.pAuthMgr              = pAuthMgr;
    restoreInfo.pAuthObj              = pAuthObj;

    pCurrAuthDataID->strParamID       = HIGHAVAIL_AUTHDATA_HEADER_ID;
    pCurrAuthDataID->pParamObj        = &restoreInfo;
    pCurrAuthDataID->pfnSetParamValue = RestoreADStructHeader;
    pCurrAuthDataID += 1; /* Promote the pointer to next Subs ID*/
#ifdef RV_SIP_IMS_ON
    pCurrAuthDataID->strParamID       = HIGHAVAIL_AUTHDATA_AUTS_ID;
    pCurrAuthDataID->pParamObj        = &restoreInfo;
    pCurrAuthDataID->pfnSetParamValue = RestoreADStructAutsStr;
    pCurrAuthDataID += 1; /* Promote the pointer to next Subs ID*/
#endif /*RV_SIP_IMS_ON*/

    rv = SipCommonHighAvalRestoreFromBuffer(buffer, buffLen,
      AuthObjIDsArray,(RvUint32)(pCurrAuthDataID-AuthObjIDsArray),pAuthMgr->pLogSrc);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorHighAvailRestoreAuthData - failed to restore"));
        return rv;
    }

    /* If the restored header is not valid, and the application didn't
    registered to unsupported credentials callback, remove the restored header
    from the list */
    if (RV_FALSE == pAuthObj->bValid  &&
        RV_FALSE == AuthenticatorActOnCallback(pAuthMgr, AUTH_CALLBACK_UNSUPPORTED_CHALLENGE))
    {
        RvLogWarning(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "AuthenticatorHighAvailRestoreAuthData - unsupported credentials were removed"));
        
        AuthObjDestructAndRemoveFromList(pAuthMgr, hNewList, pAuthObj);
        
        /* Don't free Authentication Data header page,
        since this is a page of the while list */
    }

    return RV_OK;
}

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
                        IN    void*                 pParentObj,
                        IN    ObjLockFunc           pfnLockAPI,
                        IN    ObjUnLockFunc         pfnUnLockAPI,
                        INOUT AuthObjListInfo*     *ppAuthListInfo,
                        INOUT RLIST_HANDLE*         phListAuthObj)
{
    RvStatus                   rv;
    RLIST_HANDLE               hNewList = *phListAuthObj;       
    AuthObj*                   pAuthObj;
    
    if (0 > wwwAuthHeaderOffset && 0 > proxyAuthHeaderOffset)
    {
        return RV_OK;
    }

    /* Create list, if was not created yet */
    if (NULL==*phListAuthObj)
    {
        rv = SipAuthenticatorConstructAuthObjList((RvSipAuthenticatorHandle)pAuthMgr, hPage, 
            pParentObj, pfnLockAPI, pfnUnLockAPI, ppAuthListInfo, &hNewList);
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailRestoreAuthData3_0 - failed to construct list(rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        *phListAuthObj = hNewList;
    }

	if (wwwAuthHeaderOffset > 0) { 
		/* Restore WWW-Authentication header into the list */
		rv = AuthObjCreateInList(pAuthMgr, *ppAuthListInfo, hNewList, &pAuthObj);
		if (RV_OK != rv)
		{
			RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
				"AuthenticatorHighAvailRestoreAuthData3_0 - failed to add element 1 to list 0x%p (rv=%d:%s)",
				*phListAuthObj, rv, SipCommonStatus2Str(rv)));
			return rv;
		}
		
        rv = AuthObjSetParams(pAuthMgr, hNewList, pAuthObj, NULL, 
                            (buffer + wwwAuthHeaderOffset), ((*ppAuthListInfo)->bListIsValid));
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            	"AuthenticatorHighAvailRestoreAuthData3_0 - AuthObjSetParams (www-auth) failed (rv=%d:%s)",
            	rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        pAuthObj->nonceCount = nonceCount401;
	}

	if (proxyAuthHeaderOffset > 0) { 
		/* Restore proxy Authentication header into the list */
        rv = AuthObjCreateInList(pAuthMgr, *ppAuthListInfo, hNewList, &pAuthObj);
		if (RV_OK != rv)
		{
			RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
				"AuthenticatorHighAvailRestoreAuthData3_0 - failed to add element 2 to list 0x%p (rv=%d:%s)",
				*phListAuthObj, rv, SipCommonStatus2Str(rv)));
			return rv;
		}
        rv = AuthObjSetParams(pAuthMgr, hNewList, pAuthObj, NULL, 
                        (buffer + proxyAuthHeaderOffset),((*ppAuthListInfo)->bListIsValid));
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailRestoreAuthData3_0 - AuthObjSetParams (proxy-auth) failed (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        pAuthObj->nonceCount = nonceCount407;
	}

    /* If application registered to unsupported credentials, set the Validity*/
    if (RV_TRUE == AuthenticatorActOnCallback(pAuthMgr, AUTH_CALLBACK_UNSUPPORTED_CHALLENGE))
    {
        (*ppAuthListInfo)->bListIsValid = RV_TRUE;
    }

    return RV_OK;
}
#endif /* #ifdef  RV_SIP_HIGH_AVAL_3_0 */

/******************************************************************************
 * AuthenticatorHighAvailGetAuthDataStorageSize
 * ----------------------------------------------------------------------------
 * General: calculates the size of buffer consumed by the list of
 *          Auth objects in High Availability record format.
 *          For details, see High Availability module documentation.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthObj - Handle of the list.
 * Output: pStorageSize        - Requested size.
 *****************************************************************************/
RvStatus AuthenticatorHighAvailGetAuthDataStorageSize(
                          IN  AuthenticatorInfo*  pAuthMgr,
                          IN  RLIST_HANDLE        hListAuthObj,
                          OUT RvInt32             *pStorageSize)
{
    RvStatus rv;
    RLIST_ITEM_HANDLE   hListElement = NULL;
    AuthObj  *pAuthObj;
    RvInt32             currADStructStorageSize;
    RvInt32             totalStorageSize = 0;

    /* Goes through the list of data structures and store them one by one */
    pAuthObj = AuthenticatorGetAuthObjFromList(hListAuthObj, RVSIP_FIRST_ELEMENT, &hListElement);
    while (NULL!=pAuthObj)
    {
        /*Calculate the size, which the structure will consume in the storage*/
        rv = GetADStructStorageSize(pAuthMgr, pAuthObj,
                                     &currADStructStorageSize);
        if (RV_OK != rv)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "AuthenticatorHighAvailGetAuthDataStorageSize - hListAuthData 0x%p - Failed to get storage size (rv=%d:%s)",
                hListAuthObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        
        /*Calculate the size, which the prefix will consume in the storage*/
        totalStorageSize += SipCommonHighAvalGetTotalStoredParamLen(
            currADStructStorageSize, (RvUint32)strlen(HIGHAVAIL_AUTHDATA_ID));

        /* Get the next Auth object */
        pAuthObj = AuthenticatorGetAuthObjFromList(hListAuthObj, RVSIP_NEXT_ELEMENT, &hListElement);
    }

    *pStorageSize = totalStorageSize;

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*               STATIC FUNCTIONS                                            */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * GetADStructStorageSize
 * ----------------------------------------------------------------------------
 * General: calculates the size of buffer consumed by the Authentication Data
 *          structure in the High Availability record format.
 *          For details, see High Availability module documentation.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr            - Pointer to the Authenticator.
 *         hAuthObj            - Handle of the list.
 * Output: pStorageSize        - Requested size in bytes.
 *****************************************************************************/
static RvStatus GetADStructStorageSize(
                                IN  AuthenticatorInfo*  pAuthMgr,
                                IN  AuthObj* pAuthObj,
                                OUT RvInt32  *pADStructStorageSize)
{
    RvStatus rv;
    RvInt32  size = 0;
    RvInt32  tmpSize;
    RvUint32 encodedHeaderSize;
    HPAGE    hPage;

    /* Get storage size of the bValid field */
    tmpSize = SipCommonHighAvalGetTotalStoredBoolParamLen(
        pAuthObj->bValid, (RvUint32)strlen(HIGHAVAIL_AUTHDATA_VALIDITY_ID));
    size += tmpSize;

    /* Get storage size of the nonceCount field */
    tmpSize = SipCommonHighAvalGetTotalStoredInt32ParamLen(
        pAuthObj->nonceCount, (RvUint32)strlen(HIGHAVAIL_AUTHDATA_NONCECOUNT_ID));
    size += tmpSize;
    
    /* Get storage size of the Authentication Header field */
    rv = RvSipAuthenticationHeaderEncode(pAuthObj->hHeader,
        pAuthMgr->hGeneralPool, &hPage, &encodedHeaderSize);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "GetADStructStorageSize - failed to encode Authentication header 0x%p (rv=%d:%s)",
            pAuthObj->hHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    size += SipCommonHighAvalGetTotalStoredParamLen(
                    encodedHeaderSize, (RvUint32)strlen(HIGHAVAIL_AUTHDATA_HEADER_ID));
    RPOOL_FreePage(pAuthMgr->hGeneralPool, hPage);

#ifdef RV_SIP_IMS_ON
    if(pAuthObj->autsParamOffset>UNDEFINED)
    {
        size += SipCommonHighAvalGetTotalStoredParamLen(
                    RPOOL_Strlen(pAuthMgr->hElementPool,pAuthObj->hAuthObjPage,pAuthObj->autsParamOffset), 
                    (RvUint32)strlen(HIGHAVAIL_AUTHDATA_AUTS_ID));
    }
#endif

    *pADStructStorageSize = size;

    return RV_OK;
}

/******************************************************************************
 * StoreADStruct
 * ----------------------------------------------------------------------------
 * General: stores Auth object into the string buffer
 *          in the High Availability record format.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthMgr      - Pointer to the Authenticator.
 *         pAuthObj      - Authentication Data structure to be stored.
 *         maxBuffLen    - Size of the buffer.
 *         ppCurrBuffPos - Pointer to the place in buffer, where the structure
 *                         should be stored.
 *         pCurrLen      - Length of the record, currently stored in the buffer 
 * Output: ppCurrBuffPos - Pointer to the end of the Authentication Data
 *                         structure record in the buffer.
 *         pCurrLen      - Length of the record, currently stored in the buffer,
 *                         including currently stored structure record.
 *****************************************************************************/
static RvStatus StoreADStruct(  IN    AuthenticatorInfo*  pAuthMgr,
                                IN    AuthObj*            pAuthObj,
                                IN    RvUint32            maxBuffLen,
                                INOUT RvChar**            ppCurrBuffPos,
                                INOUT RvUint32*           pCurrLen)
{
    HPAGE    hPage;
    RvUint32 encodedHeaderSize;
    RvStatus rv;

    /* Store Validity Flag */
    rv = SipCommonHighAvalStoreSingleBoolParam(HIGHAVAIL_AUTHDATA_VALIDITY_ID,
        pAuthObj->bValid, maxBuffLen, ppCurrBuffPos, pCurrLen);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "StoreADStruct: AuthData 0x%p, SipCommonHighAvalStoreSingleBoolParam() failed (rv=%d:%s)",
            pAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Store nonceCount field */
    rv = SipCommonHighAvalStoreSingleInt32Param(HIGHAVAIL_AUTHDATA_NONCECOUNT_ID,
        pAuthObj->nonceCount, maxBuffLen, ppCurrBuffPos, pCurrLen);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "StoreADStruct: AuthData 0x%p, StoreSingleInt32Param() failed (rv=%d:%s)",
            pAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Store Authentication header */
    rv = RvSipAuthenticationHeaderEncode(pAuthObj->hHeader,
        pAuthMgr->hGeneralPool, &hPage, &encodedHeaderSize);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "StoreADStruct - AuthData 0x%p, failed to encode Authentication header 0x%p (rv=%d:%s)",
            pAuthObj, pAuthObj->hHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    rv = SipCommonHighAvalStoreSingleParamFromPage(HIGHAVAIL_AUTHDATA_HEADER_ID,
        pAuthMgr->hGeneralPool,hPage, 0/*offset on page*/,RV_TRUE/*bFreePage*/,
        maxBuffLen, encodedHeaderSize, ppCurrBuffPos, pCurrLen);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "StoreADStruct - AuthData 0x%p, failed to store Authentication header 0x%p (rv=%d:%s)",
            pAuthObj, pAuthObj->hHeader, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
#ifdef RV_SIP_IMS_ON
    if(pAuthObj->autsParamOffset > UNDEFINED)
    {
        encodedHeaderSize = RPOOL_Strlen(pAuthMgr->hElementPool,pAuthObj->hAuthObjPage, pAuthObj->autsParamOffset);
        rv = SipCommonHighAvalStoreSingleParamFromPage(HIGHAVAIL_AUTHDATA_AUTS_ID, 
            pAuthMgr->hElementPool, pAuthObj->hAuthObjPage, pAuthObj->autsParamOffset,
            RV_FALSE, maxBuffLen, encodedHeaderSize,ppCurrBuffPos, pCurrLen); 
        if (rv != RV_OK)
        {
            RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
                "StoreADStruct - AuthData 0x%p, failed to store AUTS string (rv=%d:%s)",
                pAuthObj, rv,SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif

    return RV_OK;
}

/******************************************************************************
 * RestoreADStructHeader
 * ----------------------------------------------------------------------------
 * General: restores Authentication Header into the provided Authentication
 *          Data structure. The header is restored from the buffer, containing
 *          string in the High Availability record format.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pHeaderID       - Pointer to the string, identifying the start of
 *                           Authentication header string in the buffer.
 *         headerIDLen     - Length of the pHeaderID string.
 *         pStoredHeader   - Pointer to the Authentication header string itself
 *         storedHeaderLen - Length of the Authentication header string.
 *         pObj            - Pointer to the auxiliary structure, unifying data,
 *                           required for restoring.
 * Output: none.
 *****************************************************************************/
static RvStatus RVCALLCONV RestoreADStructHeader(
                                      IN    RvChar   *pHeaderID,
                                      IN    RvUint32  headerIDLen,
                                      IN    RvChar   *pStoredHeader,
                                      IN    RvUint32  storedHeaderLen,
                                      IN    void     *pObj)
{
    RvStatus rv;
    RvChar   termChar;
    RestoreInfo*        pRestoreInfo        = (RestoreInfo*)pObj;
    AuthenticatorInfo*  pAuthMgr            = pRestoreInfo->pAuthMgr;
    AuthObj* pAuthObj = pRestoreInfo->pAuthObj;

    if (RV_TRUE != SipCommonMemBuffExactlyMatchStr(pHeaderID, headerIDLen,
                                                 HIGHAVAIL_AUTHDATA_HEADER_ID))
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RestoreADStructHeader - Authentication Data 0x%p - received illegal parameter ID",
            pAuthObj));
        return RV_ERROR_BADPARAM;
    }

    rv = RvSipAuthenticationHeaderConstruct(
            pAuthMgr->hMsgMgr, pAuthMgr->hElementPool, pAuthObj->hAuthObjPage, &pAuthObj->hHeader);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RestoreADStructHeader - Authentication Data 0x%p - failed to construct header (rv=%d:%s)",
            pAuthObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pStoredHeader,storedHeaderLen,&termChar);
    rv = RvSipAuthenticationHeaderParse(pAuthObj->hHeader, pStoredHeader);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar, storedHeaderLen, pStoredHeader);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RestoreADStructHeader - Authentication Data 0x%p - failed to header 0x%p",
            pAuthObj, pAuthObj->hHeader));
        return rv;
    }

    return RV_OK;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RestoreADStructAutsStr
 * ----------------------------------------------------------------------------
 * General: restores Auts string into the provided Authentication
 *          Data structure. The string is restored from the buffer, containing
 *          string in the High Availability record format.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pHeaderID       - Pointer to the string, identifying the start of
 *                           Authentication header string in the buffer.
 *         headerIDLen     - Length of the pHeaderID string.
 *         pStoredHeader   - Pointer to the Authentication header string itself
 *         storedHeaderLen - Length of the Authentication header string.
 *         pObj            - Pointer to the auxiliary structure, unifying data,
 *                           required for restoring.
 * Output: none.
 *****************************************************************************/
static RvStatus RVCALLCONV RestoreADStructAutsStr(
                                      IN    RvChar   *pHeaderID,
                                      IN    RvUint32  headerIDLen,
                                      IN    RvChar   *pStoredStr,
                                      IN    RvUint32  storedStrLen,
                                      IN    void     *pObj)
{
    RvStatus rv;
    RvChar   termChar;
    RestoreInfo*        pRestoreInfo        = (RestoreInfo*)pObj;
    AuthenticatorInfo*  pAuthMgr            = pRestoreInfo->pAuthMgr;
    AuthObj* pAuthObj = pRestoreInfo->pAuthObj;

    if (RV_TRUE != SipCommonMemBuffExactlyMatchStr(pHeaderID, headerIDLen,
                                                 HIGHAVAIL_AUTHDATA_AUTS_ID))
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RestoreADStructAutsStr - Authentication Data 0x%p - received illegal parameter ID",
            pAuthObj));
        return RV_ERROR_BADPARAM;
    }

    /* Prepare the string to copy */
    SipCommonHighAvalPrepareParamBufferForParse(pStoredStr,storedStrLen,&termChar);
    /* copy the AUTS string */
    rv = RPOOL_AppendFromExternalToPage(pAuthMgr->hElementPool, pAuthObj->hAuthObjPage, 
                                       pStoredStr, storedStrLen+1, /* +1 for the NULL termination */
                                       &pAuthObj->autsParamOffset);

    SipCommonHighAvalRestoreParamBufferAfterParse(termChar, storedStrLen, pStoredStr);
    if (rv != RV_OK)
    {
        RvLogError(pAuthMgr->pLogSrc,(pAuthMgr->pLogSrc,
            "RestoreADStructAutsStr - Authentication Data 0x%p - failed to set AUTS string ",
            pAuthObj));
        return rv;
    }

    return RV_OK;
}
#endif /*RV_SIP_IMS_ON*/

#endif /* #ifdef  RV_SIP_AUTH_ON */
#endif /* #ifdef  RV_SIP_HIGHAVAL_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES */


