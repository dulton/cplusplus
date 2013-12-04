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
 *                              <SecAgreeServerObject.h>
 *
 * This file implement the server security-agreement behavior, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/

#ifndef SEC_AGREE_SERVER_OBJECT_H
#define SEC_AGREE_SERVER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
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
 * SecAgreeServerInit
 * ------------------------------------------------------------------------
 * General: Used in states IDLE and SERVER_REMOTE_LIST_READY,
 *          to indicate that the application has completed to initialize the local 
 *          security list; and local digest list for server security-agreement. Once 
 *          called, the application can no longer modify the local security information. 
 *          Calling RvSipSecAgreeInit() modifies the state of the security-agreement 
 *          object to allow proceeding in the security-agreement process. It will invoke 
 *          the creation and initialization of a security object according to the supplied 
 *          parameters.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer   - Pointer to the server security-agreement object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerInit(IN   SecAgree*    pSecAgreeServer);

/***************************************************************************
 * SecAgreeServerHandleMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message about to be sent: adds security information
 *          and updates the server security-agreement object accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 *         hMsg            - Handle to the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgToSend(
							 IN  SecAgree                *pSecAgreeServer,
							 IN  RvSipMsgHandle           hMsg);

/***************************************************************************
 * SecAgreeServerHandleMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Invalidates a security-agreement whenever a message send failure
 *          occurs.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgSendFailure(
							 IN  SecAgree                      *pSecAgreeServer);

/***************************************************************************
 * SecAgreeServerHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a received message : loads security information, verifies
 *          the security agreement and updates the server security-agreement object 
 *          accordingly.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr     - Pointer to the security-agreement manager
 *         pOwner           - The owner of the server security-agreement
 *         pOwnerLock       - The triple lock of the owner, to open before callback
 *         hMsg             - Handle to the message
 *         hLocalAddr       - The address that the message was received on
 *         pRcvdFromAddr    - The address the message was received from
 *         hRcvdOnConn      - The connection the message was received on
 * Inout:  ppSecAgreeServer - Pointer to the security-agreement object. If the
 *                            security-agreement object is replaced, the new 
 *                            security-agreement is updated here
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerHandleMsgRcvd(
							 IN    SecAgreeMgr* 				  pSecAgreeMgr,
							 IN    RvSipSecAgreeOwner*            pOwner,
							 IN    SipTripleLock*                 pOwnerLock,
							 IN    RvSipMsgHandle                 hMsg,
							 IN    RvSipTransportLocalAddrHandle  hLocalAddr,
							 IN    SipTransportAddress*           pRcvdFromAddr,
							 IN	   RvSipTransportConnectionHandle hRcvdOnConn,
							 INOUT SecAgree** 		              ppSecAgreeServer);

#if (RV_IMS_IPSEC_ENABLED == RV_YES) 
/***************************************************************************
 * SecAgreeServerForceIpsecSecurity
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
 * Input:  pSecAgreeServer - Pointer to the server security-agreement
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeServerForceIpsecSecurity(
                                   IN  SecAgree              *pSecAgreeServer);
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SEC_AGREE_SERVER_OBJECT_H*/

