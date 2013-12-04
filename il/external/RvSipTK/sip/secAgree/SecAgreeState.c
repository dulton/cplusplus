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
 *                              <SecAgreeState.c>
 *
 * This file manages the security-agreement state machines: defines the states allowed
 * for actions, the change in states, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeState.h"
#include "SecAgreeUtils.h"
#include "SecAgreeCallbacks.h"
#include "_SipTransport.h"
#include "SecAgreeSecurityData.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV SecAgreeStateChange(
									IN  SecAgree*            pSecAgree,
							 		IN  SecAgreeProcessing   eProcessing,
									IN  RvSipSecAgreeState   eOldState,
									IN  RvSipSecAgreeState   eNewState,
									IN  RvSipSecAgreeReason  eReason,
									IN  RvBool               bSecAgreeOptionTagRcvd);

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeStateCanAttachOwner
 * ------------------------------------------------------------------------
 * General: Check if the current state allows attaching a new owner.
 * Return Value: Boolean indication to attach owner
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateCanAttachOwner(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateCanAttachSeveralOwners
 * ------------------------------------------------------------------------
 * General: Check if the current state allows several owners to be attached
 *          to the same security-agreement (steady states).
 * Return Value: Boolean indication to attach several owners
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateCanAttachSeveralOwners(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateCanModifyLocalSecurityList
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for modifying the local security list, hence legal for calling
 *          Init()
 * Return Value: Boolean indication to allow changing local security list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateCanModifyLocalSecurityList(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldProcessRcvdMsg
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement requires
 *          processing of received messages
 * Return Value: Boolean indication to process received messages
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldProcessRcvdMsg(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldProcessMsgToSend
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement requires
 *          processing of received messages
 * Return Value: Boolean indication to process messages to send
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldProcessMsgToSend(IN  SecAgree*        pSecAgree)
{
	if (pSecAgree == NULL)
	{
		/* no security-agreement object indicates the application does not wish to negotiate */
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY)
	{
		return RV_TRUE;
	}

	if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT &&
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		/* special for clients: add security-verify list and possibly security-client lists
		   to outgoing requests */
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldProcessDestResolved
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement requires
 *          processing of destination resolved
 * Return Value: Boolean indication to process destination resolved
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldProcessDestResolved(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL)
	{
		/* no security-agreement object indicates that the application does not wish to negotiate */
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateCanLoadRemoteSecList
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for receiving a remote security-list in a message.
 * Return Value: Boolean indication to load remote security list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateCanLoadRemoteSecList(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL)
	{
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateClientShouldLoadRemoteSecList
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for loading remote security list.
 * Return Value: Boolean indication to load remote security list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateClientShouldLoadRemoteSecList(
												IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL)
	{
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldCompareRemoteSecList
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for comparing received remote security list to previous one that
 *          was copied to the object.
 * Return Value: Boolean indication to compare remote security list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldCompareRemoteSecList(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL)
	{
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldCompareLocalSecList
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for comparing received security list to the local list (for server).
 * Return Value: Boolean indication to compare local security list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 * Output: pbIsTispan - Indicates whether this is a TISPAN comparison. If it is,
 *                      compares the old list of local security to the received
 *                      Security-Verify list. If it is not, compares the local
 *                      security list that was set to this security agreement 
 *                      to the received Security-Verify list.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldCompareLocalSecList(IN     SecAgree*     pSecAgree,
														 OUT    RvBool       *pbIsTispan)
{
	if (pbIsTispan != NULL)
	{
		*pbIsTispan = RV_FALSE;
	}
	
	if (pSecAgree == NULL)
	{
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		return RV_TRUE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_READY)
	{
		if (SecAgreeUtilsIsGmInterfaceSupport(pSecAgree) &&
			pSecAgree->securityData.hTISPANLocalList != NULL)
		{
			if (pbIsTispan != NULL)
			{
				*pbIsTispan = RV_TRUE;
			}
			return RV_TRUE;
		}
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateCanChooseSecurity
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement is legal
 *          for choosing a security mechanism
 * Return Value: Boolean indication to allow choosing security
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateCanChooseSecurity(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldInitiateSecObj
 * ------------------------------------------------------------------------
 * General: Check if the current state and processing require
 *          initiating security object
 * Return Value: Boolean indication to allow initiating security object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object.
 *         eProcessing  - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldInitiateSecObj(IN  SecAgree*            pSecAgree,
												    IN  SecAgreeProcessing   eProcessing)
{
	if (eProcessing == SEC_AGREE_PROCESSING_INIT &&
		(pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		 pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY ||
		 pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY))
	{
		return RV_TRUE;
	}

	if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY &&
		(pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY ||
		 pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY))
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateClientIsSpecialChoosing
 * ------------------------------------------------------------------------
 * General: Indicates whether the client is choosing the security mechanism
 *          to use without having initialized the local list.
 * Return Value: Boolean indication it is a special initialization
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object.
 *         eProcessing  - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateClientIsSpecialChoosing(IN  SecAgree*            pSecAgree,
												       IN  SecAgreeProcessing   eProcessing)
{
	if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY &&
		(pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY))
	{
		/* We choose only based on the remote list since there is no local list */
		return RV_TRUE;
	}

	if (pSecAgree->securityData.selectedSecurity.eType == RVSIP_SECURITY_MECHANISM_TYPE_TLS)
	{
		/* Tls was already chosen: used when the local list did not contain TLS and the server list did */
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateShouldActivateSecObj
 * ------------------------------------------------------------------------
 * General: Check if the current state of this security-agreement requires
 *          activating the security object 
 * Return Value: Boolean indication to allow activating security object
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree    - Pointer to the security-agreement object.
 *         eProcessing  - Indicates the event being processed (message to send,
 *                        message received, etc)
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldActivateSecObj(IN  SecAgree*            pSecAgree,
												    IN  SecAgreeProcessing   eProcessing)
{
	if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY && 
		(pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY ||
		 pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY))
	{
		return RV_TRUE;
	}

	if (eProcessing == SEC_AGREE_PROCESSING_INIT && 
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY)
	{
		return RV_TRUE;
	}

	if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD && 
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY)
	{
		return RV_TRUE;
	}


	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateServerConsiderNoAgreementTls
 * ------------------------------------------------------------------------
 * General: Check if the state should change to NO_AGREEMENT_TLS if a message
 *          was received in TLS.
 * Return Value: Boolean indication to whether the state can change to
 *               NO_AGREEMENT_TLS
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateServerConsiderNoAgreementTls(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL ||
		(pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER &&
		 (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		  pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY)))
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateIsClientChooseSecurity
 * ------------------------------------------------------------------------
 * General: Check if the state of this security-agreement is CLIENT_CHOOSE_SECURITY
 * Return Value: Boolean indication to whether the state is CLIENT_CHOOSE_SECURITY
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsClientChooseSecurity(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateIsActive
 * ------------------------------------------------------------------------
 * General: Check if the state of this security-agreement is ACTIVE
 * Return Value: Boolean indication to whether the state is ACTIVE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsActive(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_ACTIVE)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeStateIsValid
 * ------------------------------------------------------------------------
 * General: Check if there current state is valid
 * Return Value: Boolean indication to whether the current state is valid.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsValid(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree == NULL)
	{
		return RV_FALSE;
	}

	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_UNDEFINED ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_INVALID ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_TERMINATED)
	{
		return RV_FALSE;
	}

	return RV_TRUE;
}

/***************************************************************************
 * SecAgreeStateProgress
 * ------------------------------------------------------------------------
 * General: Move to the next state of this security-agreement based on the
 *          last actions taken
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree      - Pointer to the security-agreement object.
 *         eProcessing    - Indicates the event being processed (message to send,
 *                          message received, etc)
 *         hSecObj        - The security object involved in the event being processed (if relevant)
 *         eTransportType - Indicates the transport type of the message that
 *                          caused the change in state (if relevant)
 *         bRemoteSecurityRcvd - Indicates whether remote security list was
 *                               received in the message (if relevant)
 *         bLocalSecurityRcvd  - Indicates whether local security list was
 *                               received in the message (if relevant)
 *         bSecAgreeOptionTagRcvd - Indicates whether Require or Supported headers
 *									with sec-agree were received in the message (if relevant)
 *************************************************************************/
RvStatus RVCALLCONV SecAgreeStateProgress(IN  SecAgree*          pSecAgree,
										  IN  SecAgreeProcessing eProcessing,
										  IN  RvSipSecObjHandle  hSecObj,
										  IN  RvSipTransport     eTransportType,
										  IN  RvBool             bRemoteSecurityRcvd,
										  IN  RvBool             bLocalSecurityRcvd,
										  IN  RvBool             bSecAgreeOptionTagRcvd)
{
	RvStatus rv = RV_OK;
	switch (pSecAgree->eState) {
	case RVSIP_SEC_AGREE_STATE_IDLE:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD &&
				bRemoteSecurityRcvd == RV_TRUE)
			{
				if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
				{
					/* The client received a Security-Server list without being initialized.
					   State changes to CLIENT_UNINITIALIZED_RCVD_SERVER_LIST */
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
				}
				else /* RVSIP_SEC_AGREE_ROLE_SERVER */
				{
					/* The server received a Security-Client list without being initialized.
					   State changes to SERVER_UNINITIALIZED_RCVD_CLIENT_LIST */
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);

				}
			}
			else if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD &&
					 eTransportType == RVSIP_TRANSPORT_TLS)
			{
				if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_SERVER)
				{
					/* The server received a request on TLS. State changes to NO_AGREEMENT_TLS*/
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
				}
			}
			else if (eProcessing == SEC_AGREE_PROCESSING_INIT)
			{
				if (pSecAgree->eRole == RVSIP_SEC_AGREE_ROLE_CLIENT)
				{
					/* The client initialized local security information */
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
				}
				else /* RVSIP_SEC_AGREE_ROLE_SERVER */
				{
					/* The server initialized local security information */
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);

				}
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD)
			{
				if (bRemoteSecurityRcvd == RV_TRUE)
				{
					/* The client received Security-Server list. State changes to CHOOSE_SECURITY */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY,
										RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD,
										bSecAgreeOptionTagRcvd);
				}
			}
			else if (eProcessing == SEC_AGREE_PROCESSING_MSG_TO_SEND)
			{
				if (hSecObj == NULL)
				{
					/* The client list was inserted to a message. We change state to
					   HANDLING_INITIAL_MSG until the destination is resolved. */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG,
										RVSIP_SEC_AGREE_REASON_UNDEFINED,
										bSecAgreeOptionTagRcvd);
				}
				else
				{
					/* The message will be sent via security object, hence no address resolution
					   will occur. The security-agreement adds security information to the message
					   even if it is sent via TLS, because the security object is a result of an 
					   agreement. Skip states to get to LOCAL_LIST_SENT */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT,
										RVSIP_SEC_AGREE_REASON_UNDEFINED,
										bSecAgreeOptionTagRcvd);
				}
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_DEST_RESOLVED)
			{
				/* The destination was resolved */
				if (eTransportType == RVSIP_TRANSPORT_TLS)
				{
					/* Since transport is TLS we change state to NO_AGREEMENT_TLS */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
				 						RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED,
										RVSIP_SEC_AGREE_REASON_UNDEFINED,
										bSecAgreeOptionTagRcvd);
				}
				else
				{
					/* Since transport is not TLS the security list was sent and the state
					   changes to CLIENT_LIST_SENT */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT,
										RVSIP_SEC_AGREE_REASON_UNDEFINED,
										bSecAgreeOptionTagRcvd);
				}
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD)
			{
				if (bRemoteSecurityRcvd == RV_TRUE)
				{
					/* The client received Security-Server list. State changes to CHOOSE_SECURITY */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY,
										RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD,
										bSecAgreeOptionTagRcvd);
				}
				else
				{
					/* The client did not receive a Security-Server list even though it wanted to negotiate an agreement */
					RvLogWarning(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
							     "SecAgreeStateProgress - client security-agreement=0x%p: received response without Security-Server list in state %s",
							     pSecAgree, SecAgreeUtilsStateEnum2String(pSecAgree->eState)));
				}
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_INIT)
			{
				/* The client initialized local security information */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY,
									RVSIP_SEC_AGREE_REASON_USER_INIT,
									bSecAgreeOptionTagRcvd);
			}
			else if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY)
			{
				/* The application used SetChosenSecurity() instead of Init().
				   We reset the local security list */
				pSecAgree->securityData.hLocalSecurityList = NULL;
				/* The client set chosen security */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_ACTIVE,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_SET_SECURITY)
			{
				/* The client initialized local security information */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_ACTIVE,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
			}
            break;
		}
	case RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_TO_SEND)
			{
				/* The server list was inserted to a message. We change state to ACTIVE. */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_ACTIVE,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
			}
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD)
			{
				if (bRemoteSecurityRcvd == RV_TRUE)
				{
					/* The server received a Security-Client list.
					   State changes to SERVER_AND_CLIENT_LISTS_READY_TO_RESPOND */
					rv = SecAgreeStateChange(pSecAgree,
										eProcessing,
										pSecAgree->eState,
										RVSIP_SEC_AGREE_STATE_SERVER_READY,
										RVSIP_SEC_AGREE_REASON_UNDEFINED,
										bSecAgreeOptionTagRcvd);
				}
				else if (eTransportType == RVSIP_TRANSPORT_TLS)
				{
					/* The server received a request on TLS. State changes to NO_AGREEMENT_TLS*/
					rv = SecAgreeStateChange(
									pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
				}
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_SERVER_READY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_MSG_TO_SEND)
			{
				/* The server list was inserted to a message. We change state to ACTIVE. */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_ACTIVE,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
			}
			break;
		}
	case RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY:
		{
			if (eProcessing == SEC_AGREE_PROCESSING_INIT)
			{
				/* The server initialized local security information */
				rv = SecAgreeStateChange(pSecAgree,
									eProcessing,
									pSecAgree->eState,
									RVSIP_SEC_AGREE_STATE_SERVER_READY,
									RVSIP_SEC_AGREE_REASON_UNDEFINED,
									bSecAgreeOptionTagRcvd);
			}
			break;
		}
	default:
		return RV_OK;
	}
	return rv;

	RV_UNUSED_ARG(bLocalSecurityRcvd)
}

