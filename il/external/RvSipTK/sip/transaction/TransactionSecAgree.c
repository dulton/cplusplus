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
 *                              <TransactionSecAgree.c>
 * In this file the transaction attaches to a security agreement-object and uses it 
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

#include "TransactionSecAgree.h"
#include "_SipSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "RvSipSecAgreeTypes.h"
#include "_SipSecuritySecObj.h"
#include "_SipTransmitter.h"
#include "_SipCommonUtils.h"

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
										IN  Transaction          *pTransaction,
										IN  RvSipSecAgreeHandle   hSecAgree);

static RvStatus RVCALLCONV AttachToSecObjIfNeeded(IN  Transaction        *pTransaction,
												  IN  RvSipSecObjHandle   hSecObj);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransactionSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this transaction. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the transaction will exploit this 
 *          agreement and the data it brings. If not, the transaction will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction   - Pointer to the transaction.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeAttachSecAgree(
								IN  Transaction                     *pTransaction,
								IN  RvSipSecAgreeHandle          hSecAgree)
{
	RvSipSecObjHandle    hSecObj = NULL;
	RvStatus             rv;

	if (pTransaction->hSecAgree == hSecAgree)
	{
		return RV_OK;
	}
	
	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				"TransactionSecAgreeAttachSecAgree - transaction 0x%p - Attaching to security-agreement 0x%p",
				pTransaction, hSecAgree));

	if (NULL != hSecAgree)
	{
		/* notify the new security-agreement on the attachment */
		rv = SipSecAgreeAttachOwner(hSecAgree, pTransaction, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION, pTransaction, &hSecObj);
		if (RV_OK != rv)
		{	
			RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
						"TransactionSecAgreeAttachSecAgree - transaction 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
						pTransaction, hSecAgree, rv));
			return rv;
		}
	}
	
	/* Update the new security object in the transaction */
	rv = AttachToSecObjIfNeeded(pTransaction, hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
						"TransactionSecAgreeAttachSecAgree - transaction 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
						pTransaction, hSecAgree, rv));
		SipSecAgreeDetachOwner(hSecAgree, pTransaction);
		return rv;
	}
	
	/* only when we know the attachment succeeded, we detach from the old security-agreement */
	TransactionSecAgreeDetachSecAgree(pTransaction);
	
	/* Update the new security-agreement in the transaction */
	pTransaction->hSecAgree = hSecAgree;
	
	return RV_OK;
}

/***************************************************************************
 * TransactionSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this transaction. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeDetachSecAgree(
										IN  Transaction            *pTransaction)
{
	RvSipSecAgreeHandle       hSecAgree;
	RvStatus                  rv;

	hSecAgree = pTransaction->hSecAgree;
	
	if (hSecAgree == NULL)
	{
		return RV_OK;
	}
	
	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				"TransactionSecAgreeDetachSecAgree - transaction 0x%p - Detaching from security-agreement 0x%p",
				pTransaction, hSecAgree));

	pTransaction->hSecAgree = NULL;
	
	DetachFromSecObjIfNeeded(pTransaction, hSecAgree);

	/* Notify the security-agreement object on the detachment */
	rv = SipSecAgreeDetachOwner(hSecAgree, pTransaction);
	if (RV_OK != rv)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
                   "TransactionSecAgreeDetachSecAgree - transaction 0x%p - Failed to detach security-agreement 0x%p, rv=%d",
					pTransaction, hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TransactionSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the transaction:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgToSend(
											IN  Transaction                     *pTransaction,
											IN  RvSipMsgHandle               hMsg)
{
	RvStatus               rv;
	
	if (pTransaction->hSecAgree == NULL)
	{
		RvLogDebug(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				   "TransactionSecAgreeMsgToSend - transaction 0x%p - No security-agreement to process message to send",
				   pTransaction));
		return RV_OK;
	}

	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				"TransactionSecAgreeMsgToSend - transaction 0x%p - Notifying security-agreement 0x%p on message to send",
				pTransaction, pTransaction->hSecAgree));
	
	rv = SipSecAgreeHandleMsgToSend(pTransaction->hSecAgree, pTransaction->hSecObj, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
                   "TransactionSecAgreeMsgToSend - transaction 0x%p - security-agreement 0x%p failed to hanlde message to sent, rv=%d",
					pTransaction, pTransaction->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TransactionSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction        - Pointer to the transaction.
 *          hMsg            - Handle to the message
 *          hTrx            - The transmitter that notified destination resolution
 *          hTransc         - The transaction that notified destination resolution
 *                            Notice: only one of hTrx and hTransc is valid.
 ***************************************************************************/
void RVCALLCONV TransactionSecAgreeDestResolved(
								IN  Transaction                  *pTransaction,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTransmitterHandle        hTrx)
{
	if (pTransaction->hSecAgree == NULL)
	{
		RvLogDebug(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				   "TransactionSecAgreeDestResolved - transaction 0x%p - No security-agreement to process message dest resolved",
				   pTransaction));
		return;
	}

	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				"TransactionSecAgreeDestResolved - transaction 0x%p - Notifying security-agreement 0x%p on dest resolved",
				pTransaction, pTransaction->hSecAgree));

	SipSecAgreeHandleDestResolved(pTransaction->hSecAgree, hMsg, hTrx);
}

