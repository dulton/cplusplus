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
 *                              <RvSipSecAgree.h>
 *
 * This file defines the security-agreement object, attributes and interface functions.
 * The security-agreement object is used to negotiate a security agreement with the
 * remote party, and to maintain this agreement in further requests, as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef RVSIP_SEC_AGREE_H
#define RVSIP_SEC_AGREE_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "RvSipCommonList.h"
#include "RvSipTransportTypes.h"
#include "RvSipAuthenticator.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/*------------------ M A N A G E R S  F U N C T I O N S -----------------*/

/***************************************************************************
 * RvSipSecAgreeMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own applicative Security-Agreement 
 *			Manager handle. Use this function to store the application handle
 *			in the Stack Security-AgreementMgr.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr    - Handle to the stack security-agreement manager.
 *        hAppSecAgreeMgr - The application security-agreement manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetAppMgrHandle(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  void*                         hAppSecAgreeMgr);

/***************************************************************************
 * RvSipSecAgreeMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application security-agreement manger object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr     - Handle to the stack security-agreement manager.
 *        phAppSecAgreeMgr - The application security-agreement manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetAppMgrHandle(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  void**                        phAppSecAgreeMgr);

/***************************************************************************
 * RvSipSecAgreeMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this 
 *          security-agrement manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgreeMgr    - Handle to the stack security-agreement manager.
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                              handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetStackInstance(
                                   IN   RvSipSecAgreeMgrHandle    hSecAgreeMgr,
								   OUT  void					**phStackInstance);

/***************************************************************************
 * RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all security-agreement manager events.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr - Handle to the security-agreement manager.
 *        pEvHandlers  - Pointer to structure containing application event handler
 *                       pointers.
 *        structSize   - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetSecAgreeMgrEvHandlers(
							 IN  RvSipSecAgreeMgrHandle        hSecAgreeMgr,
                             IN  RvSipSecAgreeMgrEvHandlers   *pMgrEvHandlers,
                             IN  RvInt32                       structSize);

/***************************************************************************
 * RvSipSecAgreeMgrSetSecAgreeEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all security-agreement object events.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSecAgreeMgr - Handle to the security-agreement manager.
 *        pEvHandlers  - Pointer to structure containing application event handler
 *                       pointers.
 *        structSize   - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrSetSecAgreeEvHandlers(
							 IN  RvSipSecAgreeMgrHandle     hSecAgreeMgr,
                             IN  RvSipSecAgreeEvHandlers   *pEvHandlers,
                             IN  RvInt32                    structSize);

/***************************************************************************
 * RvSipSecAgreeMgrCreateSecAgree
 * ------------------------------------------------------------------------
 * General: Creates a new security-agreement object. Use RvSipSecAgreeSetRole()
 *          to set the role of the newly created security-agreement: client
 *          or server.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr  - Handle to the security-agreement Mgr
 *         hAppSecAgree  - The application handle for the new security-agreement
 * Output: phSecAgree    - The sip stack's newly created security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrCreateSecAgree(
							 IN   RvSipSecAgreeMgrHandle        hSecAgreeMgr,
							 IN   RvSipAppSecAgreeHandle        hAppSecAgree,
							 OUT  RvSipSecAgreeHandle          *phSecAgree);

/***************************************************************************
 * RvSipSecAgreeMgrGetMsgRcvdInfo
 * ------------------------------------------------------------------------
 * General: Obtains arrival information of a received message: the local address
 *          it was received on, the remote address it was received from, etc.
 *          This information is required for manual processing of an incoming message
 *			via RvSipSecAgreeHandleMsgRcvd().
 *          Notice: You can use this function from within the RvSipXXXMsgReceivedEv event
 *          of a security-agreement owner (call-leg, transaction, etc); or from within the
 *          RvSipXXXStateChangedEv when the new state indicates that a message was received.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgreeMgr      - Handle to the security-agreement Mgr
 *         ownerStructSize   - The size of RvSipSecAgreeOwner
 *         pOwner            - The Sip object that received the message (Call-leg,
 *                             transaction, register-client, etc). If this is a forked
 *                             call-leg, the local address and connection parameters
 *                             will return empty.
 *         hMsg              - The received message
 *         msgInfoStructSize - The size of RvSipSecAgreeMsgRcvdInfo
 * Output: pMsgInfo          - The message information structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeMgrGetMsgRcvdInfo(
							 IN     RvSipSecAgreeMgrHandle         hSecAgreeMgr,
                             IN     RvInt32                        ownerStructSize,
							 IN     RvSipSecAgreeOwner            *pOwner,
							 IN     RvSipMsgHandle                 hMsg,
                             IN     RvInt32                        msgInfoStructSize,
							 INOUT  RvSipSecAgreeMsgRcvdInfo      *pMsgInfo);


/*----------- S E C U R I T Y - A G R E E M E N T  C O N T R O L ----------*/

