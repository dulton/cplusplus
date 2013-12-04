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
 *                              <TranscClientSecAgree.c>
 * In this file the transc-client attaches to a security agreement-object and uses it 
 * to obtain and maintain a security-agreement according to RFC3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Gilad Govring                 Aug 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                          */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON
#include "TranscClientSecAgree.h"
#include "_SipTranscClient.h"
#include "TranscClientObject.h"
#include "_SipSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "RvSipSecAgreeTypes.h"
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


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TranscClientSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this Transc-client. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the Transc-client will exploit this 
 *          agreement and the data it brings. If not, the Transc-client will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient   - Pointer to the Transc-client.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeAttachSecAgree(
								IN  SipTranscClient             *pTranscClient,
								IN  RvSipSecAgreeHandle          hSecAgree)
{
	RvSipSecObjHandle    hSecObj = NULL;
	RvStatus             rv;

	if (pTranscClient->hSecAgree == hSecAgree)
	{
		return RV_OK;
	}
	
	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSecAgreeAttachSecAgree - Transc-client 0x%p - Attaching to security-agreement 0x%p",
				pTranscClient, hSecAgree));

	if (NULL != hSecAgree)
	{
		/* notify the new security-agreement on the attachment */
		rv = SipSecAgreeAttachOwner(hSecAgree, (void *)pTranscClient->hOwner, pTranscClient->eOwnerType, pTranscClient, &hSecObj);
		if (RV_OK != rv)
		{	
			RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
						"TranscClientSecAgreeAttachSecAgree - transc-client 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
						pTranscClient, hSecAgree, rv));
			return rv;
		}
	}
	
	/* Update the new security object in the transaction */
	rv = TranscClientAttachToSecObjIfNeeded(pTranscClient, hSecObj);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeAttachSecAgree - Transc-client 0x%p - Failed to attach security-agreement 0x%p, rv=%d",
				   pTranscClient, hSecAgree, rv));
		SipSecAgreeDetachOwner(hSecAgree, (void *)pTranscClient->hOwner);
		return rv;
	}
	
	/* only when we know the attachment succeeded, we detach from the old security-agreement */
	TranscClientSecAgreeDetachSecAgree(pTranscClient);
	
	/* Update the new security-agreement in the Transc-client */
	pTranscClient->hSecAgree = hSecAgree;

	return RV_OK;
}

/***************************************************************************
 * TranscClientSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this Transc-client. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient         - Pointer to the Transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeDetachSecAgree(
										IN  SipTranscClient   *pTranscClient)
{
	RvSipSecAgreeHandle       hSecAgree;
	RvStatus                  rv;

	hSecAgree = pTranscClient->hSecAgree;
	
	if (hSecAgree == NULL)
	{
		return RV_OK;
	}
	
	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSecAgreeDetachSecAgree - Transc-client 0x%p - Detaching from security-agreement 0x%p",
				pTranscClient, hSecAgree));

	pTranscClient->hSecAgree = NULL;

	TranscClientDetachFromSecObjIfNeeded(pTranscClient, hSecAgree);

	/* Notify the security-agreement object on the detachment */
	rv = SipSecAgreeDetachOwner(hSecAgree, (void *)pTranscClient->hOwner);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "TranscClientSecAgreeDetachSecAgree - Transc-client 0x%p - Failed to detach security-agreement 0x%p, rv=%d",
					pTranscClient, hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TranscClientSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the Transc-client:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient         - Pointer to the Transc-client.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgToSend(
											IN  SipTranscClient      *pTranscClient,
											IN  RvSipMsgHandle        hMsg)
{
	RvStatus               rv;
	
	if (pTranscClient->hSecAgree == NULL)
	{
		RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeMsgToSend - Transc-client 0x%p - No security-agreement to process message to send",
				   pTranscClient));
		return RV_OK;
	}

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSecAgreeMsgToSend - Transc-client 0x%p - Notifying security-agreement 0x%p on message to send",
				pTranscClient, pTranscClient->hSecAgree));
	
	rv = SipSecAgreeHandleMsgToSend(pTranscClient->hSecAgree, pTranscClient->hSecObj, hMsg);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "TranscClientSecAgreeMsgToSend - Transc-client 0x%p - security-agreement 0x%p failed to hanlde message to sent, rv=%d",
					pTranscClient, pTranscClient->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TranscClientSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient        - Pointer to the Transc-client.
 *          hMsg            - Handle to the message
 *          hTransc         - The transaction that notified destination resolution
 *                            Notice: only one of hTrx and hTransc is valid.
 ***************************************************************************/
