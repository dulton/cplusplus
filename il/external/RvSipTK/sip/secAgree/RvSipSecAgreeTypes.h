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
 *                              <RvSipSecAgreeTypes.h>
 *
 *
 * The security-agreement layer of the RADVISION SIP toolkit allows you to negotiatie
 * security ageement between a UA and its first hop.
 * This file defines security agreement types, such as security-agreement objects,
 * security-agreement states, security-agreement callbacks, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2005
 *********************************************************************************/


#ifndef RVSIP_SEC_AGREE_TYPES_H
#define RVSIP_SEC_AGREE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipCommonTypes.h"
#include "RvSipSecurityTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-------------------- SECURITY - AGREEMENT HANDLES ----------------------*/

/*
 * RvSipSecAgreeMgrHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to a Security-Agreement Manager instance. The manager
 * object manages the security-agreement module and its handle is needed in all
 * manager related API such as creation of new security-agreement.
 */
RV_DECLARE_HANDLE(RvSipSecAgreeMgrHandle);

/*
 * RvSipSecAgreeHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to a Security-Agreement. The security-agreement object
 * negotiates and maintains a security-agreement and its handle is needed in
 * all security-agreement related API.
 */
RV_DECLARE_HANDLE(RvSipSecAgreeHandle);

/*
 * RvSipAppSecAgreeHandle
 * ---------------------------------------------------------------------------
 * Declaration of application handle to a Security-Agreement. This handle is used by
 * the application in order to associate sip stack security-agreement objects with
 * application security-agreement objects. The application supplies the application
 * handle to the sip stack upon the creation of a new sip stack security-agreement.
 * The sip stack will supply this handle back to the application in every callback
 * function.
 */
RV_DECLARE_HANDLE(RvSipAppSecAgreeHandle);


/*-------------------- SECURITY - AGREEMENT ROLE ---------------------------*/

/* RvSipSecAgreeRole
 * --------------------------------------------------------------------------
 * Each party of a security-agreement assumes one role - client or server. You must
 * set a role to the security-agreement object to enable any processing.
 *
 * RVSIP_SEC_AGREE_ROLE_UNDEFINED - Security-agreement object is not in use
 * RVSIP_SEC_AGREE_ROLE_CLIENT    - This is the client party of a security-agreement
 * RVSIP_SEC_AGREE_ROLE_SERVER    - This is the server party of a security-agreement
 */
typedef enum
{
	RVSIP_SEC_AGREE_ROLE_UNDEFINED = -1,
	RVSIP_SEC_AGREE_ROLE_CLIENT,
	RVSIP_SEC_AGREE_ROLE_SERVER

} RvSipSecAgreeRole;


/*----------------- SECURITY - AGREEMENT STATES AND STATUSES ----------------*/

/* RvSipSecAgreeState
 * --------------------------------------------------------------------------
 * Defines the inner states that the security-agreement object can assume
 *
 * RVSIP_SEC_AGREE_STATE_UNDEFINED - Security-Agreement is allocated but not in use
 * RVSIP_SEC_AGREE_STATE_IDLE - Security-Agreement has a role set and is ready for use
 * RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY - The local security list of the client security-agreement
 *												    is initialized and ready to be sent.
 * RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY - The client security-agreement received a Security-Server
 *													list from the server, but is not yet initialized with
 *													local security list.
 * RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG - The client security-agreement
 *													inserted security-client list to a message
 *													that is about to be sent, and is about to become the
 *													initiator of a security-agreement with the server.
 *													Once the destination of this message is resolved,
 *													the client will change its state to either CLIENT_LIST_SENT -
 *													if the resolved address is other than TLS, hence the message will be
 *                                                  sent containing the security-client list;
 *                                                  or to NO_AGREEMENT_REQUIRED - if the resolved address is TLS
 *													hence the message will be send without any security-agreement information.
 * RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT	- The client security-agreement initiated a negotiation with the server,
 *													by adding security-client list to the outgoing request.
 * RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY - The client security-agreement is ready to choose the security mechanism
 *                                                agreed with the server, to use in further requests. Use
 *                                                RvSipSecAgreeClientSetChosenSecurity() to set the chosen mechanism and
 *                                                to move to state ACTIVE.
 * RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY - The local security list of the server security-agreement
 *													is initialized and ready to be sent.
 * RVSIP_SEC_AGREE_STATE_SERVER_READY	- Both the local security list and remote security list
 *													of the server security-agreement are initialized. Responding
 *                                                  would cause the sending of Security-Server list, and would
 *                                                  change the state of the server security-agreement to ACTIVE.
 * RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY - The server security-agreement received a Security-Client
 *                                                  list from the client, but is not yet initialized with
 *                                                  local security list.
 * RVSIP_SEC_AGREE_STATE_ACTIVE	- The negotiation completed and all required information
 *												    was obtained. The security-agreement is therefore assumed
 *													to be active, and no new security information is expected
 * RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED - The address resolution resulted in TLS, therefore there is no
 *													negotiation of agreement between the client and the server.
 * RVSIP_SEC_AGREE_STATE_INVALID - The security-agreement is no longer valid. You can either terminate the security-agreement
 *                                 with the remote party, or renegotiate an agreement using a new security-agreement object
 *                                 (initialized from the INVALID security-agreement via RvSipSecAgreeCopy()). A security-agreement
 *								   can become INVALID due to: indication from the remote party, expiration of connection, internal ERROR etc.
 * RVSIP_SEC_AGREE_STATE_TERMINATED - The security-agreement is terminated
 */
