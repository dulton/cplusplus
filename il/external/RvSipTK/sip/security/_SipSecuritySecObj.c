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
 *                              <_SipSecuritySecObj.c>
 *
 * The _SipSecurity.h file contains Internal API functions of Security Object.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          February 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCore.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipTransaction.h"

#include "_SipSecurityUtils.h"
#include "SecurityCallbacks.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                             MODULE VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTIONS PROTOTYPES                      */
/*---------------------------------------------------------------------------*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
static RvStatus UpdateOwnerCounterForIncomingOwner(
                                    IN RvLogSource* pLogSrc,
                                    IN SecObj*      pSecObj,
                                    IN void*        pOwner,
                                    IN RvInt32      delta);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

static RvStatus UpdateOwnerCounter(
                                    IN RvLogSource*               pLogSrc,
                                    IN SecObj*                    pSecObj,
                                    IN void*                      pOwner,
                                    IN RvSipCommonStackObjectType eOwnerType,
                                    IN RvInt32                    delta);

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
static RvStatus UpdateTransactionCounter(
                                    IN RvLogSource*               pLogSrc,
                                    IN SecObj*                    pSecObj,
                                    IN void*                      pOwner,
                                    IN RvInt32                    delta);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

static RvStatus UpdateCounter(
                              IN RvLogSource*               pLogSrc,
                              IN SecObj*                    pSecObj,
                              IN void*                      pOwner,
                              IN RvInt32                    delta);

/*---------------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SipSecObjSetSecAgreeObject
 * ----------------------------------------------------------------------------
 * General: set Security Agreement owner into the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr   - Handle to the Security Manager
 *          hSecObj   - Handle to the Security Object
 *          pSecAgree - Pointer to Security Agreement object
 *          secMechsToInitiate - bit mask of the mechanisms to be initiated
 *                      by the Security Object.
 *                      Example: 
 * RVSIP_SECURITY_MECHANISM_TYPE_TLS & RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP
 *****************************************************************************/
RvStatus SipSecObjSetSecAgreeObject(
                        IN  RvSipSecMgrHandle   hSecMgr,
                        IN  RvSipSecObjHandle   hSecObj,
                        IN  void*               pSecAgree,
                        IN  RvInt32             secMechsToInitiate)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SipSecObjSetSecAgreeObject(hSecObj=%p): SecAgree %p, requested mechanism Mask: 0x%lu",
        hSecObj, pSecAgree, secMechsToInitiate));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetSecAgreeObject(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    if (NULL != pSecObj->pSecAgree  &&  NULL != pSecAgree)
    {
        RvLogWarning(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetSecAgreeObject(hSecObj=%p): override existing SecAgree %p",
            hSecObj, pSecObj->pSecAgree));
    }

    /* If Security Agreement is performed, no default mechanism should be
    proposed. Security Agreement Object is responsible to provide
    the mechanism. */
    if(pSecAgree != NULL)
    {
        pSecObj->eSecMechanism = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
        rv = SecObjSetMechanismsToInitiate(pSecObj, secMechsToInitiate);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SipSecObjSetSecAgreeObject(hSecObj=%p) - Failed to set mechanisms to be initiated (rv=%d:%s)",
                hSecObj, rv, SipCommonStatus2Str(rv)));
            SecObjUnlockAPI(pSecObj);
            return rv;
        }
    }

    pSecObj->pSecAgree = pSecAgree;
    /* If SecAgree owner was removed, check and remove Security Object */
    if (NULL == pSecObj->pSecAgree)
    {
        rv = SecObjRemoveIfNoOwnerIsLeft(pSecObj);
        if (RV_OK != rv)
        {
            SecObjFree(pSecObj);
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SipSecObjSetSecAgreeObject(pSecObj=%p) - Failed to remove object",
                pSecObj));
            SecObjUnlockAPI(pSecObj);
            return rv;
        }
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * SipSecObjUpdateOwnerCounter
 * ----------------------------------------------------------------------------
 * General: increments or decrements counter of owners, using the Security
 *          Object for message protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr         - Handle to the Security Manager
 *          hSecObj         - Handle to the Security Object
 *          delta           - Delta to be added to the counter. Can be negative
 *          eOwnerType      - Type of the Owner
 *          pOwner          - Pointer to the Owner
 *****************************************************************************/
RvStatus SipSecObjUpdateOwnerCounter(
                            IN  RvSipSecMgrHandle           hSecMgr,
                            IN  RvSipSecObjHandle           hSecObj,
                            IN  RvInt32                     delta,
                            IN  RvSipCommonStackObjectType  eOwnerType,
                            IN  void*                       pOwner)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjUpdateOwnerCounter(hSecObj=%p): Owner %p, Delta=%d",
        hSecObj, pOwner, delta));

    /* Transactions and Transmitter should update counterTransc,used by IPsec*/
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    /* Handle Incoming Transactions separately since they cause
       Msg locking scheme for the Security Object.
    */
    if (RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION == eOwnerType)
    {
        RvBool   bIsOutgoing;
        rv = RvSipTransactionIsUAC((RvSipTranscHandle)pOwner, &bIsOutgoing);
        if (RV_OK != rv)
        {
            RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                "SipSecObjUpdateOwnerCounter(hSecObj=%p) - RvSipTransactionIsUAC failed for %p object(rv=%d:%s)",
                hSecObj, pOwner, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        if (RV_FALSE == bIsOutgoing)
        {
            rv = UpdateOwnerCounterForIncomingOwner(pSecMgr->pLogSrc, pSecObj,
                                                    pOwner, delta);
            if (RV_OK != rv)
            {
                RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
                    "SipSecObjUpdateOwnerCounter(hSecObj=%p) - UpdateOwnerCounterForIncomingOwner failed for %p object(rv=%d:%s)",
                    hSecObj, pOwner, rv, SipCommonStatus2Str(rv)));
                return rv;
            }
            return RV_OK;
        }
    }