/***************************************************************************
 * SecAgreeStateInvalidate
 * ------------------------------------------------------------------------
 * General: Change the state of this security-agreement to INVALID
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 *         eReason    - The reason for the change in state.
 ***************************************************************************/
void RVCALLCONV SecAgreeStateInvalidate(IN   SecAgree*           pSecAgree,
							 	        IN   RvSipSecAgreeReason eReason)
{
	if (pSecAgree == NULL ||
		SecAgreeStateIsValid(pSecAgree) == RV_FALSE)
	{
		/* The security-agreement is already invalid */
		return;
	}

	/* Reset the Security Association type to undefined */
	pSecAgree->eSecAssocType = RVSIP_SEC_AGREE_SEC_ASSOC_UNDEFINED;
	
	SecAgreeStateChange(pSecAgree,
						SEC_AGREE_PROCESSING_UNDEFINED, /* irrelevant */
						pSecAgree->eState,
						RVSIP_SEC_AGREE_STATE_INVALID,
						eReason,
						RV_FALSE /* irrelevant */);
}

/***************************************************************************
 * SecAgreeStateTerminate
 * ------------------------------------------------------------------------
 * General: Change the state of this security-agreement to TERMINATED
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 *         eOldState  - The old state that is about to be replaced with terminated
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStateTerminate(IN   SecAgree*           pSecAgree,
                                           IN   RvSipSecAgreeState  eOldState)
{
	return SecAgreeStateChange(pSecAgree,
								SEC_AGREE_PROCESSING_UNDEFINED, /* irrelevant */
								eOldState,
								RVSIP_SEC_AGREE_STATE_TERMINATED,
								RVSIP_SEC_AGREE_REASON_UNDEFINED,
								RV_FALSE /* irrelevant */);

}

