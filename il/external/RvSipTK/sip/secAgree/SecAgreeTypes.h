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
 *                              <SecAgreeTypes.h>
 *
 *
 * This file defines the internal types for the security-agreement module.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2005
 *********************************************************************************/


#ifndef SEC_AGREE_TYPES_H
#define SEC_AGREE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "_SipSecAgreeTypes.h"
#include "RvSipCommonList.h"
#include "rpool_API.h"
#include "AdsRlist.h"
#include "AdsHash.h"
#include "rvlog.h"
#include "RvSipTransport.h"
#include "_SipTransportTypes.h"
#include "RvSipAuthenticator.h"
#include "_SipAuthenticator.h"
#include "RvSipSecurityTypes.h"
#include "rvaddress.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define SEC_AGREE_MAX_LOOP(pSecAgreeObject) (100000)

#define SEC_AGREE_OBJECT_NAME_STR               "SecAgree"
#define SEC_AGREE_STATE_TERMINATED_STR          "Terminated"
#define SEC_AGREE_STATE_CHANGED_STR             "StateChanged"
#define SEC_AGREE_STATE_ATTACH_OWNER_STR        "AttachOwner"

#define SEC_AGREE_STRING     "sec-agree"
#define REQUIRE_STRING       "Require"
#define PROXY_REQUIRE_STRING "Proxy-Require"
#define SUPPORTED_STRING     "Supported"
#define RESPONSE_494         494
#define RESPONSE_421         421
#define RESPONSE_502		 502

/* SecAgreeProcessing
 * --------------------------------------------------------------------------
 * Defines the possible events processed by the security-agreement object, that
 * might affect the state or the status of the security-agreement
 * SEC_AGREE_PROCESSING_UNDEFINED        - Undefined
 * SEC_AGREE_PROCESSING_INIT             - Init() was called for the security-agreement
 * SEC_AGREE_PROCESSING_MSG_TO_SEND      - Message to send
 * SEC_AGREE_PROCESSING_DEST_RESOLVED    - Dest resolved
 * SEC_AGREE_PROCESSING_MSG_RCVD         - Message received
 * SEC_AGREE_PROCESSING_SET_SECURITY     - SetChosenSecurity() was called for client security-agreement
 * SEC_AGREE_PROCESSING_IPSEC_ACTIVATION - Ipsec is being activated via the security-object
 */
typedef enum
{
	SEC_AGREE_PROCESSING_UNDEFINED  = -1,
	SEC_AGREE_PROCESSING_INIT,
	SEC_AGREE_PROCESSING_MSG_TO_SEND,
	SEC_AGREE_PROCESSING_DEST_RESOLVED,
	SEC_AGREE_PROCESSING_MSG_RCVD,
	SEC_AGREE_PROCESSING_SET_SECURITY,
	SEC_AGREE_PROCESSING_IPSEC_ACTIVATION

} SecAgreeProcessing;


/*------------------- SECURITY - AGREEMENT MANAGER --------------------------*/

/* SecAgreeMgr
 * --------------------------------------------------------------------------
 * The security-agreement manager object manages security-agreement clients and servers
 *
 * hSipStack                  - Handle to the stack instance to which this security-agrement manager belongs to
 * hMemoryPool                - The memory pool of the security-agreement module. It will be
 *						    	used for security-agreement lists, keys, etc.
 * pLogSrc                    - The log source of the security-agreement module
 * pLogMgr                    - The log manager of the stack
 * hTransport                 - A handle to the transport manager
 * hSecMgr                    - A handle to the security manager
 * hMsgMgr                    - A handle to the message manager
 * seed                       - The seed for random generation
 * hMutex                     - A mutex to lock the security-agreement manager
 * hAppSecAgreeMgr            - Application handle to the security-agreement manager
 * appEvHandlers              - Application event handlers
 * hSecAgreePool			  - A pool of security-agreement objects, containing one list
 *                              of security-agreement objects.
 * hSecAgreeList			  - A list of security-agreement objects (taken from the pool)
 * maxNumOfOwners             - The maximal number of owners in order to define the hash size.
 * hOwnersHash                - Hash table to handle the owners of security-agreement objects
 * ownersHashSize             - The size of the owners hash
 * pEvHandlers                - An array of event handlers, for each possible owner to the security-agreement
 *                              objects. Each owner resides on the entry indexed by its enumeration value
 *                              in RvSipCommonStackObjectType.
 * evHandlersSize             - The size of pEvHandlers
 */