/***************************************************************************
 * RvSipSecAgreeInit
 * ------------------------------------------------------------------------
 * General: Indicates that a security-agreement is now initialized with all 
 *			the required parameters and can be used to process outgoing messages 
 *			sent by its owner. When a client whishs to start a security 
 *			negotiation process with the server, it must first create and attach 
 *			a security-agreement to a chosen SIP Stack object. The client then has
 *			to set all the required negotiation parameters to the securityagreement.
 *			After completing the initiation process, the client must call the
 *			RvSipSecAgreeInit() API function to indicate that the security-agreement 
 *			is ready for use. The following request sent by the SIP Stack object 
 *			(the owner) will contain all the required negotiation information.
 *			When a server receives a request with negotiation information and 
 *			creates a new server security-agreement, the server application must 
 *			initialize all required negotiation information. The application must
 *			call the RvSipSecAgreeInit() on the server security-agreement before 
 *			responding to the negotiation request.
 *			Note: If you send a message via the SIP owner before calling
 *			RvSipSecAgreeInit(), the message will not be processed by the 
 *			securityagreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeInit(
							 IN   RvSipSecAgreeHandle    hSecAgree);

/***************************************************************************
 * RvSipSecAgreeCopy
 * ------------------------------------------------------------------------
 * General: Used to initialize a new security-agreement object from an old one
 *          as part of a renegotiation process. In order to perform renegotiation
 *          you are required to the following set of actions:
 *          1. Create a new security-agreement object
 *          2. Set the role of the new security-agreement.
 *          3. Call RvSipSecAgreeCopy() to copy security information from the
 *             old security-agreement to the new one.
 *          4. Call RvSipSecAgreeInit() to initiate the new security-agreement
 *             object.
 *          Notice: You must call RvSipSecAgreeCopy() in order to perform
 *          renegotiation (rather than initializing a security-agreement from
 *          scratch).
 *          Notice: Not all fields are being copied by RvSipSecAgreeCopy().
 *          Remote security information will not be copied here but will be
 *          obtained by the renegotiation.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hDestinationSecAgree   - Handle to the destination security-agreement
 *         hSourceSecAgree        - Handle to the source security-agreement
 ***************************************************************************/

RVAPI RvStatus RVCALLCONV RvSipSecAgreeCopy(
							 IN   RvSipSecAgreeHandle    hDestinationSecAgree,
							 IN   RvSipSecAgreeHandle    hSourceSecAgree);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetPortC(
							 IN   RvSipSecAgreeHandle    hSecAgree,
							 IN   int portC);
#endif
/* SPIRENT _END */

/***************************************************************************
 * RvSipSecAgreeHandleMsgRcvd
 * ------------------------------------------------------------------------
 * General: Supplies a received message to the security-agreement object for manual
 *          processing. The security-agreement will use this message to load security
 *          information, verify the security-agreement validity, modify its inner state, etc.
 *          Notice: Calling RvSipSecAgreeHandleMsgRcvd() may invoke the
 *          RvSipSecAgreeMgrSecAgreeRequiredEv() callback, if the current
 *          security-agreement becomes invalid and the message requires a new
 *          security-agreement to be used.
 *          Notice: You can call RvSipSecAgreeHandleMsgRcvd() only if the given
 *          message was not processed by this security-agreement via the sip stack,
 *          i.e., do not call this function if this security-agreement is attached
 *          to the sip object (call-leg, register-client, etc) that received the
 *          message.
 *          Notice: In order to process requests, additional information on the arrival of the
 *          request is required. Use RvSipSecAgreeMgrGetMsgRcvdInfo() to obtain
 *          this information in the time specified for that (see RvSipSecAgreeMgrGetMsgRcvdInfo
 *          documentation).
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         hMsg            - Handle to the received message
 *         pMsgInfo        - The information on the message arrival. Required
 *                           for requests only.
 *         structSize      - The size of RvSipSecAgreeMsgRcvdInfo
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeHandleMsgRcvd(
							 IN  RvSipSecAgreeHandle		     hSecAgree,
							 IN  RvSipMsgHandle                  hMsg,
							 IN  RvSipSecAgreeMsgRcvdInfo*       pMsgInfo,
							 IN  RvInt32                         structSize);

/***************************************************************************
 * RvSipSecAgreeTerminate
 * ------------------------------------------------------------------------
 * General: Terminates the security agreement. All owners of this security-agreement
 *          will be notified not to use this agreement any more. All resources will be
 *          deallocated.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to terminate
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeTerminate(
							 IN   RvSipSecAgreeHandle    hSecAgree);

/*---------------------- O W N E R  F U N C T I O N S -----------------------*/

