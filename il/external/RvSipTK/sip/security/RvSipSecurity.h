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
*                              <RvSipSecurity.h>
*
*    Author                        Date
*    ------------                  -------------
*    Igor                          February 2006
*******************************************************************************
*/

/*@****************************************************************************
 * Module: Security
 * ----------------------------------------------------------------------------
 * Title: Security Functions 
 *
 * The RvSipSecurity.h file contains API functions provided by Security Module.
 * The Security Module implements:
 * 1. Security Agreement mechanism, described in RFC3329
 * 2. SIP message protection, described in 3GPP TS33.203
 * 3. IPsec for messages, sent and received by Stack objects, e.g. SecObj
 *
 * This file includes:
 * 1.Security Manager API
 * 2.Security Object API
 *
 * The Security Manager (SecurityMgr) is an object that holds the configuration
 * database of the Security Module and the storage of Security Objects.
 *
 * A Security Object (security-object) is an object that provides protection for
 * the SIP messages that are sent to a specific destination. The protection can
 * be ensured by TLS or IPsec. A security-object implements the protection
 * mechanism establishment described in 3GPP TS33.203.
 ***************************************************************************@*/

#ifndef RV_SIP_SECURITY_H
#define RV_SIP_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecurityTypes.h"
#ifdef RV_IMS_TUNING
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
#include "rvimsipsecpublic.h"
#endif
#endif /*#ifdef RV_IMS_TUNING*/

/*---------------------------------------------------------------------------*/
/*                           SECURITY Manager API                            */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvSipSecMgrSetEvHandlers (Security Manager API)
 * ----------------------------------------------------------------------------
 * General: Sets event handlers for all Security events.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr     - The handle to the SecurityMgr.
 *            pEvHandlers - The structure with event handlers.
 *            structSize  - The size of the event handler structure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecMgrSetEvHandlers(
                                   IN  RvSipSecMgrHandle   hSecMgr,
                                   IN  RvSipSecEvHandlers* pEvHandlers,
                                   IN  RvInt32             structSize);

/*@*****************************************************************************
 * RvSipSecMgrCreateSecObj
 * ----------------------------------------------------------------------------
 * General: Allocates a security-object and exchanges handles with the
 *          application. The new security-object assumes the IDLE state.
 *          For the security-object to provide protection, the application should 
 *          do the following:
 *          1. Allocate the security-object using this function.
 *          2. Set the necessary data into the security-object, such as secure
 *             local port for IPsec, and call the RvSipSecObjInitiate() function.
 *          3. Set additional data, such as secure remote ports for IPsec and
 *             call the RvSipSecObjActivate() function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecMgr    - The handle to the SecurityMgr.
 *          hAppSecObj - The application handle to the allocated security-object.
 * Output:  phSecObj   - The handle to the allocated security-object.
 ****************************************************************************@*/

RVAPI RvStatus RVCALLCONV RvSipSecMgrCreateSecObj(
                                   IN  RvSipSecMgrHandle    hSecMgr,
                                   IN  RvSipAppSecObjHandle hAppSecObj,
                                   OUT RvSipSecObjHandle*   phSecObj);

