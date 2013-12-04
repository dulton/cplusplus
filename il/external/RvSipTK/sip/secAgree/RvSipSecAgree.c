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
 *                              <RvSipSecAgree.c>
 *
 * This file defines the security-agreement object, attributes and interface functions.
 * The security-agreement object is used to negotiate a security agreement with the
 * remote party, and to maintain this agreement in further requests, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgree.h"
#include "_SipSecAgreeMgr.h"
#include "SecAgreeTypes.h"
#include "SecAgreeState.h"
#include "SecAgreeObject.h"
#include "SecAgreeClient.h"
#include "SecAgreeServer.h"
#include "SecAgreeMgrObject.h"
#include "SecAgreeState.h"
#include "SecAgreeUtils.h"
#include "SecAgreeSecurityData.h"
#include "_SipCallLeg.h"
#include "RvSipCallLeg.h"
#include "RvSipSecurityHeader.h"
#include "_SipTransport.h"
#include "_SipSecuritySecObj.h"
#include "RvSipTransmitter.h"
#include "_SipTransmitter.h"
#include "_SipRegClient.h"
#include "_SipPubClient.h"
#include "RvSipCSeqHeader.h"
#include "_SipCSeqHeader.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvSipSecurityHeaderType RVCALLCONV SecAgreeGetLocalSecurityType(
                                      IN   SecAgree*         pSecAgree);

/*-----------------------------------------------------------------------*/
/*                         MODULE FUNCTIONS                              */
/*-----------------------------------------------------------------------*/

/*------------------ M A N A G E R S  F U N C T I O N S -----------------*/

/***************************************************************************
 * RvSipSecAgreeMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own applicative Security-Agreement 
 *			Manager handle. Use this function to store the application handle
 *			in the Stack Security-AgreementMgr.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr    - Handle to the stack security-agreement manager.
 *        hAppSecAgreeMgr - The application security-agreement manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetAppMgrHandle(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  void*                         hAppSecAgreeMgr)
{
	SecAgreeMgr    *pSecAgreeMgr  = (SecAgreeMgr*)hSecAgreeMgr;

    if(hSecAgreeMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeMgrSetAppMgrHandle - Setting application handle 0x%p to security-agreement mgr",
			  hAppSecAgreeMgr));

    pSecAgreeMgr->hAppSecAgreeMgr = hAppSecAgreeMgr;

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application security-agreement manger object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr     - Handle to the stack security-agreement manager.
 *        phAppSecAgreeMgr - The application security-agreement manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetAppMgrHandle(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  void**                        phAppSecAgreeMgr)
{
	SecAgreeMgr    *pSecAgreeMgr  = (SecAgreeMgr*)hSecAgreeMgr;

    if(hSecAgreeMgr == NULL || phAppSecAgreeMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    *phAppSecAgreeMgr = pSecAgreeMgr->hAppSecAgreeMgr;

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this 
 *          security-agrement manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr    - Handle to the stack security-agreement manager.
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                              handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetStackInstance(
                                   IN   RvSipSecAgreeMgrHandle    hSecAgreeMgr,
								   OUT  void					**phStackInstance)
{
	SecAgreeMgr    *pSecAgreeMgr  = (SecAgreeMgr*)hSecAgreeMgr;

    if(hSecAgreeMgr == NULL || phStackInstance == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    *phStackInstance = pSecAgreeMgr->hSipStack;

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all security-agreement manager events.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr - Handle to the security-agreement manager.
 *        pEvHandlers  - Pointer to structure containing application event handler
 *                       pointers.
 *        structSize   - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  RvSipSecAgreeMgrEvHandlers   *pMgrEvHandlers,
                             IN  RvInt32                       structSize)
{
	SecAgreeMgr    *pSecAgreeMgr  = (SecAgreeMgr*)hSecAgreeMgr;
    RvInt32         minStructSize = RvMin(structSize,((RvInt32)sizeof(RvSipSecAgreeMgrEvHandlers)));

    if(pSecAgreeMgr == NULL || pMgrEvHandlers == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (structSize <= 0)
	{
		return RV_ERROR_BADPARAM;
	}

	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers - Setting manager event handlers to security-agreement mgr"));

    memset(&(pSecAgreeMgr->appMgrEvHandlers),0,sizeof(RvSipSecAgreeMgrEvHandlers));
    memcpy(&(pSecAgreeMgr->appMgrEvHandlers),pMgrEvHandlers,minStructSize);

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrSetSecAgreeEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all security-agreement object events.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr - Handle to the security-agreement manager.
 *        pEvHandlers  - Pointer to structure containing application event handler
 *                       pointers.
 *        structSize   - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetSecAgreeEvHandlers(
							 IN  RvSipSecAgreeMgrHandle     hSecAgreeMgr,
                             IN  RvSipSecAgreeEvHandlers   *pEvHandlers,
                             IN  RvInt32                    structSize)
{
	SecAgreeMgr    *pSecAgreeMgr  = (SecAgreeMgr*)hSecAgreeMgr;
    RvInt32         minStructSize = RvMin(structSize,((RvInt32)sizeof(RvSipSecAgreeEvHandlers)));

    if(pSecAgreeMgr == NULL || pEvHandlers == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(structSize <= 0)
    {
        return RV_ERROR_NULLPTR;
    }

	/* lock mutex */
	RvMutexLock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeMgrSetSecAgreeEvHandlers - Setting security-agreement event handlers to security-agreement mgr"));

    memset(&(pSecAgreeMgr->appEvHandlers),0,sizeof(RvSipSecAgreeEvHandlers));
    memcpy(&(pSecAgreeMgr->appEvHandlers),pEvHandlers,minStructSize);

	/* unlock mutex */
	RvMutexUnlock(&pSecAgreeMgr->hMutex,pSecAgreeMgr->pLogMgr);

    return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrCreateSecAgree
 * ------------------------------------------------------------------------
 * General: Creates a new security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr  - Handle to the Security-Mgr
 *         hAppSecAgree  - The application handle for the new security-agreement
 * Output: phSecAgree    - The sip stack's newly created security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrCreateSecAgree(
							 IN   RvSipSecAgreeMgrHandle        hSecAgreeMgr,
							 IN   RvSipAppSecAgreeHandle        hAppSecAgree,
							 OUT  RvSipSecAgreeHandle          *phSecAgree)
{
	SecAgreeMgr    *pSecAgreeMgr = (SecAgreeMgr*)hSecAgreeMgr;
	RvStatus        rv;

	if (hSecAgreeMgr == NULL || phSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* Locking is done within SecAgreeMgrCreateSecAgree() */

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeMgrCreateSecAgree - Creating new security-agreement"));

	rv = SecAgreeMgrCreateSecAgree(pSecAgreeMgr, hAppSecAgree, phSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeMgrCreateSecAgree - Failed to create new security-agreement. rv=%d", rv));
		return rv;
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeMgrGetMsgRcvdInfo
 * ------------------------------------------------------------------------
 * General: Obtains arrival information of a received message: the local address
 *          it was received on, the remote address it was received from, etc.
 *          This information is required for manual processing of an incoming message
 *			via RvSipSecAgreeHandleMsgRcvd().
 *          Notice: You can use this function from within the RvSipXXXMsgReceivedEv event
 *          of a security-agreement owner (call-leg, transaction, etc); or from within the
 *          RvSipXXXStateChangedEv when the new state indicates that a message was received.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr      - Handle to the security-agreement Mgr
 *         ownerStructSize   - The size of RvSipSecAgreeOwner
 *         pOwner            - The Sip object that received the message (Call-leg,
 *                             trnsaction, register-client, etc). If this a forked
 *                             call-leg, the local address and connection parameters
 *                             will return empty.
 *         hMsg              - The received message
 *         msgInfoStructSize - The size of RvSipSecAgreeMsgRcvdInfo
 * Output: pMsgInfo          - The message information structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetMsgRcvdInfo(
							 IN     RvSipSecAgreeMgrHandle         hSecAgreeMgr,
                             IN     RvInt32                        ownerStructSize,
							 IN     RvSipSecAgreeOwner            *pOwner,
							 IN     RvSipMsgHandle                 hMsg,
                             IN     RvInt32                        msgInfoStructSize,
							 INOUT  RvSipSecAgreeMsgRcvdInfo      *pMsgInfo)
{
	SecAgreeMgr					  *pSecAgreeMgr   = (SecAgreeMgr*)hSecAgreeMgr;
	RvSipTranscHandle			   hTransc        = NULL;
	RvSipTransmitterHandle         hTrx           = NULL;
	RvBool                         bRequireTrx    = RV_TRUE;
	RvSipTransportLocalAddrHandle *phLocalAddr;
	RvSipSecAgreeMsgRcvdInfo       msgInfo;
	RvInt32                        minMsgInfoSize = RvMin(msgInfoStructSize,((RvInt32)sizeof(RvSipSecAgreeMsgRcvdInfo)));
	RvSipCallLegInviteHandle       hCallLegInvite;
	RvSipCSeqHeaderHandle          hCSeqHeader;
	RvInt32                        cseq           = UNDEFINED;
	RvStatus					   rv             = RV_OK;

    memset(&msgInfo, 0, sizeof(RvSipSecAgreeMsgRcvdInfo));
	
	if(pSecAgreeMgr == NULL || pOwner == NULL || hMsg == NULL || pMsgInfo == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (msgInfoStructSize <= (RvInt32)0 || ownerStructSize < (RvInt32)sizeof(RvSipSecAgreeOwner)) 
	{
		/* We require the owner details to be complete. We return only as much fields of the
		   message info as the application requeted */
		return RV_ERROR_BADPARAM;
	}

	msgInfo.owner.eOwnerType = pOwner->eOwnerType;
	msgInfo.owner.pOwner     = pOwner->pOwner;

	switch (pOwner->eOwnerType) 
	{
	case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG:
		{
			/* First we check whether there is a transaction to the call-leg. If there is a transaction,
			   we will retrieve the received address info from the transaction */
			hTransc = SipCallLegGetMsgRcvdTransaction((RvSipCallLegHandle)pOwner->pOwner);

			if (hTransc == NULL)
			{
				/* There is no transaction for this message, we need to retrieve the received address info from the call-leg */
				RvSipCallLegHandle  hCallLeg = (RvSipCallLegHandle)pOwner->pOwner;
				RvSipCallLegHandle  hOrgCallLeg;
				
				rv = RvSipCallLegGetOriginalCallLeg(hCallLeg, &hOrgCallLeg);
				if (RV_OK != rv)
				{
					break;
				}

				if (hOrgCallLeg != NULL)
				{
					/* This is a forked call-leg, which is not relevant for security-agreement.
					   We will allow some of the local parameters to be returned empty */
					bRequireTrx = RV_FALSE;
				}

				hCSeqHeader = RvSipMsgGetCSeqHeader(hMsg);
				if (hCSeqHeader == NULL)
				{
					rv = RV_ERROR_UNKNOWN;
					break;
				}
				rv = SipCSeqHeaderGetInitializedCSeq(hCSeqHeader, &cseq);
				if (RV_OK != rv)
				{
					break;
				}

				/* Get the received address that was stored within the call-leg */
				rv = RvSipCallLegGetReceivedFromAddress(hCallLeg, &msgInfo.rcvdFromAddr);
				if (RV_OK != rv)
				{
					break;
				}
				
				/* Get the Invite object from the call-leg based on the CSeq of the ACK message */
				rv = SipCallLegInviteFindObjByCseq(hCallLeg, cseq, RVSIP_CALL_LEG_DIRECTION_INCOMING, &hCallLegInvite);
				if (hCallLegInvite != NULL && RV_OK == rv)
				{
					/* Get transmitter from call-leg */
					rv = SipCallLegInviteGetTrx(hCallLeg, hCallLegInvite, &hTrx);
				}
			}
			break;
		}
	case RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT:
		{
			/* Get the active transaction of the reg-client object */
			rv = SipRegClientGetActiveTransc((RvSipRegClientHandle)pOwner->pOwner, &hTransc);

			break;
		}
	case RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT:
        {
			/* Get the active transaction of the pub-client */
			rv = SipPubClientGetActiveTransc((RvSipPubClientHandle)pOwner->pOwner, &hTransc);
			
			break;
        }
	case RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION:
		{
			/* This is the transaction to use */
			hTransc = (RvSipTranscHandle)pOwner->pOwner;
			
			break;
		}
	default:
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					    "RvSipSecAgreeMgrGetMsgRcvdInfo - Invalid owner type %d", pOwner->eOwnerType));
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeMgrGetMsgRcvdInfo - Failed to get message received info from owner 0x%p.", 
					pOwner->pOwner));
		return rv;
	}

	if (hTransc != NULL)
	{
		/* A transaction was found. Retrieve the transmitter and the received from address from the transaction */
		rv = RvSipTransactionGetTransmitter(hTransc, &hTrx);
		if (RV_OK == rv)
		{
			rv = RvSipTransactionGetReceivedFromAddress(hTransc, &msgInfo.rcvdFromAddr, NULL);
		}
	}

	if ((hTrx == NULL && bRequireTrx == RV_TRUE)|| RV_OK != rv)
	{
		/* At this point we shall already have a transmitter, otherwise the function will fail to perform */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeMgrGetMsgRcvdInfo - Failed to get message received info from owner 0x%p.", 
					pOwner->pOwner));
		return RV_ERROR_UNKNOWN;
	}

	if (hTrx == NULL && bRequireTrx == RV_FALSE)
	{
		/* This function was called for a forked call-leg. Local address and connection will return empty */
		RvLogDebug(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeMgrGetMsgRcvdInfo - Called for forked call-leg 0x%p. Local address and connection return empty", 
				   pOwner->pOwner));
		return RV_OK;
	}

	/* The local address is obtained from the transmitter */
	phLocalAddr = SipTransmitterGetAddrInUse(hTrx);
	if (phLocalAddr != NULL)
	{
		msgInfo.hLocalAddr = *phLocalAddr;
	}
	/* The connection is retrieved from the transmitter */
	rv = RvSipTransmitterGetConnection(hTrx, &msgInfo.hRcvdFromConn);
	if (RV_OK == rv)
	{
		memcpy(pMsgInfo, &msgInfo, minMsgInfoSize);
	}
	else
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					    "RvSipSecAgreeMgrGetMsgRcvdInfo - Failed to get message information, rv=%d", rv));
	}

	return rv;
}