/***************************************************************************
 * RvSipSecAgreeSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement application handle. Usually the application
 *          replaces handles with the stack in the RvSipSecAgreeMgrCreateSecAgree()
 *          API function. This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement
 *            hAppSecAgree - A new application handle to the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetAppHandle(
                                      IN  RvSipSecAgreeHandle     hSecAgree,
                                      IN  RvSipAppSecAgreeHandle  hAppSecAgree);

/***************************************************************************
 * RvSipSecAgreeGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this security-agreement.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree      - Handle to the security-agreement
 * Output:    phAppSeccAgree - The application handle of the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetAppHandle(
                                      IN    RvSipSecAgreeHandle     hSecAgree,
                                      OUT   RvSipAppSecAgreeHandle *phAppSecAgree);

/***************************************************************************
 * RvSipSecAgreeGetNumOfOwners
 * ------------------------------------------------------------------------
 * General: Gets the current number of owners of this security-agreement object.
 *          Call this function in order to allocate a sufficient array for
 *          RvSipSecAgreeGetOwners().
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree         - Handle to the security-agreement object
 * Output:  pNumOfOwners      - The current number of owners of this security-
 *                              agreement object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetNumOfOwners(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvInt32                 *pNumOfOwners);

/***************************************************************************
 * RvSipSecAgreeGetOwnersList
 * ------------------------------------------------------------------------
 * General: Gets the current owners of this security-agreement object. The owners
 *          are inserted into an array you allocate prior to calling this function.
 *          To determine the size of array to allocate use RvSipSecAgreeGetNumOfOwners().
 *          If the number of actual owners is different than array size,
 *          actualNumOfOwners will indicate the actual number of owners set to
 *          the array.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree          - Handle to the security-agreement object
 *          arraySize          - The size of the given allocated array
 *          structSize         - The size of RvSipSecAgreeGetOwnersList
 * Inout:   pOwners            - A previously allocated array of owners. When function
 *                               returns, it will contain the list of current owners
 *                               of this security-agreement: handles and types.
 * Output:  pActualNumOfOwners - The actual number of owners that can be found
 *                               in the returned array, or that should be allocated
 *                               if function fails.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetOwnersList(
							 IN    RvSipSecAgreeHandle      hSecAgree,
							 IN    RvInt32                  arraySize,
							 IN    RvInt32                  structSize,
							 INOUT RvSipSecAgreeOwner      *pOwners,
							 OUT   RvInt32                 *pActualNumOfOwners);


/*------------------ S E T  A N D  G E T  F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeSetContext
 * ------------------------------------------------------------------------
 * General: Stores application context in the security-agreement object. This
 *          context is in addition to the application handle, and is used to
 *          hold the data shared by related security-agreement objects. Once 
 *			a new security-agreement is created from an old security-agreement by calling
 *          RvSipSecAgreeCopy(), the context is copied to the new security-agreement.
 *          Example of usage: For ipsec-3gpp negotiation, store IMPI details
 *          in the context parameter. Upon renegotiation, the IMPI will
 *          be copied from the old security-agreement to the new one, and will
 *          be available for you via the new security-agreement and its inner objects.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 *          pContext    - Pointer to the new application context to set to this
 *                        security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetContext(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   void*                    pContext);

/*@**************************************************************************
 * RvSipSecAgreeGetContext
 * ------------------------------------------------------------------------
 * General: Gets application context stored in the security-agreement object. This
 *          context is additional to the application handle, and shall be used to
 *          hold the data shared by related security-agreement objects. Once 
 *          a new security-agreement is created from an old security-agreement by calling
 *          RvSipSecAgreeCopy(), the context is copied to the new security-agreement.
 *          Example of usage: For ipsec-3gpp negotiation, store IMPI details
 *          in the context parameter. Then upon renegotiation, the IMPI will
 *          be copied from the old security-agreement to the new one, and will
 *          be available for you via the new security-agreement and its inner objects.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  ppContext   - Pointer to the application context of this security-agreement
 **************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetContext(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   void**                   ppContext);

/***************************************************************************
 * RvSipSecAgreeSetRole
 * ------------------------------------------------------------------------
 * General: Set the role of the security-agreement object: client or server.
 *          Notice: you must set the role of a security-agreeement to enable it
 *          for use.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 *          eRole       - The role to set to the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetRole(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 IN   RvSipSecAgreeRole        eRole);

/***************************************************************************
 * RvSipSecAgreeGetRole
 * ------------------------------------------------------------------------
 * General: Get the role of the security-agreement object: client or server.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  peState     - The role of this security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRole(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvSipSecAgreeRole       *peRole);

/***************************************************************************
 * RvSipSecAgreeGetState
 * ------------------------------------------------------------------------
 * General: Get the current state of the security-agreement object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree   - Handle to the security-agreement object
 * Output:  peState     - The state of this security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetState(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvSipSecAgreeState      *peState);

/***************************************************************************
 * RvSipSecAgreeGetRemoteSecurityList
 * ------------------------------------------------------------------------
 * General: Gets the remote list of security mechanisms. This list is
 *          received from the remote party to indicate the list of security
 *          mechanisms it supports.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree            - Handle to the security-agreement object
 * Output: phRemoteSecurityList - Pointer to the remote security list
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemoteSecurityList(
							 IN   RvSipSecAgreeHandle            hSecAgree,
							 OUT  RvSipCommonListHandle         *phRemoteSecurityList);

/***************************************************************************
 * RvSipSecAgreePushLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Push a security header to the local security list. Calling this
 *          function is allowed in states IDLE, CLIENT_REMOTE_LIST_READY and
 *          SERVER_REMOTE_LIST_READY. Once you are done building the local
 *          security list, call RvSipSecAgreeInit() to complete the initialization
 *          of the security-agreement object.
 *          Notice: You cannot set two headers with the same mechanism type
 *                  to the local list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree       - Handle to the security-agreement object
 *         hSecurityHeader - Handle to the local security header
 *         eLocation       - Inserted element location (first, last, etc).
 *         hPos            - Current list position, relevant in case that
 *							 location is next or prev.
 * Output: pNewPos         - The location of the object that was pushed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreePushLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipSecurityHeaderHandle      hSecurityHeader,
							 IN     RvSipListLocation              eLocation,
							 IN     RvSipCommonListElemHandle      hPos,
                             OUT    RvSipCommonListElemHandle*     phNewPos);

/***************************************************************************
 * RvSipSecAgreeGetLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Gets a security header from the local security list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the security-agreement object
 *         eLocation        - Location of the wanted element in the list
 *                            (first, last, next, prev).
 *         hPos             - Current list position, relevant in case that
 *							  location is next or prev.
 * Output: phSecurityHeader - The retrieved security header
 *         pNewPos          - The location of the object that was get.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipListLocation              eLocation,
							 IN     RvSipCommonListElemHandle      hPos,
                             OUT    RvSipSecurityHeaderHandle*     phSecurityHeader,
							 OUT    RvSipCommonListElemHandle*     phNewPos);

/***************************************************************************
 * RvSipSecAgreeRemoveLocalSecurityHeader
 * ------------------------------------------------------------------------
 * General: Removes a security header from the local security list. Calling this
 *          function is allowed in states IDLE, CLIENT_REMOTE_LIST_READY and
 *          SERVER_REMOTE_LIST_READY. Once you are done building the local
 *          security list, call RvSipSecAgreeInit() to complete the initialization
 *          of the security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the security-agreement object
 *         hPos             - Current list position, relevant in case that
 *							  location is next or prev.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeRemoveLocalSecurityHeader(
							 IN     RvSipSecAgreeHandle            hSecAgree,
							 IN     RvSipCommonListElemHandle      hPos);

/***************************************************************************
 * RvSipSecAgreeGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the security-agreement page, 
 *         and returns the new element handle.
 *         The application may use this function to allocate a message header 
 *         or a message address. It should then fill the element information
 *         and set it back to the security-agreement using the relevant 'set' 
 *         or 'push' functions.
 *
 *         Note: You may use this function only on initial state (before sending a message). 
 *         On any other state you must construct the header on an application page,
 *         and then set it to the stack object.
 *
 *         The function supports the following elements:
 *         Security		  - You should set these headers back with the  
 *						    RvSipSecAgreePushLocalSecurityHeader() API functions.
 *         Authentication - you should set this header back with the 
 *                          RvSipSecAgreeServerSetProxyAuthenticate() API function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the security-agreement object
 *			  eHeaderType - The type of header to allocate. 
 *            eAddrType   - For future use. Supply RVSIP_ADDRTYPE_UNDEFINED.
 * Output:    phHeader    - Handle to the newly created header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetNewMsgElementHandle (
                                      IN   RvSipSecAgreeHandle  hSecAgree,
									  IN   RvSipHeaderType      eHeaderType,
									  IN   RvSipAddressType     eAddrType,
                                      OUT  void*               *phHeader);

/*------------------ C L I E N T   F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeClientSetChosenSecurity
 * ------------------------------------------------------------------------
 * General: The security-agreement process completes in choosing a security
 *          mechanism to use in further requests. Once the local security
 *          list and the remote security list are obtained, the client security-agreement
 *          object moves to the CLIENT_CHOOSE_SECURITY state. Then, in order to activate
 *          the security-agreement object, you must call RvSipSecAgreeClientSetChosenSecurity().
 *          You can either specify the security mechanism you choose, or indicate that you wish
 *          the sip stack to compute the best mechanism based on the server preferences.
 *          Once the security mechanism is chosen the client security-agreement object moves to
 *          the ACTIVE state, and assures that the next request will be sent according to the
 *          selected mechanism.
 *          If an uninitialized client security-agreement received a remote security list,
 *          it assumes the CLIENT_REMOTE_LIST_READY state. In this state you can avoid
 *          initializing a local security list, by manually determining the chosen security
 *          via RvSipSecAgreeSetChosenSecurity().
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the client security-agreement object
 * Inout:  peSecurityType   - The chosen security mechanism. Use this parameter
 *                            to specify your choice for a security mechanism. If you set
 *                            it to RVSIP_SECURITY_MECHANISM_UNDEFINED, the best
 *                            security mechanism will be computed by the security-agreement
 *                            object, and will be returned here.
 * Output: phLocalSecurity  - The local security header specifying the chosen security
 *         phRemoteSecurity - The remote security header specifying the chosen security
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientSetChosenSecurity(
							 IN     RvSipSecAgreeHandle             hSecAgree,
							 INOUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT    RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT    RvSipSecurityHeaderHandle      *phRemoteSecurity);

/***************************************************************************
 * RvSipSecAgreeClientGetChosenSecurity
 * ------------------------------------------------------------------------
 * General: Gets the security mechanism chosen by this client security-agreement.
 *          If the client security-agreement is in state CLIENT_CHOOSE_SECURITY and
 *          a chosen  mechanism was not set to it yet, calling 
 *          RvSipSecAgreeClientGetChosenSecurity() will invoke the computation of 
 *          the most preferable security mechanism, which will be returned here.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree        - Handle to the client security-agreement object
 * Output: peSecurityType   - The security mechanism chosen by this security-agreement
 *         phLocalSecurity  - The local security header specifying the chosen security
 *         phRemoteSecurity - The remote security header specifying the chosen security
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetChosenSecurity(
							 IN   RvSipSecAgreeHandle             hSecAgree,
							 OUT  RvSipSecurityMechanismType     *peSecurityType,
							 OUT  RvSipSecurityHeaderHandle      *phLocalSecurity,
							 OUT  RvSipSecurityHeaderHandle      *phRemoteSecurity);

/***************************************************************************
 * RvSipSecAgreeClientReturnToLocalListReady
 * ------------------------------------------------------------------------
 * General: Returns a client security-agreement object to state CLIENT_LOCAL_LIST_READY. 
 *          Returning to the local list ready state allows you to reinsert security-agreement
 *          information to messages. For example, when a message fails to be sent,
 *          the client security-agreement object moves to the INVALID state and you
 *          receive a message send failure indication on the SIP object. Then you should
 *          call RvSipSecAgreeClientReturnToLocalListReady() before proceeding to the
 *          next DNS entry, in order for the next message to contain the security-agreement
 *          information.
 *          Notice: You cannot call this function if the security-agreement was once
 *                  ACTIVE.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to return to state
 *                       CLIENT_LOCAL_LIST_READY
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientReturnToLocalListReady(
							 IN   RvSipSecAgreeHandle    hSecAgree);

/***************************************************************************
 * RvSipSecAgreeClientSetAddSecurityClientToMsg
 * ------------------------------------------------------------------------
 * General: Used in state ACTIVE to indicate that the following outgoing requests 
 *          should contain the list of Security-Client headers.
 *          Notice: If you do not set this flag, the client security-agreement
 *                  object will autonomously add the list of Security-Client 
 *                  headers to outgoing requests according to RFC3329 and TS33.203.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree     - Handle to the client security-agreement object
 *			  bAddSecClient - RV_TRUE to add Security-Client headers to outgoing
 *                            requests. RV_FALSE not to.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientSetAddSecurityClientToMsg(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      IN   RvBool                bAddSecClient);

/***************************************************************************
 * RvSipSecAgreeClientGetAddSecurityClientToMsg
 * ------------------------------------------------------------------------
 * General: Used in state ACTIVE to get an indication to whether the Security-
 *          Client list will be added to the following outgoing request.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree      - Handle to the client security-agreement object
 * Output:	  pbAddSecClient - RV_TRUE if following requests will contain a
 *                             Security-Client list, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetAddSecurityClientToMsg(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      OUT  RvBool               *pbAddSecClient);

/***************************************************************************
 * RvSipSecAgreeClientGetAuthObj
 * ------------------------------------------------------------------------
 * General: Gets the authentication object from the client security-agreement.
 *          This authentication object was built from the Proxy-Authenticate
 *          header that was received in the server response.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the client security-agreement object
 * Output:    phAuthObj   - Handle to the authentication object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeClientGetAuthObj(
                                      IN   RvSipSecAgreeHandle   hSecAgree,
                                      OUT  RvSipAuthObjHandle*   phAuthObj);

/*------------------ S E R V E R   F U N C T I O N S -------------------*/