/*@****************************************************************************
 * RvSipSecMgrSetAppHandle (Security Manager API)
 * ----------------------------------------------------------------------------
 * General: The application can have its own SecurityMgr handle.
 *          This function saves the handle in the SIP Stack SecurityMgr
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    hSecMgr    - The handle to the SIP Stack SecurityMgr.
 *           pAppSecMgr - The application SecurityMgr handle.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecMgrSetAppHandle(
                                   IN RvSipSecMgrHandle hSecMgr,
                                   IN void*             pAppSecMgr);

/*@****************************************************************************
 * RvSipSecMgrGetAppHandle (Security Manager API)
 * ----------------------------------------------------------------------------
 * General: Returns the handle to the application SecurityMgr that the
 *          application set into the SIP Stack SecurityMgr, using the
 *          RvSipSecMgrSetAppHandle() function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr    - The handle to the SIP Stack SecurityMgr.
 * Output:    pAppSecMgr - The application SecurityMgr handle.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecMgrGetAppHandle(
                                   IN RvSipSecMgrHandle hSecMgr,
                                   OUT void**           pAppSecMgr);

/*@****************************************************************************
 * RvSipSecMgrGetStackInstance (Security Manager API)
 * ----------------------------------------------------------------------------
 * General: Returns the handle to the SIP Stack instance to which this
 *          SecurityMgr belongs.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr         - The handle to the SIP Stack SecurityMgr.
 * Output:    phStackInstance - The handle to the SIP Stack instance.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecMgrGetStackInstance(
                                   IN   RvSipSecMgrHandle  hSecMgr,
                                   OUT  void**             phStackInstance);

/*@****************************************************************************
 * RvSipSecMgrCleanAllSAs (Security Manager API)
 * ----------------------------------------------------------------------------
 * General: Removes all IPsec security associations existing in the OS.
 *          This function will fail if there are security-objects in use.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecMgr         - The handle to the SIP Stack SecurityMgr.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecMgrCleanAllSAs(
                                   IN   RvSipSecMgrHandle  hSecMgr);

/*---------------------------------------------------------------------------*/
/*                            SECURITY OBJECT API                            */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvSipSecObjInitiate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: After creating a security-object and setting the required link 
 *          configuration and IPsec parameters, this function opens secure local 
 *          addresses for Port-C and Port-S. Calling this function opens four secure 
 *          local addresses: UDP and TCP on Port-C, and UDP and TCP on Port-S. 
 *          Before calling the initiate functions, local ports and SPIs that will 
 *          be used for IPsec should be provided. The rest of the IPsec parameters
 *          can be provided at a later stage. If the application does not provide
 *          these parameters, the system will allocate them. After calling this 
 *          function the security-object assumes the INITIATED state.
 *
 *          Note the following:
 *          - Once the ports were allocated or SPIs were generated, these
 *            parameters cannot be changed.
 *          - The link configuration into the security-object must be set
 *            before calling RvSipSecObjInitiate().
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecObj - The handle to the security-object.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjInitiate(IN RvSipSecObjHandle hSecObj);