#else
    RV_UNUSED_ARG(eOwnerType);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    rv = UpdateOwnerCounter(pSecMgr->pLogSrc,pSecObj,pOwner,eOwnerType,delta);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjUpdateOwnerCounter(hSecObj=%p) - UpdateOwnerCounter failed for %p object(rv=%d:%s)",
            hSecObj, pOwner, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * SipSecObjConnStateChangedEv
 * ----------------------------------------------------------------------------
 * General: This is an implementation for the callback function which is called
 *          by the Transport Layer on connection state change.
 * Return Value: not in use.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn   - The connection handle
 *          hSecObj - Handle to the Security Object owner.
 *          eState  - The Connection state
 *          eReason - A reason for the state change
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjConnStateChangedEv(
            IN  RvSipTransportConnectionHandle             hConn,
            IN  RvSipTransportConnectionOwnerHandle        hSecObj,
            IN  RvSipTransportConnectionState              eState,
            IN  RvSipTransportConnectionStateChangedReason eReason)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;

    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* ASSUMPTION: pSecObj->pMgr is never NULL. Even for vacant objects. */

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SipSecObjConnStateChangedEv(pSecObj=%p) - handle %s state (reason - %s) of %p connection",
        pSecObj, SipCommonConvertEnumToStrConnectionState(eState),
        SipCommonConvertEnumToStrConnectionStateChangeRsn(eReason), hConn));

    switch (eState)
    {
        case RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED:
        case RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED:
            rv = SecObjHandleConnStateConnected(pSecObj, hConn);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SipSecObjConnStateChangedEv(pSecObj=%p) - failed to handle CONNECTED state. Connection %p(rv=%d:%s)",
                    pSecObj, hConn, rv, SipCommonStatus2Str(rv)));
                SecObjUnlockMsg(pSecObj);
                return rv;
            }
            break;

        case RVSIP_TRANSPORT_CONN_STATE_CLOSED:
        case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:
            rv = SecObjHandleConnStateClosed(pSecObj, hConn, eState);
            if (RV_OK != rv)
            {
                RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                    "SipSecObjConnStateChangedEv(pSecObj=%p) - failed to handle %s state. Connection %p(rv=%d:%s)",
                    pSecObj, SipCommonConvertEnumToStrConnectionState(eState),
                    hConn, rv, SipCommonStatus2Str(rv)));
                SecObjUnlockMsg(pSecObj);
                return rv;
            }
            break;

        default:
            SecObjUnlockMsg(pSecObj);
            return RV_OK;
    }

	SecObjUnlockMsg(pSecObj);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(eReason);
#endif

	return RV_OK;
}

/******************************************************************************
 * SipSecObjMsgReceivedEv
 * ----------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a message is received.
 *          The function runs some security checks on the message and approves
 *          or disapprove the message in accordance to the check results.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - Handle to the received message
 *          pRecvFromAddr - The address from which the message was received.
 *          pSecOwner     - Pointer to the Security Object, owning the local
 *                          address or connection, on which the message was
 *                          received.
 * Output:
 *          pbApproved    - RV_TRUE, if the message was approved by Security
 *                          Module, RV_FALSE - otherwise. In case of RV_FALSE
 *                          the Message will be discarded.
 *****************************************************************************/
void RVCALLCONV SipSecObjMsgReceivedEv(
                                       IN  RvSipMsgHandle       hMsg,
                                       IN  SipTransportAddress* pRecvFromAddr,
                                       IN  void*                pSecOwner,
                                       OUT RvBool*              pbApproved)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)pSecOwner;

    *pbApproved = RV_FALSE;

    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        return;
    }

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SipSecObjMsgReceivedEv(pSecObj=%p) - handle Message %p",
        pSecObj, hMsg));

    SecObjHandleMsgReceivedEv(pSecObj, hMsg, pRecvFromAddr, pbApproved);

    SecObjUnlockMsg(pSecObj);
}

/******************************************************************************
 * SipSecObjMsgSendFailureEv
 * ----------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a protected message sending fails.
 *          The function notifies the owners about this.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgPage   - Handle to the page,where the encoded message is stored
 *          hLocalAddr - Handle to the local address, if the message was being
 *                       sent over UDP
 *          hConn      - Handle to the connection, if the message was being
 *                       sent over TCP / TLS / SCTP
 *          pSecOwner  - Pointer to the Security Object, owning the local
 *                       address or connection, on which the message was
 *                       being sent.
*****************************************************************************/
void RVCALLCONV SipSecObjMsgSendFailureEv(
                                IN HPAGE                          hMsgPage,
                                IN RvSipTransportLocalAddrHandle  hLocalAddr,
                                IN RvSipTransportConnectionHandle hConn,
                                IN void*                          pSecOwner)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)pSecOwner;

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        return;
    }

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SipSecObjMsgSendFailureEv(pSecObj=%p) - handle Message Page %p, Local Address %p, Connection %p",
        pSecObj, hMsgPage, hLocalAddr, hConn));

    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
    {
        SecObjUnlockAPI(pSecObj);
        return;
    }

    SecurityCallbackSecObjStatusEv(pSecObj,
        RVSIP_SECOBJ_STATUS_MSG_SEND_FAILED, NULL);

    SecObjUnlockAPI(pSecObj);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((hConn,hLocalAddr,hMsgPage));
