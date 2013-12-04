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

/******************************************************************************
 *                              <SecurityObject.c>
 *
 * The SecurityObject.c file implements the Security Object object.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          February 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "_SipCommonConversions.h"
#include "_SipTransport.h"
#include "RvSipTransport.h"
#include "RvSipViaHeader.h"
#include "RvSipMsg.h"

#ifdef RV_SIP_IMS_ON

#include "_SipSecurityUtils.h"
#include "_SipSecuritySecObj.h"
#include "SecurityObject.h"
#include "SecurityCallbacks.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"

#endif
/* SPIRENT_END */

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTIONS PROTOTYPES                      */
/*---------------------------------------------------------------------------*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)

static RvStatus AllocateIpsecSession(IN SecObj* pSecObj);
static RvStatus InitiateIpsec(IN SecObj* pSecObj);
static RvStatus ActivateIpsec(IN SecObj* pSecObj);
static RvStatus ShutdownIpsec(
                              IN SecObj*  pSecObj,
                              OUT RvBool* pShutdownCompleted);

static RvStatus RemoveIpsec(IN SecObj*  pSecObj);

static RvBool CheckIpsecParams(IN SecObj* pSecObj);

static RvStatus InitializeCcSAData(IN SecObj* pSecObj);

static RvStatus OpenLocalAddress(
                            IN  SecObj*                        pSecObj,
                            IN  RvSipTransportAddr*            pAddressDetails,
                            IN  RvBool                         bDontOpen,
                            OUT RvSipTransportLocalAddrHandle* phLocalAddr);

static RvStatus CloseLocalAddress(
                            IN    SecObj*                         pSecObj,
                            INOUT RvSipTransportLocalAddrHandle*  phLocalAddr);

static RvStatus CloseConnection(
                            IN SecObj*                        pSecObj,
                            IN RvSipTransportConnectionHandle hConn,
                            IN RvBool                         bDetachFromConn);

#ifdef SIP_DEBUG
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void LogIpsecParameters(IN SecObj* pSecObj);
#endif
#endif

#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/



#if (RV_TLS_TYPE != RV_TLS_NONE)

static RvStatus ShutdownTls(IN  SecObj*  pSecObj,
                            OUT RvBool*  pShutdownCompleted);

static RvStatus RemoveTls(IN SecObj*  pSecObj);

#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/



#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus LockSecObj(IN SecObj*        pSecObj,
                           IN SecMgr*        pSecMgr,
                           IN SipTripleLock* pTtripleLock,
                           IN RvInt32        identifier);
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

static RvBool IsConnOwnedBySecObj(
                                  IN SecObj* pSecObj,
                                  IN RvSipTransportConnectionHandle  hConn);

static RvBool IsIpEquals(IN RvLogSource*         pLogSrc,
                         IN RvSipMsgHandle       hMsg,
                         IN SipTransportAddress* pRecvFromAddr);


static RvStatus CloseLocalAddresses(IN SecObj* pSecObj);

static RvStatus CloseConnections(
                            IN  SecObj*                    pSecObj,
                            IN  RvSipSecurityMechanismType eSecMechanism,
                            IN  RvBool                     bDetachFromConn,
                            OUT RvUint32*                  pNumOfClosedConn);

static RvStatus HandleUnownedConnState(
                            IN SecObj*                         pSecObj,
                            IN RvSipTransportConnectionHandle  hConn,
                            IN RvSipTransportConnectionState   eConnState);

/* Prevent compilation warning UNUSED FUNCTION */
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)

static RvStatus OpenConnection(
                            IN  SecObj*                         pSecObj,
                            IN  RvSipTransport                  eTransport,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN  RvAddress*                      pRemoteAddr,
                            OUT RvSipTransportConnectionHandle* phConn);

#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)*/

static RvStatus GetDefaultTransport(IN  SecObj*         pSecObj,
                                    OUT RvSipTransport* peTransportType);

/*---------------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecObjInitiate
 * ----------------------------------------------------------------------------
 * General: initiates Security Object in context of security mechanism set into
 *          the object.
 *          As a result, the object will move to INITIATED state.
 *          For IPsec, for example, this function generates local parameters,
 *          such as SPIs and Secure Ports.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjInitiate(IN SecObj* pSecObj)
{
    RvStatus rv;

    /* Initiating can be done for IDLE objects only */
    if (RVSIP_SECOBJ_STATE_IDLE != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjInitiate(pSecObj=%p) - wrong state %s. Should be IDLE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Initiate IPsec, if it was requested */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (RV_TRUE == SecObjSupportsIpsec(pSecObj))
    {
        rv = InitiateIpsec(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjInitiate(pSecObj=%p) - failed to initiate IPsec(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    rv = SecObjChangeState(pSecObj,
                RVSIP_SECOBJ_STATE_INITIATED, RVSIP_SECOBJ_REASON_UNDEFINED);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjInitiate(pSecObj=%p) - failed to change state to INITIATED(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SecObjActivate
 * ----------------------------------------------------------------------------
 * General: activates message protection using the security mechanism, set into
 *          the Security Object.
 *          As a result, the object will move to ACTIVE state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjActivate(IN SecObj* pSecObj)
{
    RvStatus rv;

    /* Initiating can be done for INITIATED objects only */
    if (RVSIP_SECOBJ_STATE_INITIATED != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjActivate(pSecObj=%p) - Not in INITIATED state. Can't be started",
            pSecObj));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    switch (pSecObj->eSecMechanism)
    {
        /* Nothing to be done for TLS */
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
            break;
#endif

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
            rv = ActivateIpsec(pSecObj);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjActivate(pSecObj=%p) - failed to start IPsec(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            break;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

        case RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED:
        {
			RvBool bTlsInitiated   = RV_FALSE;
			RvBool bIpsecInitiated = RV_FALSE;
#if (RV_TLS_TYPE != RV_TLS_NONE)
            if (RVSIP_SECURITY_MECHANISM_TYPE_TLS & pSecObj->secMechsToInitiate)
            {
                bTlsInitiated = RV_TRUE;
            }
#endif
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
            if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & pSecObj->secMechsToInitiate)
            {
                rv = ActivateIpsec(pSecObj);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjActivate(pSecObj=%p) - failed to start IPsec(rv=%d:%s)",
                        pSecObj, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
                bIpsecInitiated = RV_TRUE;
            }
#endif
            if (RV_FALSE==bTlsInitiated  &&  RV_FALSE==bIpsecInitiated)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjActivate(pSecObj=%p) - unsupported mechanism %s, requested mechanisms: 0x%x",
                    pSecObj,
                    SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism),
                    pSecObj->secMechsToInitiate));
                return RV_ERROR_UNKNOWN;
            }
        }
        break;

        default:
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjActivate(pSecObj=%p) - unsupported mechanism %s, requested mechanisms: 0x%x",
                pSecObj,
                SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism),
                pSecObj->secMechsToInitiate));
            return RV_ERROR_UNKNOWN;
    }

    rv = SecObjChangeState(pSecObj,
            RVSIP_SECOBJ_STATE_ACTIVE, RVSIP_SECOBJ_REASON_UNDEFINED);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjActivate(pSecObj=%p) - failed to change state to ACTIVE(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SecObjShutdown
 * ----------------------------------------------------------------------------
 * General: stops message protection.
 *          As a result of this function call, the Security Object will be not
 *          available for new owners. Now new SIP requests will be delivered
 *          using this Security Object. Only SIP responses.
 *          On termination of existing owners, CLOSED state will be raised.
 *          The object can't be restarted.
 *          Number of owners is monitored using COUNTER field of the object.
 *          The counter is increased on RvSipXXXSetSecObj(hSecObj) call and is
 *          decreased on RvSipXXXSetSecObj(NULL) call, where XXX stands for
 *          any Toolkit object, such as CallLeg, Subscription, Transmitter etc.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdown(IN SecObj* pSecObj)
{
    RvStatus rv;

#if (RV_TLS_TYPE != RV_TLS_NONE) || (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvStatus rv1;
#endif
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvBool   pShutdownCompletedTls = RV_TRUE;
#endif
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvBool   pShutdownCompletedIpsec = RV_TRUE;
#endif

    /* Only ACTIVE object can be shutdowned */
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjShutdown(pSecObj=%p) - wrong state %s. Should be ACTIVE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    switch (pSecObj->eSecMechanism)
    {
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
            rv = ShutdownTls(pSecObj, &pShutdownCompletedTls);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjShutdown(pSecObj=%p) - failed to shutdown TLS(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
            rv = ShutdownIpsec(pSecObj, &pShutdownCompletedIpsec);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjShutdown(pSecObj=%p) - failed to shutdown IPsec(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            break;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

        /* If mechanism was not set, shutdown all requested mechanisms */
        case RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED:
            rv = RV_OK;

#if (RV_TLS_TYPE != RV_TLS_NONE)
            if (RVSIP_SECURITY_MECHANISM_TYPE_TLS & pSecObj->secMechsToInitiate)
            {
                rv1 = ShutdownTls(pSecObj, &pShutdownCompletedTls);
                if (RV_OK != rv1)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjShutdown(pSecObj=%p) - failed to shutdown TLS(rv=%d:%s)",
                        pSecObj, rv1, SipCommonStatus2Str(rv1)));
                    rv = rv1;
                }
            }
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
            if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & pSecObj->secMechsToInitiate)
            {
                rv1 = ShutdownIpsec(pSecObj, &pShutdownCompletedIpsec);
                if (RV_OK != rv1)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjShutdown(pSecObj=%p) - failed to shutdown IPsec(rv=%d:%s)",
                        pSecObj, rv1, SipCommonStatus2Str(rv1)));
                    rv = rv1;
                }
            }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
            if (RV_OK != rv)
            {
                return rv;
            }
            break;

        default:
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjShutdown(pSecObj=%p) - unsupported mechanism %s",
                pSecObj,
                SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism)));
            return RV_ERROR_UNKNOWN;
    }

    if (0
#if (RV_TLS_TYPE != RV_TLS_NONE)
        || RV_FALSE == pShutdownCompletedTls
#endif
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        || RV_FALSE == pShutdownCompletedIpsec
#endif
        )
    {
        RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjShutdown(pSecObj=%p) - Shutdown was not completed. Wait.",
            pSecObj));

        rv = SecObjChangeState(pSecObj,
                    RVSIP_SECOBJ_STATE_CLOSING,RVSIP_SECOBJ_REASON_UNDEFINED);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjShutdown(pSecObj=%p) - failed to change state to CLOSING(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
        }
        return rv;
    }

    rv = SecObjChangeState(pSecObj,
                    RVSIP_SECOBJ_STATE_CLOSED, RVSIP_SECOBJ_REASON_UNDEFINED);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjShutdown(pSecObj=%p) - failed to change state to CLOSING(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SecObjTerminate
 * ----------------------------------------------------------------------------
 * General: free resources, used by the Security Object.
 *          This function is permitted for objects in IDLE, INITIATED or
 *          CLOSED state.
 *          In order to move the object to CLOSED state RvSipSecObjShutdown
 *          should be called.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjTerminate(IN SecObj* pSecObj)
{
    RvStatus rv;

    /* Termination can be done for IDLE, INITIATED, ACTIVE or CLOSED objects */
    if (RVSIP_SECOBJ_STATE_IDLE      != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_CLOSED    != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjTerminate(pSecObj=%p) - wrong state %s. Should be IDLE, INITIATED or CLOSED",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Termination can be done only if there are no owners */
    if (0 != pSecObj->counterOwners)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjTerminate(pSecObj=%p) - Can't terminate: there are %d owners",
            pSecObj, pSecObj->counterOwners));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = SecObjChangeState(pSecObj,
                RVSIP_SECOBJ_STATE_TERMINATED, RVSIP_SECOBJ_REASON_UNDEFINED);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjTerminate(pSecObj=%p) - failed to change state to TERMINATED(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SecObjSetMechanisms
 * ----------------------------------------------------------------------------
 * General: Sets security mechanism to be used by Security Object for message
 *          protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj       - Pointer to the Security Object
 *          eSecMechanism - Mechanism, e.g. TLS or IPsec
 *****************************************************************************/
RvStatus SecObjSetMechanism(
                        IN SecObj* pSecObj,
                        IN RvSipSecurityMechanismType eSecMechanism)
{
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE == RV_TLS_OPENSSL)
    RvStatus rv;
#endif

#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP == eSecMechanism)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjSetMechanism(pSecObj=%p) - IPsec is not supported",
            pSecObj));
        return RV_ERROR_NOTSUPPORTED;
    }
#endif
#if !(RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_SECURITY_MECHANISM_TYPE_TLS == eSecMechanism)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjSetMechanism(pSecObj=%p) - TLS is not supported",
            pSecObj));
        return RV_ERROR_NOTSUPPORTED;
    }
#endif

    /* Set can be done for IDLE, INITIATED or ACTIVE objects only */
    if (RVSIP_SECOBJ_STATE_IDLE      != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_ACTIVE    != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjSetMechanism(pSecObj=%p) - wrong state %s. Should be IDLE, INITIATED or ACTIVE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    switch (eSecMechanism)
    {
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
            /* Security Object may have a TLS. Remove it.*/
#if (RV_TLS_TYPE != RV_TLS_NONE)
            if (RVSIP_SECURITY_MECHANISM_TYPE_TLS & pSecObj->secMechsToInitiate)
            {
                rv = RemoveTls(pSecObj);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc,
                        "SecObjSetMechanism(pSecObj=%p): failed to remove TLS(rv=%d:%s)",
                        pSecObj, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
            }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
            break;

        case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
            /* Security Object may have an IPsec. Remove it.*/
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
            if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & pSecObj->secMechsToInitiate)
            {
                rv = RemoveIpsec(pSecObj);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjSetMechanism(pSecObj=%p): failed to remove IPsec(rv=%d:%s)",
                        pSecObj, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
            }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
            break;

        default:
            /* IPsec and TLS mechanisms are supported only */
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjSetMechanism(pSecObj=%p) - %s is not supported. TLS or IPsec-3GPP is expected",
                pSecObj, SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism)));
            return RV_ERROR_BADPARAM;
    }

    pSecObj->eSecMechanism = eSecMechanism;

    return RV_OK;
}

/******************************************************************************
 * SecObjSetMechanismsToInitiate
 * ----------------------------------------------------------------------------
 * General: Sets masks of security mechanisms to be initiated by Security
 *          Object. One of them should be chose finally.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj            - Pointer to the Security Object
 *          secMechsToInitiate - Mask of Security Mechanisms,
 *                               e.g.(RVSIP_SECURITY_MECHANISM_TYPE_TLS &
 *                                    RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
 *****************************************************************************/