typedef enum
{
	RVSIP_SEC_AGREE_STATE_UNDEFINED  = -1,
	RVSIP_SEC_AGREE_STATE_IDLE,
	RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY,
	RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY,
	RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG,
	RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT,
	RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY,
	RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY,
	RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY,
	RVSIP_SEC_AGREE_STATE_SERVER_READY,
	RVSIP_SEC_AGREE_STATE_ACTIVE,
	RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED,
	RVSIP_SEC_AGREE_STATE_INVALID,
	RVSIP_SEC_AGREE_STATE_TERMINATED

} RvSipSecAgreeState;

/* RvSipSecAgreeReason
 * --------------------------------------------------------------------------
 * Defines the possible reasons for a change in inner state of a client
 * security-agreement or server security-agreement.
 *
 * RVSIP_SEC_AGREE_REASON_UNDEFINED  - The reason is undefined
 * RVSIP_SEC_AGREE_REASON_USER_INIT  - RvSipSecAgreeInit() was called for the security-agreement
 * RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_RCVD - The server security-agreement received Security-Client list
 * RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD - The client security-agreement received Security-Server list
 * RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_SENT - The server security-agreement sent Security-Server list
 * RVSIP_SEC_AGREE_REASON_MSG_SEND_FAILURE - A message failed to be sent.
 * RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_MISMATCH - The server security-agreement is invalidated, because a received Security-Client
 *                                                   list does not match the remote security list.
 * RVSIP_SEC_AGREE_REASON_SECURITY_VERIFY_MISMATCH - The server security-agreement is invalidated, because a received Security-Verify
 *                                                   list does not match the local security list.
 * RVSIP_SEC_AGREE_REASON_RESPONSE_MISSING - The client security-agreement initiated a security-agreement but
 *                                           did not received appropriate response with server security-agreement
 *                                           information.
 * RVSIP_SEC_AGREE_REASON_INVALID_OR_MISSING_DIGEST - The client security-agreement did not received complete and
 *                                                    valid digest information from the server
 * RVSIP_SEC_AGREE_REASON_SECURITY_OBJ_EXPIRED - The security object expired, i.e. secured connection is no longer valid
 * RVSIP_SEC_AGREE_REASON_OUT_OF_RESOURCES - The required resources were not available for the security-agreement
 */
typedef enum
{
	RVSIP_SEC_AGREE_REASON_UNDEFINED  = -1,
	RVSIP_SEC_AGREE_REASON_USER_INIT,
	RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_RCVD,
	RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD,
	RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_SENT,
	RVSIP_SEC_AGREE_REASON_MSG_SEND_FAILURE,
	RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_MISMATCH,
	RVSIP_SEC_AGREE_REASON_SECURITY_VERIFY_MISMATCH,
	RVSIP_SEC_AGREE_REASON_RESPONSE_MISSING,
	RVSIP_SEC_AGREE_REASON_INVALID_OR_MISSING_DIGEST,
	RVSIP_SEC_AGREE_REASON_SECURITY_OBJ_EXPIRED,
	RVSIP_SEC_AGREE_REASON_OUT_OF_RESOURCES

} RvSipSecAgreeReason;

