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
 *                              <OaSessionObject.h>
 *
 * The OaSessionObject.h file defines interface for Session object.
 * The Session object is used by the Offer-Answer module in order to implement
 * abstraction of SDP session between two endpoints, as it is described in
 * RFC 3264.
 * The Session object generates OFFERs and ANSWERs according to the session
 * state as it is described in RFC 3264. For details about state machine see
 * RvOaSessionState enumeration.
 * The OFFERs are generated as a result of Application request. For details see
 * RvOaSessionGenerateOffer() API function.
 * The ANSWERs are created as a result of incoming OFFER handling. For details
 * see RvOaSessionSetRcvdMsg() API function.
 * At any moment the Session object holds two SDP messages - local and remote.
 * The local message is the message, generated locally by the Session object.
 * The remote message is the message, received from the remote side of session.
 * These messages can represent both OFFER and ANSWER. It depends on session
 * state.
 *
 * The Session object derives Stream objects from the media descriptors of
 * OFFER message. The Stream objects enable the Application to manipulate media
 * descriptions in convenient way. For details see OaStreamObject.c.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

#ifndef _OA_SESSION_OBJECT_H
#define _OA_SESSION_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "rvthread.h"

#include "RvOaTypes.h"
#include "AdsRlist.h"

#include "OaMgrObject.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * OaSession
 * ----------------------------------------------------------------------------
 * OaSession structure represents Session object, which implements the SDP
 * Session abstraction defined in RFC 3264.
 *****************************************************************************/
typedef struct
{
    OaMgr*               pMgr;          /* Manager of the Offer-Answer module*/
    RvOaAppSessionHandle hAppHandle;    /* Application data stored in object */

    RvBool               bInitialized;
        /* RV_TRUE, if the object was initialized. During initialization
        the object lock is constructed and pointer to manager is set.
        The initialization is performed once in life of the module - on module
        construction, when all objects in the session pool are initialized. */

    RvOaSessionState     eState;        /* Current state of the Session */
    RvOaSessionState     ePrevState;    /* Previous state of the Session */

    OaSdpMsg  msgLocal;
        /* Message, generated locally. May represent OFFER or ANSWER. */
    OaSdpMsg  msgRemote;
        /* Message, received from remote side of the Session.
        May represent OFFER or ANSWER. */

    /* Capabilities */
    OaPSdpMsg    capMsg;
        /* Local Capabilities, set into the Session object by Application using
        RvOaSessionSetCapabilities API function. The Capabilities are stored in
        form of proper SDP message. */
    RLIST_HANDLE hCodecHashElements;
        /* When Local Capabilities are set, the codecs they include are
        inserted into the hash. The hash is used, when ANSWER is generated from
        OFFER: codecs that appear in OFFER and doesn't appear in hash are
        not removed from ANSWER. */

    /* Streams */
    RvUint32      numOfStreams;
        /* Number of Stream objects, generated by the Session object */
    RLIST_HANDLE  hStreamList;
        /* List of Stream objects, generated by the Session object */

    /* Lock */
    RvMutex     lock;
    RvThreadId  currThreadId;   /* ID of thread that holds the lock */

} OaSession;

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaSessionConstruct
 * ----------------------------------------------------------------------------
 * General:
 *  Construct the Session object in the given piece of memory.
 *  Session object should be used by the Application for media capabilities
 *  negotiation. The Application should provided the OFFER and ANSWER messages,
 *  received as a bodies of SIP messages, to the Session object.
 *  As a result, the Session object will generate Stream objects for each
 *  negotiated media descriptions. Using this objects the Application can find
 *  various parameters of the media stream to be transmitted, including remote
 *  address, media format to be used, direction of transmission and others.
 *  Session object can generate either OFFER to be sent to answerer, based on
 *  the local or default capabilities, or ANSWER to be sent to offerer, based
 *  on the received OFFER and local or default capabilities.
 *  The generated message can be get anytime during life cycle of the Session
 *  object, using RvOaSessionGetMsgToBeSent function.
 *
 *  The created Session object stays in the IDLE state until
 *  RvOaSessionGenerateOffer or RvOaSessionSetRcvdMsg functions is called.
 *
 * Arguments:
 * Input:  pOaSession  - pointer to the piece of memory for Session object.
 *         hAppSession - handle of the Application object, which is going.
 *                       to be served by the Session.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionConstruct(
                                    IN  OaSession*            pOaSession,
                                    IN  RvOaAppSessionHandle  hAppSession);

