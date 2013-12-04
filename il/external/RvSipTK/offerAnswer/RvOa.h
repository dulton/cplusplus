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
 *                              <RvOa.h>
 *
 *    Author                        Date
 *    ------                        --------
 *    Igor							Aug 2006
 *****************************************************************************/

/*@****************************************************************************
 * Module: Offer-Answer
 * ----------------------------------------------------------------------------
 * Title: Offer-Answer Functions
 *
 * The RvOa.h file defines API, provided by the Offer-Answer (OA) Module.
 * The API functions can be divided in following groups:
 *  1. Offer-Answer Manager API
 *  2. Offer-Answer Session API
 *  3. Offer-Answer Stream API
 *
 * Each group contains functions for creation of correspondent object and
 * for inspection & modifying the object parameters.
 *
 * The Offer-Answer Module implements the Offer-Answer model of negotiation
 * of media capabilities by SIP endpoints, described in RFC3264.
 * The Offer-Answer Module implements the abstractions and their behavior, as
 * it is defined by RFC 3264.
 ***************************************************************************@*/

#ifndef _RV_OA_H
#define _RV_OA_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "RvOaTypes.h"

/*---------------------------------------------------------------------------*/
/*                      OFFER-ANSWER MANAGER API                             */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaInitCfg (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Initializes the RvOaCfg structure. The application should provide the
 *  RvOaCfg structure to RvOaConstruct(), to construct the
 *  Offer-Answer Module. RvOaInitCfg() fills the structure fields with the
 *  default values.
 *
 * Arguments:
 * Input:  sizeOfCfg - The size of the structure to be initialized in bytes.
 * Output: pOaCfg    - The structure to be initialized.
 *
 * Return Values: None.
 ***************************************************************************@*/
RVAPI void RVCALLCONV RvOaInitCfg(IN  RvUint32 sizeOfCfg,
                                  OUT RvOaCfg* pOaCfg);

/*@****************************************************************************
 * RvOaConstruct (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs the Offer-Answer Module using the configuration that the application
 *  provided.
 *
 * Arguments:
 * Input:  pOaCfg    - The Offer-Answer Module configuration.
 *         sizeOfCfg - The size of the configuration structure in bytes.
 * Output: phOaMgr   - The handle of the Offer-Answer Manager object (OAMgr) of the Offer-Answer Module.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaConstruct(
                                IN  RvOaCfg*       pOaCfg,
                                IN  RvUint32       sizeOfCfg,
                                OUT RvOaMgrHandle* phOaMgr);

/*@****************************************************************************
 * RvOaDestruct (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs the OAMgr and frees all resources that the
 *  Offer-Answer Module uses.
 *
 * Arguments:
 * Input:  hOaMgr - The handle to the OAMgr of the Offer-Answer Module to be
 *                  destructed.
 * Output: None.
 *
 * Return Values: None.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaDestruct(IN RvOaMgrHandle    hOaMgr);

/*@****************************************************************************
 * RvOaGetResources (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the various statistics about the consumption of resources by the
 *  Offer-Answer Module.
 *
 * Arguments:
 * Input:  hOaMgr          - The handle to the OAMgr.
 *         sizeOfResources - The size of the resources structure in bytes.
 * Output: pOaResources    - The resource consumption statistics.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaGetResources(
                                IN  RvOaMgrHandle   hOaMgr,
                                IN  RvUint32        sizeOfResources,
                                OUT RvOaResources*  pOaResources);

/*@****************************************************************************
 * RvOaSetLogFilters (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the filters for filtering the information that the Offer-Answer Module printed
 *  into the log. This function can be called any time during the life cycle
 *  of the Offer-Answer Module. The filters are represented by the filter mask, which
 *  combines different filters using a bit-wise OR operator.
 *  For example, if the application wants to log errors and exceptions that only
 *  occur in the Offer-Answer Module during SDP session processing, it should set the filter
 *  mask to the (RVOA_LOG_ERROR_FILTER | RVOA_LOG_EXCEP_FILTER) value.
 *
 * Arguments:
 * Input:  hOaMgr     - The handle to the OAMgr.
 *         eLogSource - The source of the data to be logged.
 *                      An example of a log source can be a capabilities-related
 *                      code, or message handling functions, and so on.
 *         filters    - The filter mask to be set.
 * Output: None
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSetLogFilters(
                                IN RvOaMgrHandle  hOaMgr,
                                IN RvOaLogSource  eLogSource,
                                IN RvInt32        filters);

/*@****************************************************************************
 * RvOaDoesLogFilterExist (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Checks if the specific log filter is set into the Offer-Answer Module.
 *
 * Arguments:
 * Input:  hOaMgr - The handle to the OAMgr.
 *         filter - The filter to be checked.
 * Output: none.
 *
 * Return Values: RV_TRUE if the filter is set. Otherwise, RV_FALSE.
 ***************************************************************************@*/
RVAPI RvBool RVCALLCONV RvOaDoesLogFilterExist(
                                IN RvOaMgrHandle  hOaMgr,
                                IN RvOaLogFilter  filter);

/*@****************************************************************************
 * RvOaSetCapabilities (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads default media capabilities into the Offer-Answer Module.
 *  The capabilities should be provided in the form of a proper SDP message.
 *  If the local capabilities were not set using RvOaSessionSetCapabilities(),
 *  the default capabilities are used for:
 *      1. Generating an OFFER message
 *      2. Identifying the set of codecs supported by both offerer and answerer
 *
 *  Note: The OFFER message is generated by copy-construct of the SDP message
 *        that contains the capabilities. For this reason, the capabilities
 *        should be provided in the form of a proper SDP message.
 *
 * Arguments:
 * Input:  hOaMgr    - The handle to the OAMgr.
 *         strSdpMsg - The default capabilities as a NULL-terminated string.
 *                     Can be NULL.
 *         pSdpMsg   - The default capabilities in the form of a RADVISION
 *                     SDP Stack message. Can be NULL.
 *                     This parameter has a lower priority than strSdpMsg.
 * Output: none.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSetCapabilities(
                            IN  RvOaMgrHandle   hOaMgr,
                            IN  RvChar*         strSdpMsg,
                            IN  RvSdpMsg*       pSdpMsg);

/*@****************************************************************************
 * RvOaGetCapabilities (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the default media capabilities of the Offer-Answer Module.
 *  The application can set the capabilities into the Offer-Answer Module using
 *  RvOaSetCapabilities().
 *  The default capabilities are held in the form of a RADVISION SDP Stack message
 *  object. This object uses RPOOL pages to hold its data. It manages pages
 *  by means of the allocator object. Therefore, the application has to provide
 *  the allocator object that is to be used while building a copy of default capabilities
 *  message. The allocator object can be created using RADVISION SDP Stack API.
 *
 * Arguments:
 * Input:  hOaMgr         - The handle to the OAMgr.
 *         lenOfStrSdpMsg - The maximal length of the string returned by the Offer-Answer
 *                          Module using the strSdpMsg parameter.
 *                          This parameter is meaningful only if the strSdpMsg
 *                          parameter is not NULL.
 *         pSdpAllocator  - The allocator object to be used while building
 *                          the message that holds the default capabilities.
 *                          This parameter is meaningful only if the pSdpMsg
 *                          parameter is not NULL.
 * Output: strSdpMsg      - The default capabilities as a NULL-terminated string.
 *                          Can be NULL.
 *         pSdpMsg        - The default capabilities in form of a RADVISION SDP Stack
 *                          message. Can be NULL.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaGetCapabilities(
                            IN  RvOaMgrHandle   hOaMgr,
                            IN  RvUint32        lenOfStrSdpMsg,
                            IN  RvAlloc*        pSdpAllocator,
                            OUT RvChar*         strSdpMsg,
                            OUT RvSdpMsg*       pSdpMsg);

/*@****************************************************************************
 * RvOaCreateSession (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Creates a session object (session). The application should use the session object for
 *  the negotiation of media capabilities. The application should provide the
 *  OFFER and ANSWER messages (that were received as bodies of SIP messages)
 *  to the session. As a result, the session will generate
 *  stream objects (stream) for each negotiated media description. Using these
 *  objects, the application can find various parameters of the Offer-Answer
 *  stream to be transmitted, the including remote
 *  address and media format to be used, the direction of the transmission,
 *  and so on.
 *  The session can generate either OFFER to be sent to answerer, based on
 *  the local or default capabilities, or ANSWER to be sent to the offerer, based
 *  on the received OFFER and local or default capabilities. The generated message
 *  can be gotten anytime during life cycle of the session, using
 *  RvOaSessionGetMsgToBeSent().
 *  The created session stays in the IDLE state until
 *  RvOaSessionGenerateOffer() or RvOaSessionSetRcvdMsg() is called.
 *
 * Arguments:
 * Input:  hOaMgr      - The handle to the OAMgr.
 *         hAppSession - The handle to the application object, which is going
 *                       to be served by the session.
 * Output: phSession   - The handle to the created session.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaCreateSession(
                                IN  RvOaMgrHandle           hOaMgr,
                                IN  RvOaAppSessionHandle    hAppSession,
                                OUT RvOaSessionHandle*      phSession);

/*---------------------------------------------------------------------------*/
/*                                SESSION API                                */
/*---------------------------------------------------------------------------*/
/*@****************************************************************************
 * RvOaSessionGenerateOffer (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Causes a session to build an OFFER message.
 *  After calling this function, the application can get the OFFER message in the
 *  format of a NULL-terminated string and send it with the SIP message.
 *  This can be done using RvOaSessionGetMsgToBeSent() any time during
 *  the life cycle of the session.
 *  RvOaSessionGenerateOffer() performs the following actions:
 *  1. Builds the OFFER message in the RADVISION SDP Stack message format.
 *     The OFFER message is built by the copy-construct operation, when the local
 *     or default capabilities are used as a source.
 *	   The local capabilities can be set into the session using
 *     RvOaSessionSetCapabilities().
 *	   The default capabilities can be set into the session using
 *     RvOaSetCapabilities().
 *     The local capabilities have a higher priority than the default ones.
 *     If local or default capabilities were not set, the empty message
 *     will be built.
 *  2. Adds the missing mandatory lines to the built message, using the default values.
 *  3. Generates a stream for each media descriptor found in the built
 *     message, and inserts it into the list that the session manages.
 *
 *  Any time during the life cycle of a session, the application can iterate
 *  through the list of streams that belong to the session.
 *  The application can inspect and modify its attributes and parameters for
 *  each stream. This can be done using the Offer-Answer Module API for the stream,
 *  or using the RADVISION SDP Stack API for the message object.
 *  Note: The SDP Stack API requires the handle to the message structure that
 *  contains the session description. This handle can be gotten from the session
 *  object using RvOaSessionGetSdpMsg().
 *  RvOaSessionGenerateOffer() moves the session from the IDLE and
 *  ANSWER_READY states to the OFFER_READY state. The session stays
 *  in the OFFER_READY state until RvOaSessionSetRcvdMsg() is called.
 *  RvOaSessionGenerateOffer() can be called for sessions in the IDLE,
 *  ANSWER_RCVD, or ANSWER_READY states only.
 *
 * Arguments:
 * Input:  hSession - The the handle to the session.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGenerateOffer(
                                    IN  RvOaSessionHandle     hSession);

/*@****************************************************************************
 * RvOaSessionTerminate (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Terminates a session. This function frees the resources that were
 *  consumed by the session. After calling this function the session
 *  object can no longer be referenced.
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 *                This function should not normally fail. A failure should be
 *                treated as an exception.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionTerminate(
                                    IN  RvOaSessionHandle     hSession);

/*@****************************************************************************
 * RvOaSessionSetAppHandle (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the handle to the application object that is served by the session.
 *  Actually, the application handle is sizeof(RvOaAppSessionHandle) bytes of
 *  application data, stored in the session.
 *  This data can be retrieved using RvOaSessionGetAppHandle().
 *
 * Arguments:
 * Input:  hSession    - The handle to the session.
 *         hAppSession - The handle to the application object.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetAppHandle(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvOaAppSessionHandle  hAppSession);

/*@****************************************************************************
 * RvOaSessionGetAppHandle (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the handle to the application object. The application handle can be
 *  set into the session using RvOaSessionSetAppHandle() or
 *  RvOaCreateSession().
 *
 * Arguments:
 * Input:  hSession    - The handle to the session.
 * Output: hAppSession - The handle to the application object.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetAppHandle(
                                    IN  RvOaSessionHandle     hSession,
                                    OUT RvOaAppSessionHandle* phAppSession);

/*@****************************************************************************
 * RvOaSessionSetCapabilities (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads the local media capabilities into the session.
 *  The local capabilities are used for:
 *      1. Generating an OFFER message
 *      2. Identifying the set of codecs supported by both offerer and answerer
 *
 *  Note: The OFFER message is generated by copy-construct of the SDP message
 *        that contains the capabilities. For this reason, the capabilities
 *        should be provided in the form of a proper SDP message.
 *
 *  The capabilities can be set into IDLE sessions only.
 *
 * Arguments:
 * Input:  hsession  - The handle to the session.
 *         strSdpMsg - The local capabilities as a NULL-terminated string.
 *                     Can be NULL.
 *         pSdpMsg   - The local capabilities in form of RADVISION SDP Stack
 *                     message. Can be NULL.This parameter has a lower priority
 *                     than strSdpMsg.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetCapabilities(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvChar*               strSdpMsg,
                                    IN  RvSdpMsg*             pSdpMsg);

/*@****************************************************************************
 * RvOaSessionSetRcvdMsg (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads an incoming SDP message into a session. The incoming SDP
 *  message can be OFFER or ANSWER, which is determined by the state of the
 *  session. If RvOaSessionGenerateOffer() was previously called for the
 *  session , the message is ANSWER. Otherwise, it is OFFER.
 *
 *  For the offerer, RvOaSessionSetRcvdMsg() moves the session from the
 *  OFFER_READY state to the ANSWER_RCVD state.
 *  For the answerer, RvOaSessionSetRcvdMsg() moves the session from the
 *  IDLE state to the ANSWER_READY state.
 *
 * Arguments:
 * Input:  hSession  - The handle to the session.
 *         strSdpMsg - The SDP message in the form of a NULL-terminated string.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetRcvdMsg(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvChar*               strSdpMsg);

/*@****************************************************************************
 * RvOaSessionGetMsgToBeSent (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the outgoing SDP message from the session.
 *  The outgoing SDP message can be OFFER or ANSWER. If the SDP message was
 *  generated as a result of calling RvOaSessionGenerateOffer(), the message
 *  is OFFER. If the SDP message was generated as a result of calling
 *  RvOaSessionSetRcvdMsg(), the message is ANSWER.
 *
 * Arguments:
 * Input:  hSession  - The handle to the session.
 *         buffSize  - The size in bytes of the memory buffer, pointed by strSdpMsg.
 * Output: strSdpMsg - The SDP message in the form of a NULL-terminated string.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetMsgToBeSent(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvUint32              buffSize,
                                    OUT RvChar*               strSdpMsg);

/*@****************************************************************************
 * RvOaSessionGetStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the handle of the streams that belong to the session.
 *  Using this handle and Stream API, the application can retrieve capabilities
 *  and addresses of the remote side, or inspect and update local capabilities
 *  and local addresses.
 *  The remote capabilities and addresses are stored in the remote SDP message.
 *  The local capabilities and addresses are stored in the local SDP message.
 *  Pointers to the local and/or remote SDP messages be retrieved using
 *  RvOaSessionGetSdpMsg().
 *  The local and remote SDP messages can be referenced as OFFER or ANSWER.
 *  If the application is an offerer, the local message represents OFFER and
 *  the remote message represents ANSWER.
 *  If the application is an answerer, the local message represents ANSWER and
 *  the remote message represents OFFER.
 *
 *  The application can retrieve and update the session description using
 *  a pointer to the SDP message and the RADVISION SDP Stack API functions.
 *
 * Arguments:
 * Input:  hSession    - The handle to the session.
 *         hPrevStream - The handle to the stream that appears before
 *                       phStrem in the session's stream list.
 *                       Can be NULL. In this case, the handle of the first stream
 *                       will be supplied by the function.
 * Output: phStream    - The handle to the requested stream.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetStream(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvOaStreamHandle      hPrevStream,
                                    OUT RvOaStreamHandle*     phStream);

/*@****************************************************************************
 * RvOaSessionAddStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Adds a new stream to the session. As a result of this operation,
 *  the new stream is created, and a local SDP message is extended with
 *  a new media descriptor. This media descriptor describes the media to be
 *  transmitted over this stream. The application can use the handle of the
 *  new stream to update various parameters of the media using the Stream API
 *  functions. The configuration of the new stream, which is provided to this
 *  function as a parameter, supplies the data required for building the
 *  media descriptor.
 *  The application can retrieve and update the stream description using a pointer
 *  to the media descriptor and the RADVISION SDP Stack API functions. This pointer
 *  can be retrieved anytime using RvOaStreamGetMediaDescriptor().
 *
 * Arguments:
 * Input:  hSession   - The handle to the session.
 *         pStreamCfg - The stream configuration (various parameters of media).
 *         sizeOfCfg  - The size of the configuration that the application provides.
 * Output: phStream   - The handle of the new stream.
 *
 * Return Values: Returns RvStatus.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvOaSessionAddStream(
                                    IN  RvOaSessionHandle  hSession,
                                    IN  RvOaStreamCfg*     pStreamCfg,
                                    IN  RvUint32           sizeOfCfg,
                                    OUT RvOaStreamHandle*  phStream);

/*@****************************************************************************
 * RvOaSessionResetStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Makes the specified stream to be not usable for media delivery.
 *  To be compliant to the RFC 3264, the Stream object is not removed
 *  physically from the Session. Instead, it's port is set to zero and all
 *  media description parameters are removed.
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 *         hStream  - The handle to the stream.
 * Output: none.
 *
 * Return Values: Returns RvStatus.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvOaSessionResetStream(
                                    IN  RvOaSessionHandle  hSession,
                                    IN  RvOaStreamHandle   hStream);

/*@****************************************************************************
 * RvOaSessionGetSdpMsg (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the pointer to the local or remote message of the session.
 *  Using this pointer the application can inspect and modify the local message,
 *  or inspect the remote message.
 *  The local message can represent an OFFER or ANSWER message. This depends
 *  the on role of the application. The local message is OFFER if the application
 *  an is offerer. Or it is ANSWER  if the application is an answerer.
 *  The offerer builds the local message based on local or default capabilities.
 *  The answerer builds the local message based on local or default capabilities
 *  and a received OFFER message. The local capabilities can be set into the
 *  session using RvOaSessionSetCapabilities(). The default capabilities can be
 *  set into the session using RvOaSetCapabilities.
 *  The remote message can represent an OFFER or ANSWER message. This depends
 *  on the role of the application. The remote message is ANSWER if the
 *  application is an offerer. Or it is OFFER if the application is an answerer.
 *  The remote message is built by the session while parsing the string,
 *  provided to the session by the application using RvOaSessionSetRcvdMsg().
 *
 * Arguments:
 * Input:  hSession       - The handle to the session.
 * Output: ppSdpMsgLocal  - The pointer to the RADVISION SDP Stack Message
 *                          object, used as a local message. Can be NULL.
 * Output: ppSdpMsgRemote - The pointer to the RADVISION SDP Stack Message
 *                          object, used as a local message. Can be NULL.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetSdpMsg(
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvSdpMsg**          ppSdpMsgLocal,
                                    OUT RvSdpMsg**          ppSdpMsgRemote);

/*@****************************************************************************
 * RvOaSessionGetState (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets current state of the session.
 *  The state can be OFFER_READY and ANSWER_RCVD for an offerer, ANSWER_READY for
 *  an answerer and IDLE or TERMINATED for both sides.
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 * Output: peState  - The state.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetState(
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvOaSessionState*   peState);

/*@****************************************************************************
 * RvOaSessionHold (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Changes the media description parameters in order to put the remote side
 *  on hold, for each stream in the session.
 *  For more details, see the "Put-on-hold" procedure described in RFC 3264.
 *  To get the SDP message with the changed parameters, use
 *  RvOaSessionGetMsgToBeSent().
 *
 * Arguments:
 * Input:  hStream  - The handle to the stream.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionHold(IN  RvOaSessionHandle   hSession);

/*@****************************************************************************
 * RvOaSessionResume (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Changes the media description parameters in order to resume the remote side
 *  from hold, for each stream in the session.
 *  For more details, see the "Put-on-hold" procedure described in RFC 3264.
 *  To get the SDP message with the changed parameters, use
 *  RvOaSessionGetMsgToBeSent().
 *
 * Arguments:
 * Input:  hStream  - The handle to the stream.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionResume(IN  RvOaSessionHandle   hSession);

/*---------------------------------------------------------------------------*/
/*                                STREAM API                                 */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaStreamSetAppHandle (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the handle to the application object that the stream serves.
 *  Actually, the application handle is sizeof(RvOaAppStreamHandle) bytes of
 *  the application data, stored in the stream. This data can
 *  be retrieved using RvOaStreamGetAppHandle().
 *
 * Arguments:
 * Input:  hStream     - The handle to the stream.
 *         hSession    - The handle to the session.
 *         hAppSession - The handle to the application object.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamSetAppHandle(
                                    IN  RvOaStreamHandle    hStream,
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvOaAppStreamHandle hAppStream);

/*@****************************************************************************
 * RvOaStreamGetAppHandle (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the handle to the Application object. The application handle can be set
 *  into the stream using RvOaStreamSetAppHandle().
 *
 * Arguments:
 * Input:  hStream     - The handle to the stream.
 *         hSession    - The handle to the session.
 * Output: phAppStream - The handle to the Application object.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetAppHandle(
                                    IN  RvOaStreamHandle      hStream,
                                    IN  RvOaSessionHandle     hSession,
                                    OUT RvOaAppStreamHandle*  phAppStream);

/*@****************************************************************************
 * RvOaStreamGetMediaDescriptor (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the pointer to the media-descriptor object in the local message.
 *  Using this pointer, the application can inspect and modify the stream
 *  parameters that are contained in the corresponding media-descriptor sub-tree.
 *  For more details, see the documentation for RvOaSessionGetSdpMsg().
 *
 * Arguments:
 * Input:  hstream      - The handle to the stream.
 *         hSession     - The handle to the session.
 * Output: ppMediaDescrLocal  - The pointer to the media-descriptor object from
 *                              the locally generated SDP message. Can be NULL.
 *         ppMediaDescrRemote - The pointer to the media-descriptor object from
 *                              the SDP message received from remote side.
 *                              Can be NULL.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetMediaDescriptor(
                                    IN  RvOaStreamHandle   hStream,
                                    IN  RvOaSessionHandle  hSession,
                                    OUT RvSdpMediaDescr**  ppMediaDescrLocal,
                                    OUT RvSdpMediaDescr**  ppMediaDescrRemote);

/*@****************************************************************************
 * RvOaStreamGetStatus (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 * Gets the status of stream. The status can be affected by an incoming OFFER
 * message. As a result of incoming OFFER processing, the stream can be held,
 * resumed, modified, and so on.
 *
 * Arguments:
 * Input:  hStream  - The handle to the stream.
 *         hSession - The handle to the session.
 * Output: pStatus  - The status.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetStatus(
                                    IN  RvOaStreamHandle    hStream,
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvOaStreamStatus*   pStatus);

/*@****************************************************************************
 * RvOaStreamHold (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Changes the parameters of stream that put the remote side on hold.
 *  For more details, see the "Put-on-hold" procedure described in RFC 3264.
 *  To get the SDP message with the changed parameters, use
 *  RvOaSessionGetMsgToBeSent().
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 *         hStream  - The handle to the stream.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamHold(
                                    IN  RvOaSessionHandle hSession,
                                    IN  RvOaStreamHandle  hStream);

/*@****************************************************************************
 * RvOaStreamResume (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Changes the parameters of the stream that resume the other side from hold.
 *  For more details, see the "Put-on-hold" procedure described in RFC 3264.
 *  To get the SDP message with the changed parameters, use
 *  RvOaSessionGetMsgToBeSent().
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 *         hStream  - The handle to the stream.
 * Output: None.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamResume(
                                    IN  RvOaSessionHandle hsession,
                                    IN  RvOaStreamHandle  hStream);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _RV_OA_H */