/* RvSipSecAgreeStatus
 * --------------------------------------------------------------------------
 * Defines the statuses reported by the security-agreement
 *
 * RVSIP_SEC_AGREE_STATUS_UNDEFINED - The status is undefined
 * 
 * General Ipsec Statuses
 * ----------------------
 * RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED - Indicates that the lifetime of the Security Associations
 *                                                 expired. You can begin renegotiation with the remote
 *                                                 party to renew the Ipsec connection, or 
 *                                                 terminate the security-agreement.
 *
 * Special Standard Statuses
 * --------------------------
 * You must set RVSIP_SEC_AGREE_SPECIAL_STANDARD_3GPP_24229, RVSIP_SEC_AGREE_SPECIAL_STANDARD_TISPAN_283003 
 * or RVSIP_SEC_AGREE_SPECIAL_STANDARD_PKT_2 to the security-agreement object via RvSipSecAgreeSetSpecialStandardFlag() 
 * in order to be notified on these statuses
 * RVSIP_SEC_AGREE_STATUS_IPSEC_TEMPORARY_ASSOCIATION   - Indicates that a temporary set of Security Associations
 *                                                        was obtained. The application can set a lifetime timer
 *                                                        for the temporary Security Associations by calling 
 *                                                        RvSipSecAgreeStartIpsecLifetimeTimer(). To obtain 
 *                                                        an established set of Security Associations, the application 
 *                                                        shall reREGISTER to  the proxy.
 * RVSIP_SEC_AGREE_STATUS_IPSEC_ESTABLISHED_ASSOCIATION - Indicates that an established set of Security Associations
 *                                                        was obtained. The application can set a lifetime timer
 *                                                        for the established Security Associations,
 *                                                        by calling RvSipSecAgreeStartIpsecLifetimeTimer().
 * 
 */
typedef enum
{
	RVSIP_SEC_AGREE_STATUS_UNDEFINED = -1,
    RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED,
	RVSIP_SEC_AGREE_STATUS_IPSEC_TEMPORARY_ASSOCIATION,
	RVSIP_SEC_AGREE_STATUS_IPSEC_ESTABLISHED_ASSOCIATION

} RvSipSecAgreeStatus;

/*---------------------- SECURITY - AGREEMENT OWNER -------------------------*/

/* RvSipSecAgreeOwner
 * --------------------------------------------------------------------------
 * A security-agreement can be used by SIP objects such as call-legs, transaction,
 * register-clients and subscriptions to negotiate or maintain it. Any SIP object
 * that uses the security-agreement is an owner of this security-agreement.
 * Owner information can be required in security-agreement API and can be supplied in
 * callbacks. This structure encapsulates owner information.
 *
 * pOwner     - Handle to the owner
 * eOwnerType - Enumerative indication to owner type.
 */
typedef struct
{
    void                        *pOwner;
	RvSipCommonStackObjectType   eOwnerType;

} RvSipSecAgreeOwner;


/*-------------------- SECURITY - AGREEMENT PARAMETERS ----------------------*/

/* RvSipSecAgreeMsgRcvdInfo
 * --------------------------------------------------------------------------
 * Encapsulates the information needed for manual processing of a received request
 * message. This information is obtained by RvSipSecAgreeMgrGetMsgRcvdInfo() and used
 * in RvSipSecAgreeHandleMsgRcvd()
 *
 * hLocalAddr      - The local address on which the message was received.
 * rcvdFromAddr    - The address from which the message was received
 * hRcvdFromConn   - The connection on which the message was received.
 * owner           - The SIP owner that received the given message.
 */
typedef struct
{
	RvSipTransportLocalAddrHandle   hLocalAddr;
	RvSipTransportAddr              rcvdFromAddr;
	RvSipTransportConnectionHandle  hRcvdFromConn;
	RvSipSecAgreeOwner              owner;

} RvSipSecAgreeMsgRcvdInfo;