/******************************************************************************
 * OaSessionDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs the Session object and frees it's memory to the pool os Sessions.
 *  Destructs Stream object, generated by the Session, frees memory used by
 *  local and remote messages.
 *
 * Arguments:
 * Input:  pOaSession  - pointer to the piece of memory for Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionDestruct(IN OaSession*  pOaSession);

/******************************************************************************
 * OaSessionTerminate
 * ----------------------------------------------------------------------------
 * General:
 *  Terminates Session object.
 *  This function frees resources, consumed by the Session object.
 *  After call to this function the Session object can't be more referenced.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *               The function should not fail normally. The failure should be
 *               treated as an exception.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionTerminate(IN OaSession*  pOaSession);

/******************************************************************************
 * OaSessionGenerateOffer
 * ----------------------------------------------------------------------------
 * General:
 *  Causes Session object to built OFFER message.
 *  After call to this function the Application can get the OFFER in format of
 *  NULL-terminated string and send it with SIP message.
 *  This can be done using RvOaSessionGetMsgToBeSent() function anytime during
 *  Session life cycle.
 *  The RvOaSessionGenerateOffer function performs following actions:
 *  1. Builds the OFFER in RADVISION SDP Stack Message format. Local
 *     capabilities, if were set by the Application, are used as a source for
 *     copy-construct operation, as a result of which the OFFER is created.
 *     Local capabilities can be set into the Session by the Application using
 *     RvOaSessionSetCapabilities function. If local capabilities were not set,
 *     the default capabilities, set into the OA Module by the Application
 *     using RvOaSetCapabilities function.
 *  2. Adds missing mandatory lines to the built message,using default values.
 *  3. Generates Stream object for each media descriptor, found in the built
 *     message, and insert it into the list, managed by the Session object.
 *
 *  Anytime during Session life cycle the Application can iterate through list
 *  of Stream objects belonging to the Session object. For each Stream
 *  the Application can inspect and modify it's attributes and parameters.
 *  This can be done using the OA Module API for Stream object,
 *  or this can be done using the RADVISION SDP Stack API for Message object.
 *  Note the SDP Stack API requires handle to the Message structure that
 *  contains session description. This handle can be got from the Session
 *  object using RvOaSessionGetSdpMsg function.
 *
 *  RvOaSessionGenerateOffer moves the Session object from IDLE and from
 *  ANSWER_READY state into OFFER_READY state.
 *  The Session object stays in OFFER_READY state until RvOaSessionSetRcvdMsg
 *  function is called.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionGenerateOffer(IN OaSession*  pOaSession);

/******************************************************************************
 * OaSessionSetCapabilities
 * ----------------------------------------------------------------------------
 * General:
 *  Loads local media capabilities into the Session object.
 *  The capabilities should be provided in form of proper SDP message.
 *  Client Session (offerer) uses the local capabilities in order to generate
 *  OFFER message.
 *  Server Session (answerer) uses the default capabilities in order to
 *  identify set of codecs supported by both offerer and answerer. This set
 *  will be reflected in the ANSWER message.
 *  Note the Client Session parses the local capabilities in the RADVISION SDP
 *  stack message, which should be sent as OFFER. That is why the capabilities
 *  should be represented as a proper SDP message.
 *
 *  The capabilities can be set into IDLE sessions only.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - local capabilities as a NULL-terminated string.
 *                      Can be NULL.
 *         pSdpMsg    - local capabilities in form of RADVISION SDP Stack
 *                      message.
 *                      Can be NULL.
 *                      Has lower priority than strSdpMsg.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionSetCapabilities(
                                    IN  OaSession*  pOaSession,
                                    IN  RvChar*     strSdpMsg,
                                    IN  RvSdpMsg*   pSdpMsg);

/******************************************************************************
 * OaSessionSetRcvdMsg
 * ----------------------------------------------------------------------------
 * General:
 *  Notifies the Session object about received SDP message.
 *  Type of message, OFFER or ANSWER, is deduced by the object based on state.
 *  For example, if the Session is IDLE, the received message handled as OFFER.
 *  The received message is parsed by the Session object and stored in
 *  msgRemote field.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - received SDP message in string form.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionSetRcvdMsg(
                                    IN OaSession* pOaSession,
                                    IN RvChar*    strSdpMsg);

/******************************************************************************
 * OaSessionAddStream
 * ----------------------------------------------------------------------------
 * General:
 *  Allocates new Stream object and adds it to the Session object.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         pStreamCfg - configuration of Stream.
 * Output: phOaStream - allocated Stream element.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionAddStream(
                                    IN  OaSession*        pOaSession,
                                    IN  RvOaStreamCfg*    pStreamCfg,
                                    OUT RvOaStreamHandle* phOaStream);

/******************************************************************************
 * OaSessionHold
 * ----------------------------------------------------------------------------
 * General:
 *  For each stream in the session changes it parameters,
 *  that put other side on hold.
 *  Moves the Session to OFFER_READY state.
 *  For more details see put on hold procedure, described in RFC 3264.
 *  Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 *  where all media descriptions indicate put on hold.
 *
 * Arguments:
 * Input:  hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionHold(IN  OaSession*  pOaSession);

/******************************************************************************
 * OaSessionResume
 * ----------------------------------------------------------------------------
 * General:
 *  For each stream in the session changes it parameters,
 *  that resume the other side from hold.
 *  Moves the Session to OFFER_READY state.
 *  For details see put on hold procedure, described in RFC 3264.
 *  Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 *  where all media descriptions indicate resume.
 *
 * Arguments:
 * Input:  hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionResume(IN  OaSession*  pOaSession);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/******************************************************************************
 * OaSessionLock
 * ----------------------------------------------------------------------------
 * General:
 *  Locks the Session object.
 *  If the locked object represents not valid session, the failure is returned.
 *  Note all Session objects are created on Offer-Answer module construction
 *  and are stored in the pool. The Session object locks are constructed on
 *  module construction also, and they lives as long as the module lives.
 *  Destruction of Session object just "returns" the object back to pool, no
 *  real memory free is performed.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSessionLock(IN OaSession *pOaSession);

/******************************************************************************
 * OaSessionUnlock
 * ----------------------------------------------------------------------------
 * General:
 *  Unlocks the Session object.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaSessionUnlock(IN OaSession *pOaSession);

#else
#define OaSessionLock(a) (RV_OK)
#define OaSessionUnlock(a)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _OA_SESSION_OBJECT_H */