#endif
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SipSecObjLifetimeTimerHandler
 * ----------------------------------------------------------------------------
 * General: This is a callback function implementation called by the system on
 *          lifetime timer expiration.
 * Return Value: RV_TRUE, if timer should be rescheduled.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   ctx        - Handle to Security Object
 *          pTimerInfo - Pointer to the structure, containing timer info
 *****************************************************************************/
RvBool SipSecObjLifetimeTimerHandler(IN void *ctx,IN RvTimer *pTimerInfo)
{
    RvStatus rv;
	RvBool   bIgnore;
    SecObj*  pSecObj = (SecObj*)ctx;

    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        return rv;
    }

	bIgnore = SipCommonCoreRvTimerIgnoreExpiration(&pSecObj->pIpsecSession->timerLifetime, pTimerInfo);
	if (RV_TRUE == bIgnore)
	{
		RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SipSecObjLifetimeTimerHandler(pSecObj=%p): Old Timer expired, ignoring",
					pSecObj));
        SecObjUnlockMsg(pSecObj);
		return RV_FALSE;
	}

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
        "SipSecObjLifetimeTimerHandler(pSecObj=%p): Lifetime Timer expired",
        pSecObj));

    /* check state */
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
    {
        SecObjUnlockMsg(pSecObj);
        return RV_FALSE;
    }

    SipCommonCoreRvTimerExpired(&pSecObj->pIpsecSession->timerLifetime);

    /* Report lifetime expiration to application */
    SecurityCallbackSecObjStatusEv(
        pSecObj, RVSIP_SECOBJ_STATUS_IPSEC_LIFETIME_EXPIRED, NULL/*pInfo*/);

    SecObjUnlockMsg(pSecObj);
	RV_UNUSED_ARG(pTimerInfo);
    return RV_FALSE;
}

/******************************************************************************
 * SipSecObjSetLifetimeTimer
 * ----------------------------------------------------------------------------
 * General: Sets the lifetime timer to the given interval. If the timer was
 *          previously set, we reset the timer before setting the new interval
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj			 - The security-object
 *          lifetimeInterval - The lifetime interval in seconds
 *****************************************************************************/
RvStatus SipSecObjSetLifetimeTimer(IN RvSipSecObjHandle  hSecObj,
								   IN RvUint32           lifetimeInterval)
{
	RvStatus rv      = RV_OK;
	SecObj*  pSecObj = (SecObj*)hSecObj;

	rv = SecObjLockAPI(pSecObj);
	if (RV_OK != rv)
	{
	    return rv;
	}

	if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState)
	{
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SipSecObjSetLifetimeTimer(pSecObj=%p): Illegal action in state %s",
					pSecObj, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
		SecObjUnlockAPI(pSecObj);
		return rv;
	}

	rv = SecObjStartTimer(pSecObj, lifetimeInterval);
	if (RV_OK != rv)
	{
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SipSecObjSetLifetimeTimer(pSecObj=%p): Failed to start timer",
					pSecObj));
	}
	
	SecObjUnlockAPI(pSecObj);
	return rv;
}

/******************************************************************************
 * SipSecObjGetRemainingLifetime
 * ----------------------------------------------------------------------------
 * General: Returns the remaining time of the lifetime timer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj        - The security-object
 * Output:  pRemainingTime - The remaining time of the lifetime timer in seconds. 
 *							 UNDEFINED if the timer is not set
 *****************************************************************************/
RvStatus SipSecObjGetRemainingLifetime(IN  RvSipSecObjHandle  hSecObj,
									   OUT RvInt32*           pRemainingTime)
{
	RvStatus rv      = RV_OK;
	SecObj*  pSecObj = (SecObj*)hSecObj;

	rv = SecObjLockAPI(pSecObj);
	if (RV_OK != rv)
	{
	    return rv;
	}
	
	rv = SecObjGetRemainingTime(pSecObj, pRemainingTime);
	if (RV_OK != rv)
	{
		RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
					"SipSecObjGetRemainingLifetime(pSecObj=%p): Failed to get remaining time",
					pSecObj));
	}
	
	SecObjUnlockAPI(pSecObj);
	return rv;
}

#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * SipSecObjChangeStateEventHandler
 * ----------------------------------------------------------------------------
 * General: This function processes State Change event from the Transport
 *          Processing Queue, raised for the Security Object.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj   - The Pointer to the Security Object
 *          state  - State, represented by integer value
 *          reason - Reason, represented by integer value
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjChangeStateEventHandler(
                                                IN void*    pObj,
                                                IN RvInt32  state,
                                                IN RvInt32  reason,
												IN RvInt32  objUniqueIdentifier)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)pObj;
    RvSipSecObjState eState = (RvSipSecObjState)state;
    RvSipSecObjStateChangeReason eReason = (RvSipSecObjStateChangeReason)reason;

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        return rv;
    }

    if(objUniqueIdentifier != pSecObj->secObjUniqueIdentifier)
    {
		/* The security object was reallocated before this event
		   came out of the event queue, therefore we do not handle it */
		RvLogInfo(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
			  "SipSecObjChangeStateEventHandler - pSecObj 0x%p: different identifier. Ignore the event.",
			  pSecObj));
		SecObjUnlockMsg(pSecObj);
		return RV_OK;
	
    }