void RVCALLCONV TranscClientSecAgreeDestResolved(
								IN  SipTranscClient              *pTranscClient,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTranscHandle             hTransc)
{
	RvSipTransmitterHandle   hTrx;
	RvStatus                 rv;

	if (pTranscClient->hSecAgree == NULL)
	{
		RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeDestResolved - Transc-client 0x%p - No security-agreement to process message dest resolved",
				   pTranscClient));
		return;
	}

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSecAgreeDestResolved - Transc-client 0x%p - Notifying security-agreement 0x%p on dest resolved",
				pTranscClient, pTranscClient->hSecAgree));

	rv = RvSipTransactionGetTransmitter(hTransc, &hTrx);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeDestResolved - Transc-client 0x%p - Failed to get transmitter from Transc=0x%p",
				   pTranscClient, hTransc));
		return;
	}

	SipSecAgreeHandleDestResolved(pTranscClient->hSecAgree, hMsg, hTrx);
}

/***************************************************************************
 * TranscClientSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the transaction-client:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient       - Pointer to the Transc-client.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgSendFailure(
											IN  SipTranscClient       *pTranscClient)
{
	RvStatus               rv;
	if (pTranscClient->hSecAgree == NULL)
	{
		RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeMsgSendFailure - Transc-client 0x%p - No security-agreement to process message send failure",
				   pTranscClient));
		return RV_OK;
	}

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				"TranscClientSecAgreeMsgSendFailure - Transc-Client 0x%p - Notifying security-agreement 0x%p on message send failure",
				pTranscClient, pTranscClient->hSecAgree));

	/* The type of the sent messages is always REQUEST */	
	rv = SipSecAgreeHandleMsgSendFailure(pTranscClient->hSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
                   "TranscClientSecAgreeMsgSendFailure - Transc-Client 0x%p - security-agreement 0x%p failed to hanlde message send failure, rv=%d",
					pTranscClient, pTranscClient->hSecAgree, rv));
	}
	
	return rv;
}

/***************************************************************************
 * TranscClientSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the Transc-client:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient    - Pointer to the Transc-client.
 *          hMsg          - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeMsgRcvd(
										IN  SipTranscClient        *pTranscClient,
										IN  RvSipMsgHandle          hMsg)
{
	RvStatus                        rv;

	if (pTranscClient->pMgr->hSecAgreeMgr == NULL)
	{
		RvLogDebug(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				   "TranscClientSecAgreeMsgRcvd - Transc-client 0x%p - No security-agreement to process message received",
				   pTranscClient));
		return RV_OK;
	}

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			"TranscClientSecAgreeMsgRcvd - Transc-client 0x%p - Notifying security-agreement layer on message received",
			pTranscClient));
	
	rv = SipSecAgreeMgrHandleMsgRcvd(pTranscClient->pMgr->hSecAgreeMgr, pTranscClient->hSecAgree, 
								     (void*)pTranscClient->hOwner, pTranscClient->eOwnerType, pTranscClient->tripleLock,
								     hMsg, NULL, NULL, NULL);
	if (RV_OK != rv)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				  "TranscClientSecAgreeMsgRcvd - Transc-client 0x%p - security-agreement 0x%p failed to handle message received, message=0x%p, rv=%d",
                  pTranscClient, pTranscClient->hSecAgree, hMsg, rv));
	}
		
	return RV_OK;
}

/***************************************************************************
 * TranscClientSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this transc-client. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the Transc-client.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj)
{
	RvStatus		rv = RV_OK;
	SipTranscClient *pTranscClient = (SipTranscClient*)hOwner;
	
	/* We lock the API lock of the trans-client. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = SipTranscClientLockAPI(pTranscClient);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	if (pTranscClient->hSecAgree == hSecAgree)
	{	
		TranscClientAttachToSecObjIfNeeded(pTranscClient, hSecObj);
	}

	/* unlock the transc-client */
	SipTranscClientUnLockAPI(pTranscClient); 

	return RV_OK;
}