/*@****************************************************************************
 * RvSipSecObjActivate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: After a security-object was initiated, this function causes the 
 *          security-object to create SAs in the system. Before calling this 
 *          function, all IPsec parameters that were not already provided must 
 *          be allocated, such as remote secure ports and algorithms. After 
 *          calling this function the security-object assumes the ACTIVE
 *          state. Once the associations were created, messages that are sent 
 *          through these secure ports will be protected with IPsec.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjActivate(IN RvSipSecObjHandle hSecObj);

/*@****************************************************************************
 * RvSipSecObjShutdown (Security Object API)
 * ----------------------------------------------------------------------------
 * General: This function should be called in the ACTIVE state of the 
 *          security-object to shutdown the security-object protection. The 
 *          function performs the following actions:
 *          - Closes secure connections if they were established.
 *          - Closes local addresses opened by the security-object.
 *          - Removes SAs from the system.
 *          A security-object can be shutdown only if it is in ACTIVE state. 
 *          Otherwise it should be terminated using RvSipSecObjTerminate(). 
 *			After calling this function the security-object assumes the CLOSING or 
 *			CLOSED state. Note that after calling this function, the security-object
 *			can no longer be used for sending secure requests.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjShutdown(IN RvSipSecObjHandle hSecObj);

/*@****************************************************************************
 * RvSipSecObjTerminate (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Frees resources that were consumed by a security-object, after which
 *			frees the security-object. After calling this function the security-object
 *			assumes the TERMINATED state.
 *			Note that a security-object that was created by the application must 
 *			also be terminated by the application. The SIP Stack will never 
 *			terminate applicative security-objects. This function should be 
 *			called after the security-object has reached the CLOSED state.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj - The handle to the security-object.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjTerminate(IN RvSipSecObjHandle hSecObj);

/*@****************************************************************************
 * RvSipSecObjSetAppHandle (Security Object API)
 * ----------------------------------------------------------------------------
 * General: The application can have its own security-object handle.
 *          You can use the RvSipSecObjSetAppHandle() function to
 *          save this handle in the SIP Stack security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - The handle to the SIP Stack security-object.
 *          hAppSecObj - The application security-object handle.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetAppHandle(
                                   IN RvSipSecObjHandle    hSecObj,
                                   IN RvSipAppSecObjHandle hAppSecObj);

/*@****************************************************************************
 * RvSipSecObjGetAppHandle (Security Object API)
 * ----------------------------------------------------------------------------
 * General:  Gets the application handle stored in the SIP Stack security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj     - The handle to the SIP Stack security-object.
 * Output:  phAppSecObj - The application security-object handle.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetAppHandle(
                                   IN  RvSipSecObjHandle     hSecObj,
                                   OUT RvSipAppSecObjHandle* phAppSecObj);

/*@****************************************************************************
 * RvSipSecObjSetLinkCfg (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the configuration of the link to be protected by the
 *          security-object. The configuration consists of such parameters as
 *          local and remote IPs. Either of the IPs can be NULL.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj  - The handle to the security-object.
 *          pLinkCfg - The link configuration.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetLinkCfg(
                        IN RvSipSecObjHandle    hSecObj,
                        IN RvSipSecLinkCfg*     pLinkCfg);

/*@****************************************************************************
 * RvSipSecObjGetLinkCfg (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the configuration of the link protected by the security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj  - The handle to the security-object.
 * Output:  pLinkCfg - The link configuration.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetLinkCfg(
                        IN  RvSipSecObjHandle    hSecObj,
                        OUT RvSipSecLinkCfg*     pLinkCfg);

/*@****************************************************************************
 * RvSipSecObjSetIpsecParams (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the IPsec parameters required for message protection.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object.
 *          pIpsecGeneralParams - The parameters that are agreed upon for
 *                                the local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters that are generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters that are generated by the
 *                                remote side. This pointer can be NULL.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetIpsecParams(
                        IN RvSipSecObjHandle        hSecObj,
                        IN RvSipSecIpsecParams*     pIpsecGeneralParams,
                        IN RvSipSecIpsecPeerParams* pIpsecLocalParams,
                        IN RvSipSecIpsecPeerParams* pIpsecRemoteParams);

/*@****************************************************************************
 * RvSipSecObjGetIpsecParams (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the IPsec parameters set into the security-object.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object
 * Output:  pIpsecGeneralParams - The parameters that are agreed upon for
 *                                the local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters that are generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters that are generated by the
 *                                remote side. This pointer can be NULL.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetIpsecParams(
                        IN  RvSipSecObjHandle           hSecObj,
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams);

/*@****************************************************************************
 * RvSipSecObjGetNumOfOwners (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the current number of SIP Stack Objects, using the security-object,
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj      - The handle to the security-object.
 * Output:  pNumOfOwners - The requested number.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetNumOfOwners(
                        IN  RvSipSecObjHandle hSecObj,
                        IN  RvUint32*         pNumOfOwners);

/*@****************************************************************************
 * RvSipSecObjGetNumOfPendingOwners (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the current number of transactions and transmitters that use
 *          a security-object that are in the middle of sending a message or waiting 
 *			for an incoming response.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj             - The handle to the security-object
 * Output:  pNumOfPendingOwners - The requested number.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetNumOfPendingOwners(
                        IN  RvSipSecObjHandle hSecObj,
                        IN  RvUint32*         pNumOfPendingOwners);

/******************************************************************************
 * RvSipSecObjGetSecAgreeObject
 * ----------------------------------------------------------------------------
 * General: Get the Security-Agreement owner of this Security Object if exists.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj    - Handle to the Security Object
 * Output:  ppSecAgree - Pointer to the Security-Agreement object
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetSecAgreeObject(
                        IN  RvSipSecObjHandle   hSecObj,
                        OUT void**              ppSecAgree);

/*@****************************************************************************
 * RvSipSecUtilInitIpsecParams (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Initializes the IPsec parameter structures to be used with API
 *          of the parameter.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Output:  pIpsecGeneralParams - The parameters that are agreed upon for
 *                                the local and remote sides. This pointer
 *                                can be NULL.
 *          pIpsecLocalParams   - The parameters that are generated by the
 *                                local side. This pointer can be NULL.
 *          pIpsecRemoteParams  - The parameters that are generated by the
 *                                remote side. This pointer can be NULL.
 ***************************************************************************@*/