/* RvSipSecAgreeIpsecInfo
 * --------------------------------------------------------------------------
 * Encapsulates ipsec-3gpp information held by the security-agreement. You supply
 * this structure to the security-agreement via RvSipSecAgreeGetIpsecInfo(). Note
 * that this is not complete ipsec-3gpp information. Additional information (ports
 * and SPIs) can be supplied via a local Security header.
 * Notice: You must supply this ipsec-3gpp information before calling
 * RvSipSecAgreeClientSetChosenSecurity() on client side, and before calling
 * RvSipSecAgreeInit() on server side.
 *
 * lifetime       - Lifetime of protection by IPsec in seconds.
 * IKlen          - Length of Integrity Key in bits.
 *                  Should be 128 for MD5 and 160 for SHA1.
 * CKlen          - Length of Encryption Key in bytes.
 *                  Should be 192 for 3DES and 128 for AES.
 * IK             - Integration Key, as it is defined in TS33.203.
 * CK             - Cipher Key, as it is defined in TS33.203.
 */
typedef struct
{
	RvUint32                          lifetime;
	RvUint16                          IKlen;
    RvUint16                          CKlen;
    RvUint8                           IK[RVSIP_SEC_MAX_KEY_LENGTH/8];
    RvUint8                           CK[RVSIP_SEC_MAX_KEY_LENGTH/8];
} RvSipSecAgreeIpsecInfo;

/* RvSipSecAgreeSendOptions
 * ------------------------------------------------------------------
 * This structure provides sending attributes to the security agreement.
 * The security agreement will set these attributes to the security 
 * object, which will use them when sending messages. 
 * In the current version the application can instruct the security object
 * to compress its' outgoing message.
 *
 * eMsgCompType - Compression type
 * strSigcompId - The unique sigcomp-id that identifies the remote sigcomp 
 *                compartment associated with this seucrity-agreement
 *                The sigcomp-id string length is bounded by 
 *                RVSIP_COMPARTMENT_MAX_URN_LENGTH and should be NULL terminated 
 */
typedef struct
{
    RvInt dummy; /* So that the structure will never be empty */
#ifdef RV_SIGCOMP_ON
    RvSipTransmitterMsgCompType  eMsgCompType;
	RvChar                      *strSigcompId;
#endif /* RV_SIGCOMP_ON */
} RvSipSecAgreeSendOptions; 


/*------------ SECURITY-AGREEMENT SPECIAL-STANDARD TYPES -----------------*/

/* RvSipSecAgreeSpecialStandardType
 * --------------------------------------------------------------------------
 * The security-agreement module is an implementation of RFC3329, with adjustments
 * to support TS-33.203 as well. Special standards such as 3GPP TS 24.229 and TISPAN 
 * ES 283.003 are supported by the security-agreement module, but since this standard 
 * imposes changes on the basic RFC3329 behavior, the application must specifically 
 * indicate for a security-agreement object that it shall support this standard, by using
 * RvSipSecAgreeSetSpecialStandardFlag().
 * RvSipSecAgreeSpecialStandardType enumerates special standards that are supported 
 * by the security-agreement layer.
 * 
 * RVSIP_SEC_AGREE_SPECIAL_STANDARD_UNDEFINED     - Undefined special standard
 * RVSIP_SEC_AGREE_SPECIAL_STANDARD_TISPAN_283003 - TISPAN ETSI ES 283.003
 * RVSIP_SEC_AGREE_SPECIAL_STANDARD_3GPP_24229    - 3GPP TS 24.229
 * RVSIP_SEC_AGREE_SPECIAL_STANDARD_PKT_2         - PACKET CABLE 2.0
 */
typedef enum
{
	RVSIP_SEC_AGREE_SPECIAL_STANDARD_UNDEFINED = -1,
	RVSIP_SEC_AGREE_SPECIAL_STANDARD_TISPAN_283003,
	RVSIP_SEC_AGREE_SPECIAL_STANDARD_3GPP_24229,
	RVSIP_SEC_AGREE_SPECIAL_STANDARD_PKT_2

} RvSipSecAgreeSpecialStandardType;


/*------------ SECURITY-AGREEMENT TISPAN ES-283.003 TYPES -----------------*/

/* RvSipSecAgreeSecurityAssociationType
 * --------------------------------------------------------------------------
 * Indicates the status of the Security Associations held by this security-agreement 
 * object. A Security Association is either temporary or established, as defined 
 * in ES-283.003
 * 
 * RVSIP_SEC_AGREE_SEC_ASSOC_UNDEFINED   - Undefined type of Security Association
 * RVSIP_SEC_AGREE_SEC_ASSOC_TEMPORARY   - The Security Association is temporary
 * RVSIP_SEC_AGREE_SEC_ASSOC_ESTABLISHED - The Security Association is established 
 */