RvStatus SecObjSetMechanismsToInitiate(
                        IN SecObj* pSecObj,
                        IN RvInt32 secMechsToInitiate)
{
    /* If IPsec stops to be mechanism, requested for initiating, close it and
    free it resources */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvStatus rv = RV_OK;

    if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & pSecObj->secMechsToInitiate &&
        !(RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & secMechsToInitiate))
    {
        rv = RemoveIpsec(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
                "SecObjSetMechanismsToInitiate(pSecObj=%p): failed to remove IPsec(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    pSecObj->secMechsToInitiate = secMechsToInitiate;
    return RV_OK;
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecParams
 * ----------------------------------------------------------------------------
 * General: Sets IPsec parameters required for message protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj             - Pointer to the Security Object
 *          pIpsecGeneralParams - Parameters, agreed for local and remote sides
 *                                This pointer can be NULL.
 *          pIpsecLocalParams   - Parameters, generated by Local side
 *                                This pointer can be NULL.
 *          pIpsecRemoteParams  - Parameters, generated by Remote side
 *                                This pointer can be NULL.
 *****************************************************************************/
RvStatus SecObjSetIpsecParams(
                        IN SecObj*                  pSecObj,
                        IN RvSipSecIpsecParams*     pIpsecGeneralParams,
                        IN RvSipSecIpsecPeerParams* pIpsecLocalParams,
                        IN RvSipSecIpsecPeerParams* pIpsecRemoteParams)
{
    SecMgr*  pSecMgr = pSecObj->pMgr;
    RvBool   bLocalSpisWereSet = RV_FALSE;
    RvBool   bRemoteSpisWereSet = RV_FALSE;
    RvStatus crv;

    if (RVSIP_SECOBJ_STATE_IDLE         != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED    != pSecObj->eState)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetIpsecParams(pSecObj=%p) - wrong state %s. Should be IDLE or INITIATED",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* If session was not allocated yet, allocate it now */
    if (NULL == pSecObj->pIpsecSession)
    {
        RvStatus rv;
        rv = AllocateIpsecSession(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecParams(pSecObj=%p) - failed to allocate IPsec Session(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Update Session parameters */
    if (NULL != pIpsecGeneralParams)
    {
        if (0 < pIpsecGeneralParams->lifetime)
        {
            if (pIpsecGeneralParams->lifetime > RV_UINT32_MAX/1000)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Can't start lifetime timer: interval %d too big. Should be less than %d",
                    pSecObj, pIpsecGeneralParams->lifetime, RV_UINT32_MAX/1000));
                return RV_ERROR_BADPARAM;
            }
            pSecObj->pIpsecSession->lifetime = pIpsecGeneralParams->lifetime;
        }
        if (RVSIP_TRANSPORT_UNDEFINED != pIpsecGeneralParams->eTransportType)
        {
            if (RVSIP_TRANSPORT_UDP != pIpsecGeneralParams->eTransportType &&
                RVSIP_TRANSPORT_TCP != pIpsecGeneralParams->eTransportType)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Transport should be UDP or TCP. Provided %s",
                    pSecObj, SipCommonConvertEnumToStrTransportType(pIpsecGeneralParams->eTransportType)));
                return RV_ERROR_BADPARAM;
            }
            pSecObj->pIpsecSession->eTransportType = pIpsecGeneralParams->eTransportType;
        }
        if (RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED != pIpsecGeneralParams->eEAlg)
        {
            pSecObj->pIpsecSession->ccSAData.iEncrAlg = SipCommonConvertSipToCoreEncrAlg(pIpsecGeneralParams->eEAlg);
        }
        if (RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED != pIpsecGeneralParams->eIAlg)
        {
            pSecObj->pIpsecSession->ccSAData.iAuthAlg = SipCommonConvertSipToCoreIntegrAlg(pIpsecGeneralParams->eIAlg);
        }
        if (0 < pIpsecGeneralParams->CKlen/8)
        {
            if (RVSIP_SEC_MAX_KEY_LENGTH < pIpsecGeneralParams->CKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Cypher Key should be less than %d bits. Provided %d bits",
                    pSecObj, RVSIP_SEC_MAX_KEY_LENGTH, pIpsecGeneralParams->CKlen));
                return RV_ERROR_BADPARAM;
            }
            if (RvImsEncr3Des == pSecObj->pIpsecSession->ccSAData.iEncrAlg &&
                192 != pIpsecGeneralParams->CKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Size of 3DES Key (%d bits) should be 192 bits",
                    pSecObj, pIpsecGeneralParams->CKlen));
                return RV_ERROR_BADPARAM;
            }
            if (RvImsEncrAes == pSecObj->pIpsecSession->ccSAData.iEncrAlg &&
                128 != pIpsecGeneralParams->CKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Size of AES Key (%d bits) should be 128 bits",
                    pSecObj, pIpsecGeneralParams->CKlen));
                return RV_ERROR_BADPARAM;
            }

            memcpy(pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyValue,
                pIpsecGeneralParams->CK, pIpsecGeneralParams->CKlen/8);
            pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits =
                pIpsecGeneralParams->CKlen;
        }
        if (0 < pIpsecGeneralParams->IKlen/8)
        {
            if (RVSIP_SEC_MAX_KEY_LENGTH < pIpsecGeneralParams->IKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Integrity Key should be less than %d bits. Provided %d bits",
                    pSecObj, RVSIP_SEC_MAX_KEY_LENGTH, pIpsecGeneralParams->IKlen));
                return RV_ERROR_BADPARAM;
            }
            if (RvImsAuthMd5 == pSecObj->pIpsecSession->ccSAData.iAuthAlg &&
                128 != pIpsecGeneralParams->IKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Size of MD5 Key (%d bits) should be 128 bits",
                    pSecObj, pIpsecGeneralParams->IKlen));
                return RV_ERROR_BADPARAM;
            }
            if (RvImsAuthSha1 == pSecObj->pIpsecSession->ccSAData.iAuthAlg &&
                160 != pIpsecGeneralParams->IKlen)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Size of SHA-1 Key (%d bits) should be 160 bits",
                    pSecObj, pIpsecGeneralParams->IKlen));
                return RV_ERROR_BADPARAM;
            }

            memcpy(pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyValue,
                pIpsecGeneralParams->IK, pIpsecGeneralParams->IKlen/8);
            pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits =
                pIpsecGeneralParams->IKlen;
        }
    } /* ENDOF: if (NULL != pIpsecGeneralParams) */

    if (NULL != pIpsecLocalParams)
    {
        if (0 != pIpsecLocalParams->portC)
        {
            pSecObj->pIpsecSession->ccSAData.iLclPrtClnt = pIpsecLocalParams->portC;
        }
        if (0 != pIpsecLocalParams->portS)
        {
            pSecObj->pIpsecSession->ccSAData.iLclPrtSrv = pIpsecLocalParams->portS;
        }
        if (0 != pIpsecLocalParams->spiC)
        {
            if (pSecMgr->spiRangeStart > pIpsecLocalParams->spiC ||
                pSecMgr->spiRangeEnd   < pIpsecLocalParams->spiC)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Local SPI-C (%u) should be in range [%u-%u] (see cfg.spiRange)",
                    pSecObj, pIpsecLocalParams->spiC,
                    pSecMgr->spiRangeStart, pSecMgr->spiRangeEnd));
                return RV_ERROR_BADPARAM;
            }
            if (0 != pSecObj->pIpsecSession->ccSAData.iSpiInClnt)
            {
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Local SPI-C (%u) can't be overrode",
                    pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiInClnt));
            }
            else
            {
                pSecObj->pIpsecSession->ccSAData.iSpiInClnt = pIpsecLocalParams->spiC;
                bLocalSpisWereSet = RV_TRUE;
            }
        }
        if (0 != pIpsecLocalParams->spiS)
        {
            if (pSecMgr->spiRangeStart > pIpsecLocalParams->spiS ||
                pSecMgr->spiRangeEnd   < pIpsecLocalParams->spiS)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Local SPI-S (%u) should be in range [%u-%u]  (see cfg.spiRange)",
                    pSecObj, pIpsecLocalParams->spiS,
                    pSecMgr->spiRangeStart, pSecMgr->spiRangeEnd));
                return RV_ERROR_BADPARAM;
            }
            if (0 != pSecObj->pIpsecSession->ccSAData.iSpiInSrv)
            {
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Local SPI-S (%u) can't be overrode",
                    pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiInSrv));
            }
            else
            {
                pSecObj->pIpsecSession->ccSAData.iSpiInSrv = pIpsecLocalParams->spiS;
                bLocalSpisWereSet = RV_TRUE;
            }
        }
    } /* ENDOF: if (NULL != pIpsecLocalParams) */

    if (NULL != pIpsecRemoteParams)
    {
        if (0 != pIpsecRemoteParams->portC)
        {
            pSecObj->pIpsecSession->ccSAData.iPeerPrtClnt = pIpsecRemoteParams->portC;
        }
        if (0 != pIpsecRemoteParams->portS)
        {
            pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv = pIpsecRemoteParams->portS;
        }
        if (0 != pIpsecRemoteParams->spiC)
        {
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   #if 0
            if (pSecMgr->spiRangeStart > pIpsecRemoteParams->spiC ||
                pSecMgr->spiRangeEnd   < pIpsecRemoteParams->spiC)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Remote SPI-C (%u) should be in range [%u-%u]  (see cfg.spiRange)",
                    pSecObj, pIpsecRemoteParams->spiC,
                    pSecMgr->spiRangeStart, pSecMgr->spiRangeEnd));
                return RV_ERROR_BADPARAM;
            }
   #endif   // #if 0
#endif
//SPIRENT_END
            if (0 != pSecObj->pIpsecSession->ccSAData.iSpiOutSrv)
            {
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Remote SPI-C (%u) can't be overriden",
                    pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiOutClnt));
            }
            else
            {
                pSecObj->pIpsecSession->ccSAData.iSpiOutSrv = pIpsecRemoteParams->spiC;
                bRemoteSpisWereSet = RV_TRUE;
            }
        }
        if (0 != pIpsecRemoteParams->spiS)
        {
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
   #if 0
            if (pSecMgr->spiRangeStart > pIpsecRemoteParams->spiS ||
                pSecMgr->spiRangeEnd   < pIpsecRemoteParams->spiS)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Remote SPI-S (%u) should be in range [%u-%u] (see cfg.spiRange)",
                    pSecObj, pIpsecRemoteParams->spiS,
                    pSecMgr->spiRangeStart, pSecMgr->spiRangeEnd));
                return RV_ERROR_BADPARAM;
            }
   #endif   // #if 0
#endif
//SPIRENT_END
            if (0 != pSecObj->pIpsecSession->ccSAData.iSpiOutClnt)
            {
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetIpsecParams(pSecObj=%p) - Remote SPI-S (%u) can't be overriden",
                    pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiOutSrv));
            }
            else
            {
                pSecObj->pIpsecSession->ccSAData.iSpiOutClnt = pIpsecRemoteParams->spiS;
                bRemoteSpisWereSet = RV_TRUE;
            }
        }
    } /* ENDOF: if (NULL != pIpsecRemoteParams) */

    /* If SPIs were set, mark them as used in Common Core in order to prevent
    generation of the same value in future */
    if (RV_TRUE == bLocalSpisWereSet)
    {
        crv = rvImsSetAsBusyPairOfSPIs(pIpsecLocalParams->spiC,
                                       pIpsecLocalParams->spiS,
                                       &pSecMgr->ccImsCfg,
                                       pSecMgr->pLogMgr);
        if (RV_OK != crv)
        {
            RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecParams(pSecObj=%p) - Failed to mark local SPI-C(%u) and SPI-S(%u) as used. SPI collision is possible(crv=%d)",
                pSecObj, pIpsecLocalParams->spiC, pIpsecLocalParams->spiS,
                RvErrorGetCode(crv)));
        }
    }
    if (RV_TRUE == bRemoteSpisWereSet)
    {
        crv = rvImsSetAsBusyPairOfSPIs(pIpsecRemoteParams->spiC,
                                       pIpsecRemoteParams->spiS,
                                       &pSecMgr->ccImsCfg,
                                       pSecMgr->pLogMgr);
        if (RV_OK != crv)
        {
            RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecParams(pSecObj=%p) - Failed to mark remote SPI-C(%u) and SPI-S(%u) as used. SPI collision is possible(crv=%d)",
                pSecObj, pIpsecRemoteParams->spiC, pIpsecRemoteParams->spiS,
                RvErrorGetCode(crv)));
        }
    }

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjGetIpsecParams
 * ----------------------------------------------------------------------------
 * General: Gets IPsec parameters set into Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj             - Pointer to the Security Object
 * Output:  pIpsecGeneralParams - Parameters, agreed for local and remote sides
 *                                This pointer can be NULL.
 *          pIpsecLocalParams   - Parameters, generated by Local side
 *                                This pointer can be NULL.
 *          pIpsecRemoteParams  - Parameters, generated by Remote side
 *                                This pointer can be NULL.
 *****************************************************************************/
RvStatus SecObjGetIpsecParams(
                        IN SecObj*                      pSecObj,
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams)
{
    SecMgr* pSecMgr = pSecObj->pMgr;

    /* If session was not allocated yet, return error*/
    if (NULL == pSecObj->pIpsecSession)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjGetIpsecParams(pSecObj=%p) - There is no IPsec session(rv=%d:%s)",
            pSecObj, RV_ERROR_ILLEGAL_ACTION,
            SipCommonStatus2Str(RV_ERROR_ILLEGAL_ACTION)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Update Session parameters */
    if (NULL != pIpsecGeneralParams)
    {
        pIpsecGeneralParams->lifetime = pSecObj->pIpsecSession->lifetime;
        pIpsecGeneralParams->eTransportType = pSecObj->pIpsecSession->eTransportType;
        pIpsecGeneralParams->eEAlg    =
            SipCommonConvertCoreToSipEncrAlg(pSecObj->pIpsecSession->ccSAData.iEncrAlg);
        pIpsecGeneralParams->eIAlg    =
            SipCommonConvertCoreToSipIntegrAlg(pSecObj->pIpsecSession->ccSAData.iAuthAlg);
        pIpsecGeneralParams->CKlen = pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits;
        memcpy(pIpsecGeneralParams->CK, pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyValue,
            pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits/8);
        pIpsecGeneralParams->IKlen = pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits;
        memcpy(pIpsecGeneralParams->IK, pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyValue,
            pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits/8);
    }
    if (NULL != pIpsecLocalParams)
    {
        pIpsecLocalParams->portC = pSecObj->pIpsecSession->ccSAData.iLclPrtClnt;
        pIpsecLocalParams->portS = pSecObj->pIpsecSession->ccSAData.iLclPrtSrv;
        pIpsecLocalParams->spiC  = pSecObj->pIpsecSession->ccSAData.iSpiInClnt;
        pIpsecLocalParams->spiS  = pSecObj->pIpsecSession->ccSAData.iSpiInSrv;
    }
    if (NULL != pIpsecRemoteParams)
    {
        pIpsecRemoteParams->portC = pSecObj->pIpsecSession->ccSAData.iPeerPrtClnt;
        pIpsecRemoteParams->portS = pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv;
        pIpsecRemoteParams->spiC  = pSecObj->pIpsecSession->ccSAData.iSpiOutSrv;
        pIpsecRemoteParams->spiS  = pSecObj->pIpsecSession->ccSAData.iSpiOutClnt;
    }
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#ifdef RV_IMS_TUNING
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecExtensionData
 * ----------------------------------------------------------------------------
 * General: Sets data required for IPsec extensions, like a tunneling mode.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj          - Pointer to the Security Object
 *          pExtDataArray    - The array of Extension Data elements.
 *          extDataArraySize - The number of elements in pExtDataArray array.
 *****************************************************************************/
RvStatus SecObjSetIpsecExtensionData(
                        IN SecObj*                  pSecObj,
                        IN RvImsAppExtensionData*   pExtDataArray,
                        IN RvSize_t                 extDataArraySize)
{
    RvStatus rv;

    if (NULL == pSecObj->pIpsecSession)
    {
        rv = AllocateIpsecSession(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjSetIpsecExtensionData(pSecObj=%p) - failed to allocate IPsec Session (rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    pSecObj->pIpsecSession->ccSAData.iExtData = pExtDataArray;
    pSecObj->pIpsecSession->ccSAData.iExtDataLen = extDataArraySize;
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#endif /*#ifdef RV_IMS_TUNING*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecLocalAddressesPortS
 * ----------------------------------------------------------------------------
 * General: Sets Local Addresses, bound to Port-S, into the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          hLocalAddrUdp - Handle to the Local UDP Address
 *          hLocalAddrTcp - Handle to the Local TCP Address
 *****************************************************************************/
RvStatus SecObjSetIpsecLocalAddressesPortS(
                        IN SecObj*                       pSecObj,
                        IN RvSipTransportLocalAddrHandle hLocalAddrUdp,
                        IN RvSipTransportLocalAddrHandle hLocalAddrTcp)
{
    RvStatus rv;

    SecMgr* pSecMgr = pSecObj->pMgr;

    if (RVSIP_SECOBJ_STATE_IDLE         != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED    != pSecObj->eState)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - wrong state %s. Should be IDLE or INITIATED",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* If session was not allocated yet, allocate it now */
    if (NULL == pSecObj->pIpsecSession)
    {
        RvStatus rv;
        rv = AllocateIpsecSession(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - failed to allocate IPsec Session(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpS &&
        NULL != hLocalAddrUdp)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - UDP address already set (%p)",
            pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpS));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL != pSecObj->pIpsecSession->hLocalAddrTcpS &&
        NULL != hLocalAddrTcp)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - TCP address already set (%p)",
            pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpS));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Set the UDP address */
    if (NULL != hLocalAddrUdp)
    {
        pSecObj->pIpsecSession->hLocalAddrUdpS = hLocalAddrUdp;
        rv = SipTransportLocalAddressSetSecOwner(hLocalAddrUdp, (void*)pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - failed to set Security Object into the UDP address %p (rv=%d:%s)",
                pSecObj, hLocalAddrUdp, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    else
    /* Reset the address, if NULL was supplied */
    if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpS)
    {
        pSecObj->pIpsecSession->hLocalAddrUdpS = NULL;
        rv = SipTransportLocalAddressSetSecOwner(
                pSecObj->pIpsecSession->hLocalAddrUdpS, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - failed to reset Security Object in UDP address %p (rv=%d:%s)",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpS,
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Set the TCP address */
    if (NULL != hLocalAddrTcp)
    {
        pSecObj->pIpsecSession->hLocalAddrTcpS = hLocalAddrTcp;
        rv = SipTransportLocalAddressSetSecOwner(hLocalAddrTcp, (void*)pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - failed to set Security Object into the TCP address %p (rv=%d:%s)",
                pSecObj, hLocalAddrTcp, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    /* Reset the address, if NULL was supplied */
    if (NULL == hLocalAddrTcp  &&
        NULL != pSecObj->pIpsecSession->hLocalAddrTcpS)
    {
        pSecObj->pIpsecSession->hLocalAddrTcpS = NULL;
        rv = SipTransportLocalAddressSetSecOwner(
                pSecObj->pIpsecSession->hLocalAddrTcpS, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetIpsecLocalAddressesPortS(pSecObj=%p) - failed to reset Security Object in UDP address %p (rv=%d:%s)",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpS,
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#ifdef RV_SIGCOMP_ON
/******************************************************************************
 * SecObjSetSigcompParams
 * ----------------------------------------------------------------------------
 * General: Sets sigcomp parameters to the security-object. The security-object
 *          will pass those parameters to the transmitter in order to obtain
 *          sigcomp compression
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj      - Handle to the Security Object
 *          eMsgCompType - The compression type
 *          pSigcompId   - The remote sigcomp-id
 *          pLinkCfg     - The remote sigcomp-id obtained within link config 
 *                         instead as rpool pointer
 *****************************************************************************/
RvStatus SecObjSetSigcompParams(
                        IN SecObj                      *pSecObj,
						IN RvSipTransmitterMsgCompType  eMsgCompType,
                        IN RPOOL_Ptr				   *pSigcompId,
						IN RvSipSecLinkCfg             *pLinkCfg)
{
	RvInt32     len;
	RvStatus    rv;

	if (pLinkCfg != NULL)
	{
		if (pLinkCfg->eMsgCompType != RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED)
		{
			pSecObj->eMsgCompType = pLinkCfg->eMsgCompType;
		}

		if (pLinkCfg->strSigcompId != NULL)
		{
			len = (RvInt32)strlen(pLinkCfg->strSigcompId);

			if (len > RVSIP_COMPARTMENT_MAX_URN_LENGTH-1)
			{
				RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
							"SecObjSetSigcompParams(hSecObj=%p) - sigcomp-id is too long (len=%d,allowed-len=%d)",
							pSecObj, len, RVSIP_COMPARTMENT_MAX_URN_LENGTH-1));
				return RV_ERROR_BADPARAM;
			}
    
			strcpy(pSecObj->strSigcompId, pLinkCfg->strSigcompId);
		}
	}
	else if (eMsgCompType != RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED)
	{
		pSecObj->eMsgCompType = eMsgCompType;
		
		if (pSigcompId != NULL && pSigcompId->offset != UNDEFINED)
		{
			len = RPOOL_Strlen(pSigcompId->hPool, pSigcompId->hPage, pSigcompId->offset);
			rv = RPOOL_CopyToExternal(pSigcompId->hPool, pSigcompId->hPage, pSigcompId->offset, pSecObj->strSigcompId, len+1);
			if (RV_OK != rv)
			{
				RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
							"SecObjSetSigcompParams(hSecObj=%p) - Failed in RPOOL_CopyToExternal (rv=%d)",
							pSecObj, rv));
				return rv;
			}
		}
	}

	RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
			   "SecObjSetSigcompParams(hSecObj=%p) - the security-object holds the following values: sigcomp=%d, sigcomp-id=%s",
			   pSecObj, pSecObj->eMsgCompType, pSecObj->strSigcompId));
				
	
	return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */



#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjSetTlsParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters required for TLS establishment.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus SecObjSetTlsParams(
                        IN SecObj*          pSecObj,
                        IN SipSecTlsParams* pTlsParams)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SecMgr*  pSecMgr = pSecObj->pMgr;
#endif
	RvStatus rv      = RV_OK;

    if (RVSIP_SECOBJ_STATE_IDLE         != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED    != pSecObj->eState)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetTlsParams(pSecObj=%p) - wrong state %s. Should be IDLE or INITIATED",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL != pTlsParams->hLocalAddr)
    {   /* The local address should never be reset */
        pSecObj->tlsSession.hLocalAddr = pTlsParams->hLocalAddr;
    }
    if (NULL != pTlsParams->pRemoteAddr)
    {
        memcpy(&pSecObj->tlsSession.ccRemoteAddr, pTlsParams->pRemoteAddr,
               sizeof(RvAddress));
    }

#ifdef RV_SIGCOMP_ON
	rv = SecObjSetSigcompParams(pSecObj, pTlsParams->eMsgCompType, &pTlsParams->strSigcompId, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"SecObjSetTlsParams(pSecObj=%p) - Failed to set sigcomp parameters to security-object",
					pSecObj));
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 
	
    return rv;
}
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjGetTlsParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters set into Security Object.
 *
 * Return Value: status.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus SecObjGetTlsParams(
                        IN  SecObj*          pSecObj,
                        OUT SipSecTlsParams* pTlsParams)
{
    pTlsParams->hLocalAddr = pSecObj->tlsSession.hLocalAddr;
    memcpy(pTlsParams->pRemoteAddr, &pSecObj->tlsSession.ccRemoteAddr,
           sizeof(RvAddress));
#ifdef RV_SIGCOMP_ON
	pTlsParams->eMsgCompType = pSecObj->eMsgCompType;
	if (pTlsParams->strSigcompId.offset != UNDEFINED && 
		pTlsParams->strSigcompId.hPool != NULL &&
		pTlsParams->strSigcompId.hPage != NULL_PAGE)
	{
		RPOOL_ITEM_HANDLE  hItem;
		RvInt32            len = strlen(pSecObj->strSigcompId);
		RvStatus           rv;

		if (len > 0)
		{
			rv = RPOOL_AppendFromExternal(pTlsParams->strSigcompId.hPool, pTlsParams->strSigcompId.hPage, 
										  pSecObj->strSigcompId, len+1, RV_FALSE,
										  &pTlsParams->strSigcompId.offset, &hItem);
			if (RV_OK != rv)
			{
				RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
							"SecObjGetTlsParams(pSecObj=%p) - Failed to get sigcomp-id",
							pSecObj));
				return rv;
			}
		}
		else
		{
			pTlsParams->strSigcompId.offset = UNDEFINED;
		}
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 

	return RV_OK;
}
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjSetTlsConnection
 * ----------------------------------------------------------------------------
 * General: Sets connection, to be used by the Security Object for protection
 *          by TLS.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          hConn   - Handle to the Connection to be set
 *****************************************************************************/
RvStatus SecObjSetTlsConnection(
                        IN  SecObj*                         pSecObj,
                        IN  RvSipTransportConnectionHandle  hConn)
{
    RvStatus rv;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SecMgr*  pSecMgr = pSecObj->pMgr;
#endif

    if (RVSIP_SECOBJ_STATE_IDLE         != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_INITIATED    != pSecObj->eState &&
        RVSIP_SECOBJ_STATE_ACTIVE       != pSecObj->eState)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetTlsConnection(pSecObj=%p) - wrong state %s. Should be IDLE,INITIATED or ACTIVE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (RV_FALSE == SecObjSupportsTls(pSecObj))
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetTlsConnection(pSecObj=%p) - wrong mechanism %s. Should be UNDEFINED or TLS",
            pSecObj, SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (NULL != pSecObj->tlsSession.hConn)
    {
        RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjSetTlsConnection(pSecObj=%p) - overriding Connection %p",
            pSecObj, pSecObj->tlsSession.hConn));
        rv = SipTransportConnectionSetSecOwner(
                pSecObj->tlsSession.hConn, NULL, RV_FALSE/*bDisconnect*/);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetTlsConnection(pSecObj=%p) - Failed to detach from TLS Connection %p",
                pSecObj, pSecObj->tlsSession.hConn));
        }
        pSecObj->tlsSession.hConn = NULL;
    }

    pSecObj->tlsSession.hConn = hConn;

    if (NULL != hConn)
    {
        rv = SipTransportConnectionSetSecOwner(pSecObj->tlsSession.hConn,
                (void*)pSecObj, RV_FALSE/*bDisconnect*/);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetTlsConnection(pSecObj=%p) - Failed to attach to TLS Connection %p(rv=%d:%s)",
                pSecObj, pSecObj->tlsSession.hConn, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    return RV_OK;
}
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

