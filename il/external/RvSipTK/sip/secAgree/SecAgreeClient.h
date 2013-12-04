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
 *                              <SecAgreeClientObject.h>
 *
 * This file implement the client security-agreement behavior, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

#ifndef SEC_AGREE_CLIENT_OBJECT_H
#define SEC_AGREE_CLIENT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
#include "RvSipMsg.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipTransactionTypes.h"
#include "_SipTransportTypes.h"

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
 * SecAgreeClientInit
 * ------------------------------------------------------------------------
 * General: Used in states IDLE and CLIENT_REMOTE_LIST_READY,
 *          to indicate that the application has completed to initialize the local 
 *          security list; and local digest list for server security-agreement. Once 
 *          called, the application can no longer modify the local security information. 
 *          Calling RvSipSecAgreeInit() modifies the state of the security-agreement 
 *          object to allow proceeding in the security-agreement process. It will also 
 *          cause a client security-agreement object to choose the security-mechanism 
 *          to use. Calling this function will invoke the creation and initialization of 
 *          a security object according to the supplied parameters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient   - Pointer to the client security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientInit(IN   SecAgree*    pSecAgreeClient);

/***************************************************************************
 * SecAgreeClientHandleMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message about to be sent: adds security information
 *          and updates the client security-agreement object accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 *         hSecObj         - Handle to the security object of the SIP object
 *                           sending the message
 *         hMsg            - Handle to the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgToSend(
							 IN  SecAgree                      *pSecAgreeClient,
							 IN  RvSipSecObjHandle              hSecObj,
							 IN  RvSipMsgHandle                 hMsg);

/***************************************************************************
 * SecAgreeClientHandleDestResolved
 * ------------------------------------------------------------------------
 * General: Performs last state changes and message changes based on the
 *          transport type that is obtained via destination resolution.
 *          If this is TLS on initial state, we abort negotiation attempt by
 *          removing all negotiation info from the message, and changing state
 *          to NO_AGREEMENT_TLS. If this is not TLS we change state to
 *          CLIENT_LOCAL_LIST_SENT.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 *         hMsg            - Handle to the message
 *         hTrx            - The transmitter that notified destination resolution
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleDestResolved(
							 IN  SecAgree*                     pSecAgreeClient,
							 IN  RvSipMsgHandle                hMsg,
							 IN  RvSipTransmitterHandle        hTrx);

/***************************************************************************
 * SecAgreeClientHandleMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Invalidates a security-agreement whenever a message send failure
 *          occurs.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient - Pointer to the client security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgSendFailure(
							 IN  SecAgree                      *pSecAgreeClient);

/***************************************************************************
 * SecAgreeClientHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : loads and stores security information,
 *          and change client security-agreement state accordingly
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the Security-Mgr
 *         pOwner           - Owner details to supply to the application
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 *         hMsg             - Handle to the message
 * Inout:  ppSecAgreeClient - Pointer to the security-agreement object. If the
 *                            security-agreement object is replaced, the new 
 *                            security-agreement is updated here
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientHandleMsgRcvd(
							 IN    SecAgreeMgr*                  pSecAgreeMgr,
							 IN    RvSipSecAgreeOwner*           pOwner,
							 IN    SipTripleLock*				 pOwnerLock,
							 IN    RvSipMsgHandle                hMsg,
							 INOUT SecAgree**                    ppSecAgreeClient);

/***************************************************************************
 * SecAgreeClientChooseMechanism
 * ------------------------------------------------------------------------
 * General: Compares the local and remote lists and chooses the best security
 *          mechanism
 *          Notice: we choose from the list of supported mechanisms only.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgreeClient        - Pointer to the client security-agreement object
 * Output:    peSelectedMechanism    - Enumeration of the selected mechanism
 *            phSelectedLocalHeader  - Pointer to the selected local security header
 *            phSelectedRemoteHeader - Pointer to the selected local security header
 *            pbIsValid              - RV_TRUE if the chosen security is valid
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientChooseMechanism(
											IN  SecAgree*                      pSecAgreeClient,
											OUT RvSipSecurityMechanismType*    peSelectedMechanism,
											OUT RvSipSecurityHeaderHandle*     phSelectedLocalHeader,
											OUT RvSipSecurityHeaderHandle*     phSelectedRemoteHeader,
											OUT RvBool*                        pbIsValid);

/***************************************************************************
 * SecAgreeClientCheckSecurity
 * ------------------------------------------------------------------------
 * General: Checks that the chosen security is valid, and that the client has
 *          all the information it needs in order to apply this mechanism
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         eSecurityType      - The type of the chosen security mechanism
 * Output: pbIsValid          - RV_TRUE if the chosen security is valid,
 *                              RV_FALSE otherwise
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeClientCheckSecurity(
							 IN  SecAgree*                  pSecAgreeClient,
							 IN  RvSipSecurityMechanismType eSecurityType,
							 OUT RvBool*					pbIsValid);

/***************************************************************************
 * SecAgreeClientSetChosenSecurity
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
RvStatus RVCALLCONV SecAgreeClientSetChosenSecurity(
							 IN     SecAgree                       *pSecAgree,
							 INOUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT    RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT    RvSipSecurityHeaderHandle      *phRemoteSecurity);

#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_CLIENT_OBJECT_H*/