/*----------- S E C U R I T Y - A G R E E M E N T  C O N T R O L ----------*/

/***************************************************************************
 * RvSipSecAgreeInit
 * ------------------------------------------------------------------------
 * General: Indicates that a security-agreement is now initialized with all 
 *			the required parameters and can be used to process outgoing messages 
 *			sent by its owner. When a client whishs to start a security 
 *			negotiation process with the server, it must first create and attach 
 *			a security-agreement to a chosen SIP Stack object. The client then has
 *			to set all the required negotiation parameters to the securityagreement.
 *			After completing the initiation process, the client must call the
 *			RvSipSecAgreeInit() API function to indicate that the security-agreement 
 *			is ready for use. The following request sent by the SIP Stack object 
 *			(the owner) will contain all the required negotiation information.
 *			When a server receives a request with negotiation information and 
 *			creates a new server security-agreement, the server application must 
 *			initialize all required negotiation information. The application must
 *			call the RvSipSecAgreeInit() on the server security-agreement before 
 *			responding to the negotiation request.
 *			Note: If you send a message via the SIP owner before calling
 *			RvSipSecAgreeInit(), the message will not be processed by the 
 *			securityagreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeInit(
							 IN   RvSipSecAgreeHandle    hSecAgree)
{
	SecAgree   *pSecAgree = (SecAgree*)hSecAgree;
	RvBool		bIsEmpty;
	RvBool		bIsLegal;
	RvStatus    rv;

	if (pSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
    rv = SecAgreeLockAPI(pSecAgree);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* Check that the state of the security-agreement allows calling Init() */
	if (RV_FALSE == SecAgreeStateCanModifyLocalSecurityList(pSecAgree))
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeInit - Trying to Init security-agreement 0x%p in forbidden state = %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeInit - Security-agreement 0x%p being initialized",
			  pSecAgree));

	/* Make sure that the local list of security mechanisms is not empty */
	rv = SecAgreeUtilsIsListEmpty(pSecAgree->securityData.hLocalSecurityList, &bIsEmpty);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeInit - Security-agreement=0x%p failed to check local security list, rv=%d",
				   pSecAgree, rv));
        SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	/* We do not allow to init if the local security list is empty */
	if (RV_TRUE == bIsEmpty)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeInit - Security-agreement=0x%p: cannot initialize because local security list is empty",
				   pSecAgree));
        SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Make sure that the local list of security mechanisms is legal (the ipsec header has algorithm parameters */
	rv = SecAgreeUtilsIsListLegal(pSecAgree->securityData.hLocalSecurityList, &bIsLegal);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeInit - Security-agreement=0x%p failed to check local security list, rv=%d",
				   pSecAgree, rv));
        SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	/* We do not allow to init if the local security list is empty */
	if (RV_FALSE == bIsLegal)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeInit - Security-agreement=0x%p: Failed to initialize sec-agree due no illegal local security header (missing or unsupported parameters)",
				   pSecAgree));
        SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Init the security-agreement according to its role */
	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
	{
		rv = SecAgreeClientInit(pSecAgree);
	}
	else /* server */
	{
		rv = SecAgreeServerInit(pSecAgree);
	}

	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeInit - Failed to Init security-agreement 0x%p, rv=%d",
					pSecAgree, rv));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeCopy
 * ------------------------------------------------------------------------
 * General: Copies local information from source security-agreement to
 *          destination security-agreement. The fields being copied here are:
 *          1. Local security list.
 *          2. Auth header for server security-agreement only.
 *          Notice: You can call this function only in states IDLE,
 *          CLIENT_REMOTE_LIST_READY and SERVER_REMOTE_LIST_READY
 *          Notice: Not all fields are being copied here. For example, the
 *          remote security list is not copied.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hDestinationSecAgree   - Handle to the destination security-agreement
 *         hSourceSecAgree        - Handle to the source security-agreement
 ***************************************************************************/

RVAPI RvStatus RVCALLCONV RvSipSecAgreeCopy(
							 IN   RvSipSecAgreeHandle    hDestinationSecAgree,
							 IN   RvSipSecAgreeHandle    hSourceSecAgree)
{
	SecAgree   *pDestSecAgree = (SecAgree*)hDestinationSecAgree;
	SecAgree   *pSourceSecAgree = (SecAgree*)hSourceSecAgree;
	RvStatus    rv;

	if (pDestSecAgree == NULL || pSourceSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the destination security-agreement */
    rv = SecAgreeLockAPI(pDestSecAgree);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* Check that the state of the destination security-agreement allows copying */
	if (pDestSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_UNDEFINED &&
		RV_FALSE == SecAgreeStateCanModifyLocalSecurityList(pDestSecAgree))
	{
		RvLogError(pDestSecAgree->pSecAgreeMgr->pLogSrc,(pDestSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeCopy - Security-agreement 0x%p: copying security-agreement 0x%p is forbidden in state = %s",
				 	pDestSecAgree, pSourceSecAgree, SecAgreeUtilsStateEnum2String(pDestSecAgree->eState)));
		SecAgreeUnLockAPI(pDestSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	RvLogInfo(pDestSecAgree->pSecAgreeMgr->pLogSrc,(pDestSecAgree->pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeCopy - Copying source security-agreement 0x%p onto dest security-agreement 0x%p",
			  pSourceSecAgree, pDestSecAgree));

	/* lock the source security-agreement */
    rv = SecAgreeLockAPI(pSourceSecAgree);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	/* Copy security-agreement */
	rv = SecAgreeCopy(pDestSecAgree, pSourceSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pDestSecAgree->pSecAgreeMgr->pLogSrc,(pDestSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeCopy - Failed to copy source security-agreement 0x%p to destination 0x%p, rv=%d",
					pDestSecAgree, pSourceSecAgree, rv));
	}

	/* unlock the source security-agreement */
	SecAgreeUnLockAPI(pSourceSecAgree);
	/* unlock the destination security-agreement */
	SecAgreeUnLockAPI(pDestSecAgree);

	return rv;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetPortC(
                                                IN   RvSipSecAgreeHandle    hSecAgree,
                                                IN   int portC) {
  
  SecAgree   *pSecAgree = (SecAgree*)hSecAgree;
  RvStatus    rv=0;
  
  if (pSecAgree == NULL)
  {
    return RV_ERROR_INVALID_HANDLE;
  }
  
  /* lock the security-agreement */
  rv = SecAgreeLockAPI(pSecAgree);
  if(rv != RV_OK)
  {
    return RV_ERROR_INVALID_HANDLE;
  }
  
  RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
    "%s - setting portC %d onto security-agreement 0x%p",
    __FUNCTION__,portC, pSecAgree));
  
  /* Copy security-agreement */
  rv = SecAgreeSetPortC(pSecAgree, portC);
  if (RV_OK != rv)
  {
    RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
      "%s - Failed to set portC %d to secagree 0x%p, rv=%d",
      __FUNCTION__,portC,pSecAgree, rv));
  }
  
  /* unlock the security-agreement */
  SecAgreeUnLockAPI(pSecAgree);
  
  return rv;
}

#endif
/* SPIRENT_END */

