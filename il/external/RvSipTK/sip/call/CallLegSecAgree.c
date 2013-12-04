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
 *                              <CallLegSecAgree.c>
 * In this file the call-leg attaches to a security agreement-object and uses it 
 * to obtain and maintain a security-agreement according to RFC3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                          */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_ON

#include "CallLegSecAgree.h"
#include "_SipSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "RvSipSecAgreeTypes.h"
#include "_SipSubs.h"
#include "_SipSecuritySecObj.h"
#include "_SipTransmitter.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void RVCALLCONV DetachFromSecObjIfNeeded(
										IN  CallLeg              *pCallLeg,
										IN  RvSipSecAgreeHandle   hSecAgree);

static RvStatus RVCALLCONV AttachToSecObjIfNeeded(IN  CallLeg              *pCallLeg,
												  IN  RvSipSecObjHandle   hSecObj);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * CallLegSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this call-leg. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the call-leg will exploit this 
 *          agreement and the data it brings. If not, the call-leg will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Pointer to the call-leg.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeAttachSecAgree(
								IN  CallLeg                     *pCallLeg,
								IN  RvSipSecAgreeHandle          hSecAgree)
{
	RvSipSecObjHandle    hSecObj = NULL;
	RvStatus             rv;

	if (pCallLeg->hSecAgree == hSecAgree)
	{
		return RV_OK;
	}
	
	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSecAgreeAttachSecAgree - Call-leg 0x%p - Attaching to security-agreement 0x%p",
				pCallLeg, hSecAgree));

	if (NULL != hSecAgree)
	{
		/* notify the new security-agreement on the attachment */
		rv = SipSecAgreeAttachOwner(hSecAgree, pCallLeg, RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG, pCallLeg, &hSecObj);
		if (RV_OK != rv)
		{	
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
						"CallLegSecAgreeAttachSecAgree - Call-leg 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
						pCallLeg, hSecAgree, rv));
			return rv;
		}
	}
	
	/* Update the new security object in the transaction */
	rv = AttachToSecObjIfNeeded(pCallLeg, hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "CallLegSecAgreeAttachSecAgree - call-leg 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
				   pCallLeg, hSecAgree, rv));
		SipSecAgreeDetachOwner(hSecAgree, pCallLeg);
		return rv;
	}
	
	/* only when we know the attachment succeeded, we detach from the old security-agreement */
	CallLegSecAgreeDetachSecAgree(pCallLeg);
	
	/* Update the new security-agreement in the call-leg */
	pCallLeg->hSecAgree = hSecAgree;

	return RV_OK;
}

/***************************************************************************
 * CallLegSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this call-leg. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeDetachSecAgree(
										IN  CallLeg            *pCallLeg)
{
	RvSipSecAgreeHandle       hSecAgree;
		RvStatus                  rv;

	hSecAgree = pCallLeg->hSecAgree;
	
	if (hSecAgree == NULL)
	{
		return RV_OK;
	}
	
	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSecAgreeDetachSecAgree - Call-leg 0x%p - Detaching from security-agreement 0x%p",
				pCallLeg, hSecAgree));

	pCallLeg->hSecAgree = NULL;
	
	DetachFromSecObjIfNeeded(pCallLeg, hSecAgree);

	/* Notify the security-agreement object on the detachment */
	rv = SipSecAgreeDetachOwner(hSecAgree, pCallLeg);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegSecAgreeDetachSecAgree - Call-leg 0x%p - Failed to detach security-agreement 0x%p, rv=%d",
					pCallLeg, hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * CallLegSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the call-leg:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgToSend(
											IN  CallLeg                     *pCallLeg,
											IN  RvSipMsgHandle               hMsg)
{
	RvStatus               rv;

	if (pCallLeg->hSecAgree == NULL)
	{
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "CallLegSecAgreeMsgToSend - Call-leg 0x%p - No security-agreement to process message to send",
				   pCallLeg));
		return RV_OK;
	}

	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSecAgreeMsgToSend - Call-leg 0x%p - Notifying security-agreement 0x%p on message to send",
				pCallLeg, pCallLeg->hSecAgree));
	
	rv = SipSecAgreeHandleMsgToSend(pCallLeg->hSecAgree, pCallLeg->hSecObj, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegSecAgreeMsgToSend - Call-leg 0x%p - security-agreement 0x%p failed to hanlde message to sent, rv=%d",
					pCallLeg, pCallLeg->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * CallLegSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - Pointer to the call-leg.
 *          hMsg            - Handle to the message
 *          hTrx            - The transmitter that notified destination resolution
 *          hTransc         - The transaction that notified destination resolution
 *                            Notice: only one of hTrx and hTransc is valid.
 ***************************************************************************/
void RVCALLCONV CallLegSecAgreeDestResolved(
								IN  CallLeg                      *pCallLeg,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTransmitterHandle        hTrx,
							    IN  RvSipTranscHandle             hTransc)
{
	RvStatus   rv;

	if (pCallLeg->hSecAgree == NULL)
	{
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "CallLegSecAgreeDestResolved - Call-leg 0x%p - No security-agreement to process dest resolved",
				   pCallLeg));
		return;
	}

	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSecAgreeDestResolved - Call-leg 0x%p - Notifying security-agreement 0x%p on dest resolved",
				pCallLeg, pCallLeg->hSecAgree));

	if (NULL == hTrx)
	{
		rv = RvSipTransactionGetTransmitter(hTransc, &hTrx);
		if (RV_OK != rv)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					   "CallLegSecAgreeDestResolved - Call-leg 0x%p - Failed to get transmitter from Transc=0x%p",
					   pCallLeg, hTransc));
			return;
		}
	}

	SipSecAgreeHandleDestResolved(pCallLeg->hSecAgree, hMsg, hTrx);
}

