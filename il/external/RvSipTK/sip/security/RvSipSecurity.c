/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/*********************************************************************************
 *                              <RvSipSecurity.c>
 *
 * The RvSipSecurity.c file contains implementation of API functions,
 * provided by Security module.
 * Security module implements:
 * 1. Security Agreement mechanism, described in RFC3329
 * 2. SIP message protection, described in 3GPP TS33.203
 * 3. IPsec for messages, sent and received by Stack objects, e.g. SecObj
 *
 * This file includes:
 * 1.Security Manager API functions
 * 2.Security Object API functions
 *
 * Manager is object, which holds configuration database of the Suciruty Module
 * and storage of Security Objects.
 *
 * Security Object is object, which provides protection of the SIP messages,
 * sent to specific destination. The protection can be ensured by TLS or IPsec.
 * Security Object implements protection mechanism establishment,
 * as it is described in 3GPP TS33.203.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonUtils.h"
#include "_SipCommonConversions.h"
#include "_SipTransport.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecurityTypes.h"
#include "_SipSecurityUtils.h"
#include "SecurityMgrObject.h"
#include "SecurityObject.h"
#include "rvimsipsec.h"

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTION PROTOTYPES                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         SECURITY MANAGER FUNCTIONS                        */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * RvSipSecMgrSetEvHandlers
 * ----------------------------------------------------------------------------
 * General: Sets event handlers for all Security events.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr     - The handle to the SecurityMgr.
 *            pEvHandlers - The structure with event handlers.
 *            structSize  - The size of the event handler structure.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrSetEvHandlers(
                                   IN  RvSipSecMgrHandle   hSecMgr,
                                   IN  RvSipSecEvHandlers* pEvHandlers,
                                   IN  RvInt32             structSize)
{
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;
    RvInt32 minStructSize = RvMin(structSize,((RvInt32)sizeof(RvSipSecEvHandlers)));

    if(NULL==pSecMgr  || NULL==pEvHandlers)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrSetEvHandlers(hSecMgr=%p) - Set event handlers", hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    memset(&(pSecMgr->appEvHandlers), 0, sizeof(RvSipSecEvHandlers));
    memcpy(&(pSecMgr->appEvHandlers), pEvHandlers, minStructSize);

    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * RvSipSecMgrCreateSecObj
 * ----------------------------------------------------------------------------
 * General: Allocates a security-object and exchanges handles with the
 *          application. The new security-object assumes the IDLE state.
 *          For the security-object to provide protection, the application should 
 *          do the following:
 *          1. Allocate the security-object using this function.
 *          2. Set the necessary data into the security-object, such as secure
 *             local port for IPsec, and call the RvSipSecObjInitiate() function.
 *          3. Set additional data, such as secure remote ports for IPsec and
 *             call the RvSipSecObjActivate() function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - The handle to the SecurityMgr.
 *          hAppSecObj - The application handle to the allocated security-object.
 * Output:  phSecObj   - The handle to the allocated security-object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrCreateSecObj(
                                   IN  RvSipSecMgrHandle    hSecMgr,
                                   IN  RvSipAppSecObjHandle hAppSecObj,
                                   OUT RvSipSecObjHandle*   phSecObj)
{
    RvStatus rv;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==phSecObj  ||  NULL==hSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrCreateSecObj(hSecMgr=%p) - Creating new Security Object",
        hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    rv = SecMgrCreateSecObj(pSecMgr, hAppSecObj, phSecObj);
    if(rv != RV_OK)
    {
        SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "RvSipSecMgrCreateSecObj(hSecMgr=%p) - Failed(rv=%d:%s)",
            hSecMgr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrCreateSecObj(hSecMgr=%p) - SecObj %p was created",
        pSecMgr, *phSecObj));

    return RV_OK;
}