/***************************************************************************
 * RvSipSecAgreeHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Supplies a received message to the security-agreement object. The
 *          security-agreement may use this message to load security information,
 *          verify the security-agreement validity and modify its inner state.
 *          In order to process requests, additional information on the aarival of the
 *          request is required. Use RvSipSecAgreeMgrGetMsgRcvdInfo() to obtain
 *          this information from the MsgRcvdEv of the Sip object receiving the message.
 *          Notice: Calling RvSipSecAgreeHandleMsgRcvd() may invoke the
 *          RvSipSecAgreeMgrSecAgreeRequiredEv() callback, if the current
 *          security-agreement becomes invalid and the message requires a new
 *          security-agreement to be used.
 *          Notice: You can call RvSipSecAgreeHandleMsgRcvd() only if the given
 *          message is not processed by this security-agreement via the sip stack,
 *          i.e., do not call this function if this security-agreement is attached
 *          to the sip object (call-leg, register-client, etc) that received the
 *          message.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         hMsg            - Handle to the received message
 *         pMsgInfo        - The information on the message arrival. Required
 *                           for requests only.
 *         structSize      - The size of RvSipSecAgreeMsgRcvdInfo  
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeHandleMsgRcvd(
							 IN  RvSipSecAgreeHandle		     hSecAgree,
							 IN  RvSipMsgHandle                  hMsg,
							 IN  RvSipSecAgreeMsgRcvdInfo*       pMsgInfo,
							 IN  RvInt32                         structSize)
{
	SipTransportAddress   rcvdFromAddr;
	RvStatus              rv;

	if (hSecAgree == NULL || hMsg == NULL || pMsgInfo == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize < (RvInt32)sizeof(RvSipSecAgreeMsgRcvdInfo))
	{
		/* We require all fields of the message received info structure */
		return RV_ERROR_BADPARAM;
	}

	if (RVSIP_MSG_REQUEST == RvSipMsgGetMsgType(hMsg) && 
		(NULL == pMsgInfo->hLocalAddr || '\0' == pMsgInfo->rcvdFromAddr.strIP[0] ||
		 (NULL == pMsgInfo->hRcvdFromConn && pMsgInfo->rcvdFromAddr.eTransportType == RVSIP_TRANSPORT_TLS)))
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	if ('\0' != pMsgInfo->rcvdFromAddr.strIP[0])
	{
		/* Convert received from address type */
		rv = SipTransportConvertApiTransportAddrToCoreAddress(
                ((SecAgreeMgr*)hSecAgree)->hTransport, &pMsgInfo->rcvdFromAddr,
                &rcvdFromAddr.addr);
		if (RV_OK != rv)
		{
			return rv;
		}
		rcvdFromAddr.eTransport = pMsgInfo->rcvdFromAddr.eTransportType;
	}

	/* use internal API to handle the received message */
	return SipSecAgreeMgrHandleMsgRcvd(NULL, hSecAgree, pMsgInfo->owner.pOwner,
									   pMsgInfo->owner.eOwnerType, NULL,
									   hMsg, pMsgInfo->hLocalAddr, &rcvdFromAddr, pMsgInfo->hRcvdFromConn);
}

/***************************************************************************
 * RvSipSecAgreeTerminate
 * ------------------------------------------------------------------------
 * General: Terminates the security agreement. All owners of this security-agreement
 *          will be notified not to use this agreement any more. All resources will be
 *          deallocated.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to terminate
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeTerminate(
							 IN   RvSipSecAgreeHandle    hSecAgree)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	SecAgreeMgr *pSecAgreeMgr;
	RvStatus     rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* If the security-agreement was already terminated, return */
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_TERMINATED)
	{
		/* unlock the security-agreement */
		SecAgreeUnLockAPI(pSecAgree);
		return RV_OK;
	}

	pSecAgreeMgr = pSecAgree->pSecAgreeMgr;

	RvLogInfo(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeTerminate - Terminating security-agreement 0x%p",
			  pSecAgree));

	/* Terminate the security-agreement */
	rv = SecAgreeTerminate(pSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeTerminate - Failed to terminate security-agreement 0x%p. rv=%d",
					pSecAgree, rv));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/*---------------------- O W N E R  F U N C T I O N S -----------------------*/

/***************************************************************************
 * RvSipSecAgreeSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement application handle. Usually the application
 *          replaces handles with the stack in the RvSipSecAgreeMgrCreateSecAgree()
 *          API function. This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement
 *            hAppSecAgree - A new application handle to the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetAppHandle(
                                      IN  RvSipSecAgreeHandle     hSecAgree,
                                      IN  RvSipAppSecAgreeHandle  hAppSecAgree)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Set application handle */
	pSecAgree->hAppOwner = hAppSecAgree;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this security-agreement.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree      - Handle to the security-agreement
 * Output:    phAppSeccAgree - The application handle of the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetAppHandle(
                                      IN    RvSipSecAgreeHandle     hSecAgree,
                                      OUT   RvSipAppSecAgreeHandle *phAppSecAgree)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || phAppSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get application handle */
	*phAppSecAgree = pSecAgree->hAppOwner;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetNumOfOwners
 * ------------------------------------------------------------------------
 * General: Gets the current number of owners of this security-agreement object.
 *          Call this function in order to allocate a sufficient array for
 *          RvSipSecAgreeGetOwners().
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree         - Handle to the security-agreement object
 * Output:  pNumOfOwners      - The current number of owners of this security-
 *                              agreement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetNumOfOwners(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvInt32                 *pNumOfOwners)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || pNumOfOwners == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get number of owners */
	*pNumOfOwners = pSecAgree->numOfOwners;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetOwnersList
 * ------------------------------------------------------------------------
 * General: Gets the current owners of this security-agreement object. The owners 
 *          are inserted into an array you allocate prior to calling this function.
 *          To determine the size of array to allocate use RvSipSecAgreeGetNumOfOwners().
 *          If the number of actual owners is different than array size, 
 *          actualNumOfOwners will indicate the actual number of owners set to 
 *          the array. 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree          - Handle to the security-agreement object
 *          arraySize          - The size of the given allocated array
 *          structSize         - The size of RvSipSecAgreeGetOwnersList  
 * Inout:   pOwners            - A previously allocated array of owners. When function
 *                               returns, it will contain the list of current owners
 *                               of this security-agreement: handles and types.
 * Output:  pActualNumOfOwners - The actual number of owners that can be found
 *                               in the returned array, or that should be allocated
 *                               if function fails.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetOwnersList(
							 IN    RvSipSecAgreeHandle      hSecAgree,
							 IN    RvInt32                  arraySize,
							 IN    RvInt32                  structSize,
							 INOUT RvSipSecAgreeOwner      *pOwners,
							 OUT   RvInt32                 *pActualNumOfOwners)
{
	SecAgree                  *pSecAgree = (SecAgree*)hSecAgree;
	RLIST_ITEM_HANDLE          hListElem;
	SecAgreeOwnerRecord       *pOwnerRec;
	RvInt                      index = 0;
	RvInt                      safeCounter = 0;
	RvInt32                    minStructSize = RvMin(structSize,((RvInt32)sizeof(RvSipSecAgreeOwner)));
	RvStatus                   rv;

	if (hSecAgree == NULL || pOwners == NULL || pActualNumOfOwners == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize <= 0)
	{
		return RV_ERROR_BADPARAM;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get actual number of owners */
	*pActualNumOfOwners = pSecAgree->numOfOwners;

	/* Return if there are no owners */
	if (pSecAgree->numOfOwners == 0)
	{
		SecAgreeUnLockAPI(pSecAgree);
		return RV_OK;
	}

	/* Check if the given buffer is too small */
	if (arraySize < pSecAgree->numOfOwners)
	{
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_INSUFFICIENT_BUFFER;
	}

	/* Go over the list of owners in the security-agreement and set them to the application owners array */
	RLIST_GetHead(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, &hListElem);
	while (hListElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(pSecAgree))
	{
		pOwnerRec = (SecAgreeOwnerRecord*)hListElem;
		memcpy(&pOwners[index], &pOwnerRec->owner, minStructSize);

		RLIST_GetNext(pSecAgree->pSecAgreeMgr->hSecAgreeOwnersPool, pSecAgree->hOwnersList, hListElem, &hListElem);

		index += 1;
		safeCounter += 1;
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetOwnersList - Security-agreement 0x%p: Failed to produce array of owners. rv=%d",
					pSecAgree, rv));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}