/***************************************************************************
 * RvSipSecAgreeServerGetRecommendedResponseCode
 * ------------------------------------------------------------------------
 * General: Gets the response code recommended by this server security-agreement,
 *          or 0 if there is no recommendation. You can use the recommended response
 *          code to respond in compliance to RFC3329.
 *			Notice: The response code recommended here is specific to RFC3329 and 
 *                  as such it does not apply to other SIP considerations (for example,
 *                  BYE request within a connected call-leg must be responded with 2xx). 
 *					It is your responsibility to choose the correct response code to use.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree       - Handle to the server security-agreement object
 * Output:  peResponseCode  - The recommended response code. 0 if there is no
 *                            recommendation.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerGetRecommendedResponseCode(
							 IN   RvSipSecAgreeHandle      hSecAgree,
							 OUT  RvUint16*                peResponseCode);

/***************************************************************************
 * RvSipSecAgreeServerSetProxyAuthenticate
 * ------------------------------------------------------------------------
 * General: Sets a Proxy-Authenticate header to the server security-agreement.
 *          The server security-agreement will insert this header whenever it 
 *          inserts the Security-Server list to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree   - Handle to the server security-agreement object
 *            hProxyAuth  - The Proxy-Authenticate header to set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerSetProxyAuthenticate(
                                    IN   RvSipSecAgreeHandle              hSecAgree,
                                    IN   RvSipAuthenticationHeaderHandle  hProxyAuth);

/***************************************************************************
 * RvSipSecAgreeServerGetProxyAuthenticate
 * ------------------------------------------------------------------------
 * General: Gets the Proxy-Authenticate header of this server security-agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the server security-agreement object
 * Output:    phProxyAuth  - Handle to the Proxy-Authenticate header
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerGetProxyAuthenticate(
                                    IN   RvSipSecAgreeHandle                hSecAgree,
                                    OUT  RvSipAuthenticationHeaderHandle*   phProxyAuth);

/***************************************************************************
 * RvSipSecAgreeServerForceIpsecSecurity
 * ------------------------------------------------------------------------
 * General: Makes the previously obtained ipsec-3gpp channels serviceable at the server 
 *          security-agreement. Calling this function will cause any outgoing request 
 *          from this server to be sent over those ipsec-3gpp channels.
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
 * Input:     hSecAgree    - Handle to the server security-agreement object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerForceIpsecSecurity(
                                    IN   RvSipSecAgreeHandle            hSecAgree);

/***************************************************************************
 * RvSipSecAgreeServerReturnToActive
 * ------------------------------------------------------------------------
 * General: Returns a server security-agreement object to state ACTIVE.
 *          When the server security-agreement receives an indication for renegotiation
 *          (indicated by a change in Security-Client list) it changes state to INVALID. 
 *          The renegotiation is performed by a new security-agreement, which will eventually
 *          take over the old INVALID security-agreement. However according to 3GPP TS 24.229, 
 *          the renegotiation might be completed with no need to switch security-associations, 
 *          in which case the old security-agreement shall regain control over the SIP objects
 *          and their message sending. In order to recover from the INVALID state at the old
 *          security-agreement, call RvSipSecAgreeServerReturnToActive().
 *          
 *          Notice: You can invoke this function only when the security-agreement is INVALID
 *                  but the security-object is still ACTIVE.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSecAgree   - Handle to the security-agreement to return to state ACTIVE
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeServerReturnToActive(
							 IN   RvSipSecAgreeHandle    hSecAgree);

/*--------- S E C U R I T Y  A N D  N E T W O R K  F U N C T I O N S --------*/

