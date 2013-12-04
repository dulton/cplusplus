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
 *                              <SecurityObject.h>
 *
 * The SecurityObject.h file defined the Security Object object, object
 * inspector and modifier functions, object action functions.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

#ifndef SECURITY_OBJECT_H
#define SECURITY_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonTypes.h"
#include "_SipSecurityTypes.h"
#include "_SipTransportTypes.h"
#include "RvSipCompartment.h"

#include "rvimsipsec.h"

#include "SecurityMgrObject.h"

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                               */
/*---------------------------------------------------------------------------*/
#define SecObjSupportsIpsec(pSecObj) \
    ( \
      (RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP == pSecObj->eSecMechanism || \
       (RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED == pSecObj->eSecMechanism && \
        RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP & pSecObj->secMechsToInitiate) \
      ) ? RV_TRUE : RV_FALSE \
    )

#define SecObjSupportsTls(pSecObj) \
    ( \
      (RVSIP_SECURITY_MECHANISM_TYPE_TLS == pSecObj->eSecMechanism || \
       (RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED == pSecObj->eSecMechanism && \
        RVSIP_SECURITY_MECHANISM_TYPE_TLS & pSecObj->secMechsToInitiate) \
      ) ? RV_TRUE : RV_FALSE \
    )
    
/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
/* SecObjTls
 * ----------------------------------------------------------------------------
 * secObjTls represents a part of Security Object, responsible for protection
 * by TLS. TLS connection, to be used for protection, can be created and
 * connected by Security Object itself, or can be set into the Security Object.
 *
 * hConn        - Handle to the Connection.
 * bAppConn     - RV_TRUE, if the hConn was set into the Security Object and
 *                was not created by the Object by itself.
 * hLocalAddr   - Handle to the Local Address to be used for TLS connection,
 *                if the connection is created by Security Object.
 * ccRemoteAddr - Remote address (Common Core structure) to be used for TLS
 *                connection, if the connection is created by Security Object.
 */
typedef struct
{
    RvSipTransportConnectionHandle  hConn;
    RvBool                          bAppConn;
    RvSipTransportLocalAddrHandle   hLocalAddr;
    RvAddress                       ccRemoteAddr;
} SecObjTls;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/* SecObjIpsec
 * ----------------------------------------------------------------------------
 * SecObjIpsec represents a part of Security Object, responsible for protection
 * by IPsec. See Security Object declaration below.
 *
 * hLocalAddrUdpC - Handle to the UDP Local Address using secure client port.
 * hLocalAddrTcpC - Handle to the UDP Local Address using secure client port.
 *                  This address has no socket, it is used by the client 
 *                  connection only to hold the IP information, required
 *                  on connection opening.
 * hLocalAddrUdpS - Handle to the UDP Local Address using secure server port.
 * hLocalAddrTcpS - Handle to the TCP Local Address using secure server port.
 * hConnC         - Handle to the client Connection.
 *                  Client Connection is used for SIP request sending and SIP
 *                  response reception, if TCP is used.
 * hConnS         - Handle to the server Connection.
 *                  Server Connection is used for message delivering.
 * eTransportType - predefined Transport run above IPsec.
 *                  Auxilary field, where the Application can store preffered
 *                  transport. If it is not set, all Transport (UDP and TCP for
 *                  now) will be supported.
 * lifetime       - Lifetime of the IPsec session in seconds
 * counterTransc  - Counter of Transactions that uses the Security Object for
 *                  message protection. Security Object can't be terminated
 *                  if the counter is not zero.
 * timerLifetime  - Timer controlling lifetime of the IPsec protection.
 *                  The timer is run on move Security Object to ACTIVE state.
 * ccSAData       - union of data, required by Common Core for IPsec
 *                  establishment, such as algorithms, SPIs, Secure Port e.t.c
 */