/*------------------ S E T  A N D  G E T  F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeSetContext
 * ------------------------------------------------------------------------
 * General: Stores application context in the security-agreement object. This
 *          context is in addition to the application handle, and is used to
 *          hold the data shared by related security-agreement objects. Once 
 *			a new security-agreement is created from an old security-agreement by calling
 *          RvSipSecAgreeCopy(), the context is copied to the new security-agreement.
 *          Example of usage: For ipsec-3gpp negotiation, store IMPI details
 *          in the context parameter. Upon renegotiation, the IMPI will
 *          be copied from the old security-agreement to the new one, and will
 *          be available for you via the new security-agreement and its inner objects.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 *          pContext    - Pointer to the new application context to set to this
 *                        security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetContext(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   void*                    pContext)
{
	SecAgree   *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus    rv;

	if (hSecAgree == NULL || pContext == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}
	
	/* set context to the security object */
	if (pSecAgree->securityData.hSecObj != NULL)
	{
		rv = SipSecObjSetImpi(pSecAgree->pSecAgreeMgr->hSecMgr, pSecAgree->securityData.hSecObj, pContext);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"RvSipSecAgreeSetContext - Security-agreement 0x%p: Failed to set context (IMPI) to security-object 0x%p. rv=%d",
						pSecAgree, pSecAgree->securityData.hSecObj, rv));
			SecAgreeUnLockAPI(pSecAgree);
			return rv;
		}
	}

	pSecAgree->pContext  = pContext;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetContext
 * ------------------------------------------------------------------------
 * General: Gets application context stored in the security-agreement object. This
 *          context is additional to the application handle, and shall be used to
 *          hold the data shared by related security-agreement objects. Once 
 *          a new security-agreement is created from an old security-agreement by calling
 *          RvSipSecAgreeCopy(), the context is copied to the new security-agreement.
 *          Example of usage: For ipsec-3gpp negotiation, store IMPI details
 *          in the context parameter. Then upon renegotiation, the IMPI will
 *          be copied from the old security-agreement to the new one, and will
 *          be available for you via the new security-agreement and its inner objects.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  ppContext   - Pointer to the application context of this security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetContext(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   void**                   ppContext)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || ppContext == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get the context of the security-agreement */
	*ppContext = pSecAgree->pContext;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeSetRole
 * ------------------------------------------------------------------------
 * General: Set the role of the security-agreement object: client or server.
 *          Notice: you must set the role of a security-agreeement to make it
 *          valid to use.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 *          eRole       - The role to set to the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetRole(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   RvSipSecAgreeRole        eRole)
{
	SecAgree   *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus    rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (eRole != RVSIP_SEC_AGREE_ROLE_CLIENT &&
		eRole != RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		/* We do not allow setting undefined role */
		return RV_ERROR_BADPARAM;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Check if the security-agreement already has a role (illegal action) */
	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_UNDEFINED)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeSetRole - Security-agreement 0x%p already has a role: %s",
					pSecAgree, SecAgreeUtilsRoleEnum2String(pSecAgree->eRole)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
              "RvSipSecAgreeSetRole - Security-agreement 0x%p is set to be %s security-agreement",
			  pSecAgree, SecAgreeUtilsRoleEnum2String(eRole)));

	/* Set the specified role to the security-agreement */
	SecAgreeSetRole(pSecAgree, eRole);

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetRole
 * ------------------------------------------------------------------------
 * General: Get the role of the security-agreement object: client or server.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  peState     - The state of this security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRole(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvSipSecAgreeRole       *peRole)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || peRole == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get the role of the security-agreement */
	*peRole = pSecAgree->eRole;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetState
 * ------------------------------------------------------------------------
 * General: Get the current state of the security-agreement object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  peState     - The state of this security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetState(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvSipSecAgreeState      *peState)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || peState == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get the state of the security-agreement */
	*peState = pSecAgree->eState;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreePushLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Push local security header to the local security list. Calling this
 *          function is allowed in states IDLE, CLIENT_REMOTE_LIST_READY and
 *          SERVER_REMOTE_LIST_READY. Once you have completed building the local
 *          security list, call RvSipSecAgreeInit() to complete the initialization
 *          of the security-agreement
 *          Notice: You cannot set two headers with the same mechanism type
 *                  to the local list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         hSecurityHeader - Handle to the local security header
 *         eLocation       - Inserted element location (first, last, etc).
 *         hPos            - Current list position, relevant in case that
 *							 location is next or prev.
 * Output: pNewPos         - The location of the object that was pushed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreePushLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipSecurityHeaderHandle      hSecurityHeader,
							 IN     RvSipListLocation              eLocation,
							 IN     RvSipCommonListElemHandle      hPos,
                             OUT    RvSipCommonListElemHandle*     phNewPos)
{
	SecAgree                    *pSecAgree = (SecAgree*)hSecAgree;
	SecAgreeMgr                 *pSecAgreeMgr;
	RvSipSecurityMechanismType   eSecurityType;
	RvBool                       bAlreadyHasIt;
	RvStatus                     rv;

	if (hSecAgree == NULL || hSecurityHeader == NULL || phNewPos == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	pSecAgreeMgr = pSecAgree->pSecAgreeMgr;

	/* Check if the state of the security-agreement allows pushing local security header */
	if (SecAgreeStateCanModifyLocalSecurityList(pSecAgree) == RV_FALSE)
	{
		/* Changing the local list is not allowed in this state */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreePushLocalSecurityHeader - Cannot modify local security of security-agreement 0x%p in state %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Check if the give security header has a correct type for the role of this security-agreement */
	if (SecAgreeGetLocalSecurityType(pSecAgree) !=
		RvSipSecurityHeaderGetSecurityHeaderType(hSecurityHeader))
	{
		/* We only allow pushing security-client header to client and security-server header to server */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreePushLocalSecurityHeader - Wrong type of header for %s security-agreement 0x%p",
					SecAgreeUtilsRoleEnum2String(pSecAgree->eRole), pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Check the validity of the supplied mechanism */
	eSecurityType =	RvSipSecurityHeaderGetMechanismType(hSecurityHeader);
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		/* We do not allow setting security header with UNDEFINED mechanism */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreePushLocalSecurityHeader - Security-agreement 0x%p: Trying to set security header with undefined mechanism, hHeader=0x%p",
					pSecAgree, hSecurityHeader));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Check that the list of local mechanisms does not contain already the given mechanism.
	   We do not allow two security headers with the same mechanism in the local list */
	rv = SecAgreeUtilsIsMechanismInList(pSecAgree->securityData.hLocalSecurityList,
										eSecurityType, &bAlreadyHasIt, NULL, NULL);
	if (RV_OK != rv)
	{
		/* Checking the local list failed */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreePushLocalSecurityHeader - Security-agreement 0x%p: Failed to verify the given security header, hHeader=0x%p",
					pSecAgree, hSecurityHeader));
		SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}
	if (bAlreadyHasIt == RV_TRUE)
	{
		/* The given mechanism already appears in the local security list */
		RvLogError(pSecAgreeMgr->pLogSrc,(pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreePushLocalSecurityHeader - Security-agreement 0x%p failed to set header 0x%p: alreay has security header with Type %s",
					pSecAgree, hSecurityHeader, SecAgreeUtilsMechanismEnum2String(eSecurityType)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Push the given header */
	rv = SecAgreeSecurityDataPushSecurityHeader(pSecAgreeMgr, pSecAgree->hPage,
									   		   &pSecAgree->securityData, hSecurityHeader,
									   		   eLocation, hPos, SEC_AGREE_SEC_DATA_LIST_LOCAL, phNewPos, NULL);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreePushLocalSecurityHeader - Security-agreement 0x%p: Failed to push local security header 0x%p",
				   pSecAgree, hSecurityHeader));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Gets a security header from the local list of security headers.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the security-agreement object
 *         eLocation        - Location of the wanted element in the list
 *                            (first, last, next, prev).
 *         hPos             - Current list position, relevant in case that
 *							  location is next or prev.
 * Output: phSecurityHeader - The retrieved security header
 *         pNewPos          - The location of the object that was get.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipListLocation              eLocation,
							 IN     RvSipCommonListElemHandle      hPos,
                             OUT    RvSipSecurityHeaderHandle*     phSecurityHeader,
							 OUT    RvSipCommonListElemHandle*     phNewPos)
{
	SecAgree                    *pSecAgree = (SecAgree*)hSecAgree;
	RvInt                        elementType;
	RvStatus                     rv  = RV_OK;

	if (hSecAgree == NULL || phSecurityHeader == NULL || phNewPos == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	*phSecurityHeader = NULL;
	*phNewPos         = NULL;

	if (NULL != pSecAgree->securityData.hLocalSecurityList)
	{
		/* Get local security header */
		rv = RvSipCommonListGetElem(pSecAgree->securityData.hLocalSecurityList,
									eLocation, hPos, &elementType, (void**)phSecurityHeader, phNewPos);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeGetLocalSecurityHeader - Security-agreement 0x%p: Failed to get security header",
					   pSecAgree));
		}
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeRemoveLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Removes a security header from the local list of security headers.
 *          Calling this function is allowed in states IDLE, CLIENT_REMOTE_LIST_READY and
 *          SERVER_REMOTE_LIST_READY. Once you have completed building the local
 *          security list, call RvSipSecAgreeInit() to complete the initialization
 *          of the security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the security-agreement object
 *         hPos             - Current list position, relevant in case that
 *							  location is next or prev.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeRemoveLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipCommonListElemHandle      hPos)
{
	SecAgree                    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus                     rv;

	if (hSecAgree == NULL || hPos == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Check if the state of the security-agreement allows pushing local security header */
	if (SecAgreeStateCanModifyLocalSecurityList(pSecAgree) == RV_FALSE)
	{
		/* Changing the local list is not allowed in this state */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeRemoveLocalSecurityHeader - Cannot modify local security of security-agreement 0x%p in state %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Remove local security header */
	rv = RvSipCommonListRemoveElem(pSecAgree->securityData.hLocalSecurityList, hPos);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeRemoveLocalSecurityHeader - Security-agreement 0x%p: Failed to remove security header in position 0x%p",
				   pSecAgree, hPos));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetRemoteSecurityList
 * ------------------------------------------------------------------------
 * General: Gets the remote list of security mechanisms. This list is
 *          received from the remote party to indicate the list of security
 *          mechanisms it supports. This function can not be called in states
 *          IDLE, CLIENT_LOCAL_LIST_READY, HANDLING_INITIAL_MSG, CLIENT_LOCAL_LIST_SENT
 *			and SERVER_LOCAL_LIST_READY, because the list is not valid in these
 *          states.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree            - Handle to the security-agreement object
 * Output: phRemoteSecurityList - Pointer to the remote security list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemoteSecurityList(
							 IN   RvSipSecAgreeHandle            hSecAgree,
							 OUT  RvSipCommonListHandle         *phRemoteSecurityList)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || phRemoteSecurityList == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get remote security list */
	*phRemoteSecurityList = pSecAgree->securityData.hRemoteSecurityList;

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the security-agreement page, 
 *         and returns the new element handle.
 *         The application may use this function to allocate a message header 
 *         or a message address. It should then fill the element information
 *         and set it back to the security-agreement using the relevant 'set' 
 *         or 'push' functions.
 *
 *         Note: You may use this function only on initial state (before sending a message). 
 *         On any other state you must construct the header on an application page,
 *         and then set it to the stack object.
 *
 *         The function supports the following elements:
 *         Security		  - You should set these headers back with the  
 *						    RvSipSecAgreePushLocalSecurityHeader() API functions.
 *         Authentication - you should set this header back with the 
 *                          RvSipSecAgreeServerSetProxyAuthenticate() API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the security-agreement object
 *			  eHeaderType - The type of header to allocate. 
 *            eAddrType   - For future use.
 * Output:    phHeader    - Handle to the newly created header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetNewMsgElementHandle (
                                      IN   RvSipSecAgreeHandle  hSecAgree,
									  IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*               *phHeader)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || phHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Create the requested header on the security-agreement memory page */
	rv = SecAgreeCreateHeader(pSecAgree, eHeaderType, phHeader);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetNewMsgElementHandle - Security-agreement 0x%p: Failed to create header. rv=%d",
					pSecAgree, rv));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	RV_UNUSED_ARG(eAddrType)

	return rv;
}

