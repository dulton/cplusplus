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

/*
*******************************************************************************
*                              <RvSipSecurityTypes.h>
*
*    Author                        Date
*    ------------                  -------------
*    Igor                          February 2006
*******************************************************************************
*/

/*@****************************************************************************
 * Module: Security
 * ----------------------------------------------------------------------------
 * Title: Security Type Definitions
 *
 * The RvSipSecurityTypes.h file contains all type and callback function
 * definitions used by Security Module.
 * The Security Module implements:
 * 1. Security Agreement mechanism, described in RFC3329
 * 2. SIP message protection, described in 3GPP TS33.203
 * 3. IPsec for messages, sent and received by Toolkit objects, e.g. SecObj
 *
 * This file includes:
 * 1.Handle Type definitions
 * 2.Security Manager Type definitions
 * 3.Security Object Type definitions
 * 4.Security Object Callback Function definitions
 ***************************************************************************@*/


#ifndef RV_SIP_SECURITY_TYPES_H
#define RV_SIP_SECURITY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "RvSipTransportTypes.h"
#include "RvSipTransmitterTypes.h"

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                               */
/*---------------------------------------------------------------------------*/
#define RVSIP_SEC_MAX_KEY_LENGTH 192 /*Maximal length of Integration and Cipher
                                       Keys, used for IPsec according TS33.203.
                                       Based on 3DES: 192 bits */

/*---------------------------------------------------------------------------*/
/*                           HANDLE TYPE DEFINITIONS                         */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvSipSecMgrHandle (Handle Type definitions)
 * ----------------------------------------------------------------------------
 * The declaration of handle to a SecurityMgr instance. The SecurityMgr
 * manages all the module security-objects and its handle is needed in all
 * SecurityMgr-related API functions, such as the creation of a new
 * security-object.
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvSipSecMgrHandle);

/*@****************************************************************************
 * RvSipSecObjHandle (Handle Type definitions)
 * ----------------------------------------------------------------------------
 * The declaration of a security-object handle. A security-object handle is
 * needed in all security-object API functions, and is used to reference
 * the correct security-object.The security-object implements the protection
 * for SIP flow, pointed to by the application.
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvSipSecObjHandle);

/*@****************************************************************************
 * RvSipAppSecObjHandle (Handle Type definitions)
 * ----------------------------------------------------------------------------
 * The declaration of an application handle to a security-object. This handle
 * The application uses this handle to associate SIP Stack security-objects with
 * the application security-object.The application gives the application handle
 * when a new security-object is created. The SIP Stack will give this handle
 * back to the application in every callback function. The application handle
 * can be fetched from the security-object using API [tbd-which api?].
 ***************************************************************************@*/
RV_DECLARE_HANDLE(RvSipAppSecObjHandle);