/******************************************************************************
 * SecObjInitialize
 * ----------------------------------------------------------------------------
 * General: initializes Security Object. Set it state to IDLE.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *            pMgr    - Pointer to the Security Manager
 *****************************************************************************/
RvStatus SecObjInitialize(IN SecObj* pSecObj, IN SecMgr* pSecMgr)
{
	pSecObj->counterOwners  = 0;
	pSecObj->hAppSecObj     = NULL;
    pSecObj->impi           = NULL;
	pSecObj->ipv6Scope      = 0;
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
	pSecObj->pIpsecSession  = NULL;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
    /* Never reset the Manager. See ASSUMPTION.
	pSecObj->pMgr           = NULL;
    */
	pSecObj->pSecAgree      = NULL;
	memset(pSecObj->strLocalIp,0,RVSIP_TRANSPORT_LEN_STRING_IP);
	memset(pSecObj->strRemoteIp,0,RVSIP_TRANSPORT_LEN_STRING_IP);
#if (RV_TLS_TYPE != RV_TLS_NONE)
	memset(&pSecObj->tlsSession,0,sizeof(pSecObj->tlsSession));
#endif
    memset(&pSecObj->terminationInfo,0,sizeof(RvSipTransportObjEventInfo));
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
	pSecObj->pTripleLock    = &pSecObj->tripleLock;
#endif

    pSecObj->pMgr           = pSecMgr;
    pSecObj->eState         = RVSIP_SECOBJ_STATE_IDLE;
    /* Set default mechanism to be IPsec, since only IPsec is supported
    for Security Objects, created by the Application.
    If Security Object is created by Security Agreement module,
    the last should set the mechanism after the Object construction. */
    pSecObj->eSecMechanism  = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;
    pSecObj->secMechsToInitiate = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvRandomGeneratorGetInRange(pSecMgr->pSeed, RV_INT32_MAX,
                                (RvRandom*)&pSecObj->secObjUniqueIdentifier);
#endif

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (NULL != pSecMgr->hIpsecSessionPool)
    {
        RvStatus rv;
        rv = AllocateIpsecSession(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjInitialize(pSecObj=%p) - failed to allocate IPsec Session(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
		SipCommonCoreRvTimerInit(&pSecObj->pIpsecSession->timerLifetime);
    }	
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
    pSecObj->tlsSession.bAppConn = RV_FALSE;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#ifdef RV_SIGCOMP_ON
	pSecObj->eMsgCompType = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
	memset(pSecObj->strSigcompId, 0, RVSIP_COMPARTMENT_MAX_URN_LENGTH);
#endif

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SecObjInitialize(pSecObj=%p) - Done", pSecObj));

    return RV_OK;
}

/******************************************************************************
 * SecObjFree
 * ----------------------------------------------------------------------------
 * General: Remove Security Object from the list of used objects.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 *****************************************************************************/
void SecObjFree(IN SecObj* pSecObj)
{
    RvStatus rv;
    SecMgr*  pSecMgr = pSecObj->pMgr;
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvBool   bRemoveSAa = RV_FALSE;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    pSecObj->secObjUniqueIdentifier = 0;
#endif

    /* Check, if SAs should be removed from the system */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (RVSIP_SECOBJ_STATE_ACTIVE == pSecObj->eState &&
        RV_TRUE == SecObjSupportsIpsec(pSecObj))
    {
        bRemoveSAa = RV_TRUE;
    }
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

    /* Reset state */
    pSecObj->eState = RVSIP_SECOBJ_STATE_UNDEFINED;

    rv = CloseConnections(pSecObj, RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED,
                          RV_TRUE/*bDetachFromConn*/,NULL/*pNumOfClosedConn*/);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjFree(pSecObj=%p) - failed to close connections (rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
    }

    rv = CloseLocalAddresses(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjFree(pSecObj=%p) - failed to close local addresses (rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
    }

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (RV_TRUE == bRemoveSAa && NULL != pSecObj->pIpsecSession)
    {
        rv = SecObjShutdownIpsecComplete(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjFree(pSecObj=%p) - failed to remove Security Associations (rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
        }
    }
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

    if (RV_OK != SecMgrLock(&pSecMgr->hMutex, pSecMgr->pLogMgr))
	{
		RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"SecObjFree(pSecObj=%p) - failed to lock security manager %p",
					pSecObj, pSecMgr));
		return;
	}	
    /* Remove IPsec Session if need */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (NULL != pSecObj->pIpsecSession)
    {
        /* Free SPIs, if they were not free yet */
        if (0 != pSecObj->pIpsecSession->ccSAData.iSpiInClnt  ||
            0 != pSecObj->pIpsecSession->ccSAData.iSpiInSrv   ||
            0 != pSecObj->pIpsecSession->ccSAData.iSpiOutClnt ||
            0 != pSecObj->pIpsecSession->ccSAData.iSpiOutSrv)
        {
            RvStatus crv;
            crv = rvImsReturnSPIs(pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
                                  pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
                                  pSecObj->pIpsecSession->ccSAData.iSpiOutClnt,
                                  pSecObj->pIpsecSession->ccSAData.iSpiOutSrv,
                                  &pSecMgr->ccImsCfg, pSecMgr->pLogMgr);
            if (RV_OK != crv)
            {
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjFree(pSecObj=%p) - Failed to mark SPIs as not used(crv=%d)",
                    pSecObj, RvErrorGetCode(crv)));
            }
        }

        /* Remove IPsec Session */
        RLIST_Remove(pSecMgr->hIpsecSessionPool, pSecMgr->hIpsecSessionList,
                     (RLIST_ITEM_HANDLE)pSecObj->pIpsecSession);
        pSecObj->pIpsecSession = NULL;
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
    /* Remove Object itself */
    RLIST_Remove(pSecMgr->hSecObjPool, pSecMgr->hSecObjList,
                 (RLIST_ITEM_HANDLE)pSecObj);
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SecObjFree(pSecObj=%p) - was freed" , pSecObj));
}

/******************************************************************************
 * SecObjRemoveIfNoOwnerIsLeft
 * ----------------------------------------------------------------------------
 * General: Removes Security Objects, if it has no owner (Application,
 *          SecAgree, CallLeg or other). Removal process depends on the state
 *          of the object. If it is active, the object will be shutdown,
 *          if it is closed, the object will be terminated.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjRemoveIfNoOwnerIsLeft(IN SecObj* pSecObj)
{
    RvStatus rv;

    if (NULL != pSecObj->pSecAgree  ||  NULL != pSecObj->hAppSecObj  ||
        0 < pSecObj->counterOwners)
    {
        return RV_OK;
    }

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SecObjRemoveIfNoOwnerIsLeft(pSecObj=%p) - no owner for the Object. Remove it.",
        pSecObj));

    switch (pSecObj->eState)
    {
        case RVSIP_SECOBJ_STATE_ACTIVE:
            rv = SecObjShutdown(pSecObj);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjRemoveIfNoOwnerIsLeft(pSecObj=%p) - failed to shutdown(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            break;

        case RVSIP_SECOBJ_STATE_IDLE:
        case RVSIP_SECOBJ_STATE_INITIATED:
        case RVSIP_SECOBJ_STATE_CLOSED:
            rv = SecObjChangeState(pSecObj, RVSIP_SECOBJ_STATE_TERMINATED,
                                   RVSIP_SECOBJ_REASON_UNDEFINED);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjRemoveIfNoOwnerIsLeft(pSecObj=%p) - failed to change state to TERMINATED(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            break;

        default:
            break;
    }

    return RV_OK;
}

/******************************************************************************
 * SecObjChangeState
 * ----------------------------------------------------------------------------
 * General: moves Security Object to the specified state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          eState  - New state
 *          eReason - Reason of the state change
*****************************************************************************/
RvStatus SecObjChangeState(
                        IN SecObj*                      pSecObj,
                        IN RvSipSecObjState             eState,
                        IN RvSipSecObjStateChangeReason eReason)
{
    RvStatus rv = RV_OK;

    RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SecObjChangeState(pSecObj=%p) - %s -> %s", pSecObj,
        SipSecUtilConvertSecObjState2Str(pSecObj->eState),
        SipSecUtilConvertSecObjState2Str(eState)));

    if (eState == pSecObj->eState)
    {
        RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjChangeState(pSecObj=%p): object is already in %s state. Return",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_OK;
    }

    pSecObj->eState = eState;

    if(eState != RVSIP_SECOBJ_STATE_TERMINATED)
    {
       /* a state event will not have an Out-of-resource recovery */
        rv = SipTransportSendStateObjectEvent(pSecObj->pMgr->hTransportMgr,
                                    (void *)pSecObj,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
                                    pSecObj->secObjUniqueIdentifier,
#else
                                    UNDEFINED,
#endif
                                    (RvInt32)eState,
                                    (RvInt32)eReason,
                                    SipSecObjChangeStateEventHandler,
                                    RV_TRUE,
                                    SipSecUtilConvertSecObjState2Str(eState),
                                    "SecObj");
		if (rv != RV_OK)
		{
			RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
				"SecObjChangeState(pSecObj=%p): Unable to send state change to event queue, sec-obj will be terminated.",
				pSecObj));
		}
    }
	if (rv != RV_OK || eState == RVSIP_SECOBJ_STATE_TERMINATED)
    {
        /* object termination must have an Out-of-resource recovery */
        SipTransportSendTerminationObjectEvent(pSecObj->pMgr->hTransportMgr,
                                    (void *)pSecObj,
                                    &pSecObj->terminationInfo,
                                    (RvInt32)RVSIP_SECOBJ_REASON_UNDEFINED,
                                    SipSecObjTerminatedStateEventHandler,
                                    RV_TRUE,
                                    SipSecUtilConvertSecObjState2Str(eState),
                                    "SecObj");
    }
    return rv;
}

/******************************************************************************
 * SecObjGetSecureLocalAddressAndConnection
 * ----------------------------------------------------------------------------
 * General: gets handle to the secure Local Address and handle to the Secure
 *          Connection (if TCP or TLS transport is used by Security Object),
 *          that can be used for protected message sending.
 *          The Security Object should be in ACTIVE state.
 *          Otherwise the error will be returned, since the protection can't be
 *          ensured.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj           - Pointer to the Security Object
 *          eMsgType          - Request or response. For request sending client
 *                              connection should be provided, for response -
 *                              server connection.
 *          peTransportType   - Type of the Transport to be used for protected
 *                              message delivery.If UNDEFINED, forced Transport
 *                              Type, set with SetIpsecParame() API will be
 *                              used. If it is UNDEFINED also, the default -
 *                              UDP will be returned.
 *                              Can be NULL.
 * Output:  phLocalAddress    - Handle to the Local Address.
 *          pRemoteAddr       - Pointer to the Remote Address.
 *          phConn            - Handle to the TCP/SCTP/TLS Connection.
 *          peTransportType   - Type of Transport set early
 *                              into the Security Object, as a forced Transport
 *                              Type, if peTransportType is UNDEFINED.
 *			peMsgCompType	  - Pointer to the compression type of messages 
 *								sent by the Security object
 *          strSigcompId      - The remote sigcomp id associated with this 
 *                              security object. 
 *          bIsConsecutive    - Boolean indication to whether the sigcomp-id
 *                              should be consecutive on the given page
 *****************************************************************************/
RvStatus SecObjGetSecureLocalAddressAndConnection(
                    IN      SecObj*                         pSecObj,
                    IN      RvSipMsgType                    eMsgType,
                    IN OUT  RvSipTransport*                 peTransportType,
                    OUT     RvSipTransportLocalAddrHandle*  phLocalAddress,
                    OUT     RvAddress*                      pRemoteAddr,
                    OUT     RvSipTransportConnectionHandle* phConn,
					OUT     RvSipTransmitterMsgCompType	   *peMsgCompType,
					INOUT   RPOOL_Ptr                      *strSigcompId,
					IN      RvBool                          bIsConsecutive)
{
    RvSipTransport eTransportType;
#ifdef RV_SIGCOMP_ON
	RvInt32        len;
#endif /* #ifdef RV_SIGCOMP_ON */
/* Prevent compilation warning UNREFERENCED VARIABLE */
    RvStatus rv;

    /* Prevent compilation warning UNREFERENCED PARAMETER */
#if !(RV_IMS_IPSEC_ENABLED==RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
    RV_UNUSED_ARG(phLocalAddress);
    RV_UNUSED_ARG(pRemoteAddr);
    RV_UNUSED_ARG(phConn);
#endif
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(eMsgType);
#endif
#ifndef RV_SIGCOMP_ON
	RV_UNUSED_ARG(peMsgCompType); 
	RV_UNUSED_ARG(strSigcompId);
	RV_UNUSED_ARG(bIsConsecutive);
#endif 
	
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - wrong state %s. Should be ACTIVE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

#ifdef RV_SIGCOMP_ON
	*peMsgCompType = pSecObj->eMsgCompType;
	len = (RvInt32)strlen(pSecObj->strSigcompId);
	if (len > 0)
	{
		RPOOL_ITEM_HANDLE  hItem;
		rv = RPOOL_AppendFromExternal(strSigcompId->hPool, strSigcompId->hPage, 
									  pSecObj->strSigcompId, len+1, bIsConsecutive, 
									  &strSigcompId->offset, &hItem);
		if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
						"SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - failed to copy sigcomp-id(rv=%d:%s)",
						pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 

    /* Deduce Transport Type to be used */
    eTransportType = RVSIP_TRANSPORT_UNDEFINED;
    if (NULL != peTransportType)
    {
        eTransportType = *peTransportType;
    }
    /* If caller didn't request specific transport, use default */
    if (RVSIP_TRANSPORT_UNDEFINED == eTransportType)
    {
        rv = GetDefaultTransport(pSecObj, &eTransportType);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - failed to get default Transport(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Fetch required addresses or connections */
    switch(eTransportType)
    {
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        case RVSIP_TRANSPORT_UDP:
            if (RV_TRUE == SecObjSupportsIpsec(pSecObj))
            {
                *phLocalAddress = pSecObj->pIpsecSession->hLocalAddrUdpC;
                memcpy(pRemoteAddr,
                    &pSecObj->pIpsecSession->ccSAData.iPeerAddress,
                    sizeof(RvAddress));
            }
            break;
        case RVSIP_TRANSPORT_TCP:
        /* SCTP is not supported yet
        case RVSIP_TRANSPORT_SCTP:
        */
            if (RV_TRUE == SecObjSupportsIpsec(pSecObj))
            {
                /* According 3GPP TG 33.203 use Port-C (client connection) for
                requests and server connection (Port-S) for responses */
                if (RVSIP_MSG_RESPONSE == eMsgType)
                {
                    *phConn = pSecObj->pIpsecSession->hConnS;
                    *phLocalAddress = pSecObj->pIpsecSession->hLocalAddrTcpS;
                }
                else
                {
                    *phConn = pSecObj->pIpsecSession->hConnC;
                    *phLocalAddress = pSecObj->pIpsecSession->hLocalAddrTcpC;
                }
                memcpy(pRemoteAddr,
                    &pSecObj->pIpsecSession->ccSAData.iPeerAddress,
                    sizeof(RvAddress));
                if (*phConn==NULL  ||  *phLocalAddress==NULL)
                {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
                  dprintf("%s:%d: error: mtype=%d\n",__FUNCTION__,__LINE__,eMsgType);
#endif
/* SPIRENT END */
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - requested TCP connection/address can't be provided (conn=%p,addr=%p)",
                        pSecObj, *phConn, *phLocalAddress));
                    return RV_ERROR_NOT_FOUND;
                }
            }
            break;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            if (RV_TRUE == SecObjSupportsTls(pSecObj))
            {
                SipTransportAddress remoteSipAddress;

                if (NULL == pSecObj->tlsSession.hConn)
                {
                    rv = OpenConnection(pSecObj, RVSIP_TRANSPORT_TLS,
                                        pSecObj->tlsSession.hLocalAddr,
                                        &pSecObj->tlsSession.ccRemoteAddr,
                                        &pSecObj->tlsSession.hConn);
                    if (RV_OK != rv)
                    {
                        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                            "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - failed to open TLS connection(rv=%d:%s)",
                            pSecObj, rv, SipCommonStatus2Str(rv)));
                        return rv;
                    }
                }
                *phConn = pSecObj->tlsSession.hConn;
                *phLocalAddress = pSecObj->tlsSession.hLocalAddr;
                /* If Local Address was not set, take it from the connection */
                if (NULL == *phLocalAddress)
                {
                    /* Get Local Address */
                    rv = SipTransportConnectionGetLocalAddress(
                            pSecObj->tlsSession.hConn, phLocalAddress);
                    if (RV_OK != rv)
                    {
                        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                            "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - failed to get local address, used by TLS connection %p(rv=%d:%s)",
                            pSecObj, pSecObj->tlsSession.hConn, rv, SipCommonStatus2Str(rv)));
                        return rv;
                    }
                }
                /* Load Remote Address from the connection */
                rv = SipTransportConnectionGetRemoteAddress(
                        pSecObj->tlsSession.hConn, &remoteSipAddress);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - failed to get remote address, used by TLS connection %p(rv=%d:%s)",
                        pSecObj, pSecObj->tlsSession.hConn, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
                memcpy(pRemoteAddr, &remoteSipAddress.addr, sizeof(RvAddress));
            } /* ENDOF: if (RV_TRUE == SecObjSupportsTls(pSecObj)) */
            break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

        default:
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjGetSecureLocalAddressAndConnection(pSecObj=%p) - Unsupported Transport Type %s",
                pSecObj, SipCommonConvertEnumToStrTransportType(eTransportType)));
            return RV_ERROR_BADPARAM;
    } /* END OF:  switch(eTransportType) */