/***************************************************************************
 * TranscClientSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this Transc-client. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the Transc-client.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree)
{
	RvStatus			rv = RV_OK;
	SipTranscClient		*pTranscClient = (SipTranscClient*)hOwner;
	
	/* We lock the API lock of the trans-client. This is not trivial since this is 
	   a callback, and we usually lock the Msg Lock inside of callbacks. The reason
	   we choose to lock the API lock here, is that it is very likely that the invoking
	   of this callback by the security-agreement is in fact triggered by an API call and
	   not by an event. For example, by calling RvSipSecAgreeClientSetChosenSecurity or
	   RvSipSecAgreeTerminate(). */ 
	rv = SipTranscClientLockAPI(pTranscClient);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	if (pTranscClient->hSecAgree == hSecAgree)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				  "TranscClientSecAgreeDetachSecAgreeEv - Transc-client 0x%p - detaching from security-agreement 0x%p",
				  pTranscClient, pTranscClient->hSecAgree));
		pTranscClient->hSecAgree = NULL;
		TranscClientDetachFromSecObjIfNeeded(pTranscClient, hSecAgree);
	}
	else
	{
		/* Trying to detach a security-agreement object that is not the security-agreement of this Transc-client */
		RvLogWarning(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
					"TranscClientSecAgreeDetachSecAgreeEv - Called to detach Transc-client 0x%p from security-agreement 0x%p, but the Transc-client's security-agreement is 0x%p",
					pTranscClient, hSecAgree, pTranscClient->hSecAgree));
	}

	/* unlock the Transc-client */
	SipTranscClientUnLockAPI(pTranscClient);

	return RV_OK;
}

/***************************************************************************
 * TranscClientAttachToSecObjIfNeeded
 * ------------------------------------------------------------------------
 * General: Attaches to a security object.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient      - Pointer to the Transc-Client.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
RvStatus RVCALLCONV TranscClientAttachToSecObjIfNeeded(IN  SipTranscClient    *pTranscClient,
									              IN  RvSipSecObjHandle   hSecObj)
{
	RvStatus  rv;

	if (hSecObj == NULL || pTranscClient->hSecObj == hSecObj)
	{
		return RV_OK;
	}

	RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
			  "TranscClientAttachToSecObjIfNeeded - Transc-client 0x%p - Attaching to security-object 0x%p",
			  pTranscClient, hSecObj));

    rv = TranscClientSetSecObj(pTranscClient, hSecObj);
    if (RV_OK != rv)
    {
        RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
            "TranscClientAttachToSecObjIfNeeded - Transc-client 0x%p - Failed to attach to Security Object %p(rv=%d:%s)",
            pTranscClient, hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
	return RV_OK;
}

/***************************************************************************
 * TranscClientDetachFromSecObjIfNeeded
 * ------------------------------------------------------------------------
 * General: Detaches from the security object if the security agreement 
 *          instructs to do so.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscClient      - Pointer to the Transc-Client.
 *          hSecAgree       - Handle to the security agreement.
 ***************************************************************************/
void RVCALLCONV TranscClientDetachFromSecObjIfNeeded(
										IN  SipTranscClient       *pTranscClient,
										IN  RvSipSecAgreeHandle   hSecAgree)
{
	RvBool       bDetachSecObj = RV_FALSE;
	RvStatus     rv;
	
	/* Ask the security agreement whether to detach from the security object as well */
	if (NULL != pTranscClient->hSecObj)
	{
		rv = SipSecAgreeShouldDetachSecObj(hSecAgree, pTranscClient->hSecObj, &bDetachSecObj);
		if (RV_OK != rv)
		{
			RvLogExcep(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
					   "TranscClientDetachFromSecObjIfNeeded - Transc-Client 0x%p - Failed to check security object detachment, rv=%d",
						pTranscClient, rv));
		}
	}

	if (bDetachSecObj == RV_TRUE)
	{
		RvLogInfo(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
				  "TranscClientDetachFromSecObjIfNeeded - Transc-client 0x%p - Detaching from security-object 0x%p",
				  pTranscClient, pTranscClient->hSecObj));

		/* detach security object */
		rv = SipSecObjUpdateOwnerCounter(pTranscClient->pMgr->hSecMgr, pTranscClient->hSecObj, 
										 -1, pTranscClient->eOwnerType, (void*)pTranscClient->hOwner);
		if (RV_OK != rv)
		{
			RvLogError(pTranscClient->pMgr->pLogSrc,(pTranscClient->pMgr->pLogSrc,
						"TranscClientDetachFromSecObjIfNeeded - Transc-Client 0x%p - Failed to update security object 0x%p, rv=%d",
						pTranscClient, pTranscClient->hSecObj, rv));
		}

		pTranscClient->hSecObj = NULL;
	}
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/



#endif /*RV_SIP_IMS_ON */