/******************************************************************************
 * RvSipSecMgrSetAppHandle
 * ----------------------------------------------------------------------------
 * General: The application can have its own SecurityMgr handle.
 *          This function saves the handle in the SIP Stack SecurityMgr.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecMgr    - The handle to the SIP Stack SecurityMgr.
 *           pAppSecMgr - The application SecurityMgr handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrSetAppHandle(
                                   IN RvSipSecMgrHandle hSecMgr,
                                   IN void*             pAppSecMgr)
{
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrSetAppHandle(hSecMgr=%p) - Set application handler %p",
        hSecMgr, pAppSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    pSecMgr->pAppSecMgr = pAppSecMgr;
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * RvSipSecMgrGetAppHandle
 * ----------------------------------------------------------------------------
 * General: Returns the handle to the application SecurityMgr that the
 *          application set into the SIP Stack SecurityMgr, using the
 *          RvSipSecMgrSetAppHandle() function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr    - The handle to the SIP Stack SecurityMgr.
 * Output:    pAppSecMgr - The application SecurityMgr handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrGetAppHandle(
                                   IN RvSipSecMgrHandle hSecMgr,
                                   OUT void**           pAppSecMgr)
{
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecMgr || NULL==pAppSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrGetAppHandle(hSecMgr=%p) - Get application handler",
        hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    *pAppSecMgr = pSecMgr->pAppSecMgr;
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * RvSipSecMgrGetStackInstance
 * ----------------------------------------------------------------------------
 * General: Returns the handle to the SIP Stack instance to which this
 *          SecurityMgr belongs.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr         - The handle to the SIP Stack SecurityMgr.
 * Output:    phStackInstance - The handle to the SIP Stack instance.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrGetStackInstance(
                                   IN   RvSipSecMgrHandle  hSecMgr,
                                   OUT  void**             phStackInstance)
{
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecMgr || NULL==phStackInstance)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrGetStackInstance(hSecMgr=%p) - Get Stack Instance handler",
        hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    *phStackInstance = pSecMgr->hStack;
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return RV_OK;
}

/******************************************************************************
 * RvSipSecMgrCleanAllSAs
 * ----------------------------------------------------------------------------
 * General: Removes all IPsec security associations existing in the OS.
 *          This function will fail if there are security-objects in use.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr         - The handle to the SIP Stack SecurityMgr.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecMgrCleanAllSAs(
                                   IN   RvSipSecMgrHandle  hSecMgr)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecMgr);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvStatus crv;
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "RvSipSecMgrCleanAllSAs(hSecMgr=%p)", hSecMgr));

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex,pSecMgr->pLogMgr))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
    crv = rvImsIPSecCleanAll(pSecMgr->pLogMgr);
    if (RV_OK != crv)
    {
        RvStatus rv = CRV2RV(crv);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "RvSipSecMgrCleanAllSAs(hSecMgr=%p) - Failed to remove IPses from system (crv=%d,rv=%d:%s)",
            hSecMgr, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
    }
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    return CRV2RV(crv);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
}


/*---------------------------------------------------------------------------*/
/*                         SECURITY OBJECT FUNCTIONS                         */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 * RvSipSecObjInitiate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: After creating a security-object and setting the required link 
 *          configuration and IPsec parameters, this function opens secure local 
 *          addresses for Port-C and Port-S. Calling this function opens four secure 
 *          local addresses: UDP and TCP on Port-C, and UDP and TCP on Port-S. 
 *          Before calling the initiate functions, local ports and SPIs that will 
 *          be used for IPsec should be provided. The rest of the IPsec parameters
 *          can be provided at a later stage. If the application does not provide
 *          these parameters, the system will allocate them. After calling this 
 *          function the security-object assumes the INITIATED state.
 *
 *          Note the following:
 *          - Once the ports were allocated or SPIs were generated, these
 *            parameters cannot be changed.
 *          - The link configuration into the security-object must be set
 *            before calling RvSipSecObjInitiate().
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecObj - The handle to the security-object.
 ****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjInitiate(IN RvSipSecObjHandle hSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjInitiate(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjInitiate(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjInitiate(pSecObj);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjInitiate(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjInitiate(hSecObj=%p) - Done", hSecObj));

    return RV_OK;
}

/*****************************************************************************
 * RvSipSecObjActivate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: After a security-object was initiated, this function causes the 
 *          security-object to create SAs in the system. Before calling this 
 *          function, all IPsec parameters that were not already provided must 
 *          be allocated, such as remote secure ports and algorithms. After 
 *          calling this function the security-object assumes the ACTIVE
 *          state. Once the associations were created, messages that are sent 
 *          through these secure ports will be protected with IPsec.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjActivate(IN RvSipSecObjHandle hSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjActivate(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjActivate(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjActivate(pSecObj);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjActivate(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjActivate(hSecObj=%p) - Done", hSecObj));

    return RV_OK;
}

/*****************************************************************************
 * RvSipSecObjShutdown (Security Object API)
 * ----------------------------------------------------------------------------
 * General: This function should be called in the ACTIVE state of the 
 *          security-object to shutdown the security-object protection. The 
 *          function performs the following actions:
 *          - Closes secure connections if they were established.
 *          - Closes local addresses opened by the security-object.
 *          - Removes SAs from the system.
 *          A security-object can be shutdown only if it is in ACTIVE state. 
 *          Otherwise it should be terminated using RvSipSecObjTerminate(). 
 *			After calling this function the security-object assumes the CLOSING or 
 *			CLOSED state. Note that after calling this function, the security-object
 *			can no longer be used for sending secure requests.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjShutdown(IN RvSipSecObjHandle hSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjShutdown(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjShutdown(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjShutdown(pSecObj);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjShutdown(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjShutdown(hSecObj=%p) - Done", hSecObj));

    return RV_OK;
}

/*****************************************************************************
 * RvSipSecObjTerminate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Frees resources that were consumed by a security-object, after which
 *			frees the security-object. After calling this function the security-object
 *			assumes the TERMINATED state.
 *			Note that a security-object that was created by the application must 
 *			also be terminated by the application. The SIP Stack will never 
 *			terminate applicative security-objects. This function should be 
 *			called after the security-object has reached the CLOSED state.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjTerminate(
                        IN RvSipSecObjHandle hSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjTerminate(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjTerminate(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    if (RVSIP_SECOBJ_STATE_TERMINATED == pSecObj->eState)
    {
        RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjTerminate(hSecObj=%p) - Security Object is in TERMINATED state already",
            hSecObj));
        SecObjUnlockAPI(pSecObj);
        return RV_OK;
    }

    rv = SecObjTerminate(pSecObj);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjTerminate(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjTerminate(hSecObj=%p) - Done", hSecObj));

    return RV_OK;
}

/******************************************************************************
 * RvSipSecObjSetAppHandle
 * ----------------------------------------------------------------------------
 * General: The application can have its own security-object handle.
 *          This function saves the handle in the SIP Stack security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - The handle to the SIP Stack security-object.
 *          hAppSecObj - The application security-object handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetAppHandle(
                                   IN RvSipSecObjHandle    hSecObj,
                                   IN RvSipAppSecObjHandle hAppSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjSetAppHandle(hSecObj=%p, hAppSecObj=%p)",
        hSecObj, hAppSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetAppHandle(hSecObj=%p, hAppSecObj=%p) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, hAppSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    
    if (NULL!=pSecObj->hAppSecObj && NULL!=hAppSecObj)
    {
        RvLogWarning(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetAppHandle(hSecObj=%p, hAppSecObj=%p) - override existing hAppSecObj=%p",
            hSecObj, hAppSecObj, pSecObj->hAppSecObj));
    }

    pSecObj->hAppSecObj = hAppSecObj;
    
    /* If Application owner was removed, check and remove Security Object */
    if (NULL == pSecObj->hAppSecObj)
    {
        rv = SecObjRemoveIfNoOwnerIsLeft(pSecObj);
        if (RV_OK != rv)
        {
            SecObjFree(pSecObj);
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "RvSipSecObjSetAppHandle(pSecObj=%p) - Failed to remove object",
                pSecObj));
            SecObjUnlockAPI(pSecObj);
            return rv;
        }
    }
    
    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
}