/* This ifdef intends to remove Lint warnings for unreachable code */
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)

    /* Return deduced transport */
    *peTransportType = eTransportType;

    return RV_OK;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SecObjGetSecureServerPort
 * ----------------------------------------------------------------------------
 * General: gets protected Server Port, which was opened by Security Object
 *          in order to get requests in case of TCP and responses in case of
 *          UDP. This port should be set into the "send-by" field of VIA header
 *          for outgoing protected requests. See 3GPP TS 24.229 for details.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Security Object
 *          eTransportType - Type of the Transport, protected port for which is
 *                           requested.
 * Output:  pSecureServerPort - protected server port to be set in VIA header.
 *****************************************************************************/
RvStatus RVCALLCONV SecObjGetSecureServerPort(
                                IN   SecObj*            pSecObj,
                                IN   RvSipTransport     eTransportType,
                                OUT  RvUint16*          pSecureServerPort)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv;
#endif

    /* Prevent compilation warning UNREFERENCED PARAMETER */
#if !(RV_IMS_IPSEC_ENABLED==RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
    RV_UNUSED_ARG(eTransportType);
    RV_UNUSED_ARG(pSecureServerPort);
#endif

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if (pSecObj->eState <= RVSIP_SECOBJ_STATE_IDLE ||
        pSecObj->eState >  RVSIP_SECOBJ_STATE_CLOSING)
#else
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
#endif
//SPIRENT_END
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SecObjGetSecureServerPort(pSecObj=%p) - wrong state %s. Should be ACTIVE",
            pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Fetch required port */
    switch(eTransportType)
    {
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        case RVSIP_TRANSPORT_UDP:
        case RVSIP_TRANSPORT_TCP:
        /* SCTP is not supported yet
        case RVSIP_TRANSPORT_SCTP:
        */
            if (RV_TRUE == SecObjSupportsIpsec(pSecObj))
            {
                *pSecureServerPort = pSecObj->pIpsecSession->ccSAData.iLclPrtSrv;
            }
            break;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            if (RV_TRUE == SecObjSupportsTls(pSecObj))
            {
                RvSipTransportLocalAddrHandle hLocalAddr = NULL;

                /* Use Local Address, set into the Security Object.
                If it was not set, use the address, opened by Connection. */
                if (NULL != pSecObj->tlsSession.hLocalAddr)
                {
                    hLocalAddr = pSecObj->tlsSession.hLocalAddr;
                }
                else
                if (NULL != pSecObj->tlsSession.hConn)
                {
                    rv = SipTransportConnectionGetLocalAddress(
                            pSecObj->tlsSession.hConn, &hLocalAddr);
                    if (RV_OK != rv)
                    {
                        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                            "SecObjGetSecureServerPort(pSecObj=%p) - failed to get local address, used by TLS connection %p(rv=%d:%s)",
                            pSecObj, pSecObj->tlsSession.hConn, rv, SipCommonStatus2Str(rv)));
                        return rv;
                    }
                }
                if (NULL != hLocalAddr)
                {
                    rv = SipTransportLocalAddressGetPort(
                            pSecObj->pMgr->hTransportMgr,
                            hLocalAddr, pSecureServerPort);
                    if (RV_OK != rv)
                    {
                        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                            "SecObjGetSecureServerPort(pSecObj=%p) - failed to get port of local address %p(rv=%d:%s)",
                            pSecObj, hLocalAddr, rv, SipCommonStatus2Str(rv)));
                        return rv;
                    }
                }
            } /* ENDOF: if (RV_TRUE == SecObjSupportsTls(pSecObj)) */
            break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

        default:
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjGetSecureServerPort(pSecObj=%p) - Unsupported Transport Type %s",
                pSecObj, SipCommonConvertEnumToStrTransportType(eTransportType)));
            return RV_ERROR_BADPARAM;
    } /* END OF:  switch(eTransportType) */

/* This ifdef intends to remove Lint warnings for unreachable code */
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
    return RV_OK;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * SecObjLockAPI
 * ----------------------------------------------------------------------------
 * General: Locks Security Object according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - pPointer to the Security Object
 *****************************************************************************/
RvStatus SecObjLockAPI(IN SecObj* pSecObj)
{
    RvStatus       rv;
    SipTripleLock* pTripleLock;
    RvInt32        identifier;
    SecMgr*        pSecMgr;

    for(;;)
    {
        /* ASSUMPTION: pSecObj->pMgr is never NULL. Even for vacant objects. */
        RvMutexLock(&pSecObj->tripleLock.hLock, pSecObj->pMgr->pLogMgr);
        pTripleLock = pSecObj->pTripleLock;
        identifier  = pSecObj->secObjUniqueIdentifier;
        pSecMgr     = pSecObj->pMgr;
        RvMutexUnlock(&pSecObj->tripleLock.hLock, pSecMgr->pLogMgr);

        if (pTripleLock == NULL)
        {
            RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
                "SecObjLockAPI(pSecObj=%p): Triple Lock is NULL", pSecObj));
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjLockAPI(pSecObj=%p): Locking Triple Lock 0x%p",
            pSecObj, pTripleLock));

        rv = LockSecObj(pSecObj, pSecMgr, pTripleLock, identifier);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
                "SecObjLockAPI(pSecObj=%p): Failed to lock(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /*make sure the triple lock was not changed after just before the locking*/
        if (pSecObj->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjLockAPI(pSecObj=%p): Triple Lock %p was changed to %p, reentering lockAPI",
            pSecObj, pTripleLock, pSecObj->pTripleLock));
        RvMutexUnlock(&pTripleLock->hLock, pSecMgr->pLogMgr);
    }

    rv = CRV2RV(RvSemaphoreTryWait(&pTripleLock->hApiLock, NULL));
    if (RV_ERROR_TRY_AGAIN != rv  &&  RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjLockAPI(pSecObj=%p): Failed to lock API Lock %p(rv=%d:%s)",
            pSecObj, &pTripleLock->hApiLock, rv, SipCommonStatus2Str(rv)));
        RvMutexUnlock(&pTripleLock->hLock, pSecMgr->pLogMgr);
        return rv;
    }

    pTripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(pTripleLock->objLockThreadId == 0)
    {
        pTripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&pTripleLock->hLock, pSecMgr->pLogMgr,
                              &pTripleLock->threadIdLockCount);
    }

    RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
        "SecObjLockAPI(pSecObj=%p): Triple Lock %p apiCnt=%d exiting function",
        pSecObj, pTripleLock, pTripleLock->apiCnt));

    return RV_OK;
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * SecObjUnlockAPI
 * ----------------------------------------------------------------------------
 * General: Unlocks Security Object according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - pPointer to the Security Object
 *****************************************************************************/
void SecObjUnlockAPI(IN SecObj* pSecObj)
{
    SipTripleLock* pTripleLock;
    SecMgr*        pSecMgr;
    RvInt32        lockCnt;

    pTripleLock = pSecObj->pTripleLock;
    /* ASSUMPTION: pSecObj->pMgr is never NULL. Even for vacant objects. */
    pSecMgr     = pSecObj->pMgr;

    if (NULL==pTripleLock)
    {
        return;
    }

    lockCnt = 0;
    RvMutexGetLockCounter(&pTripleLock->hLock, pSecMgr->pLogMgr, &lockCnt);

    RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
        "SecObjUnlockAPI(pSecObj=%p):  Triple Lock %p apiCnt=%d lockCnt=%d",
        pSecObj, pTripleLock, pTripleLock->apiCnt, lockCnt));

    if (0 == pTripleLock->apiCnt)
    {
        RvLogExcep(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjUnlockAPI(pSecObj=%p): apiCnt is 0 in Triple Lock %p",
            pSecObj, pTripleLock));
        return;
    }

    if (1 == pTripleLock->apiCnt)
    {
        RvSemaphorePost(&pTripleLock->hApiLock, NULL/*pLogMgr*/);
    }
    pTripleLock->apiCnt--;

    /*reset the thread id in the counter that set it previously*/
    if(lockCnt == pTripleLock->threadIdLockCount)
    {
        pTripleLock->objLockThreadId = 0;
        pTripleLock->threadIdLockCount = UNDEFINED;
    }

    RvMutexUnlock(&pTripleLock->hLock, pSecMgr->pLogMgr);
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * SecObjLockMsg
 * ----------------------------------------------------------------------------
 * General: Locks Security Object according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - pPointer to the Security Object
 *****************************************************************************/
RvStatus SecObjLockMsg(IN SecObj* pSecObj)
{
    RvStatus       rv;
    RvBool         didILockAPI  = RV_FALSE;
    RvThreadId     currThreadId = RvThreadCurrentId();
    SipTripleLock* pTripleLock;
    SecMgr*        pSecMgr;
    RvInt32        identifier;

    /* ASSUMPTION: pSecObj->pMgr is never NULL. Even for vacant objects. */
    RvMutexLock(&pSecObj->tripleLock.hLock, pSecObj->pMgr->pLogMgr);
    pTripleLock = pSecObj->pTripleLock;
    pSecMgr     = pSecObj->pMgr;
    identifier  = pSecObj->secObjUniqueIdentifier;
    RvMutexUnlock(&pSecObj->tripleLock.hLock, pSecObj->pMgr->pLogMgr);

    if (NULL==pTripleLock)
    {
        RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjLockMsg(pSecObj=%p): Triple Lock is NULL", pSecObj));
        return RV_ERROR_NULLPTR;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object, locking again will be useless and will block the stack */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjLockMsg(pSecObj=%p): Exiting without locking. Triple Lock 0x%p, apiCnt=%d already locked",
            pSecObj, pTripleLock, pTripleLock->apiCnt));
        return RV_OK;
    }

    RvMutexLock(&pTripleLock->hProcessLock, pSecMgr->pLogMgr);

    for(;;)
    {
        rv = LockSecObj(pSecObj, pSecMgr, pTripleLock, identifier);
        if (RV_OK != rv)
        {
            RvMutexUnlock(&pTripleLock->hProcessLock, pSecMgr->pLogMgr);
            if (didILockAPI)
            {
                RvSemaphorePost(&pTripleLock->hApiLock,NULL);
            }
            RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
                "SecObjLockMsg(pSecObj=%p): Failed to lock (rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }

        if (0 == pTripleLock->apiCnt)
        {
            if (didILockAPI)
            {
                RvSemaphorePost(&pTripleLock->hApiLock,NULL);
            }
            break;
        }

        RvMutexUnlock(&pTripleLock->hLock, pSecMgr->pLogMgr);

        rv = RvSemaphoreWait(&pTripleLock->hApiLock,NULL);
        if (RV_OK != rv)
        {
            RvMutexUnlock(&pTripleLock->hProcessLock, pSecMgr->pLogMgr);
            RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
                "SecObjLockMsg(pSecObj=%p): Failed to get API lock (rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }

        didILockAPI = RV_TRUE;
    }

    RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
        "SecObjLockMsg(pSecObj=%p): Triple Lock %p exiting function",
        pSecObj, pTripleLock));
    return RV_OK;
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * SecObjUnlockMsg
 * ----------------------------------------------------------------------------
 * General: Unlocks Security Object according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - pPointer to the Security Object
 *****************************************************************************/
void SecObjUnlockMsg(IN SecObj* pSecObj)
{
    SipTripleLock* pTripleLock;
    SecMgr*        pSecMgr;
    RvThreadId     currThreadId = RvThreadCurrentId();

    pTripleLock = pSecObj->pTripleLock;
    /* ASSUMPTION: pSecObj->pMgr is never NULL. Even for vacant objects. */
    pSecMgr     = pSecObj->pMgr;

    if (NULL == pTripleLock)
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "SecObjUnlockMsg(pSecObj=%p): Exiting without unlocking - Triple Lock %p already locked",
            pSecObj, pTripleLock));
        return;
    }

    RvLogSync(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
        "SecObjUnlockMsg(pSecObj=%p): Triple Lock 0x%p", pSecObj, pTripleLock));

    RvMutexUnlock(&pTripleLock->hLock, pSecMgr->pLogMgr);
    RvMutexUnlock(&pTripleLock->hProcessLock, pSecMgr->pLogMgr);
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/


/******************************************************************************
 * SecObjHandleConnStateConnected
 * ----------------------------------------------------------------------------
 * General: handles CONNECTED state, raised by the Security Object's Connection
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:        pSecObj - Pointer to the Security Object
 *               hConn   - Handle of the closed connection
 *****************************************************************************/
RvStatus SecObjHandleConnStateConnected(
                                    IN SecObj* pSecObj,
                                    IN RvSipTransportConnectionHandle hConn)
{
    RvStatus                     rv;
    RvSipTransportConnectionType eConnectionType;
    RvSipTransport               eTransportType;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecObj)
#endif

    rv = RvSipTransportConnectionGetType(hConn, &eConnectionType);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "SecObjHandleConnStateConnected(pSecObj=%p): Failed to get Connection %p type(rv=%d:%s)",
            pSecObj, hConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

#ifdef SIP_DEBUG
    /* Sanity check: if the connection is outgoing,
    it should be set already in the Security Object */
    if (RVSIP_TRANSPORT_CONN_TYPE_CLIENT == eConnectionType)
    {
        RvBool bOwnedConnection = IsConnOwnedBySecObj(pSecObj, hConn);
        if (RV_FALSE == bOwnedConnection)
        {
            RvLogExcep(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
                "SecObjHandleConnStateConnected(pSecObj=%p): Unexpected connection %p",
                pSecObj, hConn));
            return RV_ERROR_UNKNOWN;
        }

    }
#endif

    /* If the connection is incoming,
       it should not be set yet in the Security Object. Set it now. */
    if (RVSIP_TRANSPORT_CONN_TYPE_SERVER != eConnectionType)
    {
        return RV_OK;
    }

    rv = RvSipTransportConnectionGetTransportType(hConn, &eTransportType);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "SecObjHandleConnStateConnected(pSecObj=%p): Failed to get transport type of connection %p(rv=%d:%s)",
            pSecObj, hConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    switch (eTransportType)
    {
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    case RVSIP_TRANSPORT_TCP:
    case RVSIP_TRANSPORT_SCTP:

        /* Sanity check */
        if (NULL == pSecObj->pIpsecSession)
        {
            RvLogExcep(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjHandleConnStateConnected(pSecObj=%p) - no IPsec Session for IPsec Connection %p",
                pSecObj, hConn));
            break;
        }
    /* 1. Deny any incoming connections, if the Security Object is not ACTIVE
       2. Deny any subsequent incoming connections */
        if (NULL != pSecObj->pIpsecSession->hConnS ||
            RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
        {
            rv = CloseConnection(pSecObj, hConn, RV_TRUE/*bDetachFromConn*/);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjHandleConnStateConnected(pSecObj=%p) - Failed to close Connection %p",
                    pSecObj, hConn));
                return rv;
            }
            else
            {
                RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjHandleConnStateConnected(pSecObj=%p) - Incoming connection %p was terminated. Existing server connection %p, Security Object state is %s",
                    pSecObj, hConn, pSecObj->pIpsecSession->hConnS,
                    SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
            }
        }
        else
        /* Update the connection. Attachment of the Security Object to
        the connection is made implicitly by inheriting of reference
        to Security Object from local address, on which the incoming connection
        was accepted. */
        {
            pSecObj->pIpsecSession->hConnS = hConn;
        }
        break;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    default:
        RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "SecObjHandleConnStateConnected(pSecObj=%p): Unsupported transport type %s of Connection %p",
            pSecObj, SipCommonConvertEnumToStrTransportType(eTransportType), hConn));
        return rv;
    }

