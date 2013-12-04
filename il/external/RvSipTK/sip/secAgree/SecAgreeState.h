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
 *                              <SecAgreeState.h>
 *
 * This file manages the security-agreement state machines: defines the states allowed
 * for actions, the change in states, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef SEC_AGREE_STATE_H
#define SEC_AGREE_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
    
/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
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
RvBool RVCALLCONV SecAgreeStateCanAttachOwner(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateCanAttachSeveralOwners(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateCanModifyLocalSecurityList(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateShouldProcessRcvdMsg(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateShouldProcessMsgToSend(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateShouldProcessDestResolved(IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateCanLoadRemoteSecList(IN     SecAgree*     pSecAgree);

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
												IN     SecAgree*     pSecAgree);

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
RvBool RVCALLCONV SecAgreeStateShouldCompareRemoteSecList(IN     SecAgree*     pSecAgree);

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
														 OUT    RvBool       *pbIsTispan);

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
RvBool RVCALLCONV SecAgreeStateCanChooseSecurity(IN     SecAgree*     pSecAgree);

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
												    IN  SecAgreeProcessing   eProcessing);

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
												       IN  SecAgreeProcessing   eProcessing);

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
												    IN  SecAgreeProcessing   eProcessing);

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
RvBool RVCALLCONV SecAgreeStateServerConsiderNoAgreementTls(IN     SecAgree*     pSecAgree);

/***************************************************************************
 * SecAgreeStateIsClientChooseSecurity
 * ------------------------------------------------------------------------
 * General: Check if the state of this security-agreement is CLIENT_CHOOSE_SECURITY
 * Return Value: Boolean indication to whether the state is CLIENT_CHOOSE_SECURITY
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsClientChooseSecurity(IN     SecAgree*     pSecAgree);

/***************************************************************************
 * SecAgreeStateIsActive
 * ------------------------------------------------------------------------
 * General: Check if the state of this security-agreement is ACTIVE
 * Return Value: Boolean indication to whether the state is ACTIVE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsActive(IN     SecAgree*     pSecAgree);

/***************************************************************************
 * SecAgreeStateIsValid
 * ------------------------------------------------------------------------
 * General: Check if there current state is valid
 * Return Value: Boolean indication to whether the current state is valid.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateIsValid(IN     SecAgree*     pSecAgree);

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
									  IN  RvBool             bSecAgreeOptionTagRcvd);

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
void RVCALLCONV SecAgreeStateInvalidate(
									IN   SecAgree*           pSecAgree,
							 	    IN   RvSipSecAgreeReason eReason);

/***************************************************************************
 * SecAgreeStateTerminate
 * ------------------------------------------------------------------------
 * General: Change the state of this security-agreement to TERMINATED
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 *         eOldState  - The old state that is about to be replaced with terminated
 *         eReason    - The reason for the change in state.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeStateTerminate(IN   SecAgree*           pSecAgree,
                                           IN   RvSipSecAgreeState  eOldState);

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
										   IN   RvSipSecAgreeState  eOldState);

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
										   IN   RvSipSecAgreeState  eOldState);

/***************************************************************************
 * SecAgreeStateShouldLockList
 * ------------------------------------------------------------------------
 * General: Checks whether locking the remote list is required. It is required
 *          only when the state allows changing the list.
 * Return Value: Boolean indication to lock list
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgree  - Pointer to the security-agreement object.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeStateShouldLockList(IN     SecAgree*     pSecAgree);

#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_STATE_H*/