/******************************************************************************
 * RvSipSecObjGetAppHandle
 * ----------------------------------------------------------------------------
 * General:  Gets the application handle stored in the SIP Stack security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj     - The handle to the SIP Stack security-object.
 * Output:  phAppSecObj - The application security-object handle.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetAppHandle(
                                   IN  RvSipSecObjHandle     hSecObj,
                                   OUT RvSipAppSecObjHandle* phAppSecObj)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj || NULL==phAppSecObj)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjGetAppHandle(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetAppHandle(hSecObj=%p) - failed to lock Security Object",
            hSecObj));
        return rv;
    }
    
    *phAppSecObj = pSecObj->hAppSecObj;
    
    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
}

/******************************************************************************
 * RvSipSecObjGetSecAgreeObject
 * ----------------------------------------------------------------------------
 * General: Get the Security-Agreement owner of this Security Object if exists.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - Handle to the Security Object
 * Output:  ppSecAgree - Pointer to the Security-Agreement object
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetSecAgreeObject(
                        IN  RvSipSecObjHandle   hSecObj,
                        OUT void**              ppSecAgree)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr;

    if(NULL==pSecObj || NULL==ppSecAgree)
    {
        return RV_ERROR_BADPARAM;
    }

	pSecMgr = pSecObj->pMgr;

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"RvSipSecObjGetSecAgreeObject(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

	RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
				"RvSipSecObjGetSecAgreeObject(hSecObj=%p): SecAgree %p",
				hSecObj, pSecObj->pSecAgree));

    *ppSecAgree = pSecObj->pSecAgree;

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/*****************************************************************************
 * RvSipSecObjSetLinkCfg (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the configuration of the link to be protected by the
 *          security-object. The configuration consists of such parameters as
 *          local and remote IPs. Either of the IPs can be NULL when calling 
 *			this function. This lets the application set the addresses in a few 
 *			steps.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj  - The handle to the security-object.
 *          pLinkCfg - The link configuration.
 ****************************************************************************/