RVAPI void RVCALLCONV RvSipSecUtilInitIpsecParams(
                        OUT RvSipSecIpsecParams*        pIpsecGeneralParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecLocalParams,
                        OUT RvSipSecIpsecPeerParams*    pIpsecRemoteParams);

#ifdef RV_IMS_TUNING
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/*@****************************************************************************
 * RvSipSecObjSetIpsecExtensionData (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Sets the data, required by various extensions to the IPsec model,
 *          supported by default by the Security Object.
 *          This model uses IPsec ESP in transport mode.
 *          An example of extension to this basic model can be establishing
 *          of ESP transport mode security associations in environment,
 *          configured for tunnneling mode. In this case the additional data
 *          should be provided by the Application.
 *          The RvSipSecObjSetIpsecExtensionData API function enables the 
 *          application to provide such configuration data. For transport mode
 *          associations in tunneling environment see
 *          the LinuxOutboundTunnelData, InboundTunnelData,
 *          OutboundPolicyPriority and InboundPolicyPriority extensions,
 *          defined by the RvImsAppExtensionType type.
 *
 *          Data for extensions is provided in form of array of elements.
 *          Each of them can provide a piece of data for any of supported
 *          extensions. Available extensions are enumerated by
 *          RvImsAppExtensionType type.
 *          On IPsec activation, performed by RvSipSecObjActivate API function,
 *          the array is searched, and extensions, data for that was found in
 *          the array, are switched on.
 *          Therefore in multithreading environment the application is
 *          responsible to prevent write access to the array memory,
 *          while any thread stays in the RvSipSecObjActivate API function.
 *
 *          The application is responsible to allocate and free the memory,
 *          used by the array.
 *
 *          The RvSipSecObjSetIpsecExtensionData API function can be called
 *          by the application any time before call to RvSipSecObjActivate API
 *          function.
 *          After call to RvSipSecObjActivate API function the memory, used by
 *          the array of data elements, can be freed.
 *          Each call overwrites pointers to the data, set by previous call.
 *
 *          For more details see documentation for RvImsAppExtensionData struct
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj          - The handle to the security-object.
 *          pExtDataArray    - The array of Extension Data elements.
 *                             The application should not free memory, used by
 *                             this array, until ACTIVE or state will be
 *                             reported for the Security Object.
 *          extDataArraySize - The number of elements in pExtDataArray array.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjSetIpsecExtensionData(
                        IN RvSipSecObjHandle        hSecObj,
                        IN RvImsAppExtensionData*   pExtDataArray,
                        IN RvSize_t                 extDataArraySize);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/*@****************************************************************************
 * RvSipSecObjGetIpsecExtensionData (Security Object API)
 * ----------------------------------------------------------------------------
 * General: Gets the IPsec extension data, set into the Security Object by
 *          the application using RvSipSecObjSetIpsecExtensionData API function
 *          For more details see documentation for
 *          RvSipSecObjSetIpsecExtensionData API function.
 *
 * Return Value: Returns RV_OK on success. Otherwise, returns an error code
 *               defined in the RV_SIP_DEF.h or rverror.h file.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecObj           - The handle to the security-object.
 * Output:  ppExtDataArray    - The array of Extension Data elements.
 *                              The application should not free memory, used by
 *                              this array, until TERMINATED state will be
 *                              reported for the Security Object.
 *          pExtDataArraySize - The number of elements in pExtDataArray array.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecObjGetIpsecExtensionData(
                        IN  RvSipSecObjHandle        hSecObj,
                        OUT RvImsAppExtensionData**  ppExtDataArray,
                        OUT RvSize_t*                pExtDataArraySize);
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#endif /*#ifdef RV_IMS_TUNING*/

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

RvStatus RVCALLCONV RvSipSecObjGetSecureServerPort(
                                IN   RvSipSecObjHandle        hSecObj,
                                IN   RvSipTransport     eTransportType,
                                OUT  RvUint16*          pSecureServerPort);

RvSipTransportConnectionHandle RvSipSecObjGetServerConnection(RvSipSecObjHandle        hSecObj);

#endif
//SPIRENT_END

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_SECURITY_H */