/***************************************************************************
 * TransactionSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the transaction:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction - Pointer to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgSendFailure(
											IN  Transaction         *pTransaction)
{
	RvStatus           rv;

	if (pTransaction->hSecAgree == NULL)
	{
		RvLogDebug(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				   "TransactionSecAgreeMsgSendFailure - transaction 0x%p - No security-agreement to process message send failure",
				   pTransaction));
		return RV_OK;
	}

	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
			  "TransactionSecAgreeMsgSendFailure - Transaction 0x%p - Notifying security-agreement 0x%p on message send failure",
			  pTransaction, pTransaction->hSecAgree));

	rv = SipSecAgreeHandleMsgSendFailure(pTransaction->hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
                   "TransactionSecAgreeMsgSendFailure - Transaction 0x%p - security-agreement 0x%p failed to hanlde message send failure, rv=%d",
					pTransaction, pTransaction->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TransactionSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the transaction:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 *          hMsg             - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgRcvd(
										IN  Transaction                 *pTransaction,
										IN  RvSipMsgHandle               hMsg)
{
	RvSipTransportLocalAddrHandle  *phLocalAddr;
	RvSipTransportLocalAddrHandle   hLocalAddr = NULL;
	RvStatus						rv;

	if (pTransaction->pMgr->hSecAgreeMgr == NULL)
	{
		RvLogDebug(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				   "TransactionSecAgreeMsgRcvd - transaction 0x%p - No security-agreement to process message received",
				   pTransaction));
		return RV_OK;
	}

	if (pTransaction->hSecAgree == NULL && pTransaction->eOwnerType != SIP_TRANSACTION_OWNER_APP)
	{
		/* If the transaction belongs to a call-leg or a reg-client it does not request for
		   a new security-agreement */
		return RV_OK;
	}

	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
			"TransactionSecAgreeMsgRcvd - transaction 0x%p - Notifying security-agreement layer on message received",
			pTransaction));
	
	if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST)
	{
		phLocalAddr = SipTransmitterGetAddrInUse(pTransaction->hTrx);
		if (phLocalAddr != NULL)
		{
			hLocalAddr = *phLocalAddr;
		}
	}
	
	rv = SipSecAgreeMgrHandleMsgRcvd(pTransaction->pMgr->hSecAgreeMgr, pTransaction->hSecAgree, 
								     pTransaction, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION, pTransaction->tripleLock,
								     hMsg, hLocalAddr, &pTransaction->receivedFrom, pTransaction->hActiveConn);
	if (RV_OK != rv)
	{
		RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				  "TransactionSecAgreeMsgRcvd - transaction 0x%p - security-agreement 0x%p failed to handle message received, message=0x%p, rv=%d",
                  pTransaction, pTransaction->hSecAgree, hMsg, rv));
	}
	
	/* check the state of the transaction. If terminated, return error */
	if (pTransaction->eState == RVSIP_TRANSC_STATE_TERMINATED)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	return RV_OK;
}