typedef struct
{
	/* stack data */
	void*                            hSipStack;
    HRPOOL							 hMemoryPool;
	RvLogSource*					 pLogSrc;
	RvLogMgr*						 pLogMgr;
	RvSipTransportMgrHandle			 hTransport;
	RvSipSecMgrHandle			     hSecMgr;
	RvSipAuthenticatorHandle		 hAuthenticator;
	RvSipMsgMgrHandle				 hMsgMgr;
	RvRandomGenerator*               seed;

	/* locking data */
	RvMutex							 hMutex;

	/* application data */
	void* 							 hAppSecAgreeMgr;
	RvSipSecAgreeMgrEvHandlers       appMgrEvHandlers;
	RvSipSecAgreeEvHandlers			 appEvHandlers;

	/* security-agreement objects */
	RLIST_POOL_HANDLE				 hSecAgreePool;
	RLIST_HANDLE					 hSecAgreeList;

	/* owners hash */
	RvInt32						 	 maxNumOfOwners;
	HASH_HANDLE						 hOwnersHash;
	RvInt32						 	 ownersHashSize;
	RLIST_POOL_HANDLE				 hSecAgreeOwnersPool;
	RvInt32						 	 ownersPerSecAgree;

	/* callbacks */
	SipSecAgreeEvHandlers*           pEvHandlers;
	RvInt32                          evHandlersSize;

} SecAgreeMgr;

/*---------------------- SECURITY - ADDITIONAL - KEYS ----------------------------*/

/* SecAgreeServerAuthInfo
 * --------------------------------------------------------------------------
 * The digest information held by the server
 *
 * hProxyAuth - A Proxy-Authenticate header that will be inserted with Security-Server lists
 */
typedef struct
{
	RvSipAuthenticationHeaderHandle  hProxyAuth;
} SecAgreeServerAuthInfo;

/* SecAgreeClientAuthInfo
 * --------------------------------------------------------------------------
 * The digest information held by the client
 *
 * hAuthList - A list of one Auth object that was built from the Proxy-Authenticate
 *             header received in the security-agreement, and will be computed into
 *             Authorization headers if needed.
 * pAuthListInfo - List info for hAuthList
 */
typedef struct
{
	RLIST_HANDLE                  hAuthList;
	AuthObjListInfo*              pAuthListInfo;
} SecAgreeClientAuthInfo;

/* SecAgreeAuthInfo
 * --------------------------------------------------------------------------
 * The digest information held within a
 *
 * pAuthInfo - Handle to the authentication info. If this is
 * hProxyAuth - A Proxy-Authenticate header used in the security-agreement process
 */
typedef union
{
	SecAgreeServerAuthInfo   hServerAuth;
	SecAgreeClientAuthInfo   hClientAuth;

} SecAgreeAuthInfo;

/* SecAgreeAdditionalKeys
 * --------------------------------------------------------------------------
 * The security-agreement additional-keys object is used to encapsulate the keys that can be used to
 * establish the security-mechanisms and are not part of the Security headers list, for example
 * Proxy-Authenticate header.
 *
 * authInfo  - The digest keys
 * ipsecInfo - The ipsec keys
 */
typedef struct
{
	/* Digest */
	SecAgreeAuthInfo        authInfo;

	/* Ipsec */
#if (RV_IMS_IPSEC_ENABLED == RV_YES)
	RvSipSecAgreeIpsecInfo  ipsecInfo;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */

} SecAgreeAdditionalKeys;


/*---------------------------- SECURITY DATA -------------------------------*/