/***************************************************************************
 * CallLegSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the call-leg:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Pointer to the call-leg.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgSendFailure(
											IN  CallLeg                     *pCallLeg)
{
	RvStatus        rv;

	if (pCallLeg->hSecAgree == NULL)
	{
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "CallLegSecAgreeMsgSendFailure - Call-leg 0x%p - No security-agreement to process message send failure",
				   pCallLeg));
		return RV_OK;
	}
	
	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				"CallLegSecAgreeMsgSendFailure - Call-leg 0x%p - Notifying security-agreement 0x%p on message send failure",
				pCallLeg, pCallLeg->hSecAgree));
	
	rv = SipSecAgreeHandleMsgSendFailure(pCallLeg->hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegSecAgreeMsgSendFailure - Call-leg 0x%p - security-agreement 0x%p failed to handle message send failure, rv=%d",
					pCallLeg, pCallLeg->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * CallLegSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the call-leg:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg         - Pointer to the call-leg.
 *          hTransc          - The transaction that received the message.
 *          hMsg             - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeMsgRcvd(
										IN  CallLeg                     *pCallLeg,
										IN  RvSipTranscHandle            hTransc,
										IN  RvSipMsgHandle               hMsg)
{
	void*                          pOwner;
	RvSipCommonStackObjectType     eOwnerType;
	RvSipTransportConnectionHandle hRcvdOnConn = NULL;	
	RvSipTransmitterHandle         hTrx        = NULL;
	RvSipTransportLocalAddrHandle *phLocalAddr = NULL;
	RvSipTransportLocalAddrHandle  hLocalAddr  = NULL;
	RvStatus                       rv;
    RvSipCallLegState              eStateBeforeCB = pCallLeg->eState;

	if (pCallLeg->pMgr->hSecAgreeMgr == NULL)
	{
		RvLogDebug(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				   "CallLegSecAgreeMsgRcvd - Call-leg 0x%p - No security-agreement to process message received",
				   pCallLeg));
		return RV_OK;
	}

	if (SipTransactionGetSubsInfo(hTransc) != NULL)
	{
		/* This is a hidden call-leg of a subscription. Therefore we supply the
		   subscription as the owner of the security-agreement */
		pOwner     = (void*)SipSubsGetSubscriptionFromTransc(hTransc);
		eOwnerType = RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION;
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				 "CallLegSecAgreeMsgRcvd - Subs 0x%p (hidden call-leg 0x%p) - Notifying security-agreement layer on message received",
				 pOwner, pCallLeg));
	}
	else
	{
		/* The owner of the security-agreement is a call-leg */
		pOwner     = pCallLeg;
		eOwnerType = RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG;
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				  "CallLegSecAgreeMsgRcvd - Call-leg 0x%p - Notifying security-agreement layer on message received",
				  pCallLeg));
	}	

	/* Get the local address from the transmitter (required only for requests) */
	if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST &&
		RvSipMsgGetRequestMethod(hMsg) != RVSIP_METHOD_ACK)
	{
		rv = RvSipTransactionGetTransmitter(hTransc, &hTrx);
		if (RV_OK == rv)
		{
			phLocalAddr = SipTransmitterGetAddrInUse(hTrx);
			if (phLocalAddr != NULL)
			{
				hLocalAddr = *phLocalAddr;
			}
			rv = RvSipTransmitterGetConnection(hTrx, &hRcvdOnConn);
		}
		if (RV_OK != rv)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
						"CallLegSecAgreeMsgRcvd - Call-leg 0x%p - Failed to get received from details from transaction 0x%p",
						pCallLeg, hTransc));
			return rv;
		}
	}
	
	rv = SipSecAgreeMgrHandleMsgRcvd(pCallLeg->pMgr->hSecAgreeMgr, pCallLeg->hSecAgree, 
									pOwner, eOwnerType, pCallLeg->tripleLock,
									hMsg, hLocalAddr, &pCallLeg->receivedFrom, hRcvdOnConn);
	if (RV_OK != rv)
	{
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				  "CallLegSecAgreeMsgRcvd - Call-leg 0x%p - security-agreement 0x%p failed to handle message received, message=0x%p, rv=%d",
                  pCallLeg, pCallLeg->hSecAgree, hMsg, rv));
	}

    /* O.W. there are cases that the call-leg is in state terminated, and we still want to handle the message,
       such as a call-leg that already received BYE, but still has active subscriptions.
       we should check that the state is terminated, and before the callback it was something else. 
       checking only TERMINATED state is not enough */
    if(eStateBeforeCB != pCallLeg->eState &&
       pCallLeg->eState == RVSIP_CALL_LEG_STATE_TERMINATED )
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	return RV_OK;
}