/*------------------ C L I E N T   F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeClientSetChosenSecurity
 * ------------------------------------------------------------------------
 * General: The security-agreement process results in choosing a security
 *          mechanism to use in further requests. Once the client security-
 *          agreement becomes active, it automatically makes the choice of a
 *          security mechanism (to view its choice use RvSipSecAgreeGetChosenSecurity).
 *          If an uninitialized client security-agreement received a Security-Server list,
 *          it assumes the CLIENT_REMOTE_LIST_READY state.
 *          In this state you can either initialize a local security list and
 *          call RvSipSecAgreeInit(), or you can avoid initialization and manually
 *          determine the chosen security by calling this function.
 *          You can also call RvSipSecAgreeSetChosenSecurity to run the automatic
 *          choice of the stack with your own choice.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the client security-agreement object
 * Inout:  peSecurityType   - The chosen security mechanism. Use this parameter
 *                            to specify the chosen security mechanism. If you set
 *                            it to RVSIP_SECURITY_MECHANISM_UNDEFINED, the best
 *                            security mechanism will be computed and returned here.
 * Output: phLocalSecurity  - The local security header specifying the chosen security
 *         phRemoteSecurity - The remote security header specifying the chosen security
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientSetChosenSecurity(
							 IN     RvSipSecAgreeHandle             hSecAgree,
							 INOUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT    RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT    RvSipSecurityHeaderHandle      *phRemoteSecurity)
{
	SecAgree                   *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus                    rv;

	if (hSecAgree == NULL || peSecurityType == NULL || phLocalSecurity == NULL || phRemoteSecurity == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* check if the state of the security-agreement allows choosing a security */
	if (RV_FALSE == SecAgreeStateCanChooseSecurity(pSecAgree))
	{
		/* The state is not valid for setting the security */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientSetChosenSecurity - Security-agreement 0x%p cannot choose security in state %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}
	
	/* check if the given security mechanism is valid */
	if (RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED != *peSecurityType &&
		RV_FALSE == SecAgreeUtilsIsSupportedMechanism(*peSecurityType))
	{
		/* The mechanism is not valid for setting the security */
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientSetChosenSecurity - Security-agreement 0x%p cannot choose unsupported mechanism %s",
					pSecAgree, SecAgreeUtilsMechanismEnum2String(*peSecurityType)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc, (pSecAgree->pSecAgreeMgr->pLogSrc,
			  "RvSipSecAgreeClientSetChosenSecurity - Security-agreement 0x%p: Setting security mechanism",
			  pSecAgree));

	rv = SecAgreeClientSetChosenSecurity(pSecAgree, peSecurityType, 
										 phLocalSecurity, phRemoteSecurity);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc, (pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeClientSetChosenSecurity - Security-agreement 0x%p: Failed to set the chosen mechanism, rv=%d",
				   pSecAgree, rv));
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeClientGetChosenSecurity
 * ------------------------------------------------------------------------
 * General: Gets the security mechanism chosen by this client security-agreement.
 *          This mechanism will be used in further requests.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the client security-agreement object
 * Output: peSecurityType   - The security mechanism chosen by this security-agreement
 *         phLocalSecurity  - The local security header specifying the chosen security
 *         phRemoteSecurity - The remote security header specifying the chosen security
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetChosenSecurity(
							 IN   RvSipSecAgreeHandle             hSecAgree,
							 OUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT  RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT  RvSipSecurityHeaderHandle      *phRemoteSecurity)
{
	SecAgree                     *pSecAgree = (SecAgree*)hSecAgree;
	RvSipSecurityMechanismType   eSecurityType;
	RvSipSecurityHeaderHandle    hLocalSecurity;
	RvSipSecurityHeaderHandle    hRemoteSecurity;
	RvBool                       bIsValid = RV_TRUE;
	RvStatus					 rv;

	if (hSecAgree == NULL || peSecurityType == NULL || phLocalSecurity == NULL || phRemoteSecurity == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Check if the client security-agreement should perform search for the best security mechanism */
	if (RV_TRUE == SecAgreeStateCanChooseSecurity(pSecAgree) &&
		pSecAgree->securityData.hLocalSecurityList != NULL &&
		pSecAgree->securityData.hRemoteSecurityList != NULL &&
		pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED)
	{
		RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				  "RvSipSecAgreeClientGetChosenSecurity - Client security-agreement=0x%p: Computing default mechanism",
				  pSecAgree));

		/* The state is appropriate for searching for the best mechanism. We perform the search */
		rv = SecAgreeClientChooseMechanism(pSecAgree, &eSecurityType, &hLocalSecurity, &hRemoteSecurity, &bIsValid);
		if (RV_OK != rv)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeClientGetChosenSecurity - Client security-agreement=0x%p failed to compute chosen security mechanism, rv=%d",
					   pSecAgree, rv));
		}
		else if (bIsValid == RV_FALSE)
		{
			RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeClientGetChosenSecurity - Client security-agreement=0x%p: The most preferenced security mechanism is invalid",
					   pSecAgree));
		}
		else
		{
			/* Store the computed mechanism in the security-agreement object */
			pSecAgree->securityData.selectedSecurity.eType         = eSecurityType;
			pSecAgree->securityData.selectedSecurity.hLocalHeader  = hLocalSecurity;
			pSecAgree->securityData.selectedSecurity.hRemoteHeader = hRemoteSecurity;
		}
	}

	*peSecurityType   = pSecAgree->securityData.selectedSecurity.eType;
	*phLocalSecurity  = pSecAgree->securityData.selectedSecurity.hLocalHeader;
	*phRemoteSecurity = pSecAgree->securityData.selectedSecurity.hRemoteHeader;

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeClientReturnToLocalListReady
 * ------------------------------------------------------------------------
 * General: Returns a security-agreement object to state CLIENT_LOCAL_LIST_READY. Returning
 *          to the local list ready state allows you to reinsert security-agreement
 *          information to messages. For example, when a message fails to be sent,
 *          the security-agreement object moves to the INVALID state and you
 *          receive a message send failure indication on the SIP object. Then you should 
 *          call RvSipSecAgreeClientReturnToLocalListReady() before proceeding to the
 *          next DNS entry, in order for the next message to contain the security-agreement
 *          information.
 *          Notice: You cannot call this function if the security-agreement was once
 *                  ACTIVE.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to return to state
 *                       CLIENT_LOCAL_LIST_READY
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientReturnToLocalListReady(
							 IN   RvSipSecAgreeHandle    hSecAgree)
{
	SecAgree                    *pSecAgree    = (SecAgree*)hSecAgree;
	RvSipSecObjState             eSecObjState = RVSIP_SECOBJ_STATE_UNDEFINED;
	RvBool                       bIsLocalListEmpty     = RV_TRUE;
	RvBool                       bIsRemoteListEmpty     = RV_TRUE;
	RvBool                       bIsSecObj    = RV_TRUE;
	RvStatus					 rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_CLIENT)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientReturnToLocalListReady - Is not allowed for server (or undefined) security-agreements (security-agreement=0x%p)",
					pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SecAgreeUtilsIsListEmpty(pSecAgree->securityData.hLocalSecurityList, &bIsLocalListEmpty);
	if (RV_OK == rv)
	{
		rv = SecAgreeUtilsIsListEmpty(pSecAgree->securityData.hRemoteSecurityList, &bIsRemoteListEmpty);
		if (RV_OK == rv)
		{
			/* Check whether there should be a security object in the security-agreement
			   (i.e., there is local TLS or ipsec security headers) */
			rv = SecAgreeSecurityDataIsLocalSecObj(&pSecAgree->securityData, &bIsSecObj);
		}
	}
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientReturnToLocalListReady - Client security-agreement=0x%p: Failed to check security data, rv=%d",
					pSecAgree, rv));
		SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	if (RV_TRUE == bIsLocalListEmpty || RV_FALSE == bIsRemoteListEmpty)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientReturnToLocalListReady - Illegal action for client security-agreement=0x%p: local list must be valis and remote list must be empty, rv=%d",
					pSecAgree, rv));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}	

	if (RV_TRUE == bIsSecObj)
	{
		if (pSecAgree->securityData.hSecObj != NULL)
		{
			rv = SipSecObjGetState(pSecAgree->pSecAgreeMgr->hSecMgr, pSecAgree->securityData.hSecObj, &eSecObjState);
			if (RV_OK != rv)
			{
				RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
							"RvSipSecAgreeClientReturnToLocalListReady - Client security-agreement=0x%p: Failed to check security-object 0x%p, rv=%d",
							pSecAgree, pSecAgree->securityData.hSecObj, rv));
				SecAgreeUnLockAPI(pSecAgree);
				return rv;
			}
		}

		if (eSecObjState != RVSIP_SECOBJ_STATE_INITIATED)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					   "RvSipSecAgreeClientReturnToLocalListReady - Client security-agreement=0x%p: Illegal when security object 0x%p is not in state INITIATED",
					   pSecAgree, pSecAgree->securityData.hSecObj));
			SecAgreeUnLockAPI(pSecAgree);
			return RV_ERROR_ILLEGAL_ACTION;
		}
	}

	rv = SecAgreeStateReturnToClientLocalReady(pSecAgree, pSecAgree->eState);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeClientReturnToLocalListReady - Client security-agreement=0x%p: Failed to to move to state LOCAL_LIST_READY, rv=%d",
				   pSecAgree, rv));
	}
	
	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeClientSetAddSecurityClientToMsg
 * ------------------------------------------------------------------------
 * General: Sets an indication to add Security-Client headers to outgoing
 *          requests.
 *          Notice: The stack security-agreement will add Security-Client
 *                  headers according to RFC3329 and TS33.203. You should
 *                  use this flag for additional cases.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree     - Handle to the client security-agreement object
 *			  bAddSecClient - RV_TRUE to add Security-Client headers to all
 *                            requests. RV_FALSE to add Security-Client headers
 *                            according to RFC3329 and TS33.203.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientSetAddSecurityClientToMsg(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      IN   RvBool                bAddSecClient)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	if (bAddSecClient == RV_TRUE)
	{
		pSecAgree->securityData.eInsertSecClient = SEC_AGREE_SET_CLIENT_LIST_BY_APP;
	}
	else
	{
		pSecAgree->securityData.eInsertSecClient = SEC_AGREE_SET_CLIENT_LIST_UNDEFINED;
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeClientGetAddSecurityClientToMsg
 * ------------------------------------------------------------------------
 * General: Gets the special indication to add Security-Client headers to
 *          outgoing requests, previously set by the application.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree      - Handle to the client security-agreement object
 * Output:	  pbAddSecClient - RV_TRUE if Security-Client headers are added to all
 *                             requests. RV_FALSE if Security-Client headers are added
 *                             according to RFC3329 and TS33.203.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetAddSecurityClientToMsg(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      OUT  RvBool               *pbAddSecClient)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	if (pSecAgree->securityData.eInsertSecClient == SEC_AGREE_SET_CLIENT_LIST_BY_APP ||
		pSecAgree->securityData.eInsertSecClient == SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC)
	{
		*pbAddSecClient = RV_TRUE;
	}
	else
	{
		*pbAddSecClient = RV_FALSE;
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeClientGetAuthObj
 * ------------------------------------------------------------------------
 * General: Gets the authentication object from the client security-agreement.
 *          This authentication object was built from the Proxy-Authenticate
 *          header that was received in the server response.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the client security-agreement object
 * Output:    phAuthObj   - Handle to the authentication object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetAuthObj(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      OUT  RvSipAuthObjHandle*   phAuthObj)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || phAuthObj == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Only client security-agreement has auth-obj list */
	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_CLIENT)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientGetAuthObj - Illegal action for %s security-agreement 0x%p",
					SecAgreeUtilsRoleEnum2String(pSecAgree->eRole), pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Get auth-obj list */
	rv = SipAuthenticatorAuthListGetObj(pSecAgree->pSecAgreeMgr->hAuthenticator,
										pSecAgree->securityData.additionalKeys.authInfo.hClientAuth.hAuthList,
                                        RVSIP_FIRST_ELEMENT, NULL, phAuthObj);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeClientGetAuthObj - Client security-agreement 0x%p: Failed to get authentication object",
					pSecAgree));
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;

}