#else
	RV_UNUSED_ARG(objUniqueIdentifier);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SipSecObjChangeStateEventHandler(pSecObj=%p) - state %s reason %s",
            pSecObj, SipSecUtilConvertSecObjState2Str(eState),
            SipSecUtilConvertSecObjStateChangeRsn2Str(eReason)));

    /* Report state to application and security owner */
    SecurityCallbackSecObjStateChangeEv(pSecObj, eState, eReason);

    /* Free Security Object resources on TERMINATED event */
    /*if (RVSIP_SECOBJ_STATE_TERMINATED == eState)
    {
        SecObjFree(pSecObj);
    }*/

    /* Terminate Security Object on CLOSED state, if there is no Application
    or Security Agreement owner. Otherwise nobody will terminate the Object. */
    if (RVSIP_SECOBJ_STATE_CLOSED == eState &&
        NULL==pSecObj->pSecAgree && NULL==pSecObj->hAppSecObj &&
        0 == pSecObj->counterOwners)
    {
        RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SipSecObjChangeStateEventHandler(pSecObj=%p) - CLOSED state: there are no owners, terminate itself",
            pSecObj));

        rv = SecObjChangeState(pSecObj, RVSIP_SECOBJ_STATE_TERMINATED,
                               RVSIP_SECOBJ_REASON_UNDEFINED);
        if (RV_OK != rv)
        {
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
            SecObjUnlockMsg(pSecObj);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */
            RvLogError(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
                "SipSecObjChangeStateEventHandler(pSecObj=%p) - failed to change state to TERMINATED(rv=%d:%s)",
                pSecObj, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    SecObjUnlockMsg(pSecObj);
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */
    return RV_OK;
}


/******************************************************************************
 * SipSecObjTerminatedStateEventHandler
 * ----------------------------------------------------------------------------
 * General: This function processes State Change event from the Transport
 *          Processing Queue, raised for the Security Object.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pObj   - The Pointer to the Security Object
 *          state  - State, represented by integer value
 *          reason - Reason, represented by integer value
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjTerminatedStateEventHandler(
                                                IN void*    pObj,
                                                IN RvInt32  reason)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)pObj;
    RvSipSecObjStateChangeReason eReason = (RvSipSecObjStateChangeReason)reason;

    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        return rv;
    }

    RvLogDebug(pSecObj->pMgr->pLogSrc,(pSecObj->pMgr->pLogSrc,
            "SipSecObjTerminatedStateEventHandler(pSecObj=%p) - state %s reason %s",
            pSecObj, SipSecUtilConvertSecObjState2Str(RVSIP_SECOBJ_STATE_TERMINATED),
            SipSecUtilConvertSecObjStateChangeRsn2Str(eReason)));

    /* Report state to application and security owner */
    SecurityCallbackSecObjStateChangeEv(pSecObj, RVSIP_SECOBJ_STATE_TERMINATED, eReason);

    /* Free Security Object resources on TERMINATED event */
    SecObjFree(pSecObj);

    SecObjUnlockMsg(pSecObj);
    return RV_OK;
}