/***************************************************************************
 * TransactionSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this transaction. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the transaction.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj)
{
	Transaction*   pTransaction = (Transaction*)hOwner;
	RvStatus       rv;

	/* We lock the API lock of the transaction. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = TransactionLockAPI(pTransaction);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	if (pTransaction->hSecAgree == hSecAgree)
	{	
		AttachToSecObjIfNeeded(pTransaction, hSecObj);
	}

	/* unlock the transaction */
	TransactionUnLockAPI(pTransaction); 
	
	return RV_OK;   
}

/***************************************************************************
 * TransactionSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this transaction. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the transaction.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree)
{
	Transaction*               pTransaction = (Transaction*)hOwner;
	RvStatus               rv;

	/* We lock the API lock of the transaction. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = TransactionLockAPI(pTransaction);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	if (pTransaction->hSecAgree == hSecAgree)
	{
		RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				  "TransactionSecAgreeDetachSecAgreeEv - transaction 0x%p - detaching from security-agreement 0x%p",
				  pTransaction, pTransaction->hSecAgree));
		pTransaction->hSecAgree = NULL;
		DetachFromSecObjIfNeeded(pTransaction, hSecAgree);
	}
	else
	{
		/* Trying to detach a security-agreement object that is not the security-agreement of this transaction */
		RvLogWarning(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
					"TransactionSecAgreeDetachSecAgreeEv - Called to detach transaction 0x%p from security-agreement 0x%p, but the transaction's security-agreement is 0x%p",
					pTransaction, hSecAgree, pTransaction->hSecAgree));
	}

	/* unlock the transaction */
	TransactionUnLockAPI(pTransaction); 
	
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
 * Input:   pTransaction    - Pointer to the transaction.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
static RvStatus RVCALLCONV AttachToSecObjIfNeeded(IN  Transaction        *pTransaction,
									              IN  RvSipSecObjHandle   hSecObj)
{
	RvStatus  rv;

	if (hSecObj == NULL || pTransaction->hSecObj == hSecObj)
	{
		return RV_OK;
	}

	RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
			  "AttachToSecObjIfNeeded - Transaction 0x%p - Attaching to security-object 0x%p",
			  pTransaction, hSecObj));

    rv = TransactionSetSecObj(pTransaction, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
            "AttachToSecObjIfNeeded - transaction 0x%p - Failed to attach to Security Object %p(rv=%d:%s)",
            pTransaction, hSecObj, rv, SipCommonStatus2Str(rv)));
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
 * Input:   pTransaction    - Pointer to the transaction.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
static void RVCALLCONV DetachFromSecObjIfNeeded(
										IN  Transaction          *pTransaction,
										IN  RvSipSecAgreeHandle   hSecAgree)
{
	RvBool      bDetachSecObj = RV_FALSE;
	RvStatus    rv;
	
	/* Ask the security agreement whether to detach from the security object as well */
	if (NULL != pTransaction->hSecObj)
	{
		rv = SipSecAgreeShouldDetachSecObj(hSecAgree, pTransaction->hSecObj, &bDetachSecObj);
		if (RV_OK != rv)
		{
			RvLogExcep(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
					   "DetachFromSecObjIfNeeded - transaction 0x%p - Failed to check security object detachment, rv=%d",
						pTransaction, rv));
		}
	}
	
	if (bDetachSecObj == RV_TRUE)
	{
		RvLogInfo(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
				  "DetachFromSecObjIfNeeded - Transaction 0x%p - Detaching from security-object 0x%p",
				  pTransaction, pTransaction->hSecObj));

		/* detach security object */
		rv = SipSecObjUpdateOwnerCounter(pTransaction->pMgr->hSecMgr, pTransaction->hSecObj, 
										 -1, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION, pTransaction);
		if (RV_OK != rv)
		{
			RvLogError(pTransaction->pMgr->pLogSrc,(pTransaction->pMgr->pLogSrc,
						"DetachFromSecObjIfNeeded - transaction 0x%p - Failed to update security object 0x%p, rv=%d",
						pTransaction, pTransaction->hSecObj, rv));
		}

		pTransaction->hSecObj = NULL;
	}
}

#endif /*RV_SIP_IMS_ON */