RVAPI RvStatus RVCALLCONV RvSipSecObjSetLinkCfg(
                        IN RvSipSecObjHandle    hSecObj,
                        IN RvSipSecLinkCfg*     pLinkCfg)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj || NULL==pLinkCfg)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjSetLinkCfg(hSecObj=%p, LocalIP=%s, RemoteIP=%s)",
        hSecObj, (NULL==pLinkCfg->strLocalIp) ? "" : pLinkCfg->strLocalIp,
        (NULL==pLinkCfg->strRemoteIp) ? "" : pLinkCfg->strRemoteIp));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetLinkCfg(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    /* Local IP can be set in IDLE objects only */
    if (NULL != pLinkCfg->strLocalIp  &&  RVSIP_SECOBJ_STATE_IDLE != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetLinkCfg(hSecObj=%p) - can't set Local IP in state %s. Should be IDLE",
            hSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockAPI(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    /* Remote IP can be set in IDLE or INITIATED objects only */
    if (NULL != pLinkCfg->strRemoteIp  &&
        (RVSIP_SECOBJ_STATE_IDLE != pSecObj->eState  &&  RVSIP_SECOBJ_STATE_INITIATED != pSecObj->eState)
       )
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetLinkCfg(hSecObj=%p) - can't set Remote IP in state %s. Should be IDLE or INITIATED",
            hSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockAPI(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL != pLinkCfg->strLocalIp)
    {
        RvSipTransportAddr tmpAddr;
        /* Make sure the IP string can be stored in our buffers  */
        if (RVSIP_TRANSPORT_LEN_STRING_IP <= strlen(pLinkCfg->strLocalIp))
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "RvSipSecObjSetLinkCfg(hSecObj=%p) - can't set Local IP: string should be shoter than %d bytes",
                hSecObj, RVSIP_TRANSPORT_LEN_STRING_IP));
            SecObjUnlockAPI(pSecObj);
            return RV_ERROR_BADPARAM;
        }
        tmpAddr.Ipv6Scope = 0;
        rv = SipTransportConvertStrIp2TransportAddress(
            pSecObj->pMgr->hTransportMgr, pLinkCfg->strLocalIp, &tmpAddr);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "RvSipSecObjSetLinkCfg(hSecObj=%p) - failed to parse set Local IP %s",
                hSecObj, pLinkCfg->strLocalIp));
            SecObjUnlockAPI(pSecObj);
            return RV_ERROR_BADPARAM;
        }
        strcpy(pSecObj->strLocalIp, tmpAddr.strIP);
        pSecObj->ipv6Scope = tmpAddr.Ipv6Scope;
    }
    if (NULL != pLinkCfg->strRemoteIp)
    {
        RvSipTransportAddr tmpAddr;
        /* Make sure the IP string can be stored in our buffers  */
        if (RVSIP_TRANSPORT_LEN_STRING_IP <= strlen(pLinkCfg->strRemoteIp))
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "RvSipSecObjSetLinkCfg(hSecObj=%p) - can't set Remote IP: string should be shoter than %d bytes",
                hSecObj, RVSIP_TRANSPORT_LEN_STRING_IP));
            SecObjUnlockAPI(pSecObj);
            return RV_ERROR_BADPARAM;
        }
        rv = SipTransportConvertStrIp2TransportAddress(
            pSecObj->pMgr->hTransportMgr, pLinkCfg->strRemoteIp, &tmpAddr);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "RvSipSecObjSetLinkCfg(hSecObj=%p) - failed to parse set Local IP %s",
                hSecObj, pLinkCfg->strLocalIp));
            SecObjUnlockAPI(pSecObj);
            return RV_ERROR_BADPARAM;
        }
        strcpy(pSecObj->strRemoteIp, tmpAddr.strIP);
    }