/*------------------ S E R V E R   F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeServerGetRecommendedResponseCode
 * ------------------------------------------------------------------------
 * General: Gets the response code recommended by this security-agreement.
 *          The recommended response code can be used by the server to respond
 *          as recommended in RFC3329.
 *			Notice: When other obligatory response code applies (for example
 *                  when responding to BYE in a connected session), use more
 *                  careful judgment before using the recommended response code.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree       - Handle to the server security-agreement object
 * Output:  peResponseCode  - The recommended response code. 0 if there is no
 *                            recommendation.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerGetRecommendedResponseCode(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvUint16*                peResponseCode)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || peResponseCode == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Get recommended response code */
	*peResponseCode = pSecAgree->responseCode;

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeServerSetProxyAuthenticate
 * ------------------------------------------------------------------------
 * General: Sets a Proxy-Authenticate to the server security-agreement.
 *          The server will insert this header whenever it inserts the
 *          local security list to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the server security-agreement object
 *            hProxyAuth  - The Proxy-Authenticate header to set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerSetProxyAuthenticate(
                                    IN   RvSipSecAgreeHandle              hSecAgree,
                                    IN   RvSipAuthenticationHeaderHandle  hProxyAuth)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv;

	if (hSecAgree == NULL || hProxyAuth == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Only server security-agreement has Proxy-Auth header */
	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerSetProxyAuthenticate - Illegal action for %s security-agreement 0x%p",
					SecAgreeUtilsRoleEnum2String(pSecAgree->eRole), pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SecAgreeSecurityDataSetProxyAuth(pSecAgree->pSecAgreeMgr, pSecAgree->hPage,
								  		  &pSecAgree->securityData, hProxyAuth);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerSetProxyAuthenticate - Server security-agreement 0x%p: Failed to set Proxy-Authenticate header",
					pSecAgree));
	}

	/* unlock the security-agreement */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeServerGetProxyAuthenticate
 * ------------------------------------------------------------------------
 * General: Gets the Proxy-Authenticate header of this server security-agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the server security-agreement object
 * Output:    phProxyAuth  - Handle to the Proxy-Authenticate header
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerGetProxyAuthenticate(
                                      IN   RvSipSecAgreeHandle                hSecAgree,
                                      OUT  RvSipAuthenticationHeaderHandle*   phProxyAuth)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv        = RV_OK;

	if (hSecAgree == NULL || phProxyAuth == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Only server security-agreement has Proxy-Auth header */
	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerGetProxyAuthenticate - Illegal action for %s security-agreement 0x%p",
					SecAgreeUtilsRoleEnum2String(pSecAgree->eRole), pSecAgree));
        SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	/* Get Proxy-Auth */
	*phProxyAuth = pSecAgree->securityData.additionalKeys.authInfo.hServerAuth.hProxyAuth;

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeServerForceIpsecSecurity
 * ------------------------------------------------------------------------
 * General: Makes the previously obtained ipsec-3gpp channels serviceable at the server 
 *          security-agreement. Calling this function will cause any outgoing request 
 *          from this server to be sent on those ipsec-3gpp channels.
 *          Normally, the server security-agreement will automatically make
 *          the ipsec-3gpp channels serviceable, once the first secured request
 *          is receives from the client. However, in some circumstances, the server
 *          might wish to make the ipsec-3gpp channels serviceable with no further delay,
 *          which can be obtained by calling RvSipSecAgreeServerForceIpsecSecurity().
 *          Note: Calling RvSipSecAgreeServerForceIpsecSecurity() is enabled only in the 
 *          ACTIVE state of the security-object. 
 *          Note: Calling RvSipSecAgreeServerForceIpsecSecurity() is enabled only when
 *          the Security-Server list contains a single header with ipsec-3gpp 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the server security-agreement object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerForceIpsecSecurity(
                                    IN   RvSipSecAgreeHandle            hSecAgree)
{
	SecAgree					 *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus					  rv        = RV_OK;
    
    if (hSecAgree == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    /* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) 
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			  "RvSipSecAgreeServerForceIpsecSecurity - Security-agreement 0x%p: forcing ipsec security",
			  pSecAgree));

    rv = SecAgreeServerForceIpsecSecurity(pSecAgree);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerForceIpsecSecurity - Security-agreement 0x%p: Failed to force ipsec security",
					pSecAgree));
	}

#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeServerForceIpsecSecurity - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp",
			   pSecAgree));
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return RV_OK;
}

/***************************************************************************
 * RvSipSecAgreeServerReturnToActive
 * ------------------------------------------------------------------------
 * General: Returns a server security-agreement object to state ACTIVE.
 *          When the server security-agreement receives an indication for renegotiation
 *          (indicated by a change in Security-Client list) it changes state to INVALID. 
 *          The renegotiation is performed by a new security-agreement, which will eventually
 *          take over the old INVALID security-agreement. However according to 3GPP TS 24.229, 
 *          he renegotiation might be completed with no need to switch security-associations, 
 *          in which case the old security-agreement shall regain control over the SIP objects
 *          and their message sending. In order to recover from the INVALID state at the old
 *          security-agreement, call RvSipSecAgreeServerReturnToActive().
 *          
 *          Notice: You can invoke this function only when the security-agreement is INVALID
 *                  but the security-object is still ACTIVE.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to return to state ACTIVE
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerReturnToActive(
							 IN   RvSipSecAgreeHandle    hSecAgree)
{
	SecAgree                    *pSecAgree    = (SecAgree*)hSecAgree;
	RvSipSecObjState             eSecObjState = RVSIP_SECOBJ_STATE_UNDEFINED;
	RvStatus					 rv;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	if (pSecAgree->eRole != RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerReturnToActive - Is not allowed for client (or undefined) security-agreements (security-agreement=0x%p)",
					pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (pSecAgree->securityData.hSecObj == NULL)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerReturnToActive - Server security-agreement=0x%p has no security-object",
					pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SipSecObjGetState(pSecAgree->pSecAgreeMgr->hSecMgr, pSecAgree->securityData.hSecObj, &eSecObjState);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeServerReturnToActive - Server security-agreement=0x%p: Failed to check security-object 0x%p, rv=%d",
					pSecAgree, pSecAgree->securityData.hSecObj, rv));
		SecAgreeUnLockAPI(pSecAgree);
		return rv;
	}

	if (eSecObjState != RVSIP_SECOBJ_STATE_ACTIVE)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeServerReturnToActive - Server security-agreement=0x%p: Illegal when security object 0x%p is not in state ACTIVE",
				   pSecAgree, pSecAgree->securityData.hSecObj));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SecAgreeStateReturnToActive(pSecAgree, pSecAgree->eState);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
				   "RvSipSecAgreeServerReturnToActive - Server security-agreement=0x%p: Failed to to move to state ACTIVE, rv=%d",
				   pSecAgree, rv));
	}
	
	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return RV_OK;
}

/*----------- S E C U R I T Y  O B J E C T  F U N C T I O N S ----------*/

/***************************************************************************
 * RvSipSecAgreeSetLocalAddr
 * ------------------------------------------------------------------------
 * General: Sets a local address to use when setting a secure channel with the 
 *          remote party. 
 *          If the security-agreement is required to initiate an ipsec-3gpp channel,
 *          or if it is required to initiate a tls connection (clients only),
 *          the local address (bound to the connection) is chosen from the set of 
 *          local addresses, according to the characteristics of the remote address. 
 *          You can use this function to force the security-agreement to choose a 
 *          specific local address for a specific transport type.
 *          If you do not set a local address to the security-agreement, a default
 *          one is used.
 *          Notice: If you wish to set a local address to the security-agreement, 
 *          you must set it before calling RvSipSecAgreeInit().
 *          Notice: If you wish to set local address for ipsec-3gpp use, specify here local
 *          IP (must be v4) and transport type (must be UDP or TCP). The local ports must be
 *          set via local Security header using RvSipSecAgreePushLocalSecurityHeader().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that should use the specified local 
 *                           address (ipsec-3gpp or TLS).
 *            pLocalAddr   - The local address structure
 *            structSize   - The size of RvSipTransportAddr
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetLocalAddr(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecurityMechanismType  eMechanism,
                                    IN  RvSipTransportAddr         *pLocalAddress,
									IN  RvInt32                     structSize)
{
	SecAgree					 *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus					  rv        = RV_OK;
    
    if (hSecAgree == NULL || pLocalAddress == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (structSize < (RvInt32)sizeof(RvSipTransportAddr))
	{
		return RV_ERROR_BADPARAM;
	}

	if (eMechanism != RVSIP_SECURITY_MECHANISM_TYPE_TLS &&
		eMechanism != RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		/* Setting address is used for TLS and ipsec only */
		return RV_ERROR_BADPARAM;
	}

	/* Check the correctness of the given TLS address */
	if (eMechanism == RVSIP_SECURITY_MECHANISM_TYPE_TLS &&
		pLocalAddress->eTransportType != RVSIP_TRANSPORT_TLS)
	{
		return RV_ERROR_BADPARAM;
	}
	
    /* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			  "RvSipSecAgreeSetLocalAddr - Security-agreement 0x%p: setting local address",
			  pSecAgree));

    rv = SecAgreeObjectSetLocalAddr(pSecAgree, pLocalAddress);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeSetLocalAddr - Security-agreement 0x%p: Failed to set local address",
					pSecAgree));
	}

#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeSetLocalAddr - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp and tls",
			   pSecAgree));
	RV_UNUSED_ARG(pLocalAddress);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetLocalAddr
 * ------------------------------------------------------------------------
 * General: Gets the local address used by this security-agreement. 
 *          The local address is used for initiating ipsec-3gpp channels, or
 *          TLS connections.
 * 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that the requested local 
 *                           address is used for (ipsec-3gpp or TLS).
 *            eAddressType - The type of the requested local address (ipv4 or ipv6). 
 *                           Relevant only for TLS addresses.
 *            structSize   - The size of RvSipTransportAddr
 * Inout:     pLocalAddr   - The local address structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetLocalAddr(
                                    IN     RvSipSecAgreeHandle         hSecAgree,
									IN     RvSipSecurityMechanismType  eMechanism,
									IN	   RvSipTransportAddressType   eAddressType,
									IN     RvInt32                     structSize,
                                    INOUT  RvSipTransportAddr         *pLocalAddress)
{
	SecAgree			*pSecAgree      = (SecAgree*)hSecAgree;
	RvSipTransportAddr   localAddr;
	RvInt32              minStructSize  = RvMin(structSize,((RvInt32)sizeof(RvSipTransportAddr)));
	RvStatus			 rv             = RV_OK;

	if (hSecAgree == NULL || pLocalAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize <= 0)
	{
		return RV_ERROR_BADPARAM;
	}

    memset(&localAddr, 0, sizeof(RvSipTransportAddr));

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Reset returned address */
	rv = RV_ERROR_NOT_FOUND;
	pLocalAddress->strIP[0]       = '\0';
	pLocalAddress->eAddrType      = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED; 
	pLocalAddress->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
	pLocalAddress->Ipv6Scope      = UNDEFINED;
	pLocalAddress->port           = 0;

	switch (eMechanism) 
	{
#if (RV_TLS_TYPE != RV_TLS_NONE)
	case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
		{
			if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
			{
				if (pSecAgree->addresses.hLocalTlsIp4Addr != NULL)
				{
                    rv = RvSipTransportMgrLocalAddressGetDetails(
                            pSecAgree->addresses.hLocalTlsIp4Addr, structSize,
                            &localAddr);
				}
			}
			else if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
			{
				if (pSecAgree->addresses.hLocalTlsIp6Addr != NULL)
				{
                    rv = RvSipTransportMgrLocalAddressGetDetails(
                            pSecAgree->addresses.hLocalTlsIp6Addr, structSize,
                            &localAddr);
				}
			}
			else
			{
				RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
							"RvSipSecAgreeGetLocalAddr - Security-agreement 0x%p: invalid address type %d",
							pSecAgree, eAddressType));
				rv = RV_ERROR_ILLEGAL_ACTION;
			}
		}
		break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