/***************************************************************************
 * CallLegSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this call-leg. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the call-leg.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj)
{
	CallLeg*     pCallLeg = (CallLeg*)hOwner;
	RvStatus     rv;

	/* We lock the API lock of the call-leg. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = CallLegLockAPI(pCallLeg);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	if (pCallLeg->hSecAgree == hSecAgree)
	{	
		AttachToSecObjIfNeeded(pCallLeg, hSecObj);
	}

	/* unlock the call-leg */
	CallLegUnLockAPI(pCallLeg); 
	
	return RV_OK;
}

/***************************************************************************
 * CallLegSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this call-leg. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the call-leg.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV CallLegSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree)
{
	CallLeg*               pCallLeg = (CallLeg*)hOwner;
	RvStatus               rv;

	/* We lock the API lock of the call-leg. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = CallLegLockAPI(pCallLeg);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	if (pCallLeg->hSecAgree == hSecAgree)
	{
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				  "CallLegSecAgreeDetachSecAgreeEv - Call-leg 0x%p - detaching from security-agreement 0x%p",
				  pCallLeg, pCallLeg->hSecAgree));
		pCallLeg->hSecAgree = NULL;
		DetachFromSecObjIfNeeded(pCallLeg, hSecAgree);
	}
	else
	{
		/* Trying to detach a security-agreement object that is not the security-agreement of this call-leg */
		RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					"CallLegSecAgreeDetachSecAgreeEv - Called to detach Call-leg 0x%p from security-agreement 0x%p, but the Call-leg's security-agreement is 0x%p",
					pCallLeg, hSecAgree, pCallLeg->hSecAgree));
	}

	/* unlock the call-leg */
	CallLegUnLockAPI(pCallLeg); 
	
	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AttachToSecObjIfNeeded
 * ------------------------------------------------------------------------
 * General: Attaches to a security object.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - Pointer to the call-leg.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
static RvStatus RVCALLCONV AttachToSecObjIfNeeded(IN  CallLeg            *pCallLeg,
												  IN  RvSipSecObjHandle   hSecObj)
{
	RvStatus  rv;

	if (hSecObj == NULL || pCallLeg->hSecObj == hSecObj)
	{
		return RV_OK;
	}

	RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
			  "AttachToSecObjIfNeeded - Call-leg 0x%p - Attaching to security-object 0x%p",
			  pCallLeg, hSecObj));

    rv = CallLegSetSecObj(pCallLeg, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "AttachToSecObjIfNeeded - Call-leg 0x%p - Failed to attach to Security Object %p(rv=%d:%s)",
            pCallLeg, hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
	return RV_OK;
}

/***************************************************************************
 * DetachFromSecObjIfNeeded
 * ------------------------------------------------------------------------
 * General: Detaches from the security object if the security agreement 
 *          instructs to do so.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - Pointer to the call-leg.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
static void RVCALLCONV DetachFromSecObjIfNeeded(
										IN  CallLeg              *pCallLeg,
										IN  RvSipSecAgreeHandle   hSecAgree)
{
	RvBool        bDetachSecObj = RV_FALSE;
	RvStatus      rv;
	
	/* Ask the security agreement whether to detach from the security object as well */
	if (pCallLeg->hSecObj != NULL)
	{
		rv = SipSecAgreeShouldDetachSecObj(hSecAgree, pCallLeg->hSecObj, &bDetachSecObj);
		if (RV_OK != rv)
		{
			RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
					   "DetachFromSecObjIfNeeded - Call-leg 0x%p - Failed to check security object detachment, rv=%d",
						pCallLeg, rv));
		}
	}

	if (bDetachSecObj == RV_TRUE)
	{
		RvLogInfo(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
				  "DetachFromSecObjIfNeeded - Call-leg 0x%p - Detaching from security-object 0x%p",
				  pCallLeg, pCallLeg->hSecObj));

		/* detach security object */
		rv = SipSecObjUpdateOwnerCounter(pCallLeg->pMgr->hSecMgr, pCallLeg->hSecObj, 
										 -1, RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG, pCallLeg);
		if (RV_OK != rv)
		{
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
						"DetachFromSecObjIfNeeded - Call-leg 0x%p - Failed to update security object 0x%p, rv=%d",
						pCallLeg, pCallLeg->hSecObj, rv));
		}

		pCallLeg->hSecObj = NULL;
	}
}

#endif /*RV_SIP_IMS_ON */