typedef struct
{
    /* Object data */
    RvSipTransportLocalAddrHandle  hLocalAddrUdpC;
    RvSipTransportLocalAddrHandle  hLocalAddrTcpC;
    RvSipTransportLocalAddrHandle  hLocalAddrUdpS;
    RvSipTransportLocalAddrHandle  hLocalAddrTcpS;
    RvSipTransportConnectionHandle hConnC;
    RvSipTransportConnectionHandle hConnS;
    RvSipTransport                 eTransportType;
    RvUint32                       lifetime;
    RvUint32                       counterTransc;
    SipTimer                       timerLifetime;

    /* Common Core level data */
    RvImsSAData                       ccSAData;
} SecObjIpsec;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/* SecObj
 * ----------------------------------------------------------------------------
 * SecObj represents a Security Object, which provides protection of SIP
 * messages on one hop.
 * The protection can be done by TLS or IPsec security mechanism.
 * SecObj implements behavior, described in TS33.203 and in RFC3329.
 *
 * hAppSecObj    - Application handle to the Security Manager
 * pSecAgree     - Pointer to the Security Agreement object,
 *                 owning the Security Object.
 * pMgr          - Pointer to the Security Manager
 * impi          - Pointer to the Application data related to IMPI,
 *                 as it is defined in TG 33.208. Is used by Security Agreement
 * eState        - State of the Security Object
 * eSecMechanism - Security Mechanism to be used
 * secMechsToInitiate - bit mask - mechainsm to be initiated.
 *                 SecAgree object may ask few mechanisms to be initiated,
 *                 but only one of them will be used finally.
 * tpqEventInfo  - Structure to be supplied to the Transport module each time
 *                 the Security Object passes event to the Transport Processing
 *                 Queue
 * tlsSession    - TLS sub object
 * pIpsecSession - IPsec sub object
 * counterOwners - counter of SIP Toolkit objects, using the SecObj for
 *                 protection of their messages.
 *                 SecObj can't be terminated, if this counter is not NULL.
 * ipv6Scope     - Scope Id of the Local Address, identifying the protected hop
 * strLocalIp    - Local Address, identifying the protected hop
 * strRemoteIp   - Remote Address, identifying the protected hop
 * secObjUniqueIdentifier - unique identifier, generated randomly
 *                 on the Security Object initialization.
 * pTripleLock   - Pointer to the Object's triple lock. NULL for vacant object
 * tripleLock    - Triple Lock. Is created on Security Module construction
 * eMsgCompType  - The compression type of the messages sent by the Security object
 * strSigcompId  - The unique sigcom id that identifies the remote sigcomp 
 *                 compartment associated with this seucrity-agreement
 * lenSigcompId	 - The length of the allocated string in strSigcompId
 */
typedef struct
{
    RvSipAppSecObjHandle        hAppSecObj;
    void*                       pSecAgree;
    SecMgr*                     pMgr;
    void*                       impi;
    RvSipSecObjState            eState;
    RvSipSecurityMechanismType  eSecMechanism;
    RvInt32                     secMechsToInitiate;
    RvSipTransportObjEventInfo      terminationInfo;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    SecObjTls                   tlsSession;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    SecObjIpsec*               pIpsecSession;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
    RvUint32                   counterOwners;
    RvInt                      ipv6Scope;
    RvChar                     strLocalIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvChar                     strRemoteIp[RVSIP_TRANSPORT_LEN_STRING_IP];
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvInt32                    secObjUniqueIdentifier;
    SipTripleLock*             pTripleLock;
    SipTripleLock              tripleLock;
#endif
#ifdef RV_SIGCOMP_ON
    RvSipTransmitterMsgCompType eMsgCompType;
	RvChar                     strSigcompId[RVSIP_COMPARTMENT_MAX_URN_LENGTH];
#endif /* #endif */
} SecObj;