/* This ifdef intends to remove Lint warnings for unreachable code */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    return RV_OK;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/******************************************************************************
 * SecObjHandleConnStateClosed
 * ----------------------------------------------------------------------------
 * General: handles CLOSED state, raised by the Security Object's Connection
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:        pSecObj    - Pointer to the Security Object
 *               hConn      - Handle of the closed connection
 *               eConnState - State of Connection (CLOSED or TERMINATED)
 *****************************************************************************/
RvStatus SecObjHandleConnStateClosed(
                                IN SecObj* pSecObj,
                                IN RvSipTransportConnectionHandle  hConn,
                                IN RvSipTransportConnectionState   eConnState)
{
    RvStatus       rv;
    RvBool         bOwnedConnection;
    RvSipTransport eTransportType;

    bOwnedConnection = IsConnOwnedBySecObj(pSecObj, hConn);
    if (RV_FALSE == bOwnedConnection)
    {
        rv = HandleUnownedConnState(pSecObj, hConn, eConnState);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjHandleConnStateClosed(pSecObj=%p) - failed to handle state of unowned connection %p(rv=%d:%s)",
                pSecObj, hConn, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        return RV_OK;
    }

    rv = RvSipTransportConnectionGetTransportType(hConn, &eTransportType);
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "SecObjHandleConnStateClosed(pSecObj=%p): Failed to get transport type of connection %p(rv=%d:%s)",
            pSecObj, hConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Reset the reference to the Connection */
    switch (eTransportType)
    {
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case RVSIP_TRANSPORT_TLS:
            if (hConn == pSecObj->tlsSession.hConn)
            {
                rv = SipTransportConnectionSetSecOwner(
                    pSecObj->tlsSession.hConn, NULL, RV_FALSE/*bDisconnect*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjHandleConnStateClosed(pSecObj=%p) - Failed to detach from TLS Connection %p",
                        pSecObj, pSecObj->tlsSession.hConn));
                }
                pSecObj->tlsSession.hConn = NULL;
            }
            break;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_SCTP:

            /* Sanity check */
            if (NULL == pSecObj->pIpsecSession)
            {
                RvLogExcep(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SecObjHandleConnStateClosed(pSecObj=%p) - no IPsec Session for IPsec Connection %p",
                    pSecObj, hConn));
                break;
            }

            if (hConn == pSecObj->pIpsecSession->hConnC)
            {
                rv = SipTransportConnectionSetSecOwner(
                    pSecObj->pIpsecSession->hConnC, NULL, RV_FALSE/*bDisconnect*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjHandleConnStateClosed(pSecObj=%p) - Failed to detach from IPsec Client Connection %p",
                        pSecObj, pSecObj->pIpsecSession->hConnC));
                }
                pSecObj->pIpsecSession->hConnC = NULL;
            }
            else
            if (hConn == pSecObj->pIpsecSession->hConnS)
            {
                rv = SipTransportConnectionSetSecOwner(
                    pSecObj->pIpsecSession->hConnS, NULL, RV_FALSE/*bDisconnect*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjHandleConnStateClosed(pSecObj=%p) - Failed to detach from IPsec Server Connection %p",
                        pSecObj, pSecObj->pIpsecSession->hConnS));
                }
                pSecObj->pIpsecSession->hConnS = NULL;
            }
            break;
    #endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

        default:
            RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
                "SecObjHandleConnStateClosed(pSecObj=%p): Unsupported transport type %s of Connection %p",
                pSecObj, SipCommonConvertEnumToStrTransportType(eTransportType), hConn));
            return rv;
    }

/* This ifdef intends to remove Lint warnings for unreachable code */
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)

    rv = RV_OK;
    switch (pSecObj->eState)
    {
        /* TLS connection can be set into the Security Object at any moment.
           Therefore CLOSED event can be caught by the Object in any state.
        */
        case RVSIP_SECOBJ_STATE_IDLE:
        case RVSIP_SECOBJ_STATE_INITIATED:
        case RVSIP_SECOBJ_STATE_ACTIVE:
            /* If CLOSED/TERMINATED was raised for the connection that serves
            the Security Object's Security Mechanism, initiate shutdown */
            if (
                (RVSIP_TRANSPORT_TLS == eTransportType &&
                 RV_TRUE == SecObjSupportsTls(pSecObj))
                ||
                ((RVSIP_TRANSPORT_TCP == eTransportType || RVSIP_TRANSPORT_SCTP == eTransportType) &&
                 RV_TRUE == SecObjSupportsIpsec(pSecObj))
               )
            {
                rv = SecObjShutdown(pSecObj);
            }
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
            else
            /* If CLOSED/TERMINATED was raised for the connection that doesn't
            serves the Security Object's Security Mechanism, it might be
            the IPsec connection, closed as a result of TLS Security set into
            the Security Object. In this case continue IPsec shutdown. */
            if (RVSIP_SECURITY_MECHANISM_TYPE_TLS == pSecObj->eSecMechanism &&
                NULL != pSecObj->pIpsecSession &&
                (RVSIP_TRANSPORT_TCP == eTransportType ||
                 RVSIP_TRANSPORT_SCTP == eTransportType)
               )
            {
                if (NULL == pSecObj->pIpsecSession->hConnC &&
                    NULL == pSecObj->pIpsecSession->hConnS)
                {
                    rv = SecObjShutdownIpsecComplete(pSecObj);
                    if (RV_OK != rv)
                    {
                        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                            "SecObjHandleConnStateClosed(pSecObj=%p) - Failed to complete IPsec removal",
                            pSecObj));
                        return rv;
                    }
                }
            }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
            break;

        /* CLOSED can be raised for Connection as a result of Shutdown, and as
        a result of network error on the Connection. In last case the Security
        Object should not be moved to the CLOSING state until all pending
        objects (transactions and transmitters) will be closed and all
        connections, opened by the Security Object, will be terminated. */
        case RVSIP_SECOBJ_STATE_CLOSING:
			{
                rv = SecObjShutdownContinue(pSecObj);
                if (RV_OK != rv)
                {
                    RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                        "SecObjHandleConnStateClosed(pSecObj=%p) - failed to continue shutdown(rv=%d:%s)",
                        pSecObj, rv, SipCommonStatus2Str(rv)));
                    return rv;
                }
			}
            break;

        default:
            RvLogExcep(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjHandleConnStateClosed(pSecObj=%p): Unexpected state %s",
                pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
            return RV_ERROR_UNKNOWN;
    }
    if (RV_OK != rv)
    {
        RvLogError(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "SecObjHandleConnStateClosed(pSecObj=%p): Unsupported transport type %s of Connection %p",
            pSecObj, SipCommonConvertEnumToStrTransportType(eTransportType), hConn));
        return rv;
    }

    return RV_OK;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SecObjHandleMsgReceivedEv
 * ----------------------------------------------------------------------------
 * General: handles MsgReceived event:
 *              1. Updates Security mechanism with the protection used for
 *                 message delivery, if it was determined yet.
 *              2. Performs some security checks on the received message.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj       - Pointer to the Security Object
 *          hMsgReceived  - Handle to the received message
 *          pRecvFromAddr - Details of the address, from which the message was
 *                          sent.
 * Output:
 *          pbApprove     - RV_TRUE, if the checks are OK
 *****************************************************************************/
void SecObjHandleMsgReceivedEv(
                                    IN  SecObj*              pSecObj,
                                    IN  RvSipMsgHandle       hMsgReceived,
                                    IN  SipTransportAddress* pRecvFromAddr,
                                    OUT RvBool*              pbApproved)
{
    /* Approve messages, protected with TLS
       (TS33.203 describes checks for messages protected with IPsec only)*/
    if (RVSIP_TRANSPORT_TLS == pRecvFromAddr->eTransport)
    {
        *pbApproved = RV_TRUE;
        return;
    }

    /* If the message is SIP request,
    check if IP in the message's VIA equals to the IP, which the message was
    received from. */
    if (RVSIP_MSG_REQUEST == RvSipMsgGetMsgType(hMsgReceived))
    {
        /* Discard incoming requests, if IP in the message's VIA doesn't equals
        to the IP, which the message was received from */
        *pbApproved = IsIpEquals(pSecObj->pMgr->pLogSrc,
            hMsgReceived, pRecvFromAddr);
        if (RV_FALSE == *pbApproved)
        {
            RvLogWarning(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjHandleMsgReceivedEv(pSecObj=%p) - request message %p is not approved: IP in VIA doesn't match peer IP",
                pSecObj, hMsgReceived));
            return;
        }
        /* Discard incoming requests, if the protection is not active */
        if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
        {
            *pbApproved = RV_FALSE;
            RvLogWarning(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjHandleMsgReceivedEv(pSecObj=%p) - request message %p is not approved: Security Object is not in ACTIVE state",
                pSecObj, hMsgReceived));
            return;
        }
    }

    *pbApproved = RV_TRUE;
    return;
}

/******************************************************************************
 * SecObjSetImpi
 * ----------------------------------------------------------------------------
 * General: Sets IMPI to be set by Security Object into it's Local Addresses
 *          and Connections.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          impi    - IMPI
 *****************************************************************************/
RvStatus SecObjSetImpi(
                        IN SecObj* pSecObj,
                        IN void*   impi)
{
	RvStatus rvFinal = RV_OK;
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv = RV_OK;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SecMgr*  pSecMgr = pSecObj->pMgr;
#endif
#endif

    pSecObj->impi = impi;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Set IMPI into the Connection, if it was opened by the Security Object */
    if (NULL     != pSecObj->tlsSession.hConn &&
        RV_FALSE == pSecObj->tlsSession.bAppConn)
    {
        rv = SipTransportConnectionSetImpi(pSecObj->tlsSession.hConn, impi);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the TLS connection %p(rv=%d:%s)",
                pSecObj, pSecObj->tlsSession.hConn, rv, SipCommonStatus2Str(rv)));
            rvFinal = rv;
        }
    }
#endif

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (NULL != pSecObj->pIpsecSession)
    {
        if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpC)
        {
            rv = SipTransportLocalAddressSetImpi(
                    pSecObj->pIpsecSession->hLocalAddrUdpC, pSecObj->impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the UDP-C Local Address %p (rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpC,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpS)
        {
            rv = SipTransportLocalAddressSetImpi(
                    pSecObj->pIpsecSession->hLocalAddrUdpS, pSecObj->impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the UDP-S Local Address %p (rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpS,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hLocalAddrTcpC)
        {
            rv = SipTransportLocalAddressSetImpi(
                    pSecObj->pIpsecSession->hLocalAddrTcpC, pSecObj->impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the TCP-C Local Address %p (rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpC,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hLocalAddrTcpS)
        {
            rv = SipTransportLocalAddressSetImpi(
                    pSecObj->pIpsecSession->hLocalAddrTcpS, pSecObj->impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the TCP-S Local Address %p (rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpS,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hConnC)
        {
            rv = SipTransportConnectionSetImpi(
                    pSecObj->pIpsecSession->hConnC, impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the Client connection %p(rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hConnC,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hConnS)
        {
            rv = SipTransportConnectionSetImpi(
                    pSecObj->pIpsecSession->hConnS, impi);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SecObjSetImpi(pSecObj=%p) - failed to set IMPI into the Server connection %p(rv=%d:%s)",
                    pSecObj, pSecObj->pIpsecSession->hConnS,
                    rv, SipCommonStatus2Str(rv)));
                rvFinal = rv;
            }
        }
    }
#endif

    return rvFinal;
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjStartTimer
 * ----------------------------------------------------------------------------
 * General: Starts the security-object timer. If interval of 0 is supplied, the
 *          time interval is determined by the security-object lifetime parameter.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj  - Pointer to the Security Object
 *          interval - The time interval to set the timer to (in seconds)
 *****************************************************************************/
RvStatus SecObjStartTimer(IN SecObj*   pSecObj,
                          IN RvUint32  interval)
{
	RvStatus  rv, crv;

	if (pSecObj->pIpsecSession == NULL)
	{
		/* There is no ipsec session in this security-object (for example, TLS is chosen),
		   therefore setting ipsec lifetime timer is not allowed */
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjStartTimer(pSecObj=%p) - Can't start lifetime timer: no active ipsec session at security-object",
					pSecObj));
        return RV_ERROR_ILLEGAL_ACTION;
	}

	if (interval == 0 && pSecObj->pIpsecSession->lifetime == 0)
	{
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjStartTimer(pSecObj=%p) - Can't start lifetime timer: interval 0 is not allowed",
					pSecObj));
        return RV_ERROR_BADPARAM;
	}
    if (interval == 0)
	{
		interval = pSecObj->pIpsecSession->lifetime;
	}
	else if (interval > RV_UINT32_MAX/1000)
    {
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjStartTimer(pSecObj=%p) - Can't start lifetime timer: interval %d too big. Should be less than %d",
					pSecObj, interval, RV_UINT32_MAX/1000));
        return RV_ERROR_BADPARAM;
    }

	/* If the timer was already started, cancel it */
	rv = SipCommonCoreRvTimerCancel(&pSecObj->pIpsecSession->timerLifetime);
	if (RV_OK != rv)
	{
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjStartTimer(pSecObj=%p) - Failed to cancel lifetime timer(rv=%d:%s)",
					pSecObj, rv, SipCommonStatus2Str(rv)));
		return rv;
	}
	
	/* Start the timer according to the time interval */
    crv = SipCommonCoreRvTimerStartEx(
                                    &pSecObj->pIpsecSession->timerLifetime,
                                    pSecObj->pMgr->pSelectEngine,
                                    interval*1000,
                                    SipSecObjLifetimeTimerHandler,
                                    (void*)pSecObj);
    if (RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjStartTimer(pSecObj=%p) - Failed to start lifetime timer(crv=%d,rv=%d:%s)",
					pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
        SipCommonCoreRvTimerExpired(&pSecObj->pIpsecSession->timerLifetime);
		return rv;
    }

	return RV_OK;
}

/******************************************************************************
 * SecObjGetRemainingTime
 * ----------------------------------------------------------------------------
 * General: Gets the remaining time of the security-object lifetime timer.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj        - Pointer to the Security Object
 * Output:    pRemainingTime - Pointer to the remaining time value.
 *****************************************************************************/
RvStatus SecObjGetRemainingTime(IN  SecObj*    pSecObj,
								OUT RvInt32*   pRemainingTime)
{
    if(SipCommonCoreRvTimerStarted(&pSecObj->pIpsecSession->timerLifetime) == RV_FALSE)
    {
        *pRemainingTime = UNDEFINED;
		RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjGetRemainingTime(pSecObj=%p) - timer is not set, returning UNDEFINED", pSecObj));
    }
    else
    {
        RvInt64 delay;
        RvTimerGetRemainingTime(&pSecObj->pIpsecSession->timerLifetime.hTimer, &delay);
        *pRemainingTime = RvInt64ToRvInt32(RvInt64Div(delay, RV_TIME64_NSECPERSEC));
		RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SecObjGetRemainingTime(pSecObj=%p) - remained time is %d", pSecObj, *pRemainingTime));
    }

    return RV_OK;
}
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */


/*---------------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * AllocateIpsecSession
 * ----------------------------------------------------------------------------
 * General: initiates Security Object in context of IPsec.
 *          Generates SPIs and Secure Ports, opens Client and Server
 *          Local Addresses.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus AllocateIpsecSession(IN SecObj* pSecObj)
{
    RvStatus          rv;
    RLIST_ITEM_HANDLE listItem;
    SecMgr*           pSecMgr = pSecObj->pMgr;

    if (NULL == pSecMgr->hIpsecSessionPool)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "AllocateIpsecSession(pSecObj=%p) - IPsec Session pool was not constructed",
            pSecObj));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Allocate and initiate IPsec session object */
    rv = RLIST_InsertTail(pSecMgr->hIpsecSessionPool,
                          pSecMgr->hIpsecSessionList, &listItem);
    if(RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "AllocateIpsecSession(pSecObj=%p) - failed to allocate new IPsec Session object(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    pSecObj->pIpsecSession = (SecObjIpsec*)listItem;
    memset(pSecObj->pIpsecSession, 0, sizeof(SecObjIpsec));
    pSecObj->pIpsecSession->eTransportType    = RVSIP_TRANSPORT_UNDEFINED;
    pSecObj->pIpsecSession->ccSAData.iEncrAlg = RvImsEncrUndefined;
    pSecObj->pIpsecSession->ccSAData.iAuthAlg = RvImsAuthUndefined;
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * InitiateIpsec
 * ----------------------------------------------------------------------------
 * General: initiates Security Object in context of IPsec.
 *          Generates SPIs and Secure Ports, opens Client and Server
 *          Local Addresses.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus InitiateIpsec(IN SecObj* pSecObj)
{
    SecMgr*            pSecMgr = pSecObj->pMgr;
    RvStatus           rv;
    RvSipTransportAddr localAddr;

    /* Open Local Addresses on secure ports */
    if ('\0' == pSecObj->strLocalIp[0])
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitiateIpsec(pSecObj=%p) - local IP was not set. Can't open secure ports",
            pSecObj));
        return RV_ERROR_UNKNOWN;
    }

    /* If session was not allocated yet, allocate it now */
    if (NULL == pSecObj->pIpsecSession)
    {
        RvStatus rv;
        rv = AllocateIpsecSession(pSecObj);
        if (RV_OK == rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "InitiateIpsec(pSecObj=%p) - failed to allocate IPsec session(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    /* Generate SPIs */
    if (0 == pSecObj->pIpsecSession->ccSAData.iSpiInClnt ||
        0 == pSecObj->pIpsecSession->ccSAData.iSpiInSrv)
    {
        RvStatus crv;
        crv = rvImsGetPairOfSPIs(&pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
                                 &pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
                                 &pSecMgr->ccImsCfg, pSecMgr->pLogMgr);
        if (RV_OK != crv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "InitiateIpsec(pSecObj=%p) - Failed to generate default SPIs(crv=%d)",
                pSecObj, RvErrorGetCode(crv)));
            return CRV2RV(crv);
        }
        else
        {
            RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "InitiateIpsec(pSecObj=%p) - set/default SPI-C=%u and SPI-S=%u will be used",
                pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
                pSecObj->pIpsecSession->ccSAData.iSpiInSrv));
        }
    }

    /* Open UDP client port */
    memset(&localAddr, 0, sizeof(RvSipTransportAddr));
    strcpy(localAddr.strIP,pSecObj->strLocalIp);
    localAddr.Ipv6Scope = pSecObj->ipv6Scope;
    localAddr.port = pSecObj->pIpsecSession->ccSAData.iLclPrtClnt;
    localAddr.eTransportType = RVSIP_TRANSPORT_UDP;
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    localAddr.eAddrType = strchr (localAddr.strIP, ':') ? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 : RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
#endif
//SPIRENT_END

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(pSecObj->pIpsecSession->eTransportType != RVSIP_TRANSPORT_TCP) {
#endif
//SPIRENT_END

      rv = OpenLocalAddress(pSecObj, &localAddr, RV_FALSE/*bDontOpen*/,
        &pSecObj->pIpsecSession->hLocalAddrUdpC);
      if (RV_OK != rv)
      {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
          "InitiateIpsec(pSecObj=%p) - failed to open UDP secure client port(rv=%d:%s)",
          pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
      }
      pSecObj->pIpsecSession->ccSAData.iLclPrtClnt = localAddr.port;

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    }
#endif
//SPIRENT_END
    
    /* Open UDP server port */
    if (NULL == pSecObj->pIpsecSession->hLocalAddrUdpS)
    {

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
      if(pSecObj->pIpsecSession->eTransportType != RVSIP_TRANSPORT_TCP) {
#endif
//SPIRENT_END

        localAddr.port = pSecObj->pIpsecSession->ccSAData.iLclPrtSrv;
        rv = OpenLocalAddress(pSecObj, &localAddr, RV_FALSE/*bDontOpen*/,
          &pSecObj->pIpsecSession->hLocalAddrUdpS);
        if (RV_OK != rv)
        {
          RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitiateIpsec(pSecObj=%p) - failed to open UDP secure server port(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
          return rv;
        }
        pSecObj->pIpsecSession->ccSAData.iLclPrtSrv = localAddr.port;
      }
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    }
#endif
//SPIRENT_END

    else /* If Local Address was set by SecAgree object, load its port*/
    {
      RvSipTransportAddr addrDetails;
      rv = SipTransportMgrLocalAddressGetDetails(
        pSecObj->pIpsecSession->hLocalAddrUdpS, &addrDetails);
      if (RV_OK != rv)
      {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
          "InitiateIpsec(pSecObj=%p) - failed to get details of UDP local address %p (rv=%d:%s)",
          pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpS, rv,
          SipCommonStatus2Str(rv)));
        return rv;
      }
      pSecObj->pIpsecSession->ccSAData.iLclPrtSrv = addrDetails.port;
    }

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    if(pSecObj->pIpsecSession->eTransportType != RVSIP_TRANSPORT_UDP) {
#endif
//SPIRENT_END

      if (RV_TRUE == pSecMgr->bTcpEnabled)
      {
        /* Open TCP client port */
        localAddr.port = pSecObj->pIpsecSession->ccSAData.iLclPrtClnt;
        localAddr.eTransportType = RVSIP_TRANSPORT_TCP;
        rv = OpenLocalAddress(pSecObj, &localAddr, RV_TRUE/*bDontOpen*/,
          &pSecObj->pIpsecSession->hLocalAddrTcpC);
        if (RV_OK != rv)
        {
          RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitiateIpsec(pSecObj=%p) - failed to open TCP secure client port %d (rv=%d:%s)",
            pSecObj, localAddr.port, rv, SipCommonStatus2Str(rv)));
          return rv;
        }

        /* Open Connection, but don't connect it yet. This operation will
        bind the connection to the Port-C. CONNECT will be performed later,
        when Transmitter will try to send through this connection. */
        rv = OpenConnection(pSecObj, RVSIP_TRANSPORT_TCP,
          pSecObj->pIpsecSession->hLocalAddrTcpC,
          NULL/*pRemoteAddr*/,
          &pSecObj->pIpsecSession->hConnC);
        if (RV_OK != rv)
        {
          RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitiateIpsec(pSecObj=%p) - failed to construct TCP client connection(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
          return rv;
        }
        rv = SipTransportConnectionOpenDontConnect(
          pSecObj->pIpsecSession->hConnC,
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
          &pSecObj->pIpsecSession->ccSAData.iLclPrtClnt);
#else
          pSecObj->pIpsecSession->ccSAData.iLclPrtClnt);
#endif
//SPIRENT_END
        if (RV_OK != rv)
        {
          RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitiateIpsec(pSecObj=%p) - failed to bind TCP client connection to port %d(rv=%d:%s)",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iLclPrtClnt,
            rv, SipCommonStatus2Str(rv)));
          return rv;
        }

        /* Open TCP server port. Server Connection will be constructed, when
        ACCEPT will be raised on this port. */
        if (NULL == pSecObj->pIpsecSession->hLocalAddrTcpS)
        {
          localAddr.port = pSecObj->pIpsecSession->ccSAData.iLclPrtSrv;
          rv = OpenLocalAddress(pSecObj, &localAddr, RV_FALSE/*bDontOpen*/,
            &pSecObj->pIpsecSession->hLocalAddrTcpS);
          if (RV_OK != rv)
          {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
              "InitiateIpsec(pSecObj=%p) - failed to open TCP secure server port %d (rv=%d:%s)",
              pSecObj, localAddr.port, rv, SipCommonStatus2Str(rv)));
            return rv;
          }
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
          pSecObj->pIpsecSession->ccSAData.iLclPrtSrv = localAddr.port;
#endif
//SPIRENT_END
        }
      } /* END OF:  if (RV_TRUE == pSecObj->pSecMgr->bTcpEnabled) */
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    }
#endif
//SPIRENT_END

    return RV_OK;
}
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus ResetIpsecAddr(IN SecObj* pSecObj)
{
  RvStatus           rv=RV_OK;

  if(pSecObj) {

    if(pSecObj->pIpsecSession) {
      if(pSecObj->pIpsecSession->hLocalAddrTcpC) {
        SipTransportLocalAddressSetSecOwner(pSecObj->pIpsecSession->hLocalAddrTcpC, (void*)pSecObj);
      }
      if(pSecObj->pIpsecSession->hLocalAddrTcpS) {
        SipTransportLocalAddressSetSecOwner(pSecObj->pIpsecSession->hLocalAddrTcpS, (void*)pSecObj);
      }
      if(pSecObj->pIpsecSession->hLocalAddrUdpC) {
        SipTransportLocalAddressSetSecOwner(pSecObj->pIpsecSession->hLocalAddrUdpC, (void*)pSecObj);
      }
      if(pSecObj->pIpsecSession->hLocalAddrUdpS) {
        SipTransportLocalAddressSetSecOwner(pSecObj->pIpsecSession->hLocalAddrUdpS, (void*)pSecObj);
      }
    }
  }

  return rv;
}
#endif
//SPIRENT_END
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * ActivateIpsec
 * ----------------------------------------------------------------------------
 * General: activates message protection using IPsec.
 *          As a result, the object will move to ACTIVE state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus ActivateIpsec(IN SecObj* pSecObj)
{
    RvStatus  rv, crv;
    SecMgr* pSecMgr = pSecObj->pMgr;

    /* Check if all parameters needed for IPsec were set */
    if (RV_FALSE == CheckIpsecParams(pSecObj))
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ActivateIpsec(pSecObj=%p) - Can't start IPsec-3GPP. Check IPsec parameters",
            pSecObj));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Fill missing fields of the Common Core ccSAData structure */
    rv = InitializeCcSAData(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ActivateIpsec(pSecObj=%p) - Can't start IPsec-3GPP. Failed to initialize Common Core SA structure",
            pSecObj));
        return RV_ERROR_UNKNOWN;
    }

    /* Set Destination address into client Connection.
    The address and port should be known at this point. */
    if (NULL != pSecObj->pIpsecSession->hConnC)
    {
        rv = SipTransportConnectionSetDestAddress(
                pSecObj->pIpsecSession->hConnC,
                &pSecObj->pIpsecSession->ccSAData.iPeerAddress);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "ActivateIpsec(pSecObj=%p) - Failed to set destination address into client connection(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

#ifdef SIP_DEBUG
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    LogIpsecParameters(pSecObj);
#endif
#endif

    /* Configure Operation System */
    crv = rvImsIPSecEstablishSAs(&pSecObj->pIpsecSession->ccSAData, pSecMgr->pLogMgr);
    if (RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ActivateIpsec(pSecObj=%p) - Can't start IPsec-3GPP. Failed to configure system(crv=%d,rv=%d:%s)",
            pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Start lifetime timer, if was set */
    if (0 < pSecObj->pIpsecSession->lifetime)
    {
		rv = SecObjStartTimer(pSecObj, 0);
		if (RV_OK != rv)
		{
			RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
						"ActivateIpsec(pSecObj=%p) - Failed to start lifetime timer(rv=%d:%s)",
						pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
		}

        RvLogInfo(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "ActivateIpsec(pSecObj=%p) - Lifetime Timer %p was set to %d seconds",
            pSecObj, &pSecObj->pIpsecSession->timerLifetime,
            pSecObj->pIpsecSession->lifetime));
    } /* END OF: timer start */

    return RV_OK;
}

#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * RemoveIpsec
 * ----------------------------------------------------------------------------
 * General: Removes IPsec protection from the Security Object.
 *          If the object is idle:
 *             (5) Free local SPIs
 *             (6) Free IPsec session memory
 *          If the object was initialized:
 *             (3) Close client connection
 *             (4) Close local addresses
 *             (5) Free local SPIs
 *             (6) Free IPsec session memory
 *          If the object was activated:
 *             (0) Remove IPsec from the system
 *             (1) Stop lifetime timer
 *             (2) Close server connection, if it was opened
 *             (3) Close client connection
 *             (4) Close local addresses
 *             (5) Free local SPIs
 *             (6) Free IPsec session memory
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus RemoveIpsec(IN SecObj*  pSecObj)
{
    RvStatus rv, crv;
    SecMgr*  pSecMgr = pSecObj->pMgr;

    switch (pSecObj->eState)
    {
        case RVSIP_SECOBJ_STATE_ACTIVE:
            /* (1) Stop lifetime timer */
            rv = SipCommonCoreRvTimerCancel(&pSecObj->pIpsecSession->timerLifetime);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
                    "RemoveIpsec(pSecObj=%p): failed to cancel lifetime timer(rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
            }
            /* (2) Close server connection if it was opened */
            if (NULL != pSecObj->pIpsecSession->hConnS)
            {
                rv = CloseConnection(pSecObj, pSecObj->pIpsecSession->hConnS,
                                     RV_TRUE/*bDetachFromConn*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                        "RemoveIpsec(pSecObj=%p) - Failed to close IPsec server Connection %p",
                        pSecObj, pSecObj->pIpsecSession->hConnS));
                }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
                pSecObj->pIpsecSession->hConnS = NULL;
#else
                pSecObj->pIpsecSession->hConnC = NULL;
#endif
/* SPIRENT_END */
            }
            /* break; BREAK is missed intentionally */

        case RVSIP_SECOBJ_STATE_INITIATED:
            /* (3) Close client connection */
            if (NULL != pSecObj->pIpsecSession->hConnC)
            {
                rv = CloseConnection(pSecObj, pSecObj->pIpsecSession->hConnC,
                                     RV_TRUE/*bDetachFromConn*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                        "RemoveIpsec(pSecObj=%p) - Failed to close IPsec client Connection %p",
                        pSecObj, pSecObj->pIpsecSession->hConnC));
                }
                pSecObj->pIpsecSession->hConnC = NULL;
            }
            /* (4) Close local addresses */
            rv = CloseLocalAddresses(pSecObj);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "RemoveIpsec(pSecObj=%p) - failed to close IPsec local addresses (rv=%d:%s)",
                    pSecObj, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            /* break; BREAK is missed intentionally */

        case RVSIP_SECOBJ_STATE_IDLE:

            /* (0) For active object only: remove IPsec from system
                   (do this after closure of connections in cases above
                    in order to enable sending of protected FIN/RST) */
            if (pSecObj->eState == RVSIP_SECOBJ_STATE_ACTIVE)
            {
                crv = rvImsIPSecDestroySAs(&pSecObj->pIpsecSession->ccSAData, pSecMgr->pLogMgr);
                if (RV_OK != crv)
                {
                    rv = CRV2RV(crv);
                    RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                        "RemoveIpsec(pSecObj=%p) - Failed to remove SAs from system(crv=%d,rv=%d:%s)",
                        pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
                }
            }

            /* (5) Free SPIs */
            crv = rvImsReturnSPIs(pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
                                  pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
                                  pSecObj->pIpsecSession->ccSAData.iSpiOutClnt,
                                  pSecObj->pIpsecSession->ccSAData.iSpiOutSrv,
                                  &pSecMgr->ccImsCfg, pSecMgr->pLogMgr);
            if (RV_OK != crv)
            {
                rv = CRV2RV(crv);
                RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "RemoveIpsec(pSecObj=%p) - Failed to mark SPIs as not used(crv=%d,rv=%d:%s)",
                    pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
            }
            pSecObj->pIpsecSession->ccSAData.iSpiInClnt  = 0;
            pSecObj->pIpsecSession->ccSAData.iSpiInSrv   = 0;
            pSecObj->pIpsecSession->ccSAData.iSpiOutClnt = 0;
            pSecObj->pIpsecSession->ccSAData.iSpiOutSrv  = 0;

            /* (6) Free IPsec session memory */
            if (RV_OK != SecMgrLock(&pSecMgr->hMutex, pSecMgr->pLogMgr))
			{
				RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
							"RemoveIpsec(pSecObj=%p) - Failed to lock manager %p",
							pSecObj, pSecMgr));
				return RV_ERROR_INVALID_HANDLE;
			}
            RLIST_Remove(pSecMgr->hIpsecSessionPool, pSecMgr->hIpsecSessionList,
                         (RLIST_ITEM_HANDLE)pSecObj->pIpsecSession);
            SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);
            pSecObj->pIpsecSession = NULL;
            break;

        default:
            return RV_OK;
    } /* ENDOF: switch (pSecObj->eState) */
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * ShutdownIpsec
 * ----------------------------------------------------------------------------
 * General: Shutdown IPsec protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 * Output: pShutdownCompleted - RV_FALSE, if IPsec was not removed from System
 *****************************************************************************/
static RvStatus ShutdownIpsec(
                            IN SecObj*  pSecObj,
                            OUT RvBool* pShutdownCompleted)
{
    RvStatus rv;
    SecMgr*  pSecMgr = pSecObj->pMgr;

    *pShutdownCompleted = RV_TRUE;

#ifdef SIP_DEBUG
    /* Sanity Check */
    if (NULL == pSecObj->pIpsecSession)
    {
        RvLogExcep(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ShutdownIpsec(pSecObj=%p) - There is no IPsec session",
            pSecObj));
        return RV_ERROR_UNKNOWN;
    }
#endif

    rv = SipCommonCoreRvTimerCancel(&pSecObj->pIpsecSession->timerLifetime);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ShutdownIpsec(pSecObj=%p) - Failed to cancel lifetime timer %p. Continue.",
            pSecObj, &pSecObj->pIpsecSession->timerLifetime));
    }

    rv = SecObjShutdownIpsecContinue(pSecObj, pShutdownCompleted);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ShutdownIpsec(pSecObj=%p) - Failed to continue shutdown",
            pSecObj));
        return rv;
    }
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * SecObjShutdownContinue
 * ----------------------------------------------------------------------------
 * General: continue Shutdown if it was suspended due to existing pending
 *          objects (transactions and transmitters) or opened connections.
 *          If no pending objects or not closed connections exist,
 *          moves the Security Object to the CLOSED state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdownContinue(IN SecObj* pSecObj)
{
    RvStatus rv;
    RvBool   bShutdownComplete = RV_TRUE;

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (NULL != pSecObj->pIpsecSession)
    {
        rv = SecObjShutdownIpsecContinue(pSecObj, &bShutdownComplete);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjShutdownContinue(pSecObj=%p) - Failed to continue IPsec shutdown(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    if (RV_TRUE == bShutdownComplete)
    {
        rv = SecObjChangeState(pSecObj, RVSIP_SECOBJ_STATE_CLOSED,
                               RVSIP_SECOBJ_REASON_UNDEFINED);
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SecObjShutdownContinue(pSecObj=%p) - Failed to change state to CLOSED(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    return RV_OK;
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjShutdownIpsecContinue
 * ----------------------------------------------------------------------------
 * General: Close Local Addresses and Connections, used by the IPsec.
 *          IPsec can be removed from the Operation System only upon
 *          reception the TERMINATED event on connections.
 *          Otherwise pending on the connection data can be sent in plain text.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 * Output: pShutdownCompleted - RV_FALSE, if IPsec was not removed from System
 *****************************************************************************/
RvStatus SecObjShutdownIpsecContinue(
                            IN SecObj*  pSecObj,
                            OUT RvBool* pShutdownCompleted)
{
    RvStatus rv, rvFinal;
    SecMgr*  pSecMgr = pSecObj->pMgr;
    RvUint32 numOfClosedConn;

    rvFinal = RV_OK;
    *pShutdownCompleted = RV_TRUE;

    /* If there are pending Transactions, using the Security Object, wait */
    if (0 < pSecObj->pIpsecSession->counterTransc)
    {
        *pShutdownCompleted = RV_FALSE;
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecContinue(pSecObj=%p) - Wait for termination of %d pending objects",
            pSecObj, pSecObj->pIpsecSession->counterTransc));
        return RV_OK;
    }

    /* Close Local Address */
    rv = CloseLocalAddresses(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecContinue(pSecObj=%p) - Failed to close Local Addresses",
            pSecObj));
        rvFinal = rv;
    }

    /* Close Connections */
    numOfClosedConn = 0;
    rv = CloseConnections(pSecObj, RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP,
                          RV_FALSE/*bDetachFromConn*/, &numOfClosedConn);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecContinue(pSecObj=%p) - Failed to close Connections",
            pSecObj));
        rvFinal = rv;
    }

    /* If there are Connections to be closed, wait for Connection TERMINATED
    event in order to ensure secure clean of the connection sockets */
    if (0 < numOfClosedConn)
    {
        *pShutdownCompleted = RV_FALSE;
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecContinue(pSecObj=%p) - Wait for termination of %d IPsec connections",
            pSecObj, numOfClosedConn));
        return RV_OK;
    }

    /* Remove IPsec from the system */
    rv = SecObjShutdownIpsecComplete(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecContinue(pSecObj=%p) - Failed to complete IPsec removal",
            pSecObj));
        rvFinal = rv;
    }

    return rvFinal;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjShutdownIpsecComplete
 * ----------------------------------------------------------------------------
 * General: Removes IPsec from the system.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdownIpsecComplete(IN SecObj*  pSecObj)
{
    RvStatus rv, crv;
    SecMgr*  pSecMgr = pSecObj->pMgr;

    /* Remove IPsec from the system */
    crv = rvImsIPSecDestroySAs(&pSecObj->pIpsecSession->ccSAData, pSecMgr->pLogMgr);
    if (RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecComplete(pSecObj=%p) - Failed to remove SAs from system(crv=%d,rv=%d:%s)",
            pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
    }
    /* Free SPIs */
    crv = rvImsReturnSPIs(pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
                          pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
                          pSecObj->pIpsecSession->ccSAData.iSpiOutClnt,
                          pSecObj->pIpsecSession->ccSAData.iSpiOutSrv,
                          &pSecMgr->ccImsCfg, pSecMgr->pLogMgr);
    if (RV_OK != crv)
    {
        rv = CRV2RV(crv);
        RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SecObjShutdownIpsecComplete(pSecObj=%p) - Failed to mark SPIs as not used(crv=%d,rv=%d:%s)",
            pSecObj, RvErrorGetCode(crv), rv, SipCommonStatus2Str(rv)));
    }
    pSecObj->pIpsecSession->ccSAData.iSpiInClnt  = 0;
    pSecObj->pIpsecSession->ccSAData.iSpiInSrv   = 0;
    pSecObj->pIpsecSession->ccSAData.iSpiOutClnt = 0;
    pSecObj->pIpsecSession->ccSAData.iSpiOutSrv  = 0;

    /* Free IPsec Session object */
            if (RV_OK != SecMgrLock(&pSecMgr->hMutex, pSecMgr->pLogMgr))
			{
				RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
							"SecObjShutdownIpsecComplete(pSecObj=%p) - Failed to lock manager %p",
							pSecObj, pSecMgr));
				return RV_ERROR_INVALID_HANDLE;
			}
    RLIST_Remove(pSecMgr->hIpsecSessionPool, pSecMgr->hIpsecSessionList,
                (RLIST_ITEM_HANDLE)pSecObj->pIpsecSession);
    SecMgrUnlock(&pSecMgr->hMutex,pSecMgr->pLogMgr);
    pSecObj->pIpsecSession = NULL;

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * ShutdownTls
 * ----------------------------------------------------------------------------
 * General: Shutdown TLS protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj            - Pointer to the Security Object
 * Output:    pShutdownCompleted - RV_FALSE, if there was connection to be
 *                                 closed. RV_TRUE - otherwise.
 *****************************************************************************/