/* SecAgreeSetClientList
 * --------------------------------------------------------------------------
 * Indicates whether to set Security-Client list to outgoing requests of
 * client security-agreement.
 * SEC_AGREE_SET_CLIENT_LIST_UNDEFINED - The special indication to set a
 *                                       Security-Client list was not set
 * SEC_AGREE_SET_CLIENT_LIST_BY_APP    - The application forces the sending of
 *                                       Security-Client headers in any request.
 * SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC  - If ipsec-3gpp is the chosen mechanism,
 *                                       Security-Client list should be sent once
 *                                       in ACTIVE state. This is controlled by this
 *                                       flag.
 */
typedef enum
{
	SEC_AGREE_SET_CLIENT_LIST_UNDEFINED = -1,
	SEC_AGREE_SET_CLIENT_LIST_BY_APP,
	SEC_AGREE_SET_CLIENT_LIST_BY_IPSEC

} SecAgreeSetClientList;

/* SelectedSecurity
 * --------------------------------------------------------------------------
 * The selected-security object is used to encapsulate the selection of a mechanism,
 * i.e., the type and parameters of the mechanism that was selected in the agreement
 * process.
 *
 * eType         - The type of security that negotiation agreed on
 * hLocalHeader  - Handle to the local Security header that was chosen. This header
 *                 may contain parameters for obtaining the selected security.
 * hRemoteHeader - Handle to the local Security header that was chosen. This header
 *                 may contain parameters for obtaining the selected security.
 */
typedef struct
{
	/* The type of the selected mechanism */
	RvSipSecurityMechanismType      eType;
	/* The local header that was selected */
	RvSipSecurityHeaderHandle       hLocalHeader;
	/* The remote header that was selected */
	RvSipSecurityHeaderHandle       hRemoteHeader;

} SelectedSecurity;

/* SecAgreeSecurityData
 * --------------------------------------------------------------------------
 * The security-agreement-data object is used to encapsulate the local
 * security lists and keys, and the remote security lists and keys that were obtained
 * during negotiation.
 *
 * hLocalSecurityList  - A list of local Security headers. The server will add this list
 *                       to the initial response via Security-Server headers, and will compare it
 *                       to Security-Verify lists that are received in further requests.
 *                       The client will add this list to initial requests via Security-Client headers.
 *                       This list must not change after initialization.
 * hRemoteSecurityList - A list of remote Security headers. The server stores the initial Security-Client list
 *                       received, and compares it to further Security-Client list received from the client.
 *                       The client stores the initial Security-Server list received. Then it uses it to build
 *                       a Security-Verify list, that will be added to further requests.
 *                       This list must not change after initialization.
 * hTISPANLocalList    - Used by a server security-agreement that supports TISPAN to store the local list of
 *                       security headers that was copied via RvSipSecAgreeCopy(). This list will be compared 
 *                       the received remote Security-Verify 
 * eInsertSecClient    - Indicates whether the client should insert Security-Client headers to outgoing requests.
 *                       It is automatically turned on in state CLIENT_LIST_SENT, and off when the list is sent
 *                       once in state ACTIVE. It can also be controlled via API.
 * eForcedTransport    - The transport type obtained in address resolution and supplied in DestResolvedEv. Used
 *                       for further requests on ipsec.
 * additionalKeys      - Additional security keys
 * hSecObj             - The security object that was obtained for this security-agreement. This security object
 *                       is used for sending requests
 * secMechsToInitiate  - A bit mask of the mechanisms that the security-agreement should initiate 
 *                       in the security object
 * selectedSecurity    - The selected security: type and header handles. Used by the client security-agreement
 *                       to obtain security with the remote party.
 * cseqStep            - Used for the security-agreement object to mark the cseq number of the negotiation transaction.
 *                       The security-agreement object will use the cseq step to insert/load security information
 *                       only to/from final responses of the negotiation transaction (and not inner transactions such 
 *                       as PRACK,  etc).
 */