/*---------------------------------------------------------------------------*/
/*                            FUNCTIONS HEADERS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecObjInitiate
 * ----------------------------------------------------------------------------
 * General: initiates Security Object in context of security mechanism set into
 *          the object.
 *          As a result, the object will move to INITIATED state.
 *          For IPsec, for example, this function generates local parameters,
 *          such as SPIs and Secure Ports.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Handle to the Security Object
 *****************************************************************************/
RvStatus SecObjInitiate(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjActivate
 * ----------------------------------------------------------------------------
 * General: activates message protection using the security mechanism, set into
 *          the Security Object.
 *          As a result, the object will move to ACTIVE state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Handle to the Security Object
 *****************************************************************************/
RvStatus SecObjActivate(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjShutdown
 * ----------------------------------------------------------------------------
 * General: stops message protection.
 *          As a result of this function call, the Security Object will be not
 *          available for new owners. Now new SIP requests will be delivered
 *          using this Security Object. Only SIP responses.
 *          On termination of existing owners, CLOSED state will be raised.
 *          The object can't be restarted.
 *          Number of owners is monitored using COUNTER field of the object.
 *          The counter is increased on RvSipXXXSetSecObj() call and is
 *          decreased on RvSipXXXRemoveSecObj() call, where XXX stands for
 *          any Toolkit object, such as CallLeg, Subscription, Transmitter etc.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdown(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjShutdownContinue
 * ----------------------------------------------------------------------------
 * General: continue Shutdown if it was suspeneded due to existing pending
 *          objects (transactions and transmitters) or opened connections.
 *          If no pending objects or not closed connections exist,
 *          moves the Securoty Object to the CLOSED state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdownContinue(IN SecObj* pSecObj);

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjShutdownIpsecContinue
 * ----------------------------------------------------------------------------
 * General: Close secure ports: Local Addresses and Connections used by IPsec.
 *          IPsec can be removed from the Operation System upon
 *          reception the TERMINATED event on connections only.
 *          Otherwise pending on the connection data can be sent in plain text.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 * Output: pShutdownCompleted - RV_FALSE, if IPsec was not removed from System
 *****************************************************************************/
RvStatus SecObjShutdownIpsecContinue(IN SecObj*  pSecObj,
                                     OUT RvBool* pShutdownCompleted);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjShutdownIpsecComplete
 * ----------------------------------------------------------------------------
 * General: Removes IPsec from the system.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjShutdownIpsecComplete(IN SecObj*  pSecObj);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

/******************************************************************************
 * SecObjTerminate
 * ----------------------------------------------------------------------------
 * General: free resources, used by the Security Object.
 *          This function is permitted for objects in IDLE, INITIATED or
 *          CLOSED state.
 *          In order to move the object to CLOSED state RvSipSecObjShutdown
 *          should be called.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjTerminate(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjInitialize
 * ----------------------------------------------------------------------------
 * General: initializes Security Object. Set it state to IDLE.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *            pMgr    - Pointer to the Security Manager
 *****************************************************************************/
RvStatus SecObjInitialize(IN SecObj* pSecObj, IN SecMgr* pMgr);

/******************************************************************************
 * SecObjFree
 * ----------------------------------------------------------------------------
 * General: Remove Security Object from the list of used objects.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecObj - Pointer to the Security Object
 *****************************************************************************/
void SecObjFree(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjRemoveIfNoOwnerIsLeft
 * ----------------------------------------------------------------------------
 * General: Removes Security Objects, if it has no owner (Application,
 *          SecAgree, CallLeg or other). Removal process depends on the state
 *          of the object. If it is active, it will be shutdowned,
 *          if it is closed, it will be terminated.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjRemoveIfNoOwnerIsLeft(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjSetMechanisms
 * ----------------------------------------------------------------------------
 * General: Sets security mechanism to be used by Security Object for message
 *          protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj       - Pointer to the Security Object
 *          eSecMechanism - Mechanism, e.g. TLS or IPsec
 *****************************************************************************/
RvStatus SecObjSetMechanism(
                        IN SecObj* pSecObj,
                        IN RvSipSecurityMechanismType eSecMechanism);

/******************************************************************************
 * SecObjSetMechanismsToInitiate
 * ----------------------------------------------------------------------------
 * General: Sets masks of security mechanisms to be initiated by Security
 *          Object. One of them should be choosen finally.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj            - Pointer to the Security Object
 *          secMechsToInitiate - Mask of Security Mechanisms,
 *                               e.g.(RVSIP_SECURITY_MECHANISM_TYPE_TLS &
 *                                    RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
 *****************************************************************************/
RvStatus SecObjSetMechanismsToInitiate(
                        IN SecObj* pSecObj,
                        IN RvInt32 secMechsToInitiate);

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecParams
 * ----------------------------------------------------------------------------
 * General: Sets IPsec parameters required for message protection.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj             - Pointer to the Security Object
 *          pIpsecGeneralParams - Parameters, agreed for local and remote sides
 *                                This pointer can be NULL.
 *          pIpsecLocalParams   - Parameters, generated by Local side
 *                                This pointer can be NULL.
 *          pIpsecRemoteParams  - Parameters, generated by Remote side
 *                                This pointer can be NULL.
 *****************************************************************************/
RvStatus SecObjSetIpsecParams(
                        IN SecObj*                  pSecObj,
                        IN RvSipSecIpsecParams*     pIpsecGeneralParams,
                        IN RvSipSecIpsecPeerParams* pIpsecLocalParams,
                        IN RvSipSecIpsecPeerParams* pIpsecRemoteParams);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjGetIpsecParams
 * ----------------------------------------------------------------------------
 * General: Gets IPsec parameters set into Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj             - Pointer to the Security Object
 * Output:  pIpsecGeneralParams - Parameters, agreed for local and remote sides
 *                                This pointer can be NULL.
 *          pIpsecLocalParams   - Parameters, generated by Local side
 *                                This pointer can be NULL.
 *          pIpsecRemoteParams  - Parameters, generated by Remote side
 *                                This pointer can be NULL.
 *****************************************************************************/
RvStatus SecObjGetIpsecParams(
                        IN  SecObj*                     pSecObj,
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#ifdef RV_IMS_TUNING
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecExtensionData
 * ----------------------------------------------------------------------------
 * General: Sets data required for IPsec extansions, like a tunneling mode.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj          - Pointer to the Security Object
 *          pExtDataArray    - The array of Extension Data elements.
 *          extDataArraySize - The number of elements in pExtDataArray array.
 *****************************************************************************/
RvStatus SecObjSetIpsecExtensionData(
                        IN SecObj*                  pSecObj,
                        IN RvImsAppExtensionData*   pExtDataArray,
                        IN RvSize_t                 extDataArraySize);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#endif /*#ifdef RV_IMS_TUNING*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjSetIpsecLocalAddressesPortS
 * ----------------------------------------------------------------------------
 * General: Sets Local Addresses, bound to Port-S, into the Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          hLocalAddrUdp - Handle to the Local UDP Address
 *          hLocalAddrTcp - Handle to the Local TCP Address
 *****************************************************************************/
RvStatus SecObjSetIpsecLocalAddressesPortS(
                        IN SecObj*                       pSecObj,
                        IN RvSipTransportLocalAddrHandle hLocalAddrUdp,
                        IN RvSipTransportLocalAddrHandle hLocalAddrTcp);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#ifdef RV_SIGCOMP_ON
/******************************************************************************
 * SecObjSetSigcompParams
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
RvStatus SecObjSetSigcompParams(
                        IN SecObj                      *pSecObj,
						IN RvSipTransmitterMsgCompType  eMsgCompType,
                        IN RPOOL_Ptr				   *pSigcompId,
						IN RvSipSecLinkCfg             *pLinkCfg);
#endif /* RV_SIGCOMP_ON */

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjSetTlsParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters required for TLS establishment.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus SecObjSetTlsParams(
                        IN SecObj*          pSecObj,
                        IN SipSecTlsParams* pTlsParams);
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjGetTlsParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters set into Security Object.
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj    - Pointer to the Security Object
 *          pTlsParams - Parameters
 *****************************************************************************/
RvStatus SecObjGetTlsParams(
                        IN  SecObj*          pSecObj,
                        OUT SipSecTlsParams* pTlsParams);
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
/******************************************************************************
 * SecObjSetTlsConnection
 * ----------------------------------------------------------------------------
 * General: Sets connection, to be used by the Security Object for protection
 *          by TLS.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          hConn   - Handle to the Connection to be set
 *****************************************************************************/
RvStatus SecObjSetTlsConnection(
                        IN  SecObj*                         pSecObj,
                        IN  RvSipTransportConnectionHandle  hConn);
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

/******************************************************************************
 * SecObjChangeState
 * ----------------------------------------------------------------------------
 * General: moves Security Object to the specified state.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          eState  - New state
 *          eReason - Reason of the state change
 *****************************************************************************/
RvStatus SecObjChangeState(
                        IN SecObj*                      pSecObj,
                        IN RvSipSecObjState             eState,
                        IN RvSipSecObjStateChangeReason eReason);

/******************************************************************************
 * SecObjGetSecureLocalAddressAndConnection
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
 * Input:   pSecObj           - Pointer to the Security Object
 *          eMsgType          - Request or response. For request sending client
 *                              connection should be provided, for response -
 *                              server connection.
 *          peTransportType   - Type of the Transport to be used for protected
 *                              message delivery.If UNDEFINED, forced Transport
 *                              Type, set with SetIpsecParame() API will be
 *                              used. If it is UNDEFINED also, the default -
 *                              UDP will be returned.
 *                              Can be NULL.
 * Output:  phLocalAddress    - Handle to the Local Address.
 *          pRemoteAddr       - Pointer to the Remote Address.
 *          phConn            - Handle to the TCP/SCTP/TLS Connection.
 *          peTransportType   - Type of Transport set early
 *                              into the Secrity Object, as a forced Transport
 *                              Type, if peTransportType is UNDEFINED.
 *			peMsgCompType	  - Pointer to the compression type of messages 
 *								sent by the Security object
 *          strSigcompId      - The remote sigcomp id associated with this 
 *                              security object. 
 *          bIsConsecutive    - Boolean indication to whether the sigcomp-id
 *                              should be consecutive on the given page
 *****************************************************************************/
RvStatus SecObjGetSecureLocalAddressAndConnection(
                    IN      SecObj*                         pSecObj,
                    IN      RvSipMsgType                    eMsgType,
                    IN OUT  RvSipTransport*                 peTransportType,
                    OUT     RvSipTransportLocalAddrHandle*  phLocalAddress,
                    OUT     RvAddress*                      pRemoteAddr,
                    OUT     RvSipTransportConnectionHandle* phConn,
					OUT     RvSipTransmitterMsgCompType	   *peMsgCompType,
					INOUT   RPOOL_Ptr                      *strSigcompId,
					IN      RvBool                          bIsConsecutive);

/******************************************************************************
 * SecObjGetSecureServerPort
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
 * Input:   pSecObj - Security Object
 *          eTransportType - Type of the Transport, protected port for which is
 *                           requested.
 * Output:  pSecureServerPort - protected server port to be ste in VIA header.
 *****************************************************************************/
RvStatus RVCALLCONV SecObjGetSecureServerPort(
                                IN   SecObj*            pSecObj,
                                IN   RvSipTransport     eTransportType,
                                OUT  RvUint16*          pSecureServerPort);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/******************************************************************************
 * SecObjLockAPI
 * ----------------------------------------------------------------------------
 * General: Locks Security Object according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjLockAPI(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjUnlockAPI
 * ----------------------------------------------------------------------------
 * General: Unlocks Security Object according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
void SecObjUnlockAPI(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjLockMsg
 * ----------------------------------------------------------------------------
 * General: Locks Security Object according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
RvStatus SecObjLockMsg(IN SecObj* pSecObj);

/******************************************************************************
 * SecObjUnlockMsg
 * ----------------------------------------------------------------------------
 * General: Unlocks Security Object according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj - Pointer to the Security Object
 *****************************************************************************/
void SecObjUnlockMsg(IN SecObj* pSecObj);

#else /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
#define SecObjLockAPI(a) (RV_OK)
#define SecObjUnlockAPI(a)
#define SecObjLockMsg(a) (RV_OK)
#define SecObjUnlockMsg(a)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) #else*/


/******************************************************************************
 * SecObjHandleConnStateConnected
 * ----------------------------------------------------------------------------
 * General: handles CONNECTED state, raised by the Security Object's Connection
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:        pSecObj - Pointer to the Security Object
 *               hConn   - Handle of the closed connection
*****************************************************************************/
RvStatus SecObjHandleConnStateConnected(
                                    IN SecObj* pSecObj,
                                    IN RvSipTransportConnectionHandle hConn);

/******************************************************************************
 * SecObjHandleConnStateClosed
 * ----------------------------------------------------------------------------
 * General: handles CLOSED state, raised by the Security Object's Connection
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:        pSecObj    - Pointer to the Security Object
 *               hConn      - Handle of the closed connection
 *               eConnState - State of Connection (CLOSED or TERMINATED)
 *****************************************************************************/
RvStatus SecObjHandleConnStateClosed(
                                IN SecObj* pSecObj,
                                IN RvSipTransportConnectionHandle  hConn,
                                IN RvSipTransportConnectionState   eConnState);

/******************************************************************************
 * SecObjHandleMsgReceivedEv
 * ----------------------------------------------------------------------------
 * General: handles MsgReceived event:
 *              1. Updates Security mechanism with the protection used for
 *                 message delivery, if it was determined yet.
 *              2. Performs some security checks on the received message.
 *
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj       - Pointer to the Security Object
 *          hMsgReceived  - Handle to the received message
 *          pRecvFromAddr - Details of the address, from which the message was
 *                          sent.
 * Output:
 *          pbApprove     - RV_TRUE, if the checks are OK
 *****************************************************************************/
void SecObjHandleMsgReceivedEv(
                                    IN  SecObj*              pSecObj,
                                    IN  RvSipMsgHandle       hMsgReceived,
                                    IN  SipTransportAddress* pRecvFromAddr,
                                    OUT RvBool*              pbApproved);

/******************************************************************************
 * SecObjSetImpi
 * ----------------------------------------------------------------------------
 * General: Sets IMPI to be set by Security Object into it's Local Addresses
 *          and Connections.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj - Pointer to the Security Object
 *          impi    - IMPI
 *****************************************************************************/
RvStatus SecObjSetImpi(
                        IN SecObj* pSecObj,
                        IN void*   impi);

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
 * SecObjStartTimer
 * ----------------------------------------------------------------------------
 * General: Starts the security-object timer. If interval of 0 is supplied, the
 *          time interval is determined by the security-object lifetime parameter.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecObj  - Pointer to the Security Object
 *          interval - The time interval to set the timer to (in seconds)
 *****************************************************************************/
RvStatus SecObjStartTimer(IN SecObj*   pSecObj,
                          IN RvUint32  interval);

/******************************************************************************
 * SecObjGetRemainingTime
 * ----------------------------------------------------------------------------
 * General: Gets the remaining time of the security-object lifetime timer.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecObj        - Pointer to the Security Object
 * Output:    pRemainingTime - Pointer to the remaining time value.
 *****************************************************************************/
RvStatus SecObjGetRemainingTime(IN  SecObj*    pSecObj,
								OUT RvInt32*   pRemainingTime);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RvStatus ResetIpsecAddr(IN SecObj* pSecObj);

#endif
//SPIRENT_END

#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SECURITY_OBJECT_H*/