static RvStatus ShutdownTls(IN SecObj*  pSecObj,
                            OUT RvBool* pShutdownCompleted)
{
    RvStatus rv;
    RvUint32 numOfClosedConn;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SecMgr*  pSecMgr = pSecObj->pMgr;
#endif

    /* Close Connections */
    numOfClosedConn = 0;
    rv = CloseConnections(pSecObj, RVSIP_SECURITY_MECHANISM_TYPE_TLS,
                          RV_FALSE/*bDetachFromConn*/, &numOfClosedConn);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ShutdownTls(pSecObj=%p) - Failed to close Connections",
            pSecObj));
        return rv;
    }

    /* If there are Connections to be closed, wait for Connection TERMINATED
    event in order to ensure secure clean of the connection sockets */
    if (0 < numOfClosedConn)
    {
        *pShutdownCompleted = RV_FALSE;
        RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "ShutdownTls(pSecObj=%p) - Wait for termination of %d TLS connections",
            pSecObj, numOfClosedConn));
        return RV_OK;
    }

    return RV_OK;
}
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * RemoveTls
 * ----------------------------------------------------------------------------
 * General: Removes TLS protection from Security Object.
 *          If the object is active, the TLS is shutdown first.
 *          Frees resource, allocated for TLS.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus RemoveTls(IN SecObj*  pSecObj)
{
    if (RVSIP_SECOBJ_STATE_ACTIVE == pSecObj->eState)
    {
        RvStatus rv;
        RvBool   bUnusedParam;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
		SecMgr*  pSecMgr = pSecObj->pMgr;
#endif

        rv = ShutdownTls(pSecObj, &bUnusedParam/*pShutdownCompleted*/);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "RemoveTls(pSecObj=%p) - Failed to shutdown TLS(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
        }
    }
    return RV_OK;
}
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * CheckIpsecParams
 * ----------------------------------------------------------------------------
 * General: check, if all parameters, needs for IPsec-3GPP are set in
 *          the Security Object.
 *
 * Return Value: RV_TRUE if the parameters are set. RV_FALSE - otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvBool CheckIpsecParams(IN SecObj* pSecObj)
{
    SecMgr* pMgr = pSecObj->pMgr;

    if (0 == strlen(pSecObj->strLocalIp))
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Local IP is not set",
            pSecObj));
        return RV_FALSE;
    }
    if (0 == strlen(pSecObj->strRemoteIp))
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Remote IP is not set",
            pSecObj));
        return RV_FALSE;
    }
    if (RvImsEncrUndefined == pSecObj->pIpsecSession->ccSAData.iEncrAlg)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Encryption Algorithm is not set",
            pSecObj));
        return RV_FALSE;
    }
    if (RvImsAuthUndefined == pSecObj->pIpsecSession->ccSAData.iAuthAlg)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Integrity Algorithm is not set",
            pSecObj));
        return RV_FALSE;
    }
    if (0==pSecObj->pIpsecSession->ccSAData.iLclPrtClnt  ||
        0==pSecObj->pIpsecSession->ccSAData.iLclPrtSrv   ||
        0==pSecObj->pIpsecSession->ccSAData.iPeerPrtClnt ||
        0==pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Secure Ports were not set: local Port-C=%d Port-S=%d, remote Port-C=%d, Port-S=%d",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iLclPrtClnt,
            pSecObj->pIpsecSession->ccSAData.iLclPrtSrv,
            pSecObj->pIpsecSession->ccSAData.iPeerPrtClnt,
            pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv));
        return RV_FALSE;
    }
    /* All known Operation Systems, supporting IPsec (May 2006 - Linux,Solaris,
    VxWorks), reserve [0-255] values for SPI for their own usage.*/
    if (256 > pSecObj->pIpsecSession->ccSAData.iSpiInClnt   ||
        256 > pSecObj->pIpsecSession->ccSAData.iSpiInSrv    ||
        256 > pSecObj->pIpsecSession->ccSAData.iSpiOutClnt  ||
        256 > pSecObj->pIpsecSession->ccSAData.iSpiOutSrv)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - SPIs should not be in range [0-255]: local SPI-C=%u SPI-S=%u, remote SPI-C=%u, SPI-S=%u",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
            pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
            pSecObj->pIpsecSession->ccSAData.iSpiOutClnt,
            pSecObj->pIpsecSession->ccSAData.iSpiOutSrv));
        return RV_FALSE;
    }
    if (0 == pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Integrity Key is not set",
            pSecObj));
        return RV_FALSE;
    }
    if (0 == pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits &&
        RvImsEncrNull != pSecObj->pIpsecSession->ccSAData.iEncrAlg)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Cypher Key is not set",
            pSecObj));
        return RV_FALSE;
    }

    /* Check length of the keys */
    if (RvImsEncr3Des == pSecObj->pIpsecSession->ccSAData.iEncrAlg &&
        192 != pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Size of 3DES Key (%d bits) should be 192 bits",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits));
        return RV_FALSE;
    }
    if (RvImsEncrAes == pSecObj->pIpsecSession->ccSAData.iEncrAlg &&
        128 != pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Size of AES Key (%d bits) should be 128 bits",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iEncrKey.iKeyBits));
        return RV_FALSE;
    }
    if (RvImsAuthMd5 == pSecObj->pIpsecSession->ccSAData.iAuthAlg &&
        128 != pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Size of MD5 Key (%d bits) should be 128 bits",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits));
        return RV_FALSE;
    }
    if (RvImsAuthSha1 == pSecObj->pIpsecSession->ccSAData.iAuthAlg &&
        160 != pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits)
    {
        RvLogWarning(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckIpsecParams(pSecObj=%p) - Size of SHA-1 Key (%d bits) should be 160 bits",
            pSecObj, pSecObj->pIpsecSession->ccSAData.iAuthKey.iKeyBits));
        return RV_FALSE;
    }

    return RV_TRUE;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * CloseLocalAddresses
 * ----------------------------------------------------------------------------
 * General: Close Local Addresses, used by the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus CloseLocalAddresses(IN SecObj* pSecObj)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(pSecObj);
    return RV_OK;
#else
    RvStatus rvFinal;
    SecMgr*  pMgr = pSecObj->pMgr;
    RvStatus rv;

    rvFinal=RV_OK;

    /* Local Addresses can be opened for IPsec only */
    if (NULL == pSecObj->pIpsecSession ||
        RV_FALSE == SecObjSupportsIpsec(pSecObj))
    {
        return RV_OK;
    }

    if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpC)
    {
        rv = CloseLocalAddress(pSecObj,&pSecObj->pIpsecSession->hLocalAddrUdpC);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CloseLocalAddresses(pSecObj=%p) - Failed to close UDP Client Local Address %p",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpC));
            rvFinal = rv;
        }
    }
    if (NULL != pSecObj->pIpsecSession->hLocalAddrUdpS)
    {
        rv = CloseLocalAddress(pSecObj,&pSecObj->pIpsecSession->hLocalAddrUdpS);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CloseLocalAddresses(pSecObj=%p) - Failed to close UDP Server Local Address %p",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrUdpS));
            rvFinal = rv;
        }
    }
    if (NULL != pSecObj->pIpsecSession->hLocalAddrTcpC)
    {
        rv = CloseLocalAddress(pSecObj,&pSecObj->pIpsecSession->hLocalAddrTcpC);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CloseLocalAddresses(pSecObj=%p) - Failed to close TCP Client Local Address %p",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpC));
            rvFinal = rv;
        }
    }
    if (NULL != pSecObj->pIpsecSession->hLocalAddrTcpS)
    {
        rv = CloseLocalAddress(pSecObj,&pSecObj->pIpsecSession->hLocalAddrTcpS);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CloseLocalAddresses(pSecObj=%p) - Failed to close TCP Server Local Address %p",
                pSecObj, pSecObj->pIpsecSession->hLocalAddrTcpS));
            rvFinal = rv;
        }
    }
    return rvFinal;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
}

/******************************************************************************
 * CloseConnections
 * ----------------------------------------------------------------------------
 * General: Close Connections, used by the Security Object for specified
 *          mechanism. If no mechanism was specified, all opened connections
 *          will be closed.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj          - Pointer to the Security Object
 *         eSecMechanism    - Security Mechanism served by the connections
 *                            to be closed
 *         bDetachFromConn  - If RV_TRUE - object will be detach from
 *                            connections.
 *                            Generally the Security Object should not be
 *                            detached form connection till CLOSED/TERMINATED
 *                            event for the connection will be handled.
 *                            Such policy ensures secured shutdown of
 *                            the connection's socket.
 * Output: pNumOfClosedConn - Number of Connections, closure of that was
 *                            initiated. Can be NULL.
 *****************************************************************************/
static RvStatus CloseConnections(
                            IN  SecObj*                    pSecObj,
                            IN  RvSipSecurityMechanismType eSecMechanism,
                            IN  RvBool                     bDetachFromConn,
                            OUT RvUint32*                  pNumOfClosedConn)
{
    RvStatus rvFinal;
    RvUint32 numOfClosedConn;

#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SecMgr*  pSecMgr = pSecObj->pMgr;
#endif
    RvStatus rv;
#endif

#if !(RV_IMS_IPSEC_ENABLED==RV_YES) && (RV_TLS_TYPE==RV_TLS_NONE)
    RV_UNUSED_ARG(pSecObj);
    RV_UNUSED_ARG(eSecMechanism);
#endif

    numOfClosedConn = 0;
    rvFinal = RV_OK;

    /* Close TLS connections */
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_SECURITY_MECHANISM_TYPE_TLS       == eSecMechanism ||
        RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED == eSecMechanism)
    {
        if (NULL != pSecObj->tlsSession.hConn)
        {
            RvSipTransportConnectionHandle hConn = pSecObj->tlsSession.hConn;

            /* Reset security owner and disconnect the connection. */
            if (RV_TRUE == bDetachFromConn ||
                RV_TRUE == pSecObj->tlsSession.bAppConn)
            {
                rv = SipTransportConnectionSetSecOwner(
                    pSecObj->tlsSession.hConn, NULL, RV_TRUE/*bDisconnect*/);
                if (RV_OK != rv)
                {
                    RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                        "CloseConnections(pSecObj=%p) - Failed to detach from Connection %p",
                        pSecObj, pSecObj->tlsSession.hConn));
                    rvFinal = rv;
                }
                pSecObj->tlsSession.hConn = NULL;
            }
            else
            {
                /* Disconnect the connection only */
                rv = RvSipTransportConnectionTerminate(hConn);
                if (RV_OK != rv)
                {
                    RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                        "CloseConnections(pSecObj=%p) - Failed to close TLS Connection %p",
                        pSecObj, hConn));
                    rvFinal = rv;
                }
                numOfClosedConn++;
                RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "CloseConnections - pSecObj=%p: terminated TLS Connection %p",pSecObj,hConn));
            }
        }
    }
#else
	RV_UNUSED_ARG(bDetachFromConn)
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)

    /* If there is no IPsec, just return */
    if (NULL == pSecObj->pIpsecSession)
    {
        if (NULL != pNumOfClosedConn)
        {
            *pNumOfClosedConn = numOfClosedConn;
        }
        return rvFinal;
    }

    if (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP == eSecMechanism ||
        RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED  == eSecMechanism)
    {
        if (NULL != pSecObj->pIpsecSession->hConnC)
        {
            numOfClosedConn++;
            rv = CloseConnection(pSecObj, pSecObj->pIpsecSession->hConnC,
                                 bDetachFromConn);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "CloseConnections(pSecObj=%p) - Failed to close IPsec client Connection %p",
                    pSecObj, pSecObj->pIpsecSession->hConnC));
            }
            if (RV_TRUE == bDetachFromConn)
            {
                pSecObj->pIpsecSession->hConnC = NULL;
            }
        }
        if (NULL != pSecObj->pIpsecSession->hConnS)
        {
            numOfClosedConn++;
            rv = CloseConnection(pSecObj, pSecObj->pIpsecSession->hConnS,
                                 bDetachFromConn);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "CloseConnections(pSecObj=%p) - Failed to close IPsec server Connection %p",
                    pSecObj, pSecObj->pIpsecSession->hConnS));
            }
            if (RV_TRUE == bDetachFromConn)
            {
                pSecObj->pIpsecSession->hConnS = NULL;
            }
        }
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    if (NULL != pNumOfClosedConn)
    {
        *pNumOfClosedConn = numOfClosedConn;
    }

    return rvFinal;
}

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * LockSecObj
 * ----------------------------------------------------------------------------
 * General: Locks Security Object, while performing check, if the object
 *          is not vacant.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj       - Pointer to the Security Object
 *          pSecMgr       - Pointer to the Security Manager
 *          pTtripleLock  - Security Object's triple lock
 *          identifier    - Security Object's unique identifier
 *****************************************************************************/
static RvStatus LockSecObj(IN SecObj*        pSecObj,
                           IN SecMgr*        pSecMgr,
                           IN SipTripleLock* pTtripleLock,
                           IN RvInt32        identifier)
{
    RvStatus rv;

    rv = RvMutexLock(&pTtripleLock->hLock, pSecMgr->pLogMgr);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc ,(pSecMgr->pLogSrc ,
            "LockSecObj(pSecObj=%p): Failed to lock hLock=%p (rv=%d:%s)",
            pSecObj, &pTtripleLock->hLock, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    if (pSecObj->secObjUniqueIdentifier != identifier ||
        pSecObj->secObjUniqueIdentifier == 0)
    {
        /*The Security Object is invalid*/
        RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "LockSecObj(pSecObj=%p): RLIST_IsElementVacant returned RV_TRUE",
            pSecObj));
        RvMutexUnlock(&pTtripleLock->hLock, pSecMgr->pLogMgr);
        return RV_ERROR_DESTRUCTED;
    }

    return RV_OK;
}
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/******************************************************************************
 * IsConnOwnedBySecObj
 * ----------------------------------------------------------------------------
 * General: Checks if the Connection is referenced by the Security Object
 *
 * Return Value: RV_TRUE, if it does, RV_FALSE - otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:        pSecObj - Pointer to the Security Object
 *               hConn   - Handle of the connection
 *****************************************************************************/
static RvBool IsConnOwnedBySecObj(
                            IN SecObj* pSecObj,
                            IN RvSipTransportConnectionHandle  hConn)
{
    /* Prevent compilation warning UNREFERENCED PARAMETER */
#if !(RV_IMS_IPSEC_ENABLED==RV_YES) && (RV_TLS_TYPE == RV_TLS_NONE)
    RV_UNUSED_ARG(pSecObj);
    RV_UNUSED_ARG(hConn);
#endif

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (hConn == pSecObj->tlsSession.hConn)
    {
        return RV_TRUE;
    }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (NULL != pSecObj->pIpsecSession &&
        (hConn == pSecObj->pIpsecSession->hConnC ||
        hConn == pSecObj->pIpsecSession->hConnS)
        )
    {
        return RV_TRUE;
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
    return RV_FALSE;
}

/******************************************************************************
 * IsIpEquals
 * ----------------------------------------------------------------------------
 * General: checks, if IP of the address, from which the Message was received,
 *          is equal to the IP, mentioned in the message's VIA header.
 *
 * Return Value: RV_TRUE if it does, RV_FALSE - otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLogSrc       - Pointer to the Log Source
 *          hMsgReceived  - Handle to the Message
 *          pRecvFromAddr - Address, from which the message was received
 *****************************************************************************/
static RvBool IsIpEquals(IN RvLogSource*         pLogSrc,
                         IN RvSipMsgHandle       hMsg,
                         IN SipTransportAddress* pRecvFromAddr)
{
    RvStatus                  rv;
    RvSipHeaderListElemHandle hListElem;
    RvSipViaHeaderHandle      hVia;
    RvUint                    strViaIpLen;
    RvChar                    strViaIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    SipTransportAddress       viaAddr;
    RvBool                    bViaIpV4;

    /* Get the top VIA element */
    hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(
                hMsg, RVSIP_HEADERTYPE_VIA, RVSIP_FIRST_HEADER, &hListElem);
    if(hVia == NULL)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "IsIpEquals - No via header in message %p", hMsg));
        return RV_FALSE;
    }

    /* Get the VIA's IP */
    rv = RvSipViaHeaderGetHost(
        hVia, strViaIp, RVSIP_TRANSPORT_LEN_STRING_IP, &strViaIpLen);
    if (RV_OK != rv)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "IsIpEquals - Message %p, VIA header %p: failed to get host(rv=%d:%s)",
            hMsg, hVia, rv, SipCommonStatus2Str(rv)));
        return RV_FALSE;
    }

    /* Convert string IP into Common Core address to be compared */
    bViaIpV4 = RV_TRUE;