typedef struct
{
	/* security headers lists */
	RvSipCommonListHandle           hLocalSecurityList;
	RvSipCommonListHandle           hRemoteSecurityList;

	/* TISPAN parameters */
	RvSipCommonListHandle           hTISPANLocalList;

	/* behavior indicators */
	SecAgreeSetClientList           eInsertSecClient;

	/* The additional keys used to obtain the possible security mechanisms */
	SecAgreeAdditionalKeys          additionalKeys;

	/* Security Object*/
	RvSipSecObjHandle               hSecObj;

	/* Security mechanisms to Initiate */
	RvInt32                         secMechsToInitiate;

	/* The security mechanism agreed on */
	SelectedSecurity                selectedSecurity;

	/* The CSeq step of the negotiation transaction */
	RvUint32                        cseqStep;

} SecAgreeSecurityData;


/*------------------- SECURITY - AGREEMENT  ADDRESSES -------------------------*/

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
/* SecAgreeAddresses
 * --------------------------------------------------------------------------
 * The remote and local address information used by this security-agreement to
 * initiate its security object
 *
 * pLocalIpsecIp        - Local IP for ipsec.
 *                        Pointer to the string on the SecAgree page.
 * hLocalIpsecUdpAddr   - Local UDP address for ipsec
 * hLocalIpsecTcpAddr   - Local TCP address for ipsec
 * hLocalTlsIp4Addr     - Local Tls ipv4 address
 * hLocalTlsIp6Addr     - Local Tls ipv6 address
 * remoteAddr           - Remote address details
 * eRemoteTransportType - Remote transport (UDP/TCP for ipsec, or TLS)
 * eMsgCompType			- Compression type
 * strSigcompId         - The unique sigcomp id that identifies the remote sigcomp 
 *                        compartment associated with this seucrity-agreement
 */
typedef struct
{
	/* Local Addresses */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
  RvSipTransport              eTransportType;
#endif
//SPIRENT_END
	/* IP for ipsec */
	RvChar*                         pLocalIpsecIp;
    /* Address type: IPv4 or IPv6 */
    RvSipTransportAddressType       localIpsecAddrType;
    /* scope for ipv6 local ipsec */
    RvInt                           localIpsecIpv6Scope;
	/* Local UDP address for ipsec */
	RvSipTransportLocalAddrHandle   hLocalIpsecUdpAddr;
	/* Local TCP address for ipsec */
	RvSipTransportLocalAddrHandle   hLocalIpsecTcpAddr;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
	/* TLS ipv4 */
	RvSipTransportLocalAddrHandle   hLocalTlsIp4Addr;
	/* TLS ipv6 */
	RvSipTransportLocalAddrHandle   hLocalTlsIp6Addr;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

	/* Remote Addresses */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
	/* Remote Ip and Port for ipsec */
	RvAddress                       remoteIpsecAddr;
	/* Remote transport type for ipsec (UDP or TCP) */
	RvSipTransport                  eRemoteIpsecTransportType;
#endif /* #if (RV_IMS_IPSEC_ENABLED==RV_YES) */

#if (RV_TLS_TYPE != RV_TLS_NONE)
	/* Remote Ip and Port for TLS */
	RvAddress                       remoteTlsAddr;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
	
#ifdef RV_SIGCOMP_ON
    RvSipTransmitterMsgCompType  eMsgCompType;
	RvInt32                      strSigcompId;
#endif /* RV_SIGCOMP_ON */

} SecAgreeAddresses;
#endif /* (RV_IMS_IPSEC_ENABLED==RV_YES) && (RV_TLS_TYPE != RV_TLS_NONE) */


/*---------------------- SECURITY - AGREEMENT  -------------------------------*/