/***************************************************************************
 * SecAgreeStateReturnToClientLocalReady
 * ------------------------------------------------------------------------
 * General: Change the state of this client security-agreement to LOCAL_LIST_READY
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 *         eOldState  - The old state that is about to be replaced with terminated
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStateReturnToClientLocalReady(
										   IN   SecAgree*           pSecAgree,
										   IN   RvSipSecAgreeState  eOldState)
{
	return SecAgreeStateChange(pSecAgree,
								SEC_AGREE_PROCESSING_UNDEFINED, /* irrelevant */
								eOldState,
								RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY,
								RVSIP_SEC_AGREE_REASON_UNDEFINED,
								RV_FALSE /* irrelevant */);
}

/***************************************************************************
 * SecAgreeStateReturnToActive
 * ------------------------------------------------------------------------
 * General: Change the state of this server security-agreement to ACTIVE
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 *         eOldState  - The old state that is about to be replaced with terminated
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStateReturnToActive(
										   IN   SecAgree*           pSecAgree,
										   IN   RvSipSecAgreeState  eOldState)
{
	return SecAgreeStateChange(pSecAgree,
								SEC_AGREE_PROCESSING_UNDEFINED, /* irrelevant */
								eOldState,
								RVSIP_SEC_AGREE_STATE_ACTIVE,
								RVSIP_SEC_AGREE_REASON_UNDEFINED,
								RV_FALSE /* irrelevant */);
}