#if (RV_IMS_IPSEC_ENABLED == RV_YES)	
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
		{
			RvSipTransportLocalAddrHandle  hLocalIpsecUdpAddr = NULL;
			RvSipTransportLocalAddrHandle  hLocalIpsecTcpAddr = NULL;

			if (pSecAgree->securityData.hSecObj != NULL)
			{
				rv = SipSecObjIpsecGetPortSLocalAddresses(pSecAgree->pSecAgreeMgr->hSecMgr, 
														  pSecAgree->securityData.hSecObj, 
														  &hLocalIpsecUdpAddr,
														  &hLocalIpsecTcpAddr);
				if (RV_OK != rv)
				{
					RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
								"RvSipSecAgreeGetLocalAddr - Security-agreement 0x%p: Failed to get local addresses from security-object 0x%p",
								pSecAgree, pSecAgree->securityData.hSecObj));
					break;
				}
				if (hLocalIpsecTcpAddr != NULL &&
					pSecAgree->addresses.eRemoteIpsecTransportType == RVSIP_TRANSPORT_TCP)
				{
                    rv = RvSipTransportMgrLocalAddressGetDetails(
                            hLocalIpsecTcpAddr, structSize, &localAddr);
				}
				else if (hLocalIpsecUdpAddr != NULL)
				{
                    rv = RvSipTransportMgrLocalAddressGetDetails(
                            hLocalIpsecUdpAddr, structSize, &localAddr);
				}
			}
            /* If no Local Address was opened yet, take the preset data */
			if (hLocalIpsecUdpAddr == NULL &&
				hLocalIpsecTcpAddr == NULL &&
				pSecAgree->addresses.pLocalIpsecIp != NULL)
			{
				strncpy(localAddr.strIP, pSecAgree->addresses.pLocalIpsecIp,
                        RVSIP_TRANSPORT_LEN_STRING_IP);
				localAddr.eTransportType = pSecAgree->addresses.eRemoteIpsecTransportType;
                localAddr.eAddrType      = pSecAgree->addresses.localIpsecAddrType;
				localAddr.Ipv6Scope      = pSecAgree->addresses.localIpsecIpv6Scope;
				localAddr.port           = 0;

                /* For IPv6 add square brackets and Scope Id to the strIP */
#if (RV_NET_TYPE & RV_NET_IPV6)
                if (RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 == localAddr.eAddrType)
                {
                    if ('[' != *pSecAgree->addresses.pLocalIpsecIp)
                    {
                        sprintf(localAddr.strIP,"[%s]",pSecAgree->addresses.pLocalIpsecIp);
                    }
                    if (NULL == strchr(localAddr.strIP,'%'))
                    {
                        RvChar strScopeId[32];
                        sprintf(strScopeId, "%%%d", localAddr.Ipv6Scope);
                        strcat(localAddr.strIP, strScopeId);
                    }
                }
#endif
                rv = RV_OK;
			}
		}
		break;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	default:
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"RvSipSecAgreeGetLocalAddr - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp and tls",
						pSecAgree));
			rv = RV_ERROR_ILLEGAL_ACTION;
		}
	}

	if (RV_OK == rv)
	{
		memcpy(pLocalAddress, &localAddr, minStructSize);
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	RV_UNUSED_ARG(eAddressType);
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeSetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Sets the remote address to use when setting a secure channel with the 
 *          remote party. 
 *          If the security-agreement is required to initiate ipsec-3gpp channels,
 *          or if it is required to initiate a tls connection (clients only),
 *          the remote address you set here will be used.
 *          Notice: If TLS is negotiated, you must call RvSipSecAgreeSetRemoteAddr() 
 *                  to set TLS remote details before calling RvSipSecAgreeInit().
 *          Notice: If ipsec-3gpp is negotiated you may call RvSipSecAgreeSetRemoteAddr() 
 *                  to set remote IP (must be v4) and transport to use (must be UDP or TCP). 
 *                  The remote ports are received from the server. Do not set ports here.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that should use the specified remote 
 *                           address (ipsec-3gpp or TLS).
 *            pRemoteAddr  - The remote address structure
 *            structSize   - The size of RvSipTransportAddr
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetRemoteAddr(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecurityMechanismType  eMechanism,
                                    IN  RvSipTransportAddr         *pRemoteAddress,
									IN  RvInt32                     structSize)
{
	RvStatus					  rv        = RV_OK;
    SecAgree					 *pSecAgree = (SecAgree*)hSecAgree;
    
#if (RV_LOGMASK == RV_LOGLEVEL_NONE) || (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
	RV_UNUSED_ARG(pSecAgree);
#endif

    if (hSecAgree == NULL || pRemoteAddress == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (structSize < (RvInt32)sizeof(RvSipTransportAddr))
	{
		return RV_ERROR_BADPARAM;
	}

	if (eMechanism != RVSIP_SECURITY_MECHANISM_TYPE_TLS &&
		eMechanism != RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		/* Setting address is used for TLS and ipsec only */
		return RV_ERROR_BADPARAM;
	}

	/* Check the correctness of the given TLS address */
	if (eMechanism == RVSIP_SECURITY_MECHANISM_TYPE_TLS &&
		pRemoteAddress->eTransportType != RVSIP_TRANSPORT_TLS)
	{
		return RV_ERROR_BADPARAM;
	}

    /* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			  "RvSipSecAgreeSetRemoteAddr - Security-agreement 0x%p: setting remote address",
			  pSecAgree));

    rv = SecAgreeObjectSetRemoteAddr(pSecAgree, pRemoteAddress);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeSetRemoteAddr - Security-agreement 0x%p: Failed to set remote address",
					pSecAgree));
	}
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeSetRemoteAddr - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp and tls",
			   pSecAgree));
	RV_UNUSED_ARG(pRemoteAddress);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

    /* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

    return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Gets the remote address used by this security-agreement. 
 *          The remote address is used for initiating ipsec-3gpp channels, or
 *          TLS connections.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that the requested remote 
 *                           address is used for (ipsec-3gpp or TLS).
 *            structSize   - The size of RvSipTransportAddr
 * Inout:     pRemoteAddr  - The remote address structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemoteAddr(
                                    IN    RvSipSecAgreeHandle          hSecAgree,
                                    IN    RvSipSecurityMechanismType   eMechanism,
									IN    RvInt32                      structSize,
                                    INOUT RvSipTransportAddr          *pRemoteAddress)
{
	SecAgree            *pSecAgree      = (SecAgree*)hSecAgree;
	RvSipTransportAddr   remoteAddr;
	RvInt32              minStructSize  = RvMin(structSize,((RvInt32)sizeof(RvSipTransportAddr)));
	RvStatus             rv             = RV_OK;

	if (hSecAgree == NULL || pRemoteAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize <= 0)
	{
		return RV_ERROR_BADPARAM;
	}

    memset(&remoteAddr, 0, sizeof(RvSipTransportAddr));

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	/* Reset returned address */
	rv = RV_ERROR_NOT_FOUND;
	pRemoteAddress->strIP[0]       = '\0';
	pRemoteAddress->eAddrType      = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED; 
	pRemoteAddress->eTransportType = RVSIP_TRANSPORT_UNDEFINED;
	pRemoteAddress->Ipv6Scope      = UNDEFINED;
	pRemoteAddress->port           = 0;
	
	switch (eMechanism) 
	{
#if (RV_IMS_IPSEC_ENABLED == RV_YES)	
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
		{
			if (pSecAgree->addresses.remoteIpsecAddr.addrtype != 0)
			{
				SipTransportConvertCoreAddressToApiTransportAddr(&pSecAgree->addresses.remoteIpsecAddr, &remoteAddr);	
                remoteAddr.eTransportType = pSecAgree->addresses.eRemoteIpsecTransportType;
				rv = RV_OK;
			}
		}
		break;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
#if (RV_TLS_TYPE != RV_TLS_NONE)
	case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
		{
			if (pSecAgree->addresses.remoteTlsAddr.addrtype != 0)
			{
				SipTransportConvertCoreAddressToApiTransportAddr(&pSecAgree->addresses.remoteTlsAddr, &remoteAddr);
                remoteAddr.eTransportType = RVSIP_TRANSPORT_TLS;
				rv = RV_OK;
			}
		}
		break;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */
	default:
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"RvSipSecAgreeGetRemoteAddr - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp and tls",
						pSecAgree));
			rv = RV_ERROR_ILLEGAL_ACTION;
		}
	}

	if (RV_OK == rv)
	{
		memcpy(pRemoteAddress, &remoteAddr, minStructSize);
	}
	else if (rv != RV_ERROR_NOT_FOUND)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetRemoteAddr - Security-agreement 0x%p: Failed to get remote address, rv=%d",
					pSecAgree, rv));
	}

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeSetSendOptions
 * ------------------------------------------------------------------------
 * General: Sets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp. The send options shall be set
 *          in parallel to setting the remote address via 
 *          RvSipSecAgreeSetRemoteAddr (or deciding that the default remote
 *          address shall be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 *            structSize   - The size of pOptions
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetSendOptions(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions,
									IN  RvInt32                     structSize)
{
	RvStatus	        rv        = RV_OK;
    SecAgree           *pSecAgree  = (SecAgree*)hSecAgree;
	
    if (hSecAgree == NULL || pOptions == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if (structSize < (RvInt32)sizeof(RvSipSecAgreeSendOptions))
	{
		return RV_ERROR_BADPARAM;
	}
		
    /* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	

#ifdef RV_SIGCOMP_ON
	rv = SecAgreeObjectSetSendOptions(pSecAgree, pOptions);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: Failed to set send options, rv=%d",
					pSecAgree, rv));
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 

	
    /* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);
	
    return rv; 
}

/***************************************************************************
 * RvSipSecAgreeGetSendOptions
 * ------------------------------------------------------------------------
 * General: Gets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pOptions	   - Pointer to the options structure. Make sure to
 *                           supply a buffer of size RVSIP_COMPARTMENT_MAX_URN_LENGTH
 *                           for the sigcomp-id.
 *            structSize   - The size of pOptions
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSendOptions(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions,
									IN  RvInt32                     structSize)
{
	SecAgree				*pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus				 rv             = RV_OK;
	
	if (hSecAgree == NULL || pOptions == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize < (RvInt32)sizeof(RvSipSecAgreeSendOptions))
	{
		return RV_ERROR_BADPARAM;
	}
	
	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	/* Init options */
#ifdef RV_SIGCOMP_ON
	rv = SecAgreeObjectGetSendOptions(pSecAgree, pOptions);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeSetSendOptions - Security-agreement 0x%p: Failed to get send options, rv=%d",
					pSecAgree, rv));
	}