/******************************************************************************
 * SipSecObjGetSecureLocalAddressAndConnection
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
 * Input:   hSecMgr  - Handle to the Security Manager
 *          hSecObj  - Handle to the Security Object
 *          eMsgType - Request or response. For request sending client
 *                     connection should be provided, for response - server
 *                     connection.
 *          peTransportType   - Type of the Transport to be used for protected
 *                              message delivery.If UNDEFINED, forced Transport
 *                              Type, set with SetIpsecParame() API will be
 *                              used. If it is UNDEFINED also, the default -
 *                              UDP will be returned.
 *                              If chosen security mechanism is TLS,
 *                              TLS address will be returned.
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
RvStatus RVCALLCONV SipSecObjGetSecureLocalAddressAndConnection(
                    IN      RvSipSecMgrHandle               hSecMgr,
                    IN      RvSipSecObjHandle               hSecObj,
                    IN      RvSipMsgType                    eMsgType,
                    IN OUT  RvSipTransport*                 peTransportType,
                    OUT     RvSipTransportLocalAddrHandle*  phLocalAddress,
                    OUT     RvAddress*                      pRemoteAddr,
                    OUT     RvSipTransportConnectionHandle* phConn,
					OUT     RvSipTransmitterMsgCompType	   *peMsgCompType,
					INOUT   RPOOL_Ptr                      *strSigcompId,
					IN      RvBool                          bIsConsecutive)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetSecureLocalAddressAndConnection(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetSecureLocalAddressAndConnection(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjGetSecureLocalAddressAndConnection(pSecObj, eMsgType,
            peTransportType,phLocalAddress,pRemoteAddr,phConn,peMsgCompType,strSigcompId,bIsConsecutive);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetSecureLocalAddressAndConnection(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * SipSecObjGetSecureServerPort
 * ----------------------------------------------------------------------------
 * General: gets protected Server Port, which was opened by Security Object
 *          in order to get requests in case of TCP and responses in case of
 *          UDP. This port should be set into the "send-by" field of VIA header
 *          for outgoing protected requestes. See 3GPP TS 24.229 for details.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr - Handle to the Security Manager
 *          hSecObj - Handle to the Security Object
 *          eTransportType - Type of the Transport, protected port for which is
 *                           requested.
 * Output:  pSecureServerPort - protected server port to be ste in VIA header.
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjGetSecureServerPort(
                            IN   RvSipSecMgrHandle  hSecMgr,
                            IN   RvSipSecObjHandle  hSecObj,
                            IN   RvSipTransport     eTransportType,
                            OUT  RvUint16*          pSecureServerPort)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;
    
    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetSecureServerPort(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetSecureServerPort(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }
    
    rv = SecObjGetSecureServerPort(pSecObj, eTransportType, pSecureServerPort);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetSecureServerPort(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    
    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
}

/******************************************************************************
 * SipSecObjSetMechanism
 * ----------------------------------------------------------------------------
 * General: Sets security mechanism to be used by Security Object for message
 *          protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr       - Handle to the Security Manager
 *          hSecObj       - Handle to the Security Object
 *          eSecMechanism - Mechanism, e.g. TLS or IPsec
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjSetMechanism(
                        IN RvSipSecMgrHandle          hSecMgr,
                        IN RvSipSecObjHandle          hSecObj,
                        IN RvSipSecurityMechanismType eSecMechanism)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjSetMechanism(hSecObj=%p, Security Mechanism=%s)",
        hSecObj, SipCommonConvertEnumToStrSecMechanism(eSecMechanism)));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetMechanism(hSecObj=%p, Security Mechanism=%s) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, SipCommonConvertEnumToStrSecMechanism(eSecMechanism),
            rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    rv = SecObjSetMechanism(pSecObj, eSecMechanism);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetMechanism(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * SipSecObjGetMechanism
 * ----------------------------------------------------------------------------
 * General: Gets security mechanism, used by Security Object for message
 *          protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr        - Handle to the Security Manager
 *          hSecObj        - Handle to the Security Object
 * Output:  peSecMechanism - Mechanism, e.g. TLS or IPsec
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjGetMechanism(
                        IN  RvSipSecMgrHandle           hSecMgr,
                        IN  RvSipSecObjHandle           hSecObj,
                        OUT RvSipSecurityMechanismType* peSecMechanism)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetMechanism(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetMechanism(hSecObj=%p) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    *peSecMechanism = pSecObj->eSecMechanism;

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * SipSecObjSetTlsParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters required for TLS establishment.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr     - Handle to the Security Manager
 *          hSecObj     - Handle to the Security Object
 *          pTlsParams  - Parameters
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjSetTlsParams(
                        IN RvSipSecMgrHandle  hSecMgr,
                        IN RvSipSecObjHandle  hSecObj,
                        IN SipSecTlsParams*   pTlsParams)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if (NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjSetTlsParams(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetTlsParams(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjSetTlsParams(pSecObj, pTlsParams);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetTlsParams(hSecObj=%p) - failed (rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
#else
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(pTlsParams);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SipSecObjGetTlsParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters set into Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - Handle to the Security Manager
 *          hSecObj    - Handle to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjGetTlsParams(
                        IN  RvSipSecMgrHandle hSecMgr,
                        IN  RvSipSecObjHandle hSecObj,
                        OUT SipSecTlsParams*  pTlsParams)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if (NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetTlsParams(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetTlsParams(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjGetTlsParams(pSecObj, pTlsParams);
	if (RV_OK != rv)
	{
		RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"SipSecObjGetTlsParams(hSecObj=%p) - failed to set sigcomp-id", hSecObj));
	}

    SecObjUnlockAPI(pSecObj);

    return rv;
#else
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(pTlsParams);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SipSecObjSetTlsConnection
 * ----------------------------------------------------------------------------
 * General: Sets connection, to be used by the Security Object for protection
 *          by TLS.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr  - Handle to the Security Manager
 *          hSecObj  - Handle to the Security Object
 *          hConn    - Handle to the Connection Object
*****************************************************************************/
RvStatus RVCALLCONV SipSecObjSetTlsConnection(
                            IN  RvSipSecMgrHandle               hSecMgr,
                            IN  RvSipSecObjHandle               hSecObj,
                            IN  RvSipTransportConnectionHandle  hConn)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjSetTlsConnection(hSecObj=%p,hConn=%p)", hSecObj, hConn));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetTlsConnection(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    rv = SecObjSetTlsConnection(pSecObj, hConn);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetTlsConnection(hSecObj=%p) - Failed to set connection %p",
            hSecObj, hConn));
        return rv;
    }
    pSecObj->tlsSession.bAppConn = RV_TRUE;

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
#else
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(hConn);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SipSecObjGetTlsConnection
 * ----------------------------------------------------------------------------
 * General: Gets connection, used by the Security Object for protection by TLS.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - Handle to the Security Manager
 *          hSecObj    - Handle to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjGetTlsConnection(
                        IN  RvSipSecMgrHandle               hSecMgr,
                        IN  RvSipSecObjHandle               hSecObj,
                        OUT RvSipTransportConnectionHandle* phConn)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr || NULL==phConn)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetTlsConnection(hSecObj=%p)", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetTlsConnection(hSecObj=%p) - Failed to lock", hSecObj));
        return rv;
    }

    *phConn = pSecObj->tlsSession.hConn;

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
#else
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(phConn);
    return RV_ERROR_ILLEGAL_ACTION;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
}