#ifdef RV_SIGCOMP_ON
	/* Set sigcomp parameter to security-object */
	rv = SecObjSetSigcompParams(pSecObj, RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED, 
								NULL, pLinkCfg);
	if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetLinkCfg(hSecObj=%p) - failed to set sigcomp parameters",
            hSecObj));
        SecObjUnlockAPI(pSecObj);
        return rv;
    }
#endif /* #ifdef RV_SIGCOMP_ON */

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * RvSipSecObjGetLinkCfg
 * ----------------------------------------------------------------------------
 * General: Gets the configuration of the link protected by the security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj  - The handle to the security-object.
 * Output:  pLinkCfg - The link configuration.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetLinkCfg(
                        IN  RvSipSecObjHandle    hSecObj,
                        OUT RvSipSecLinkCfg*     pLinkCfg)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj || NULL==pLinkCfg)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjGetLinkCfg(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetLinkCfg(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    if (NULL != pLinkCfg->strLocalIp)
    {
        /* Copy IPv4 addr*/
        if (NULL == strchr(pSecObj->strLocalIp,':'))
        {
            strcpy(pLinkCfg->strLocalIp, pSecObj->strLocalIp);
        }
        else
        /* Copy IPv6 addr */
        {
            if (0 == pSecObj->ipv6Scope)
            {
                sprintf(pLinkCfg->strLocalIp,"[%s]",pSecObj->strLocalIp);
            }
            else
            {
                sprintf(pLinkCfg->strLocalIp,"[%s]%%%d",
                    pSecObj->strLocalIp,pSecObj->ipv6Scope);
            }
        }
    }
    if (NULL != pLinkCfg->strRemoteIp)
    {
        /* Copy IPv4 addr*/
        if (NULL == strchr(pSecObj->strRemoteIp,':'))
        {
            strcpy(pLinkCfg->strRemoteIp, pSecObj->strRemoteIp);
        }
        else
            /* Copy IPv6 addr */
        {
            sprintf(pLinkCfg->strRemoteIp,"[%s]",pSecObj->strRemoteIp);
        }
    }

#ifdef RV_SIGCOMP_ON
	pLinkCfg->eMsgCompType = pSecObj->eMsgCompType;
	if (NULL != pLinkCfg->strSigcompId)
	{
		strcpy(pLinkCfg->strSigcompId, pSecObj->strSigcompId);
	}
#endif /* #ifdef RV_SIGCOMP_ON */

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/*****************************************************************************
 * RvSipSecObjSetIpsecParams (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the IPsec parameters required for message protection.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object.
 *          pIpsecGeneralParams - The parameters that were negotiated by the 
								  local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters generated by the
 *                                remote side. This pointer can be NULL.
 ****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetIpsecParams(
                        IN RvSipSecObjHandle        hSecObj,
                        IN RvSipSecIpsecParams*     pIpsecGeneralParams,
                        IN RvSipSecIpsecPeerParams* pIpsecLocalParams,
                        IN RvSipSecIpsecPeerParams* pIpsecRemoteParams)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(pIpsecGeneralParams);
    RV_UNUSED_ARG(pIpsecLocalParams);
    RV_UNUSED_ARG(pIpsecRemoteParams);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjSetIpsecParams(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetIpsecParams(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjSetIpsecParams(pSecObj, pIpsecGeneralParams,
                              pIpsecLocalParams, pIpsecRemoteParams);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetIpsecParams(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
#endif /*#if !(RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/******************************************************************************
 * RvSipSecObjGetIpsecParams
 * ----------------------------------------------------------------------------
 * General: Gets the IPsec parameters set into the security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object
 * Output:  pIpsecGeneralParams - The parameters that were negotiated by
 *                                the local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters that are generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters that are generated by the
 *                                remote side. This pointer can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetIpsecParams(
                        IN  RvSipSecObjHandle           hSecObj,
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(pIpsecGeneralParams);
    RV_UNUSED_ARG(pIpsecLocalParams);
    RV_UNUSED_ARG(pIpsecRemoteParams);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjGetIpsecParams(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetIpsecParams(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjGetIpsecParams(pSecObj, pIpsecGeneralParams,
                              pIpsecLocalParams, pIpsecRemoteParams);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetIpsecParams(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
#endif /*#if !(RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/******************************************************************************
 * RvSipSecObjGetNumOfOwners
 * ----------------------------------------------------------------------------
 * General: Gets the current number of SIP Stack Objects, using the security-object,
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj      - The handle to the security-object.
 * Output:  pNumOfOwners - The requested number.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetNumOfOwners(
                        IN  RvSipSecObjHandle hSecObj,
                        IN  RvUint32*         pNumOfOwners)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;
    
    if(NULL==hSecObj || NULL==pNumOfOwners)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjGetNumOfOwners(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetNumOfOwners(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    *pNumOfOwners = pSecObj->counterOwners;

    SecObjUnlockAPI(pSecObj);
    return RV_OK;
}

/*@****************************************************************************
 * RvSipSecObjGetNumOfPendingOwners (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the current number of transactions and transmitters that use
 *          a security-object that are in the middle of sending a message or waiting 
 *			for an incoming response.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object
 * Output:  pNumOfPendingOwners - The requested number.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetNumOfPendingOwners(
                        IN  RvSipSecObjHandle hSecObj,
                        IN  RvUint32*         pNumOfPendingOwners)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(pNumOfPendingOwners);
    return RV_ERROR_ILLEGAL_ACTION;
#else
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj || NULL==pNumOfPendingOwners)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjGetNumOfPendingOwners(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetNumOfPendingOwners(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    if (NULL != pSecObj->pIpsecSession)
    {
        *pNumOfPendingOwners = pSecObj->pIpsecSession->counterTransc;
    }
    else
    {
        *pNumOfPendingOwners = 0;
    }

    SecObjUnlockAPI(pSecObj);
    return RV_OK;
#endif /*#if !(RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/*****************************************************************************
 * RvSipSecUtilInitIpsecParams (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Initializes the IPsec parameter structures to be used with SET and GET
 *			API functions.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Output:  pIpsecGeneralParams - The parameters that were negotiated by
 *                                the local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters that are generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters that are generated by the
 *                                remote side. This pointer can be NULL.
 ****************************************************************************/
RVAPI void RVCALLCONV RvSipSecUtilInitIpsecParams(
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(pIpsecGeneralParams);
    RV_UNUSED_ARG(pIpsecLocalParams);
    RV_UNUSED_ARG(pIpsecRemoteParams);
    return;
#else
    if (NULL != pIpsecGeneralParams)
    {
        memset(pIpsecGeneralParams, 0, sizeof(RvSipSecIpsecParams));
        pIpsecGeneralParams->eEAlg = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
        pIpsecGeneralParams->eIAlg = RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
        pIpsecGeneralParams->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    }
    if (NULL != pIpsecLocalParams)
    {
        memset(pIpsecLocalParams, 0, sizeof(RvSipSecIpsecPeerParams));
    }
    if (NULL != pIpsecRemoteParams)
    {
        memset(pIpsecRemoteParams, 0, sizeof(RvSipSecIpsecPeerParams));
    }
#endif /*#if !(RV_IMS_IPSEC_ENABLED==RV_YES)*/
}
#ifdef RV_IMS_TUNING
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/*@****************************************************************************
 * RvSipSecObjSetIpsecExtensionData (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the data, required by various extensions to the IPsec model,
 *          supported by default by the Security Object.
 *          This model uses IPsec ESP in transport mode.
 *          An example of extension to this basic model can be establishing
 *          of ESP transport mode security associations in environment,
 *          configured for tunnneling mode. In this case the additional data
 *          should be provided by the Application.
 *          The RvSipSecObjSetIpsecExtensionData API function enables the 
 *          application to provide such configuration data. For transport mode
 *          associations in tunneling environment see
 *          the LinuxOutboundTunnelData, InboundTunnelData,
 *          OutboundPolicyPriority and InboundPolicyPriority extensions,
 *          defined by the RvImsAppExtensionType type.
 *
 *          Data for extensions is provided in form of array of elements.
 *          Each of them can provide a piece of data for any of supported
 *          extensions. Available extensions are enumerated by
 *          RvImsAppExtensionType type.
 *          On IPsec activation, performed by RvSipSecObjActivate API function,
 *          the array is searched, and extensions, data for that was found in
 *          the array, are switched on.
 *          Therefore in multithreading environment the application is
 *          responsible to prevent write access to the array memory,
 *          while any thread stays in the RvSipSecObjActivate API function.
 *
 *          The application is responsible to allocate and free the memory,
 *          used by the array.
 *
 *          The RvSipSecObjSetIpsecExtensionData API function can be called
 *          by the application any time before call to RvSipSecObjActivate API
 *          function.
 *          After call to RvSipSecObjActivate API function the memory, used by
 *          the array of data elements, can be freed.
 *          Each call overwrites pointers to the data, set by previous call.
 *
 *          For more details see documentation for RvImsAppExtensionData struct
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj          - The handle to the security-object.
 *          pExtDataArray    - The array of Extension Data elements.
 *                             The application should not free memory, used by
 *                             this array, until ACTIVE or state will be
 *                             reported for the Security Object.
 *          extDataArraySize - The number of elements in pExtDataArray array.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetIpsecExtensionData(
                        IN RvSipSecObjHandle        hSecObj,
                        IN RvImsAppExtensionData*   pExtDataArray,
                        IN RvSize_t                 extDataArraySize)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "RvSipSecObjSetIpsecExtensionData(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetIpsecExtensionData(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjSetIpsecExtensionData(pSecObj, pExtDataArray, extDataArraySize);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjSetIpsecExtensionData(hSecObj=%p) - failed to set IPsec extensions data (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        SecObjUnlockAPI(pSecObj);
        return rv;
    }

    SecObjUnlockAPI(pSecObj);
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/*@****************************************************************************
 * RvSipSecObjGetIpsecExtensionData (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the IPsec extension data, set into the Security Object by
 *          the application using RvSipSecObjSetIpsecExtensionData API function
 *          For more details see documentation for
 *          RvSipSecObjSetIpsecExtensionData API function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj           - The handle to the security-object.
 * Output:  ppExtDataArray    - The array of Extension Data elements.
 *                              The application should not free memory, used by
 *                              this array, until TERMINATED state will be
 *                              reported for the Security Object.
 *          pExtDataArraySize - The number of elements in pExtDataArray array.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetIpsecExtensionData(
                        IN  RvSipSecObjHandle        hSecObj,
                        OUT RvImsAppExtensionData**  ppExtDataArray,
                        OUT RvSize_t*                pExtDataArraySize)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;

    if(NULL==hSecObj || NULL==ppExtDataArray || NULL==pExtDataArraySize)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "RvSipSecObjGetIpsecExtensionData(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    if (NULL != pSecObj->pIpsecSession)
    {
        *ppExtDataArray = pSecObj->pIpsecSession->ccSAData.iExtData;
        *pExtDataArraySize = pSecObj->pIpsecSession->ccSAData.iExtDataLen;
    }

    SecObjUnlockAPI(pSecObj);
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#endif /*#ifdef RV_IMS_TUNING*/

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTIONS                                  */
/*---------------------------------------------------------------------------*/

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RvStatus RVCALLCONV RvSipSecObjGetSecureServerPort(
                                IN   RvSipSecObjHandle        hSecObj,
                                IN   RvSipTransport     eTransportType,
                                OUT  RvUint16*          pSecureServerPort) {

  return SecObjGetSecureServerPort((SecObj*)hSecObj,
                                eTransportType,
                                pSecureServerPort);
}

RvSipTransportConnectionHandle RvSipSecObjGetServerConnection(RvSipSecObjHandle        hSecObj) {

  RvSipTransportConnectionHandle hConn=NULL;
  SecObj* so = (SecObj*)hSecObj;

  if(so && so->pIpsecSession) hConn=so->pIpsecSession->hConnS;

  return hConn;
}

#endif
//SPIRENT_END

#endif /*#ifdef RV_SIP_IMS_ON*/