#endif /* #ifdef RV_SIGCOMP_ON */ 
	
	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);
	
	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetSecObj
 * ------------------------------------------------------------------------
 * General: Gets the security object of this security-agreement. The security
 *          object is used by the owners of the security-agreement for sending
 *          request messages.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree  - Handle to the security-agreement object
 * Output:    phSecObj   - Handle to the security object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSecObj(
                                    IN   RvSipSecAgreeHandle    hSecAgree,
                                    OUT  RvSipSecObjHandle*     phSecObj)
{
	SecAgree    *pSecAgree = (SecAgree*)hSecAgree;
	RvStatus     rv        = RV_OK;

	if (hSecAgree == NULL || phSecObj == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	*phSecObj = pSecAgree->securityData.hSecObj;

#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeGetSecObj - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp and tls",
			   pSecAgree));
	RV_UNUSED_ARG(phSecObj);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree)
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeSetIpsecInfo
 * ------------------------------------------------------------------------
 * General: Sets ipsec information to the security-agreement. This information
 *          is essential for obtaining an active ipsec channel with the remote
 *          party. 
 *          Notice: If you wish to allow sending via ipsec, you must supply ipsec 
 *          information before calling RvSipSecAgreeInit().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pIpsecInfo   - Pointer to the structure of ipsec information
 *            structSize   - The size of RvSipSecAgreeIpsecInfo
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetIpsecInfo(
                                    IN   RvSipSecAgreeHandle      hSecAgree,
                                    IN   RvSipSecAgreeIpsecInfo  *pIpsecInfo,
									IN   RvInt32                  structSize)
{
	SecAgree                *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus                 rv             = RV_OK;

	if (hSecAgree == NULL || pIpsecInfo == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (structSize < (RvInt32)sizeof(RvSipSecAgreeIpsecInfo))
	{
		return RV_ERROR_BADPARAM;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	/* Copy IK and CK */
	SecAgreeUtilsCopyKey(pSecAgree->securityData.additionalKeys.ipsecInfo.IK,
						 pSecAgree->securityData.additionalKeys.ipsecInfo.CK,
						 pIpsecInfo->IK, pIpsecInfo->CK);
	pSecAgree->securityData.additionalKeys.ipsecInfo.CKlen = pIpsecInfo->CKlen;
	pSecAgree->securityData.additionalKeys.ipsecInfo.IKlen = pIpsecInfo->IKlen;
	
	/* Copy lifetime value */
	pSecAgree->securityData.additionalKeys.ipsecInfo.lifetime = pIpsecInfo->lifetime;
	
#elif !(defined RV_SIP_IPSEC_NEG_ONLY) /* (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeSetIpsecInfo - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp",
			   pSecAgree));
	RV_UNUSED_ARG(pIpsecInfo);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE) || (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetIpsecInfo
 * ------------------------------------------------------------------------
 * General: Gets the ipsec information of this security-agreement. This information
 *          is essential for obtaining an active ipsec channel with the remote
 *          party. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            structSize   - The size of RvSipSecAgreeIpsecInfo
 * Inout:     pIpsecInfo   - Pointer to the structure of ipsec information. This
 *                           structure will be filled with ipsec information from
 *                           the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetIpsecInfo(
                                    IN    RvSipSecAgreeHandle      hSecAgree,
									IN    RvInt32                  structSize,
                                    INOUT RvSipSecAgreeIpsecInfo  *pIpsecInfo)
{
	SecAgree                *pSecAgree      = (SecAgree*)hSecAgree;
	RvSipSecAgreeIpsecInfo   ipsecInfo;
	RvInt32                  minStructSize  = RvMin(structSize,((RvInt32)sizeof(RvSipSecAgreeIpsecInfo)));
	RvStatus                 rv             = RV_OK;

	if (hSecAgree == NULL || pIpsecInfo == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	SecAgreeUtilsCopyKey(ipsecInfo.IK, ipsecInfo.CK,
						 pSecAgree->securityData.additionalKeys.ipsecInfo.IK,
						 pSecAgree->securityData.additionalKeys.ipsecInfo.CK);

	ipsecInfo.CKlen = pSecAgree->securityData.additionalKeys.ipsecInfo.CKlen;
	ipsecInfo.IKlen = pSecAgree->securityData.additionalKeys.ipsecInfo.IKlen;
	
	ipsecInfo.lifetime = pSecAgree->securityData.additionalKeys.ipsecInfo.lifetime;

	memcpy(pIpsecInfo, &ipsecInfo, minStructSize);
	
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeGetIpsecInfo - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp",
			   pSecAgree));
	RV_UNUSED_ARG(pIpsecInfo);
	RV_UNUSED_ARGS(ipsecInfo);
	RV_UNUSED_ARG(minStructSize);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeStartIpsecLifetimeTimer
 * ------------------------------------------------------------------------
 * General: Starts a lifetime timer for the ipsec-3gpp channels. The expiration
 *          of this timer invokes the RvSipSecAgreeStatusEv() callback with status
 *          RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED. Within this callback
 *          the application can handle the expiration, for example by terminating
 *          the security-agreement.
 *          Note: This timer expiration is reported to the application. It does not
 *                invoke any action at the security-agreement
 *          Note: If a lifetime timer was already started, calling 
 *                RvSipSecAgreeStartIpsecLifetimeTimer() will restart the timer with
 *                according to the given lifetime interval
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree        - Handle to the security-agreement object
 *          lifetimeInterval - The lifetime interval in seconds
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeStartIpsecLifetimeTimer(
                                    IN RvSipSecAgreeHandle    hSecAgree,
								    IN RvUint32               lifetimeInterval)
{
	SecAgree                *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus                 rv             = RV_OK;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (lifetimeInterval == 0)
	{
		return RV_ERROR_BADPARAM;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	if (SecAgreeStateIsValid(pSecAgree) == RV_FALSE)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeStartIpsecLifetimeTimer - Security-agreement 0x%p: calling this function is illegal in state %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (pSecAgree->securityData.hSecObj == NULL)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeStartIpsecLifetimeTimer - Security-agreement 0x%p: calling this function is illegal in the current settings (no security-object)",
					pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SipSecObjSetLifetimeTimer(pSecAgree->securityData.hSecObj, lifetimeInterval);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeStartIpsecLifetimeTimer - Security-agreement 0x%p: failed in SipSecObjSetLifetimeTimer(SecObj=0x%p)",
					pSecAgree, pSecAgree->securityData.hSecObj));
	}
	
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeStartIpsecLifetimeTimer - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp",
			   pSecAgree, pSecAgree->securityData.hSecObj));
	RV_UNUSED_ARG(lifetimeInterval);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetRemainingIpsecLifetime
 * ------------------------------------------------------------------------
 * General: Returns the time remaining until the lifetime timer will expire.
 *          The lifetime timer was started by a call to
 *          RvSipSecAgreeStartIpsecLifetimeTimer()
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree      - Handle to the security-agreement object
 * Output:  pRemainingTime - The remaining time in seconds. 
 *							 UNDEFINED if the timer is not started
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemainingIpsecLifetime(
                                    IN   RvSipSecAgreeHandle    hSecAgree,
									OUT  RvInt32*				pRemainingTime)
{
	SecAgree                *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus                 rv             = RV_OK;

	if (hSecAgree == NULL || pRemainingTime == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	if (SecAgreeStateIsActive(pSecAgree) == RV_FALSE)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetRemainingIpsecLifetime - Security-agreement 0x%p: calling this function is illegal in state %s",
					pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if (pSecAgree->securityData.hSecObj == NULL)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetRemainingIpsecLifetime - Security-agreement 0x%p: calling this function is illegal in the current settings (no security-object)",
					pSecAgree));
		SecAgreeUnLockAPI(pSecAgree);
		return RV_ERROR_ILLEGAL_ACTION;
	}

	rv = SipSecObjGetRemainingLifetime(pSecAgree->securityData.hSecObj, pRemainingTime);
	if (RV_OK != rv)
	{
		RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
					"RvSipSecAgreeGetRemainingIpsecLifetime - Security-agreement 0x%p: failed in SipSecObjGetRemainingLifetime(SecObj=0x%p)",
					pSecAgree, pSecAgree->securityData.hSecObj));
	}
	
#else /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeGetRemainingIpsecLifetime - Security-agreement 0x%p: calling this function is illegal when configured without ipsec-3gpp",
			   pSecAgree, pSecAgree->securityData.hSecObj));
	RV_UNUSED_ARG(pRemainingTime);
	rv = RV_ERROR_ILLEGAL_ACTION;

#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pSecAgree);
#endif

	return rv;
}

/*--------- S P E C I A L   S T A N D A R D S   F U N C T I O N S --------*/

/***************************************************************************
 * RvSipSecAgreeSetSpecialStandardFlag
 * ------------------------------------------------------------------------
 * General: Sets an indication to the security-agreement that it shall support
 *          the given special standard. 
 *          Note: This may imply changes in the behavior of the security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eStandard    - The standard enumeration
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetSpecialStandardFlag(
                                    IN   RvSipSecAgreeHandle               hSecAgree,
                                    IN   RvSipSecAgreeSpecialStandardType  eStandard)
{
	SecAgree    *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus     rv             = RV_OK;

	if (hSecAgree == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	pSecAgree->eSpecialStandard = eStandard;
	
	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			   "RvSipSecAgreeSetSpecialStandardFlag - Security-agreement 0x%p: special standard supported - %s",
			   pSecAgree, SecAgreeUtilsSpecialStandardEnum2String(eStandard)));

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetSpecialStandardFlag
 * ------------------------------------------------------------------------
 * General: Gets the special standard that was previously set to this 
 *          security-agreement via RvSipSecAgreeSetSpecialStandardFlag
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 * Output:    peStandard   - The standard enumeration
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSpecialStandardFlag(
                                    IN  RvSipSecAgreeHandle                hSecAgree,
                                    OUT RvSipSecAgreeSpecialStandardType  *peStandard)
{
	SecAgree    *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus     rv             = RV_OK;

	if (hSecAgree == NULL || peStandard == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	*peStandard = pSecAgree->eSpecialStandard;

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}

/***************************************************************************
 * RvSipSecAgreeGetSecurityAssociationType
 * ------------------------------------------------------------------------
 * General: Gets the type of the Security Associations held by this
 *          security-agreement. The type of Security Association is defined
 *          in TISPAN ES-283.003
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree        - Handle to the security-agreement object
 * Input:     peSecAssocType   - The Security Association type
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSecurityAssociationType(
                                    IN  RvSipSecAgreeHandle                    hSecAgree,
                                    OUT RvSipSecAgreeSecurityAssociationType  *peSecAssocType)
{
	SecAgree    *pSecAgree      = (SecAgree*)hSecAgree;
	RvStatus     rv             = RV_OK;

	if (hSecAgree == NULL || peSecAssocType == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	/* lock the security-agreement */
	rv = SecAgreeLockAPI(pSecAgree);
	if(rv != RV_OK)
	{
	    return RV_ERROR_INVALID_HANDLE;
	}

	*peSecAssocType = pSecAgree->eSecAssocType;

	/* unlock the security-agreement object */
	SecAgreeUnLockAPI(pSecAgree);

	return rv;
}


/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeGetLocalSecurityType
 * ------------------------------------------------------------------------
 * General: Returns the local security type of this security-agreement:
 *          client or server
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgree    - Pointer to the security-agreement object
 ***************************************************************************/
static RvSipSecurityHeaderType RVCALLCONV SecAgreeGetLocalSecurityType(
                                      IN   SecAgree*         pSecAgree)
{
	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
	{
		return RVSIP_SECURITY_SERVER_HEADER;
	}
	else /* client */
	{
		return RVSIP_SECURITY_CLIENT_HEADER;
	}
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "SecurityObject.h"

#include "rvexternal.h"

int RvSipSecAgreeIsReady(RvSipSecAgreeHandle sa, int resp) {

  int ret=0;

  SecAgree *sao=(SecAgree*)sa;
  if(sao && sao->eState==RVSIP_SEC_AGREE_STATE_ACTIVE) {
    ret=RvSipSecObjIsReady(sao->securityData.hSecObj,resp);
  }
  
  return ret;
}

int RvSipSecObjIsReady(RvSipSecObjHandle soh, int resp) {

  int ret=0;

  if(soh) {
    SecObj* so=(SecObj*)soh;
    if(so) {
      if(so->eState==RVSIP_SECOBJ_STATE_ACTIVE) {
        RvSipTransport tt=so->pIpsecSession->eTransportType;
        if(tt==RVSIP_TRANSPORT_TCP) {
          if (resp) {
            if(so->pIpsecSession->hConnS) {
              if(so->pIpsecSession->hLocalAddrTcpS) {
                ret=1;
              } else {
                //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
              }
            } else {
              //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
            }
          } else {
            if(so->pIpsecSession->hConnC) {
              if(so->pIpsecSession->hLocalAddrTcpC) {
                ret=1;
              } else {
                //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
              }
            } else {
              //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
            }
          }
        } else {
          ret=1;
        }
      } else {
        //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
      }
    } else {
      //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
    }
  } else {
    //dprintf("%s:%d\n",__FUNCTION__,__LINE__);
  }
  
  return ret;
}

RvStatus RvSipSecAgreeResetIpsecAddr(RvSipSecAgreeHandle sa) {
  
  RvStatus rv=RV_OK;
  
  if(sa) {
    SecAgree *sao=(SecAgree*)sa;
    SecObj *so=(SecObj*)(sao->securityData.hSecObj);
    if(so) {
      rv=ResetIpsecAddr(so);
    }
  }
  
  return rv;
}

#endif
/* SPIRENT_END */

#endif /* #ifdef RV_SIP_IMS_ON */


