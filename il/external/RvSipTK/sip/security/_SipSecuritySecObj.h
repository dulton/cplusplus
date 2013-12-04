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
 *                              <_SipSecuritySecObj.h>
 *
 * The _SipSecurity.h file contains Internal API functions of Security Object.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/


#ifndef SIP_SECOBJ_H
#define SIP_SECOBJ_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "_SipSecurityTypes.h"
#include "_SipTransportTypes.h"
#include "RvSipTransportTypes.h"

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            MODULE FUNCTIONS                               */
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
                        IN  RvInt32             secMechsToInitiate);

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
                            IN  void*                       pOwner);

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
            IN  RvSipTransportConnectionStateChangedReason eReason);

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
 *                          the Message will be discraded.
 *****************************************************************************/
void RVCALLCONV SipSecObjMsgReceivedEv(
                                       IN  RvSipMsgHandle       hMsg,
                                       IN  SipTransportAddress* pRecvFromAddr,
                                       IN  void*                pSecOwner,
                                       OUT RvBool*              pbApproved);

/******************************************************************************
 * SipSecObjMsgSendFailureEv
 * ----------------------------------------------------------------------------
 * General: This is a callback function implementation called by the transport
 *          layer when ever a protected message sending fails.
 *          The function notifys the owners about this.
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
                                IN void*                          pSecOwner);

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SipSecObjLifetimeTimerHandler
 * ----------------------------------------------------------------------------
 * General: This is a callback function implementation called by the system on
 *          lifetime timer expiration.
 * Return Value: not in use.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   ctx    - Handle to Security Object
 *          pTimer - Pointer to the timer, which expired
 *****************************************************************************/
RvBool SipSecObjLifetimeTimerHandler(IN void *ctx,IN RvTimer *pTimer);

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
								   IN RvUint32           lifetimeInterval);

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
									   OUT RvInt32*           pRemainingTime);
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

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
												IN RvInt32  objUniqueIdentifier);

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
                                                IN RvInt32  reason);

/******************************************************************************
 * SipSecObjGetSecureLocalAddressAndConnection
 * ----------------------------------------------------------------------------
 * General: gets handle to the secure Local Address and handle to the Secure
 *          Connection (if TCP or TLS transport is used by Security Object),
 *          that can be used for protected message sending.
 *          The Security Object should be in ACTIVE or RENEGOTIATED state.
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
					IN      RvBool                          bIsConsecutive);

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
                            OUT  RvUint16*          pSecureServerPort);

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
                        IN RvSipSecurityMechanismType eSecMechanism);

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
                        IN RvSipSecMgrHandle           hSecMgr,
                        IN RvSipSecObjHandle           hSecObj,
                        IN RvSipSecurityMechanismType* peSecMechanism);

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
                        IN SipSecTlsParams*   pTlsParams);

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
                        OUT SipSecTlsParams*  pTlsParams);

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
                        IN  RvSipTransportConnectionHandle  hConn);

/******************************************************************************
 * RvSipSecObjGetTlsConnection
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
                        OUT RvSipTransportConnectionHandle* phConn);

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
                        IN  RvSipTransportLocalAddrHandle   hLocalAddrTcp);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SipSecObjIpsecSetLocalTransport(IN  RvSipSecObjHandle               hSecObj,
                                                    IN RvSipTransport eTransportType);
#endif
//SPIRENT_END

/******************************************************************************
 * SipSecObjIpsecGetPortSLocalAddresses
 * ----------------------------------------------------------------------------
 * General: Gets Security Object's Local Addresses, bound to the Port-S.
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
                        OUT RvSipTransportLocalAddrHandle*  phLocalAddrTcp);

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
                        OUT RvSipSecObjState*    peState);

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
                        IN void*             impi);

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
						IN RvSipSecLinkCfg             *pLinkCfg);
#endif /* #ifdef RV_SIGCOMP_ON */

#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIP_SECOBJ_H */