/*---------------------------------------------------------------------------*/
/*                      SECURITY OBJECT TYPE DEFINITIONS                     */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvSipSecIpsecParams
 * ----------------------------------------------------------------------------
 * RvSipSecIpsecGeneralParams [tbd-General? structure represents a collection
 * of the parameters required for IPsec. These parameters are common for both
 * sides of the link to be protected.
 *
 * eEAlg          - The encryption algorithm, such as DES.
 * eIAlg          - The integration algorithm, such as MD5.
 * eTransportType - The type of transport forced to run above IPsec.
 *                  If UNDEFINED, both UDP and TCP will be supported.
 * lifetime       - The lifetime of the protection by IPsec, in seconds.
 * IKlen          - The length of the Integrity Key, in bits.
 *                  Should be 128 for MD5 and 160 for SHA1.
 * CKlen          - The length of the Encryption Key, in bits.
 *                  Should be 192 for 3DES and 128 for AES.
 * IK             - The Integration Key, as it is defined in TS33.203.
 * CK             - The Cipher Key, as it is defined in TS33.203.
 ***************************************************************************@*/
typedef struct
{
    RvSipSecurityEncryptAlgorithmType eEAlg;
    RvSipSecurityAlgorithmType        eIAlg;
    RvSipTransport                    eTransportType;
    RvUint32                          lifetime;
    RvUint16                          IKlen;
    RvUint16                          CKlen;
    RvUint8                           IK[RVSIP_SEC_MAX_KEY_LENGTH/8];
    RvUint8                           CK[RVSIP_SEC_MAX_KEY_LENGTH/8];
} RvSipSecIpsecParams;

/*@****************************************************************************
 * RvSipSecIpsecPeerParams (Security Object Type)
 * ----------------------------------------------------------------------------
 * Represents the collection of parameters required for IPsec. These parameters
 * should be generated by each peer of the link to be protected.
 *
 * portC - The Secure Client port, as defined in TS33.203.
 * portS - The Secure Server port, as it is defined in TS33.203.
 * spiC  - The Client Security Parameter Index, as it is defined in TS33.203.
 * spiC  - The Server Security Parameter Index, as it is defined in TS33.203.
 ***************************************************************************@*/
typedef struct
{
    RvUint16  portC;
    RvUint16  portS;
    RvUint32  spiC;
    RvUint32  spiS;
} RvSipSecIpsecPeerParams;

/*@****************************************************************************
 * RvSipSecLinkCfg (Security Object Type)
 * ----------------------------------------------------------------------------
 * Represents the collection of configuration parameters of the link to be protected.
 *
 * strLocalIp   - The local IP of the link in string representation.
 *                IPv6 should be in []%sid format.
 * strRemoteIp  - The remote IP of the link in string representation.
 *                IPv6 should be in [] format.
 * eMsgCompType - The compression type of the link.
 * strSigcompId - The unique sigcom id that identifies the remote sigcomp 
 *                compartment associated with this seucrity-agreement.
 *                NULL terminated string is expected.
 ***************************************************************************@*/
typedef struct
{
    RvChar						*strLocalIp;
    RvChar						*strRemoteIp;
#ifdef RV_SIGCOMP_ON
    RvSipTransmitterMsgCompType  eMsgCompType;
	RvChar                      *strSigcompId;
#endif /* RV_SIGCOMP_ON */

} RvSipSecLinkCfg;

/*@****************************************************************************
 * RvSipSecObjState (Security Object Type)
 * ----------------------------------------------------------------------------
 * Represents the states of the protection establishment that the security-object
 * manages. These states are reported by the security-object, using the
 * RvSipSecObjStateChangedEv() callback function.
 *
 * RVSIP_SECOBJ_STATE_UNDEFINED    The security-object is not in use.
 * RVSIP_SECOBJ_STATE_IDLE         The security-object was allocated.
 * RVSIP_SECOBJ_STATE_INITIATED    The security-object generated data needed for
 *                                 protection establishment, such as SPIs for
 *                                 protection by IPsec.
 * RVSIP_SECOBJ_STATE_ACTIVE       The security-object switched on the protection.
 * RVSIP_SECOBJ_STATE_CLOSING      The security-object started to shut down
 *                                 the protection. The protection is no longer
 *                                 guaranteed by this security-object.
 * RVSIP_SECOBJ_STATE_CLOSED       The protection was shut down. The
 *                                 security-object can be safely terminated.
 * RVSIP_SECOBJ_STATE_TERMINATED   The security-object is going to be destructed.
 ***************************************************************************@*/
typedef enum
{
    RVSIP_SECOBJ_STATE_UNDEFINED = -1,
    RVSIP_SECOBJ_STATE_IDLE,
    RVSIP_SECOBJ_STATE_INITIATED,
    RVSIP_SECOBJ_STATE_ACTIVE,
    RVSIP_SECOBJ_STATE_CLOSING,
    RVSIP_SECOBJ_STATE_CLOSED,
    RVSIP_SECOBJ_STATE_TERMINATED
} RvSipSecObjState;

/*@****************************************************************************
 * RvSipSecObjStateChangeReason (Security Object Type)
 * ----------------------------------------------------------------------------
 * Represents the reasons that caused security-object to change it state.
 *
 * RVSIP_SECOBJ_REASON_UNDEFINED The state name is self-explanatory.
 ***************************************************************************@*/
typedef enum
{
    RVSIP_SECOBJ_REASON_UNDEFINED = -1
} RvSipSecObjStateChangeReason;

/*@****************************************************************************
 * RvSipSecObjStatus (Security Object Type)
 * ----------------------------------------------------------------------------
 * Represents the status of a security-object. The status has no impact on the
 * state machine of the security-object. Upon reception of the status, the
 * application can call security-object API functions.
 *
 * RVSIP_SECOBJ_STATUS_UNDEFINED The state name is self-explanatory.
 * RVSIP_SECOBJ_STATUS_IPSEC_LIFETIME_EXPIRED For IPsec: the lifetime of security
 *                     associations ended. The application can terminate
 *                     the security-object upon reception of the status.
 * RVSIP_SECOBJ_STATUS_MSG_SEND_FAILED  The system call, [tbd?] sending message, failed.
 ***************************************************************************@*/
typedef enum
{
    RVSIP_SECOBJ_STATUS_UNDEFINED = -1,
    RVSIP_SECOBJ_STATUS_IPSEC_LIFETIME_EXPIRED,
    RVSIP_SECOBJ_STATUS_MSG_SEND_FAILED
} RvSipSecObjStatus;

/*---------------------------------------------------------------------------*/
/*                SECURITY OBJECT CALLBACK FUNCTION DEFINITIONS              */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvSipSecObjStateChangedEv (Security Object Callback Function)
 * ----------------------------------------------------------------------------
 * General: Notifies the application of a security-object state change.
 *          For each state change, the new state is supplied with the
 *          reason for the state change.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - The handle of the security-object.
 *          hAppSecObj - The application handle of the security-object.
 *          eState     - The new state of the security-object.
 *          eReason    - The reason for the state change.
 ***************************************************************************@*/
typedef
void (RVCALLCONV * RvSipSecObjStateChangedEv)(
                            IN  RvSipSecObjHandle            hSecObj,
                            IN  RvSipAppSecObjHandle         hAppSecObj,
                            IN  RvSipSecObjState             eState,
                            IN  RvSipSecObjStateChangeReason eReason);

/*@****************************************************************************
 * RvSipSecObjStatusEv (Security Object Callback Function)
 * ----------------------------------------------------------------------------
 * General: Notifies the application of the security-object status.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - The handle of the security-object.
 *          hAppSecObj - The application handle of the security-object.
 *          eStatus    - The status of the security-object.
 *          pInfo      - Auxiliary information. Not in use.
 ***************************************************************************@*/
typedef
void (RVCALLCONV * RvSipSecObjStatusEv)(
                            IN  RvSipSecObjHandle       hSecObj,
                            IN  RvSipAppSecObjHandle    hAppSecObj,
                            IN  RvSipSecObjStatus       eStatus,
                            IN  void*                   pInfo);

/*@****************************************************************************
 * RvSipSecEvHandlers (Security Manager Type)
 * ----------------------------------------------------------------------------
 * A structure which unites function pointers to the module callbacks of the
 * modules. The application should use this structure to set the
 * function pointers into the SecurityMgr, using the
 * RvSipSecObjMgrSetEvHandlers() function.
 *
 * pfnSecObjStateChangedEvHandler   - Notifies the application of the security-object
 *                                    state change.
 * pfnSecObjStatusEvHandler         - Notifies the application of the security-object
 *                                    status.
 ***************************************************************************@*/
typedef struct
{
    RvSipSecObjStateChangedEv    pfnSecObjStateChangedEvHandler;
    RvSipSecObjStatusEv          pfnSecObjStatusEvHandler;
} RvSipSecEvHandlers;

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_SECURITY_TYPES_H */