#if (RV_NET_TYPE & RV_NET_IPV6)
    if (NULL != strchr(strViaIp, ':'))
    {
        bViaIpV4 = RV_FALSE;
    }
#endif  /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    rv = SipTransportString2Address(strViaIp, &viaAddr, bViaIpV4);
    if (rv != RV_OK)
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "IsIpEquals - Message %p, VIA header %p: failed to convert %s into SIP Address(rv=%d:%s)",
            hMsg, hVia, strViaIp, rv, SipCommonStatus2Str(rv)));
        return RV_FALSE;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

    /* Compare addresses */
    return RvAddressCompare(&pRecvFromAddr->addr, &viaAddr.addr,
                            RV_ADDRESS_BASEADDRESS);
}

/* Prevent compilation warning UNUSED FUNCTION */
#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)
/******************************************************************************
 * OpenConnection
 * ----------------------------------------------------------------------------
 * General: Opens secure client connection of the requested type.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj     - Pointer to the Security Object
 *          eTransport  - Transport Type of the Connection to be opened
 *          hLocalAddr  - Handle to the secure Local Address object to be used
 *          remotePort  - destination secure server port
 *          pRemoteAddr - destination address in Common Core format.Can be NULL
 * Output:  phConn      - created connection
 *****************************************************************************/
static RvStatus OpenConnection(
                            IN  SecObj*                         pSecObj,
                            IN  RvSipTransport                  eTransport,
                            IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                            IN  RvAddress*                      pRemoteAddr,
                            OUT RvSipTransportConnectionHandle* phConn)
{
    RvStatus             rv;
    SipTransportAddress  destAddress;
    SipTransportAddress* pDestAddress = NULL;
    SecMgr*              pSecMgr = pSecObj->pMgr;

    /* Prepare SipTransportAddress structure to hold destination address */
    destAddress.eTransport = eTransport;
    if (NULL != pRemoteAddr)/*Copy RvAddress in SipTransportAddress structure*/
    {
        memcpy(&destAddress.addr, pRemoteAddr, sizeof(RvAddress));
        destAddress.eTransport = eTransport;
        pDestAddress = &destAddress;
    }

    /* Create connection */
    rv = SipTransportConnectionConstruct(
        pSecMgr->hTransportMgr, RV_TRUE/*bForceCreation*/,
        RVSIP_TRANSPORT_CONN_TYPE_CLIENT, eTransport, hLocalAddr, pDestAddress,
        NULL/*hOwner*/, NULL/*pfnStateChangedEvHandler*/,
        NULL/*pfnConnStatusEvHandle*/, NULL/*pbNewConnCreated*/, phConn);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "OpenConnection(pSecObj=%p) - failed to create connection(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Set Security Object into the connection */
    rv = SipTransportConnectionSetSecOwner(
            *phConn, (void*)pSecObj, RV_FALSE/*bDisconnect*/);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "OpenConnection(pSecObj=%p) - failed to set Security Object into the connection %p(rv=%d:%s)",
            pSecObj, *phConn, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    if (NULL != pSecObj->impi)
    {
        rv = SipTransportConnectionSetImpi(*phConn, pSecObj->impi);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "OpenConnection(pSecObj=%p) - failed to set IMPI into the connection %p(rv=%d:%s)",
                pSecObj, *phConn, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "OpenConnection - pSecObj=%p: opened Connection %p", pSecObj, *phConn));

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)*/

/* Prevent compilation warning UNUSED FUNCTION */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * OpenLocalAddress
 * ----------------------------------------------------------------------------
 * General: opens Secure Local Address: finds opened Local Address and sets
 *          the Security Object into it. If the address doesn't exist in
 *          the Stack, opens the new one.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj         - Pointer to the Security Object
 *          pAddressDetails - IP, port and other details of the address to be
 *                            opened
 *          bDontOpen       - if RV_TRUE, no socket will be opened.
 * Output:  phLocalAddr     - Handle to the opened Local Address
 *****************************************************************************/
static RvStatus OpenLocalAddress(
                            IN  SecObj*                        pSecObj,
                            IN  RvSipTransportAddr*            pAddressDetails,
                            IN  RvBool                         bDontOpen,
                            OUT RvSipTransportLocalAddrHandle* phLocalAddr)
{
    RvStatus rv;
    SecMgr*  pSecMgr = pSecObj->pMgr;

    rv = SipTransportMgrLocalAddressAdd(pSecMgr->hTransportMgr,pAddressDetails,
                                        bDontOpen, phLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "OpenLocalAddress(pSecObj=%p) - failed to add local address (port %d, rv=%d:%s)",
            pSecObj, pAddressDetails->port, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    rv = SipTransportLocalAddressSetSecOwner(*phLocalAddr, (void*)pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "OpenLocalAddress(pSecObj=%p) - failed to set Security Object into the address (rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    if (NULL != pSecObj->impi)
    {
        rv = SipTransportLocalAddressSetImpi(*phLocalAddr, pSecObj->impi);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "OpenLocalAddress(pSecObj=%p) - failed to set IMPI into the address (rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "OpenLocalAddress - pSecObj=%p: added Local Address %p(%s %s:%d)",
        pSecObj, *phLocalAddr,
        SipCommonConvertEnumToStrTransportType(pAddressDetails->eTransportType),
        pAddressDetails->strIP, pAddressDetails->port));

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/* Prevent compilation warning UNUSED FUNCTION */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * CloseLocalAddress
 * ----------------------------------------------------------------------------
 * General: close Secure Local Address.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj     - Pointer to the Security Object
 *          phLocalAddr - Handle to the Local Address to be closed
 * Output:  phLocalAddr - NULL Handle, if the Local Address was closed
 *****************************************************************************/
static RvStatus CloseLocalAddress(
                            IN    SecObj*                        pSecObj,
                            INOUT RvSipTransportLocalAddrHandle* phLocalAddr)
{
    RvStatus rv;
    SecMgr*  pSecMgr = pSecObj->pMgr;

    /* Detach the Security Object from the Local Address */
    rv = SipTransportLocalAddressSetSecOwner(*phLocalAddr,NULL/*pSecOwner*/);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "CloseLocalAddress(pSecObj=%p) - failed to reset Security Object in the Local Address %p(rv=%d:%s)",
            pSecObj, *phLocalAddr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Close Local Address */
    rv = RvSipTransportMgrLocalAddressRemove(pSecMgr->hTransportMgr,
                                             *phLocalAddr);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "CloseLocalAddress(pSecObj=%p) - failed to close Local Address %p(rv=%d:%s)",
            pSecObj, *phLocalAddr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "CloseLocalAddress - pSecObj=%p: removed Local Address %p", pSecObj, *phLocalAddr));

    *phLocalAddr = NULL;
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)*/

/* Prevent compilation warning UNUSED FUNCTION */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * CloseConnection
 * ----------------------------------------------------------------------------
 * General: initiates Connection terminations and resets the reference to
 *          the Security Object, stored in the connection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj         - Pointer to the Security Object.
 *         hConn           - Connection to be closed.
 *         bDetachFromConn - If RV_TRUE - object will be detach from connection
 *****************************************************************************/
static RvStatus CloseConnection(
                            IN SecObj*                        pSecObj,
                            IN RvSipTransportConnectionHandle hConn,
                            IN RvBool                         bDetachFromConn)
{
    RvStatus rv, rvFinal = RV_OK;
    SecMgr*  pSecMgr = pSecObj->pMgr;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if (!hConn)
       return RV_OK;
#endif
/* SPIRENT_END */

    /* Reset security owner and disconnect the connection */
    if (RV_TRUE == bDetachFromConn)
    {
        rv = SipTransportConnectionSetSecOwner(hConn, NULL,
                                               RV_TRUE/*bDisconnect*/);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "CloseConnection(pSecObj=%p) - Failed to detach from Connection %p",
                pSecObj, hConn));
            rvFinal = rv;
        }
        else
        {
            RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "CloseConnection - pSecObj=%p: was detached from Connection %p",
                pSecObj, hConn));
        }
    }
    else /* Disconnect the connection only */
    { 
        rv = RvSipTransportConnectionTerminate(hConn);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "CloseConnection(pSecObj=%p) - Failed to close Connection %p.Continue.",
                pSecObj, hConn));
            rvFinal = rv;
        }
        else
        {
            RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "CloseConnection - pSecObj=%p: terminates Connection %p", pSecObj, hConn));
        }
    }

    return rvFinal;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) || (RV_TLS_TYPE!=RV_TLS_NONE)*/


/******************************************************************************
 * GetDefaultTransport
 * ----------------------------------------------------------------------------
 * General: gets Transport, matching the Security Mechanism, provided
 *          by the Security Object. For IPsec: if no predefined Transport was
 *          set, UDP will be returned.
 *          If the Security Mechanism was not set yet, UDP will be returned.
 *          If IPsec is not supported, TLS will be returned.
 *          If TLS is not supported, error will be returned.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj           - Pointer to the Security Object
 * Output:  peTransportType   - Requested Transport
 *****************************************************************************/
static RvStatus GetDefaultTransport(IN  SecObj*         pSecObj,
                                    OUT RvSipTransport* peTransportType)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RV_TRUE == SecObjSupportsTls(pSecObj))
    {
        *peTransportType = RVSIP_TRANSPORT_TLS;
        return RV_OK;
    }
#else
	RV_UNUSED_ARG(peTransportType)
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (RV_TRUE == SecObjSupportsIpsec(pSecObj))
    {
        *peTransportType = pSecObj->pIpsecSession->eTransportType;
        if (RVSIP_TRANSPORT_UNDEFINED == *peTransportType)
        {   /* default - UDP */
            *peTransportType = RVSIP_TRANSPORT_UDP;
        }
        return RV_OK;
    }
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecObj);
#endif

    RvLogExcep(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "GetDefaultTransport(pSecObj=%p) - Security Mechanism %s is not supported. Can't deduce default Secure Transport",
        pSecObj, SipCommonConvertEnumToStrSecMechanism(pSecObj->eSecMechanism)));
    return RV_ERROR_UNKNOWN;
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * InitializeCcSAData
 * ----------------------------------------------------------------------------
 * General: initializes Common Core structure ccSAData with the IPsec
 *          parameters, loaded into the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static RvStatus InitializeCcSAData(IN SecObj* pSecObj)
{
    SecMgr* pSecMgr = pSecObj->pMgr;
    RvInt   addrType;

    /* Build Local RvAddress */
    addrType = (NULL==strchr(pSecObj->strLocalIp,':')) ?
               RV_ADDRESS_TYPE_IPV4 : RV_ADDRESS_TYPE_IPV6;
#if !(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == addrType)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitializeCcSAData - IPv6 is not supported"));
        return RV_ERROR_BADPARAM;
    }
#endif
    RvAddressConstruct(addrType,&pSecObj->pIpsecSession->ccSAData.iLclAddress);
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == addrType)
    {
        RvAddressSetIpv6Scope(&pSecObj->pIpsecSession->ccSAData.iLclAddress,
                              pSecObj->ipv6Scope);
    }
#endif /* #if(RV_NET_TYPE & RV_NET_IPV6) */
    RvAddressSetIpPort(&pSecObj->pIpsecSession->ccSAData.iLclAddress,
                       pSecObj->pIpsecSession->ccSAData.iLclPrtClnt);

    if (NULL == RvAddressSetString(pSecObj->strLocalIp,
                                &pSecObj->pIpsecSession->ccSAData.iLclAddress))
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitializeCcSAData - failed to set Local IP string"));
        return RV_ERROR_BADPARAM;
    }

    /* Build Remote RvAddress */
    addrType = (NULL==strchr(pSecObj->strRemoteIp,':')) ?
               RV_ADDRESS_TYPE_IPV4 : RV_ADDRESS_TYPE_IPV6;
#if !(RV_NET_TYPE & RV_NET_IPV6)
    if (RV_ADDRESS_TYPE_IPV6 == addrType)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitializeCcSAData - IPv6 is not supported"));
        return RV_ERROR_BADPARAM;
    }
#endif
    RvAddressConstruct(addrType,&pSecObj->pIpsecSession->ccSAData.iPeerAddress);
    RvAddressSetIpPort(&pSecObj->pIpsecSession->ccSAData.iPeerAddress,
                       pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv);
    if (NULL == RvAddressSetString(pSecObj->strRemoteIp,
                            &pSecObj->pIpsecSession->ccSAData.iPeerAddress))
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "InitializeCcSAData - failed to set Remote IP string"));
        return RV_ERROR_BADPARAM;
    }

    /* Fill transport type filed */
    switch (pSecObj->pIpsecSession->eTransportType)
    {
        case RVSIP_TRANSPORT_UDP:
            pSecObj->pIpsecSession->ccSAData.iProto = RvImsProtoUdp;
            break;
        case RVSIP_TRANSPORT_TCP:
            pSecObj->pIpsecSession->ccSAData.iProto = RvImsProtoTcp;
            break;
        default:
            pSecObj->pIpsecSession->ccSAData.iProto = RvImsProtoAny;
    }

    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#ifdef SIP_DEBUG
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * LogIpsecParameters
 * ----------------------------------------------------------------------------
 * General: prints to log parameters of the Security Associations that are
 *          going to be set in the system.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
static void LogIpsecParameters(IN SecObj* pSecObj)
{
    SecMgr* pSecMgr = pSecObj->pMgr;
    RvChar  strLocalIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvChar  strRemoteIp[RVSIP_TRANSPORT_LEN_STRING_IP];

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "LogIpsecParameters - pSecObj %p IPsec params: local IP=%s, remote IP=%s, transport=%s, lifetime=%d",
        pSecObj,
        RvAddressGetString(&pSecObj->pIpsecSession->ccSAData.iLclAddress,RVSIP_TRANSPORT_LEN_STRING_IP,strLocalIp),
        RvAddressGetString(&pSecObj->pIpsecSession->ccSAData.iPeerAddress,RVSIP_TRANSPORT_LEN_STRING_IP,strRemoteIp),
        SipCommonConvertEnumToStrTransportType(pSecObj->pIpsecSession->eTransportType),
        pSecObj->pIpsecSession->lifetime));
    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "LogIpsecParameters - pSecObj %p IPsec params: Encryption Alg=%s, Integrity Alg=%s",
        pSecObj,
        SipCommonConvertEnumToStrCoreEncrAlg(pSecObj->pIpsecSession->ccSAData.iEncrAlg),
        SipCommonConvertEnumToStrCoreIntegrAlg(pSecObj->pIpsecSession->ccSAData.iAuthAlg)));
    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "LogIpsecParameters - pSecObj %p IPsec params: Local SPI-C=%u SPI-S=%u, Remote SPI-C=%u SPI-S=%u",
        pSecObj, pSecObj->pIpsecSession->ccSAData.iSpiInClnt,
        pSecObj->pIpsecSession->ccSAData.iSpiInSrv,
        pSecObj->pIpsecSession->ccSAData.iSpiOutClnt,
        pSecObj->pIpsecSession->ccSAData.iSpiOutSrv));
    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "LogIpsecParameters - pSecObj %p IPsec params: Local PORT-C=%d PORT-S=%d, Remote PORT-C=%d PORT-S=%d",
        pSecObj, pSecObj->pIpsecSession->ccSAData.iLclPrtClnt,
        pSecObj->pIpsecSession->ccSAData.iLclPrtSrv,
        pSecObj->pIpsecSession->ccSAData.iPeerPrtClnt,
        pSecObj->pIpsecSession->ccSAData.iPeerPrtSrv));
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#endif /*#ifdef SIP_DEBUG*/

/******************************************************************************
 * HandleUnownedConnState
 * ----------------------------------------------------------------------------
 * General: handles state, reported for the connection, which is unknown by
 *          Security Object.
 *          In general, if this function is called, it means the connection
 *          was somehow related to the Security Object. That is why, if
 *          Security Object doesn't own the connection, something gone wrong.
 *          The exception should be printed.
 *          But there is one flow, where this situation is legal:
 *          incoming on secure port connection inherits reference to the port's
 *          Security Object. As a result, all states, raised for this
 *          connection, are reported to the Security Object. If error occurs,
 *          while accepting the connection, the first state, reported to
 *          the Security Object will be TERMINATED. In this case secure ports,
 *          should b closed.Therefore the Object termination will be initiated.
 *
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj    - Pointer to the Security Object
 *            hConn      - Handle to the unknown Connection
 *            eConnState - State of the connection
 *****************************************************************************/
static RvStatus HandleUnownedConnState(
                            IN SecObj*                         pSecObj,
                            IN RvSipTransportConnectionHandle  hConn,
                            IN RvSipTransportConnectionState   eConnState)
{
    RvStatus rv;
    RvSipTransportConnectionType eConnectionType;
    
    rv = RvSipTransportConnectionGetType(hConn, &eConnectionType);
    if (RV_OK != rv  ||
        (RVSIP_TRANSPORT_CONN_TYPE_SERVER == eConnectionType &&
         RVSIP_TRANSPORT_CONN_STATE_TERMINATED == eConnState))
    {
        if (RVSIP_SECOBJ_STATE_ACTIVE == pSecObj->eState)
        {
            rv = SecObjShutdown(pSecObj);
        }
        else
        {
            rv = SecObjTerminate(pSecObj);
        }
        if (RV_OK != rv)
        {
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "HandleUnownedConnState(pSecObj=%p) - failed to shutdown/terminate Security Object(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
        }
        return rv;
    }

    /* Race condition between closing connection by Security Object
    (while terminating the Security Object) and closing the connection by
    Transport Layer can cause report of CLOSED state for connection
    that is not referenced anymore by the Security Object.
    In this case warn and return OK. Otherwise - report exception. */
    if (RVSIP_TRANSPORT_CONN_STATE_CLOSED == eConnState &&
        (RVSIP_SECOBJ_STATE_CLOSING == pSecObj->eState ||
         RVSIP_SECOBJ_STATE_CLOSED  == pSecObj->eState))
    {
        RvLogWarning(pSecObj->pMgr->pLogSrc ,(pSecObj->pMgr->pLogSrc ,
            "HandleUnownedConnState(pSecObj=%p): there is no reference to Connection %p anymore",
            pSecObj, hConn));
        return RV_OK;
    }

    RvLogExcep(pSecObj->pMgr->pLogSrc, (pSecObj->pMgr->pLogSrc,
        "HandleUnownedConnState(pSecObj=%p): Unexpected state for connection %p",
        pSecObj, hConn));
    return RV_ERROR_UNKNOWN;
}

#endif /*#ifdef RV_SIP_IMS_ON*/