/******************************************************************************
 * SipSecObjIpsecSetPortSLocalAddresses
 * ----------------------------------------------------------------------------
 * General: Sets Local Addresses,bound to the Port-S, into the Security Object.
 *          The Local Addresses should be reused by the Security Objects,
 *          created by the SecAgree object for IPsec renegotiation.
 *          If the Address is set before Initiate() call, port-S, set into
 *          the Security Object, will be ignored.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr       - Handle to the Security Manager
 *          hSecObj       - Handle to the Security Object
 *          hLocalAddrUdp - Handle to the UDP Local Address
 *          hLocalAddrTcp - Handle to the TCP Local Address
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjIpsecSetPortSLocalAddresses(
                        IN  RvSipSecMgrHandle               hSecMgr,
                        IN  RvSipSecObjHandle               hSecObj,
                        IN  RvSipTransportLocalAddrHandle   hLocalAddrUdp,
                        IN  RvSipTransportLocalAddrHandle   hLocalAddrTcp)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(hLocalAddrUdp);
    RV_UNUSED_ARG(hLocalAddrTcp);
    return RV_ERROR_NOTSUPPORTED;
#else
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjIpsecSetPortSLocalAddresses(hSecObj=%p,hLocalAddrUdp=%p,hLocalAddrTcp=%p)",
        hSecObj, hLocalAddrUdp, hLocalAddrTcp));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjIpsecSetPortSLocalAddresses(hSecObj=%p) - Failed to lock",
            hSecObj));
        return rv;
    }

    rv=SecObjSetIpsecLocalAddressesPortS(pSecObj,hLocalAddrUdp,hLocalAddrTcp);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjIpsecSetPortSLocalAddresses(hSecObj=%p) - Failed to set local addresses",
            hSecObj));
        return rv;
    }
    
    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) #else*/
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SipSecObjIpsecSetLocalTransport(IN  RvSipSecObjHandle               hSecObj,
                                                    IN RvSipTransport eTransportType) {

  SecObj*  pSecObj = (SecObj*)hSecObj;

  if(pSecObj) {
    pSecObj->pIpsecSession->eTransportType = eTransportType;
  }

  return RV_OK;
}
#endif
//SPIRENT_END

/******************************************************************************
 * SipSecObjIpsecGetPortSLocalAddresses
 * ----------------------------------------------------------------------------
 * General: Gets Security Object's Local Address, bound to the Port-S.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr        - Handle to the Security Manager
 *          hSecObj        - Handle to the Security Object
 * Output:  phLocalAddrUdp - Handle to the UDP Local Address
 *          phLocalAddrTcp - Handle to the TCP Local Address
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjIpsecGetPortSLocalAddresses(
                        IN  RvSipSecMgrHandle               hSecMgr,
                        IN  RvSipSecObjHandle               hSecObj,
                        OUT RvSipTransportLocalAddrHandle*  phLocalAddrUdp,
                        OUT RvSipTransportLocalAddrHandle*  phLocalAddrTcp)
{
#if !(RV_IMS_IPSEC_ENABLED==RV_YES)
    RV_UNUSED_ARG(hSecMgr);
    RV_UNUSED_ARG(hSecObj);
    RV_UNUSED_ARG(phLocalAddrUdp);
    RV_UNUSED_ARG(phLocalAddrTcp);
    return RV_ERROR_NOTSUPPORTED;
#else
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if (NULL==pSecObj        || NULL==pSecMgr ||
        NULL==phLocalAddrUdp || NULL==phLocalAddrTcp)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjIpsecGetPortSLocalAddresses(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjIpsecGetPortSLocalAddresses(hSecObj=%p) - Failed to lock",
            hSecObj));
        return rv;
    }

    if (NULL != pSecObj->pIpsecSession)
    {
        *phLocalAddrUdp = pSecObj->pIpsecSession->hLocalAddrUdpS;
        *phLocalAddrTcp = pSecObj->pIpsecSession->hLocalAddrTcpS;
    }

    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES) #else*/
}

/******************************************************************************
 * SipSecObjGetState
 * ----------------------------------------------------------------------------
 * General: Gets state of the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr - Handle to the Security Manager
 *          hSecObj - Handle to the Security Object
 * Output:  peState - State of the Security Object
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjGetState(
                        IN  RvSipSecMgrHandle    hSecMgr,
                        IN  RvSipSecObjHandle    hSecObj,
                        OUT RvSipSecObjState*    peState)
{
    RvStatus rv;
    SecObj* pSecObj = (SecObj*)hSecObj;
    SecMgr* pSecMgr = (SecMgr*)hSecMgr;
    
    if(NULL==pSecObj || NULL==pSecMgr || NULL==peState)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjGetState(hSecObj=%p)", hSecObj));
    
    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjGetState(hSecObj=%p) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    *peState = pSecObj->eState;

    SecObjUnlockAPI(pSecObj);

    return RV_OK;
}

/******************************************************************************
 * SipSecObjSetImpi
 * ----------------------------------------------------------------------------
 * General: Sets IMPI to be set by Security Object into it's Local Addresses
 *          and Connections.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr - Handle to the Security Manager
 *          hSecObj - Handle to the Security Object
 *          impi    - IMPI
 *****************************************************************************/
RvStatus RVCALLCONV SipSecObjSetImpi(
                        IN RvSipSecMgrHandle hSecMgr,
                        IN RvSipSecObjHandle hSecObj,
                        IN void*             impi)
{
    RvStatus rv;
    SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = (SecMgr*)hSecMgr;

    if(NULL==pSecObj || NULL==pSecMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
        "SipSecObjSetImpi(hSecObj=%p, impi=%p)", hSecObj, impi));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetImpi(hSecObj=%p, impi=%p) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, impi, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    rv = SecObjSetImpi(pSecObj, impi);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
            "SipSecObjSetImpi(hSecObj=%p, impi=%p) - failed to lock Security Object(rv=%d:%s)",
            hSecObj, impi, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);
    
    return RV_OK;
}