typedef enum
{
	RVSIP_SEC_AGREE_SEC_ASSOC_UNDEFINED = -1,
	RVSIP_SEC_AGREE_SEC_ASSOC_TEMPORARY,
	RVSIP_SEC_AGREE_SEC_ASSOC_ESTABLISHED

} RvSipSecAgreeSecurityAssociationType;


/*------------- SECURITY - AGREEMENT MANAGER CALLBACKS ---------------------*/

/***************************************************************************
 * RvSipSecAgreeMgrSecAgreeRequiredEv
 * ----------------------------------
 * General: Notifies the application that a security-agreement was requests by
 *          the remote party. The owner supplied here is the sip object that
 *          received a message indicating a security-agreement (call-leg,
 *          register-client, etc). If you wish to participate in a security-agreement,
 *          you need to supply a valid security-agreement object (client or server,
 *          according to the role specified) . This can either be a new object,
 *          or an object that is already part of an agreement.
 *          Once you supply a security-agreement object, it will be responsible
 *          for uploading the relevant information from the received message, and
 *          for further security-agreement actions as defined in RFC 3329.
 *          Notice: If you wish to set the security-agreement you supply
 *          here to any sip object, use the sip object specific RvSipXXXSetSecAgree()
 *          API. The sip stack does not set the security-agreement you supply here
 *          to the owner that invoked RvSipSecAgreeMgrSecAgreeRequiredEv().
 *          Notice: If asynchronous computation needs to be done locally,
 *          for example in order to bind local ports or to compute unique keys,
 *          you can supply a security-agreement object here, and modify it later.
 *
 * Return Value: RV_OK (the returned status is ignored in the current stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr       - The sip stack handle to the security-agreement manager
 *         pOwner             - The sip object receiving the message that caused the
 *                              invoking of this event. If this event was invoked due to calling
 *                              RvSipSecAgreeHandleMsgRcvd(), pOwner will be NULL.
 *         hMsg               - The received message requesting for a security-agreement.
 *         pCurrSecAgree      - The current security-agreement object of this owner. This
 *                              security-agreement assumes a state that cannot handle negotiation
 *                              (INVALID, TERMINATED, etc), hence a new security-agreement object
 *                              is required. NULL if there is no security-agreement object to
 *                              this owner.
 *         eRole              - The role of the required security-agreement object (client
 *                              or server).
 * Output: ppSecAgree         - The new security-agreement object to use.
 **************************************************************************/