/***************************************************************************
 * RvSipSecAgreeSetLocalAddr
 * ------------------------------------------------------------------------
 * General: Sets a local address to use in establishing a secure channel with the
 *          remote party.
 *          If the security-agreement is required to initiate an ipsec-3gpp channel,
 *          or if it is required to initiate a tls connection (clients only),
 *          the local address bound to the connection is chosen from the set of
 *          local addresses, according to the characteristics of the remote address.
 *          You can use this function to force the security-agreement to choose a
 *          specific local address for a specific transport type.
 *          If you do not set a local address to the security-agreement, a default
 *          one is used.
 *          Notice: If you wish to set a local address to the security-agreement,
 *          you must set it before calling RvSipSecAgreeInit().
 *          Notice: If you wish to set local address for ipsec-3gpp use, specify here local
 *          IP (must be v4) and transport type (must be UDP or TCP). The local ports must be
 *          set via local Security header using RvSipSecAgreePushLocalSecurityHeader().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that should use the specified local
 *                           address (ipsec-3gpp or TLS).
 *            pLocalAddr   - The local address structure
 *            structSize   - The size of RvSipTransportAddr
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetLocalAddr(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecurityMechanismType  eMechanism,
                                    IN  RvSipTransportAddr         *pLocalAddress,
									IN  RvInt32                     structSize);

/***************************************************************************
 * RvSipSecAgreeGetLocalAddr
 * ------------------------------------------------------------------------
 * General: Gets the local address used by this security-agreement.
 *          The local address is used for initiating ipsec-3gpp channels, or
 *          TLS connections.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that the requested local
 *                           address is used for (ipsec-3gpp or TLS).
 *            eAddressType - The type of the requested local address (ipv4 or ipv6).
 *                           Relevant only for TLS addresses.
 *            structSize   - The size of RvSipTransportAddr
 * Inout:     pLocalAddr   - The local address structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetLocalAddr(
                                    IN     RvSipSecAgreeHandle         hSecAgree,
									IN     RvSipSecurityMechanismType  eMechanism,
									IN	   RvSipTransportAddressType   eAddressType,
									IN     RvInt32                     structSize,
                                    INOUT  RvSipTransportAddr         *pLocalAddress);

/***************************************************************************
 * RvSipSecAgreeSetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Sets the remote address to use in establishing a secure channel with the
 *          remote party.
 *          If the security-agreement is required to initiate ipsec-3gpp channels,
 *          or if it is required to initiate a tls connection (clients only),
 *          the remote address you set here will be used.
 *          Notice: If TLS is negotiated, you must call RvSipSecAgreeSetRemoteAddr()
 *                  to set TLS remote details before calling RvSipSecAgreeInit().
 *          Notice: If ipsec-3gpp is negotiated you may call RvSipSecAgreeSetRemoteAddr()
 *                  to set remote IP (must be v4) and transport to use (must be UDP or TCP),
 *                  before calling RvSipSecAgreeInit(). Otherwise, the address used
 *                  for negotiation will be used for establishing ipsec-3gpp channels.
 *                  The remote ports are received from the server, hence do not set ports here.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that should use the specified remote
 *                           address (ipsec-3gpp or TLS).
 *            pRemoteAddr  - The remote address structure
 *            structSize   - The size of RvSipTransportAddr
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetRemoteAddr(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecurityMechanismType  eMechanism,
                                    IN  RvSipTransportAddr         *pRemoteAddress,
									IN  RvInt32                     structSize);

/***************************************************************************
 * RvSipSecAgreeGetRemoteAddr
 * ------------------------------------------------------------------------
 * General: Gets the remote address used by this security-agreement.
 *          The remote address is used for initiating ipsec-3gpp channels, or
 *          TLS connections.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eMechanism   - The mechanism that the requested remote
 *                           address is used for (ipsec-3gpp or TLS).
 *            structSize   - The size of RvSipTransportAddr
 * Inout:     pRemoteAddr  - The remote address structure
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemoteAddr(
                                    IN    RvSipSecAgreeHandle          hSecAgree,
                                    IN    RvSipSecurityMechanismType   eMechanism,
									IN    RvInt32                      structSize,
                                    INOUT RvSipTransportAddr          *pRemoteAddress);

/***************************************************************************
 * RvSipSecAgreeSetSendOptions
 * ------------------------------------------------------------------------
 * General: Sets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp. The send options shall be set
 *          in parallel to setting the remote address via 
 *          RvSipSecAgreeSetRemoteAddr (or deciding that the default remote
 *          address shall be used.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pOptions	   - Pointer to the configured options structure.
 *            structSize   - The size of pOptions
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetSendOptions(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions,
									IN  RvInt32                     structSize); 

/***************************************************************************
 * RvSipSecAgreeGetSendOptions
 * ------------------------------------------------------------------------
 * General: Gets the sending options of the security agreement. The sending
 *          options affect the message sending of the security object, for
 *          example may be used to instruct the security-object to send 
 *          messages compressed with sigcomp.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pOptions	   - Pointer to the options structure. Make sure to
 *                           supply a buffer of size RVSIP_COMPARTMENT_MAX_URN_LENGTH
 *                           for the sigcomp-id.
 *            structSize   - The size of pOptions
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSendOptions(
                                    IN  RvSipSecAgreeHandle         hSecAgree,
									IN  RvSipSecAgreeSendOptions   *pOptions,
									IN  RvInt32                     structSize); 

/***************************************************************************
 * RvSipSecAgreeSetIpsecInfo
 * ------------------------------------------------------------------------
 * General: Sets ipsec-3gpp information to the security-agreement. This information
 *          is essential for obtaining an active ipsec-3gpp channel with the remote
 *          party.
 *          Notice: If you wish to allow sending via ipsec-3gpp, you must supply 
 *          ipsec-3gpp information before calling RvSipSecAgreeInit().
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            pIpsecInfo   - Pointer to the structure of ipsec-3gpp information
 *            structSize   - The size of RvSipSecAgreeIpsecInfo
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetIpsecInfo(
                                    IN   RvSipSecAgreeHandle      hSecAgree,
                                    IN   RvSipSecAgreeIpsecInfo  *pIpsecInfo,
									IN   RvInt32                  structSize);

/***************************************************************************
 * RvSipSecAgreeGetIpsecInfo
 * ------------------------------------------------------------------------
 * General: Gets the ipsec-3gpp information of this security-agreement. This information
 *          is essential for obtaining an active ipsec-3gpp channel with the remote
 *          party.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            structSize   - The size of RvSipSecAgreeIpsecInfo
 * Inout:     pIpsecInfo   - Pointer to the structure of ipsec-3gpp information. This
 *                           structure will be filled with ipsec-3gpp information from
 *                           the security-agreement
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetIpsecInfo(
                                    IN    RvSipSecAgreeHandle      hSecAgree,
									IN    RvInt32                  structSize,
                                    INOUT RvSipSecAgreeIpsecInfo  *pIpsecInfo);

/***************************************************************************
 * RvSipSecAgreeGetSecObj
 * ------------------------------------------------------------------------
 * General: Gets the security object of this security-agreement. The security
 *          object is used by the owners of this security-agreement for sending
 *          request messages. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree  - Handle to the security-agreement object
 * Output:    phSecObj   - Handle to the security object
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSecObj(
                                    IN   RvSipSecAgreeHandle    hSecAgree,
                                    OUT  RvSipSecObjHandle*     phSecObj);

/***************************************************************************
 * RvSipSecAgreeStartIpsecLifetimeTimer
 * ------------------------------------------------------------------------
 * General: Starts a lifetime timer for the ipsec-3gpp channels. The expiration
 *          of this timer invokes the RvSipSecAgreeStatusEv() callback with status
 *          RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED. 
 *
 *          Note: The security-agreement handles a lifetime timer expiration by 
 *                reporting it to the application via the RvSipSecAgreeStatusEv() 
 *                callback. No additional action is taken automatically by the 
 *                security-agreement. Upon callback invoking the application can 
 *                handle the lifetime timer expiration, for example by terminating 
 *                the security-agreement.
 *          Note: If a lifetime timer was already started, calling 
 *                RvSipSecAgreeStartIpsecLifetimeTimer() will restart the timer 
 *                according to the given lifetime interval
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree        - Handle to the security-agreement object
 *          lifetimeInterval - The lifetime interval in seconds
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeStartIpsecLifetimeTimer(
                                    IN RvSipSecAgreeHandle    hSecAgree,
								    IN RvUint32               lifetimeInterval);

/***************************************************************************
 * RvSipSecAgreeGetRemainingIpsecLifetime
 * ------------------------------------------------------------------------
 * General: Returns the time remaining until the lifetime timer will expire.
 *          The lifetime timer was started by a call to
 *          RvSipSecAgreeStartIpsecLifetimeTimer()
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecAgree      - Handle to the security-agreement object
 * Output:  pRemainingTime - The remaining time in seconds. 
 *							 UNDEFINED if the timer is not started
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetRemainingIpsecLifetime(
                                    IN   RvSipSecAgreeHandle    hSecAgree,
									OUT  RvInt32*				pRemainingTime);

/*--------- S P E C I A L   S T A N D A R D S   F U N C T I O N S --------*/