/***************************************************************************
 * SecAgreeShouldLockList
 * ------------------------------------------------------------------------
 * General: Checks whether locking the remote list is required. It is required
 *          only when the state allows changing the list.
 * Return Value: Boolean indication to lock list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldLockList(IN     SecAgree*     pSecAgree)
{
	if (pSecAgree->eState == RVSIP_SEC_AGREE_STATE_IDLE ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY ||
		pSecAgree->eState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeStateChange
 * ------------------------------------------------------------------------
 * General: Modifies security-agreement object due to change in state, and calls
 *          StateChangedEv to notify the application about the change in state
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree   - Pointer to the security-agreement object.
 *         eProcessing - Indicates the event being processed (message to send,
 *                       message received, etc)
 *         eOldState   - The old state
 *         eNewState   - The new state
 *         eReason     - The reason for the change in state, if needed.
 *         bSecAgreeOptionTagRcvd - Indicates whether Require or Supported headers
 *									with sec-agree were received in the message (if relevant)
 ***************************************************************************/
static RvStatus RVCALLCONV SecAgreeStateChange(
									IN  SecAgree*            pSecAgree,
							 		IN  SecAgreeProcessing   eProcessing,
									IN  RvSipSecAgreeState   eOldState,
									IN  RvSipSecAgreeState   eNewState,
									IN  RvSipSecAgreeReason  eReason,
									IN  RvBool               bSecAgreeOptionTagRcvd)
{
	RvStatus             rv = RV_OK;

	/* Update the new state in the security-agreement object */
	pSecAgree->eState = eNewState;

	/* Set insert Security-Client indication */
	if (eOldState == RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT &&
		eNewState == RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY)
	{
		/* A security-client list was sent and now we received a response that causes the
		   state to change to ACTIVE. We set an indication that in the next request, if
		   the security mechanism selected is ipsec, we need to add Security-Client list
		   to the request (TS33.203)*/
		pSecAgree->securityData.eInsertSecClient = SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC;
	}

	/* Find recommended response code */
	if (eProcessing == SEC_AGREE_PROCESSING_MSG_RCVD)
	{
		if (eNewState == RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY)
		{
			/* A Security-Client list was received in the message. The response recommended by
			   RFC3329 is 494 */
			pSecAgree->responseCode = RESPONSE_494;
		}
		else if (eOldState == RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY &&
				 eNewState == RVSIP_SEC_AGREE_STATE_SERVER_READY)
		{
			/* If there were Require or Supported with sec-agree, or Security-Client list
			   in the message, the client supports sec-agree hence RFC3329 recommend to respond with 494 */
			if (bSecAgreeOptionTagRcvd == RV_TRUE ||
				pSecAgree->securityData.hRemoteSecurityList != NULL)
			{
				pSecAgree->responseCode = RESPONSE_494;
			}
			else
			{
				pSecAgree->responseCode = RESPONSE_421;
			}
		}
	}

	/* Update recommended response code */
	if (eNewState == RVSIP_SEC_AGREE_STATE_ACTIVE ||
	    eNewState == RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED ||
	    eNewState == RVSIP_SEC_AGREE_STATE_INVALID ||
	    eNewState == RVSIP_SEC_AGREE_STATE_TERMINATED)
	{
		/* from now on we will not recommend a response code */
		pSecAgree->responseCode = 0;
		/* from now on we will not insert/load remote security list to/from responses */
		pSecAgree->securityData.cseqStep = 0;
		/* Reset security-data stored for TISPN if exists */
		SecAgreeSecurityDataResetData(&pSecAgree->securityData, pSecAgree->eRole, SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL);
	}

	RvLogInfo(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
			  "SecAgreeStateChange - Security-agreement 0x%p state changed from %s to %s. Reason is %s",
			  pSecAgree, SecAgreeUtilsStateEnum2String(eOldState), SecAgreeUtilsStateEnum2String(eNewState), SecAgreeUtilsReasonEnum2String(eReason)));

	/*insert the state changed notification to the event queue*/
    if(eNewState != RVSIP_SEC_AGREE_STATE_TERMINATED)
    {
        /* a state event will not have an Out-of-resource recovery */
        rv = SipTransportSendStateObjectEvent(pSecAgree->pSecAgreeMgr->hTransport,
                                        (void*)pSecAgree,
                                        pSecAgree->secAgreeUniqueIdentifier,
                                        (RvInt32)eNewState,
									     (RvInt32)eReason,
                                         SecAgreeCallbacksStateChangedEvHandler,
                                         RV_TRUE,
                                         SEC_AGREE_STATE_CHANGED_STR,
                                         SEC_AGREE_OBJECT_NAME_STR);
		if (rv != RV_OK)
		{
			RvLogError(pSecAgree->pSecAgreeMgr->pLogSrc,(pSecAgree->pSecAgreeMgr->pLogSrc,
						"SecAgreeStateChange - Security-agreement 0x%p: Failed to insert event state changed to the events queue, sec-agree will be terminated.",
						pSecAgree));

			return SecAgreeStateTerminate(pSecAgree, pSecAgree->eState);
		}
    }
	else
    {
		/* object termination must have an Out-of-resource recovery */
        SipTransportSendTerminationObjectEvent(pSecAgree->pSecAgreeMgr->hTransport,
          (void*)pSecAgree,
          &pSecAgree->terminationInfo,
          (RvInt32)eReason,
          SecAgreeCallbacksStateTerminated,
          RV_TRUE,
          SEC_AGREE_STATE_TERMINATED_STR,
          SEC_AGREE_OBJECT_NAME_STR);
    }
	

	return rv;
}

#endif /* #ifdef RV_SIP_IMS_ON */