typedef RvStatus (RVCALLCONV* RvSipSecAgreeMgrSecAgreeRequiredEv)
                       (IN  RvSipSecAgreeMgrHandle					hSecAgreeMgr,
					    IN  RvSipSecAgreeOwner*						pOwner,
						IN  RvSipMsgHandle                          hMsg,
						IN  RvSipSecAgreeHandle						hCurrSecAgree,
					    IN  RvSipSecAgreeRole				        eRole,
						OUT RvSipSecAgreeHandle*					phNewSecAgree);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipSecAgreeMgrReplaceSecAgreeEv
 * ----------------------------------
 * General: Notifies the application that a security-agreement was requests by
 *          the remote party. The owner supplied here is the sip object that
 *          received a message indicating a security-agreement (call-leg,
 *          register-client, etc). If you wish to participate in a security-agreement,
 *          you need to supply a valid security-agreement object (client or server,
 *          according to the role specified) . This can either be a new object,
 *          or an object that is already part of an agreement.
 *          Once you supply a security-agreement object, it will be responsible
 *          for uploading the relevant information from the received message, and
 *          for further security-agreement actions as defined in RFC 3329.
 *          Notice: If you wish to set the security-agreement you supply
 *          here to any sip object, use the sip object specific RvSipXXXSetSecAgree()
 *          API. The sip stack does not set the security-agreement you supply here
 *          to the owner that invoked RvSipSecAgreeMgrSecAgreeRequiredEv().
 *          Notice: If asynchronous computation needs to be done locally,
 *          for example in order to bind local ports or to compute unique keys,
 *          you can supply a security-agreement object here, and modify it later.
 *
 * Return Value: RV_OK (the returned status is ignored in the current stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr       - The sip stack handle to the security-agreement manager
 *         pOwner             - The sip object receiving the message that caused the
 *                              invoking of this event. If this event was invoked due to calling
 *                              RvSipSecAgreeHandleMsgRcvd(), pOwner will be NULL.
 *         hMsg               - The received message requesting for a security-agreement.
 *         pCurrSecAgree      - The current security-agreement object of this owner. This
 *                              security-agreement assumes a state that cannot handle negotiation
 *                              (INVALID, TERMINATED, etc), hence a new security-agreement object
 *                              is required. NULL if there is no security-agreement object to
 *                              this owner.
 *         eRole              - The role of the required security-agreement object (client
 *                              or server).
 * Output: bReplace           - RV_TRUE is to replace. RV_FALSE is not to replace the existing security-agreement.
 *         ppSecAgree         - The new security-agreement object to use.
 **************************************************************************/
typedef RvStatus (RVCALLCONV* RvSipSecAgreeMgrReplaceSecAgreeEv)
                    (IN  RvSipSecAgreeMgrHandle					hSecAgreeMgr,
                     IN  RvSipSecAgreeOwner*						pOwner,
                     IN  RvSipMsgHandle                     hMsg,
                     IN  RvSipSecAgreeHandle						hCurrSecAgree,
                     IN  RvSipSecAgreeRole				      eRole,
                     OUT RvBool*                            bReplace,
                     OUT RvSipSecAgreeHandle*					phNewSecAgree);
#endif
/* SPIRENT_END */

/* RvSipSecAgreeMgrEvHandlers
 * --------------------------------------------------------------------------
 * Encapsulates the events that the security-agreement manager reports to the application.
 * You set these event handlers via RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers().
 *
 * pfnSecAgreeMgrSecAgreeRequiredEvHandler - Notify that the remote party requested a
 *                                           security-agreement, hence a security-agreement
 *                                           object is required,
 */
typedef struct
{
    RvSipSecAgreeMgrSecAgreeRequiredEv    pfnSecAgreeMgrSecAgreeRequiredEvHandler;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RvSipSecAgreeMgrReplaceSecAgreeEv     pfnSecAgreeMgrReplaceSecAgreeEvHandler;
#endif
/* SPIRENT_END */

} RvSipSecAgreeMgrEvHandlers;

/*----------------- SECURITY - AGREEMENT CALLBACKS -------------------------*/

/***************************************************************************
 * RvSipSecAgreeStateChangedEv
 * ----------------------------------
 * General: Notifies the application of a security-agreement state changes;
 *          supplying the new state and, if informative, a reason for the state change.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree	- A handle to the security-agreement object.
 *         hAppSecAgree - The application handle for this security-agreement.
 *         eState		- The new security-agreement state.
 *         eReason		- The reason for the state change (if informative).
 **************************************************************************/
typedef void (RVCALLCONV * RvSipSecAgreeStateChangedEv)(
                                   IN  RvSipSecAgreeHandle        hSecAgree,
                                   IN  RvSipAppSecAgreeHandle     hAppSecAgree,
                                   IN  RvSipSecAgreeState         eState,
                                   IN  RvSipSecAgreeReason        eReason);

/***************************************************************************
 * RvSipSecAgreeStatusEv
 * ----------------------------------
 * General: Notifies the application of security-agreement statuses.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree	- A handle to the security-agreement object.
 *         hAppSecAgree - The application handle for this security-agreement.
 *         eStatus      - The reported status of the security-agreement.
 *         pInfo        - Auxiliary information. Not in use.
 **************************************************************************/
typedef void (RVCALLCONV * RvSipSecAgreeStatusEv)(
                                   IN  RvSipSecAgreeHandle        hSecAgree,
                                   IN  RvSipAppSecAgreeHandle     hAppSecAgree,
                                   IN  RvSipSecAgreeStatus        eStatus,
								   IN  void*					  pInfo);


/* RvSipSecAgreeEvHandlers
 * --------------------------------------------------------------------------
 * Encapsulates the events that the security-agreement object reports to the application.
 * You set these event handlers via RvSipSecAgreeMgrSetSecAgreeEvHandlers().
 *
 * pfnSecAgreeStateChangedEvHandler - notify of security-agreement state change.
 * pfnSecAgreeStatusEvHandler       - notify of security-agreement status.
 */
typedef struct
{
    RvSipSecAgreeStateChangedEv    pfnSecAgreeStateChangedEvHandler;
	RvSipSecAgreeStatusEv          pfnSecAgreeStatusEvHandler;
} RvSipSecAgreeEvHandlers;




#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RVSIP_SEC_AGREE_TYPES_H */