/***************************************************************************
 * RvSipSecAgreeSetSpecialStandardFlag
 * ------------------------------------------------------------------------
 * General: Sets an indication to the security-agreement that it shall support
 *          the given special standard. 
 *          Note: This may imply changes in the behavior of the security-agreement
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 *            eStandard    - The standard enumeration
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeSetSpecialStandardFlag(
                                    IN   RvSipSecAgreeHandle               hSecAgree,
                                    IN   RvSipSecAgreeSpecialStandardType  eStandard);

/***************************************************************************
 * RvSipSecAgreeGetSpecialStandardFlag
 * ------------------------------------------------------------------------
 * General: Gets the special standard that was previously set to this 
 *          security-agreement via RvSipSecAgreeSetSpecialStandardFlag
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree    - Handle to the security-agreement object
 * Output:    peStandard   - The standard enumeration
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSpecialStandardFlag(
                                    IN  RvSipSecAgreeHandle                hSecAgree,
                                    OUT RvSipSecAgreeSpecialStandardType  *peStandard);

/***************************************************************************
 * RvSipSecAgreeGetSecurityAssociationType
 * ------------------------------------------------------------------------
 * General: Gets the type of the Security Associations held by this
 *          security-agreement. The type of Security Association is defined
 *          in TISPAN ES-283.003. A change in Security Association type is 
 *          reported via the RvSipSecAgreeStatusEv() callback.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSecAgree        - Handle to the security-agreement object
 * Input:     peSecAssocType   - The Security Association type
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecAgreeGetSecurityAssociationType(
                                    IN  RvSipSecAgreeHandle                    hSecAgree,
                                    OUT RvSipSecAgreeSecurityAssociationType  *peSecAssocType);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
int RvSipSecAgreeIsReady(RvSipSecAgreeHandle sa, int resp);
int RvSipSecObjIsReady(RvSipSecObjHandle soh, int resp);
RvStatus RvSipSecAgreeResetIpsecAddr(RvSipSecAgreeHandle hSecAgree);
#endif
/* SPIRENT_END */

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RVSIP_SEC_AGREE_H */