#ifdef RV_SIGCOMP_ON
/******************************************************************************
 * SipSecObjSetSigcompParams
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
RvStatus RVCALLCONV SipSecObjSetSigcompParams(
                        IN RvSipSecObjHandle			hSecObj,
						IN RvSipTransmitterMsgCompType  eMsgCompType,
                        IN RPOOL_Ptr				   *pSigcompId,
						IN RvSipSecLinkCfg             *pLinkCfg)
{
	SecObj*  pSecObj = (SecObj*)hSecObj;
    SecMgr*  pSecMgr = pSecObj->pMgr;
	RvStatus rv;

	if(NULL==pSecObj)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
			  "SipSecObjSetSigcompParams(hSecObj=%p) - setting sigcomp parameters", hSecObj));

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"SipSecObjSetSigcompParams(hSecObj=%p) - failed to lock Security Object(rv=%d:%s)",
					hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

	rv = SecObjSetSigcompParams(pSecObj, eMsgCompType, pSigcompId, pLinkCfg);
	if (RV_OK != rv)
    {
        RvLogError(pSecMgr->pLogSrc,(pSecMgr->pLogSrc,
					"SipSecObjSetSigcompParams(hSecObj=%p) - failed to set sigcomp parameters to security-object (rv=%d:%s)",
					hSecObj, rv, SipCommonStatus2Str(rv)));
    }
	
	SecObjUnlockAPI(pSecObj);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecMgr);
#endif

	return rv;
}
#endif /* #ifdef RV_SIGCOMP_ON */

/*---------------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * UpdateOwnerCounterForIncomingOwner
 * ----------------------------------------------------------------------------
 * General: increments or decrements counter of Transactions, using
 *          the Security Object for message protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pLogSrc - Pointer to the Log Source object
 *          pSecObj - Pointer to the Security Object
 *          pOwner  - Owner
 *          delta   - Delta to be added to the counter. Can be negative.
 *****************************************************************************/
static RvStatus UpdateOwnerCounterForIncomingOwner(
                                        IN RvLogSource* pLogSrc,
                                        IN SecObj*      pSecObj,
                                        IN void*        pOwner,
                                        IN RvInt32      delta)
{
    RvStatus rv;

    rv = SecObjLockMsg(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounterForIncomingOwner(pSecObj=%p) - Failed to lock",
            pSecObj));
        return rv;
    }

    /* IPsec only is interested in Transaction owners */
    if (RV_FALSE == SecObjSupportsIpsec(pSecObj))
    {
        SecObjUnlockMsg(pSecObj);
        return RV_OK;
    }

    /* Incoming owner can use ACTIVE Security Object only */
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState  &&  delta > 0)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounterForIncomingOwner(pSecObj=%p): Delta=%d - Can't add owner - not ACTIVE state (%s)",
            pSecObj, delta, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockMsg(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Incoming owner can be removed from ACTIVE or CLOSING Security Object  */
    if (RVSIP_SECOBJ_STATE_ACTIVE  != pSecObj->eState  &&
        RVSIP_SECOBJ_STATE_CLOSING != pSecObj->eState  &&  delta < 0)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounterForIncomingOwner(pSecObj=%p): Delta=%d - Can't remove owner - not ACTIVE or CLOSING state (%s)",
            pSecObj, delta, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockMsg(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = UpdateTransactionCounter(pLogSrc, pSecObj, pOwner, delta);
    if (RV_OK != rv)
    {
        SecObjUnlockMsg(pSecObj);
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounterForIncomingOwner(pSecObj=%p) - Failed to update Transaction counter",
            pSecObj));
        return rv;
    }

    rv = UpdateCounter(pLogSrc, pSecObj, pOwner, delta);
    if (RV_OK != rv)
    {
        SecObjUnlockMsg(pSecObj);
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounterForIncomingOwner(pSecObj=%p) - Failed to update counter",
            pSecObj));
        return rv;
    }

    SecObjUnlockMsg(pSecObj);
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * UpdateOwnerCounter
 * ----------------------------------------------------------------------------
 * General: increments or decrements counter of owners, using the Security
 *          Object for message protection.
 *          Take a care of owners, that are not incoming.
 *          Locks the Security Object by API lock.
 *          (For incoming owners the Security Object should be locked by MSG
 *          lock, see UpdateOwnerCounterForIncomingOwner()).
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj     - The Security Object
 *          delta       - Delta to be added to the counter. Can be negative
 *          pOwner      - Pointer to the Owner
 *          eOwnerType  - Type of owner, such as Transaction or CallLeg
 *          delta       - Delta to be added to the counter. Can be negative.
 *****************************************************************************/