/* SecAgree
 * --------------------------------------------------------------------------
 * The security-agreement object is used to negotiate a security agreement with the
 * remote party, and to maintain this agreement in further requests, as defined in RFC 3329.
 *
 * pSecAgreeMgr             - The security-agreement manager that is responsible for mutual information and for
 *                            resource allocation
 * hPage                    - The memory page of the security-agreement object.
 * terminationInfo          - Structure used for sending termination event to the transport layer
 * stateInfo                - Structure used for sending state changed event to the transport layer
 * tripleLock               - Pointer to the triple lock of the security-agreement object.
 * secAgreeTripleLock       - The triple lock of the security-agreement object. Initialized on stack construct.
 * secAgreeUniqueIdentifier - A unique identifier for this security-agreement (randomly generated
 *                            when the security-agreement is created.
 * eRole                    - The role of this security-agreement object: client or server
 * hAppOwner                - The application handle to this security-agreement.
 * pContext                 - The application additional context. Used to hold the information shared by several objects
 *                            (for example IMPI). Copied by RvSipSecAgreeCopy().
 * securityData             - The security-agreement data.
 * hSecObj                  - Pointer to the security object managed by this security-agreement
 * selectedSecurity         - The security mechanism that was selected by this security-agreement. It is usually obtained
 *                            by the intersection of highest preference between the remote list and
 *                            the local list.
 * eState                   - The state of the security-agreement object.
 * numOfOwners              - A counter to indicate the number of owners for this security-agreement object.
 * hOwnersList              - The list of all current owners of this security-agreement object.
 * responseCode             - Recommended response code. Can be used by the application to determine how to respond on
 *                            server side.
 * addresses                - Local and remote address information to use when initializing the security object
 * eSpecialStandard         - An indication to support a special standard such as TISPAN ES-283.003. This affects the
 *                            behavior of the security-agreement object
 * eSecAssocType            - The current status of the Security Associations held by this security-agreement
 */
typedef struct
{
	/* general information */
    SecAgreeMgr							   *pSecAgreeMgr;
	HPAGE									hPage;
	RvSipTransportObjEventInfo				terminationInfo;

	/* Lock */
	SipTripleLock						   *tripleLock;
    SipTripleLock							secAgreeTripleLock;

	/* Unique identifier */
	RvInt32									secAgreeUniqueIdentifier;
	
	/* role */
	RvSipSecAgreeRole						eRole;

	/* application handles */
	RvSipAppSecAgreeHandle					hAppOwner;
	void*									pContext;

	/* The security-agreement data */
    SecAgreeSecurityData					securityData;

	/* The state of the security-agreement object */
	RvSipSecAgreeState						eState;

	/* owners data */
	RvInt32									numOfOwners;
	/* The list of owners */
	RLIST_HANDLE							hOwnersList;

	/* Recommended response code */
	RvUint16								responseCode;

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE)
	/* Local and Remote address inforamtion */
	SecAgreeAddresses						addresses;
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) || (RV_TLS_TYPE != RV_TLS_NONE) */
	
	/* Special standards types */
	RvSipSecAgreeSpecialStandardType		eSpecialStandard;
	
	/* TISPAN standard types */
	RvSipSecAgreeSecurityAssociationType	eSecAssocType;

} SecAgree;


/*------------------------- OWNER - INFO  ---------------------------------*/

/* SecAgreeOwnerHashKey
 * --------------------------------------------------------------------------
 * The key for hashing security-agreement owners
 *
 * hSecAgreeObject   - The handle of the security-agreement object
 * hOwner            - The handle of the owner
 */
typedef struct
{
	SecAgree          *pSecAgree;
	void              *hOwner;

} SecAgreeOwnerHashKey;

/* SecAgreeOwnerRecord
 * --------------------------------------------------------------------------
 * The information held for each owner in the hash table and owners list
 *
 * owner          - Owner's information (type and handle)
 * pInternalOwner - A pointer to the internal owner. This pointer will be supplied in any
 *                  call to internal callback (callback to the transaction, call-leg, reg-client...)
 * pEvHandlers    - A pointer to the structure of callbacks of this owner. The structure sits
 *                  within the security manager.
 * hListItem      - Pointer to the location of this owner in the security-agreement object's list
 * hHashElement   - Pointer to the location of this owner in the security-agreement mgr's hash table.
 */
typedef struct
{
	RvSipSecAgreeOwner                owner;
	void*                             pInternalOwner;
	SipSecAgreeEvHandlers*            pEvHandlers;
	RLIST_ITEM_HANDLE		          hListItem;
    void*                             pHashElement;

} SecAgreeOwnerRecord;


#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_TYPES_H */