static RvStatus UpdateOwnerCounter(
                                    IN RvLogSource*               pLogSrc,
                                    IN SecObj*                    pSecObj,
                                    IN void*                      pOwner,
                                    IN RvSipCommonStackObjectType eOwnerType,
                                    IN RvInt32                    delta)
{
    RvStatus rv;

    rv = SecObjLockAPI(pSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounter(pSecObj=%p) - Failed to lock(rv=%d:%s)",
            pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    /* Owner can be added to ACTIVE object only */
    if (RVSIP_SECOBJ_STATE_ACTIVE != pSecObj->eState  &&  delta > 0)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounter(pSecObj=%p): Owner %p, Delta=%d - Can't add owner - not ACTIVE state (%s)",
            pSecObj, pOwner, delta, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockAPI(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Owner can be removed from ACTIVE and closed object only */
    if (RVSIP_SECOBJ_STATE_ACTIVE  != pSecObj->eState  &&
        RVSIP_SECOBJ_STATE_CLOSING != pSecObj->eState  &&
        RVSIP_SECOBJ_STATE_CLOSED  != pSecObj->eState  &&
        delta < 0)
    {
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounter(pSecObj=%p): Owner %p, Delta=%d - Can't remove owner - not ACTIVE, CLOSING or CLOSED state (%s)",
            pSecObj, pOwner, delta, SipSecUtilConvertSecObjState2Str(pSecObj->eState)));
        SecObjUnlockAPI(pSecObj);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Transactions and Transmitters should also update TranscCounter,
      used by IPsec */
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if ((RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION == eOwnerType ||
         RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSMITTER == eOwnerType)
        &&
        RV_TRUE == SecObjSupportsIpsec(pSecObj)
       )
    {
        rv = UpdateTransactionCounter(pLogSrc, pSecObj, pOwner, delta);
        if (RV_OK != rv)
        {
            SecObjUnlockAPI(pSecObj);
            RvLogError(pLogSrc,(pLogSrc,
                "UpdateOwnerCounter(pSecObj=%p) - Failed to update Transaction counter",
                pSecObj));
            return rv;
        }
    }
#else
    RV_UNUSED_ARG(eOwnerType);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

    rv = UpdateCounter(pLogSrc, pSecObj, pOwner, delta);
    if (RV_OK != rv)
    {
        SecObjUnlockAPI(pSecObj);
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateOwnerCounter(pSecObj=%p) - Failed to update counter",
            pSecObj));
        return rv;
    }

    SecObjUnlockAPI(pSecObj);
    return RV_OK;
}

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * UpdateTransactionCounter
 * ----------------------------------------------------------------------------
 * General: increments or decrements counter of transaction/transmitter owners,
 *          when the security object is locked.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - The Security Object
 *          delta   - Delta to be added to the counter. Can be negative
 *          pOwner  - Pointer to the Owner
 *          delta   - Delta to be added to the counter. Can be negative.
 *****************************************************************************/
static RvStatus UpdateTransactionCounter(
                                    IN RvLogSource*               pLogSrc,
                                    IN SecObj*                    pSecObj,
                                    IN void*                      pOwner,
                                    IN RvInt32                    delta)
{
    RvStatus rv;

    pSecObj->pIpsecSession->counterTransc += delta;
    RvLogDebug(pLogSrc,(pLogSrc,
        "UpdateTransactionCounter(pSecObj=%p): Owner %p, Delta=%d. Updated Transaction Counter=%d",
        pSecObj, pOwner, delta, pSecObj->pIpsecSession->counterTransc));

#ifdef SIP_DEBUG
    /* Sanity check */
    if (0xfffffff < pSecObj->pIpsecSession->counterTransc)
    {
        RvLogExcep(pLogSrc,(pLogSrc,
            "UpdateTransactionCounter(pSecObj=%p): Delta=%d. NEGATIVE !!! Counter=%d",
            pSecObj, delta, pSecObj->pIpsecSession->counterTransc));
        pSecObj->pIpsecSession->counterTransc = 0;
    }
#endif

    /* If the Security Object is waiting for termination of pending objects
    (transactions and transmitters) and connections, and the last pending
    object was removed by this call, try to continue shutdown. */
    if (RVSIP_SECOBJ_STATE_CLOSING == pSecObj->eState &&
        0 == pSecObj->pIpsecSession->counterTransc)
    {
        rv = SecObjShutdownContinue(pSecObj);
        if (RV_OK != rv)
        {
            RvLogError(pLogSrc,(pLogSrc,
                "UpdateTransactionCounter(pSecObj=%p) - Failed to continue shutdown",
                pSecObj));
            return rv;
        }
    }
    return RV_OK;
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * UpdateCounter
 * ----------------------------------------------------------------------------
 * General: increments or decrements counter of regular (not transaction or 
 *          transmitter) owners, when the security object is locked.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - The Security Object
 *          delta   - Delta to be added to the counter. Can be negative
 *          pOwner  - Pointer to the Owner
 *          delta   - Delta to be added to the counter. Can be negative.
 *****************************************************************************/
static RvStatus UpdateCounter(
                              IN RvLogSource*               pLogSrc,
                              IN SecObj*                    pSecObj,
                              IN void*                      pOwner,
                              IN RvInt32                    delta)
{
    RvStatus rv;

    pSecObj->counterOwners += delta;
    RvLogDebug(pLogSrc,(pLogSrc,
        "UpdateCounter(pSecObj=%p): Owner %p, Delta=%d. Updated Counter=%d",
        pSecObj, pOwner, delta, pSecObj->counterOwners));

#ifdef SIP_DEBUG
    /* Sanity check */
    if (0xfffffff < pSecObj->counterOwners)
    {
        RvLogExcep(pLogSrc,(pLogSrc,
            "UpdateCounter(pSecObj=%p): Owner %p, Delta=%d. NEGATIVE !!! Counter=%d",
            pSecObj, pOwner, delta, pSecObj->counterOwners));
    }
#endif

    rv = SecObjRemoveIfNoOwnerIsLeft(pSecObj);
    if (RV_OK != rv)
    {
        SecObjFree(pSecObj);
        RvLogError(pLogSrc,(pLogSrc,
            "UpdateCounter(pSecObj=%p) - Failed to shutdown",
            pSecObj));
        return rv;
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((pOwner,pLogSrc));
#endif

    return RV_OK;
}

#endif /*#ifdef RV_SIP_IMS_ON*/

