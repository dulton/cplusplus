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
 *                              <OaSessionObject.c>
 *
 * The OaSessionObject.c file defines Session object.
 * The Session object is used by the Offer-Answer module in order to implement
 * abstraction of SDP session between two endpoints, as it is described in
 * RFC 3264 (Offer-Answer model for SDP negotiation).
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
 * These messages can represent both OFFER and ANSWER. It is depends on session
 * state.
 *
 * The Session object derives Stream objects from the media descriptors of
 * OFFER message. The Stream objects enable the Application to manipulate media
 * descriptions in convenient way. For details see OaStreamObject.c.
 *
 *    Author                        Date
 *    ------                        --------
 *    Igor							Aug 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "OaSessionObject.h"
#include "OaStreamObject.h"
#include "OaSdpMsg.h"
#include "OaUtils.h"
#include "rvsdp.h"

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTIONS DEFINITIONS                      */
/*---------------------------------------------------------------------------*/
static RvStatus RVCALLCONV DestructStreams(IN OaSession*  pOaSession);

static void RVCALLCONV ChangeSessionState(
                            IN OaSession*        pOaSession,
                            IN RvOaSessionState  eState);

static RvStatus RVCALLCONV CreateOffer(IN OaSession*  pOaSession);

static RvStatus RVCALLCONV CreateAnswer(IN OaSession*  pOaSession);

static RvStatus RVCALLCONV AnswerAddDescriptor(
                            IN  OaSession*        pOaSession,
                            IN  RvSdpMediaDescr*  pMediaDescr,
                            OUT RvSdpMediaDescr** ppAddedMediaDescr);

static RvStatus RVCALLCONV DeriveAndAddFormats(
                            IN  OaSession*       pOaSession,
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer);

static RvStatus RVCALLCONV GenerateStreamsFromMsg(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsg);

static RvStatus RVCALLCONV UpdateStreamsWithRemoteMsg(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsgRemote);

static RvStatus RVCALLCONV HandleInitialOffer(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg);

static RvStatus RVCALLCONV HandleModifyingOffer(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg);

static RvStatus RVCALLCONV HandleModifyingAnswer(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg);

static RvStatus RVCALLCONV HandleModifyingMessage(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg,
                            IN RvBool      bOffer);

static RvStatus RVCALLCONV HandleAnswer(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg);

static RvStatus RVCALLCONV ModifySessionOnRcvdOffer(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsgPrevRemoteMsg);

static RvStatus RVCALLCONV ModifySessionOnRcvdAnswer(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsgPrevRemoteMsg);

static void RVCALLCONV ModifyStreams(
                            IN   OaSession*        pOaSession,
                            IN   RvSdpMsg*         pSdpMsgRemotePrev,
                            IN   RvSdpListIter*    pMdIterator,
                            OUT  OaStream**        ppOaStream,
                            OUT  RvSdpMediaDescr** ppMediaDescr,
                            OUT  RvBool*           pbNewlyOfferedStreams,
                            OUT  RvBool*           pbSessionWasModified);

static RvStatus RVCALLCONV AllocateStream(
                                    IN  OaSession* pOaSession,
                                    OUT OaStream** ppOaStream);

static RvStatus RVCALLCONV RemoveStream(
                                    IN OaSession* pOaSession,
                                    IN void*      pOaStream);

static void RVCALLCONV CallbackDeriveFormatParams(
                            IN  OaSession*            pOaSession,
                            IN  RvSdpMediaDescr*      pMediaDescrOffer,
                            IN  RvSdpMediaDescr*      pMediaDescrCaps,
                            IN  RvSdpMediaDescr*      pMediaDescrAnswer,
                            IN  RvOaMediaFormatInfo*  pFormatInfo,
                            OUT RvBool*               pbRemoved);

static void RVCALLCONV RejectNotAnsweredStreams(
                            IN  OaSession*            pOaSession,
                            IN  OaStream*             pOaStream);

/*---------------------------------------------------------------------------*/
/*                              MODULE FUNCTIONS                             */
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
                                    IN  RvOaAppSessionHandle  hAppSession)
{
    pOaSession->hAppHandle   = hAppSession;
    pOaSession->eState       = RVOA_SESSION_STATE_IDLE;
    pOaSession->numOfStreams = 0;
    pOaSession->msgLocal.pSdpMsg          = NULL;
    pOaSession->msgLocal.pSdpAllocator    = NULL;
    pOaSession->msgRemote.pSdpMsg         = NULL;
    pOaSession->msgRemote.pSdpAllocator   = NULL;
    pOaSession->capMsg.pSdpMsg            = NULL;
    pOaSession->capMsg.pSdpAllocator      = NULL;
    pOaSession->capMsg.bSdpMsgConstructed = RV_FALSE;
    pOaSession->hCodecHashElements        = NULL;

    /* Following structures are initialized once on Offer-Answer Manager
    construction. They are never changed. Don't touch them. */
    /*
    pOaSession->pMgr;
    pOaSession->hStreamList;
    pOaSession->lock;
    */
    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionDestruct(IN OaSession*  pOaSession)
{
    RvStatus  rv;

    /* Destruct Streams */
    if (0 < pOaSession->numOfStreams)
    {
        rv = DestructStreams(pOaSession);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionDestruct(pOaSession=%p) - failed to destruct Streams (rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
    }

    /* Destruct local and remote messages */
    OaUtilsOaSdpMsgDestruct(&pOaSession->msgLocal);
    OaUtilsOaSdpMsgDestruct(&pOaSession->msgRemote);

    /* Destruct Capabilities related structures */
    if (NULL != pOaSession->hCodecHashElements)
    {
        OaCodecHashRemoveElements(&pOaSession->pMgr->hashCodecs,
                                  pOaSession->hCodecHashElements);
        RLIST_ListDestruct (pOaSession->pMgr->hashCodecs.hHashElemPool,
                            pOaSession->hCodecHashElements);
        pOaSession->hCodecHashElements = NULL;
    }
    if (NULL != pOaSession->capMsg.pSdpMsg)
    {
        OaUtilsOaPSdpMsgDestruct(&pOaSession->capMsg);
    }

    /* Following structures are initialized once on Offer-Answer Manager
    construction. They are never changed. Don't touch them. */
    /*
    pOaSession->pMgr;
    pOaSession->hStreamList;
    pOaSession->lock;
    */

    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "OaSessionDestruct - Session %p was destructed", pOaSession));

    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionTerminate(IN OaSession*  pOaSession)
{
    RvStatus rv;

    if (RVOA_SESSION_STATE_UNDEFINED == pOaSession->eState)
    {
        return RV_OK;
    }

    rv = OaSessionDestruct(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionTerminate(pOaSession=%p) - failed to destruct session (rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    pOaSession->eState = RVOA_SESSION_STATE_UNDEFINED;

    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionGenerateOffer(IN OaSession*  pOaSession)
{
    RvStatus rv;
    RvBool   bModifyingOffer = RV_TRUE;

    /* Construct Local Message, if it was not constructed yet.
    The Local message can be constructed by previous OFFER sending. */
    if (NULL == pOaSession->msgLocal.pSdpMsg)
    {
        rv = CreateOffer(pOaSession);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionGenerateOffer(pOaSession=%p) - failed to create message(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
        bModifyingOffer = RV_FALSE;
    }

    /* Create Streams, based on the Local Message */
    rv = GenerateStreamsFromMsg(pOaSession, &pOaSession->msgLocal);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionGenerateOffer(pOaSession=%p) - failed to generate Stream objects(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Increment version of SDP in case of session modifying */
    if (bModifyingOffer == RV_TRUE)
    {
		rv = OaSdpMsgIncrementVersion(pOaSession->msgLocal.pSdpMsg,
			pOaSession->pMgr->pLogSource);
		if (RV_OK != rv)
		{
			RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
				"OaSessionGenerateOffer(pOaSession=%p) - failed to increment version of session description(rv=%d:%s)",
				pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
			/* don't return */
		}
	}
    ChangeSessionState(pOaSession, RVOA_SESSION_STATE_OFFER_READY);

    return RV_OK;
}

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
                                    IN  RvSdpMsg*   pSdpMsg)
{
    RvStatus rv;
    OaMgr* pOaMgr = pOaSession->pMgr;
    OaCodecHash* pHashCodecs = &pOaSession->pMgr->hashCodecs;

    if (NULL != strSdpMsg  &&
        strlen(strSdpMsg) >= OA_CAPS_STRING_LEN)
    {
        RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
            "OaSessionSetCapabilities(pOaMgr=%p) - SDP message string is too long(%d>=%d)",
            pOaMgr, strlen(strSdpMsg), OA_CAPS_STRING_LEN));
        return RV_ERROR_UNKNOWN;
    }


    /* If the capabilities were set already, remove their codecs from hash
    before they will be overwritten. */
    if (NULL != pOaSession->hCodecHashElements)
    {
        OaCodecHashRemoveElements(pHashCodecs, pOaSession->hCodecHashElements);
    }

    /* If this is the first time when the capabilities are set, create list
       of references to hash elements */
    if (NULL == pOaSession->hCodecHashElements)
    {
        RvMutexLock(pHashCodecs->pLock, pHashCodecs->pLogMgr);
        pOaSession->hCodecHashElements = RLIST_ListConstruct(pHashCodecs->hHashElemPool);
        RvMutexUnlock(pHashCodecs->pLock, pHashCodecs->pLogMgr);
        if (NULL == pOaSession->hCodecHashElements)
        {
            RvLogError(pOaSession->pMgr->pLogSourceCaps, (pOaSession->pMgr->pLogSourceCaps,
                "OaSessionSetCapabilities(pOaSession=%p) - failed to create list for hash elements",
                pOaSession));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Create new Message from string or from the application Message */
    if (NULL != strSdpMsg)
    {
        /* Copy the provided string into the temporary buffer.
           SDP Stack requires the string to be allocated on heap. Application
           may provide string from static memory (eg. defined with define).
           To ensure the dynamic memory for SDP Stack input, use temporary
           buffer. Lock the temporary buffer.*/
        rv = OaMgrLock(pOaMgr);
        if (RV_OK != rv)
        {
            return rv;
        }
        strcpy(pOaMgr->pCapBuff, strSdpMsg);

        rv = OaUtilsOaPSdpMsgConstructParse(
                pOaMgr->hRaCapAllocs, pOaMgr->hRaCapMsgs, pOaMgr->hCapPool,
                &pOaSession->capMsg, pOaMgr->pCapBuff, pOaMgr->pLogSource,
                (void*)pOaSession);
        if (RV_OK != rv)
        {
            OaMgrUnlock(pOaMgr);
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaSessionSetCapabilities(pOaSession=%p) - failed to parse SDP message string(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_ERROR_UNKNOWN;
        }

        /* Unlock the buffer */
        OaMgrUnlock(pOaMgr);
    }
    else
    {
        rv = OaUtilsOaPSdpMsgConstructCopy(
                pOaMgr->hRaCapAllocs, pOaMgr->hRaCapMsgs, pOaMgr->hCapPool,
                &pOaSession->capMsg,pSdpMsg,pOaMgr->pLogSource,(void*)pOaSession);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaSessionSetCapabilities(pOaSession=%p) - failed to copy SDP message(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Initialize the message: set default mandatory data */
    rv = OaSdpMsgSetDefaultData(pOaSession->capMsg.pSdpMsg, pOaMgr->pLogSource);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSessionSetCapabilities(pOaSession=%p) - failed to set default data into message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Insert Capabilities codecs into hash */
    rv = OaCodecHashInsertSdpMsgCodecs(pHashCodecs, pOaSession->capMsg.pSdpMsg,
                            (void*)pOaSession, pOaSession->hCodecHashElements);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSessionSetCapabilities(pOaSession=%p) - failed to insert capabilities codecs into hash(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pOaMgr->logFilterCaps & RV_LOGLEVEL_DEBUG)
    {
        RvLogDebug(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSessionSetCapabilities(pOaSession=%p) - following capabilities were set",
            pOaSession));
        OaSdpMsgLogCapabilities(pOaSession->capMsg.pSdpMsg, pOaMgr->pLogSourceCaps);
    }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    return RV_OK;
}

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
                                        IN RvChar*    strSdpMsg)
{
    RvStatus rv;

    switch (pOaSession->eState)
    {
    case RVOA_SESSION_STATE_IDLE:
        rv = HandleInitialOffer(pOaSession, strSdpMsg);
        break;
    case RVOA_SESSION_STATE_ANSWER_RCVD:
    case RVOA_SESSION_STATE_ANSWER_READY:
        rv = HandleModifyingOffer(pOaSession, strSdpMsg);
        break;
    case RVOA_SESSION_STATE_OFFER_READY:
        if (RVOA_SESSION_STATE_IDLE == pOaSession->ePrevState)
        {
            rv = HandleAnswer(pOaSession, strSdpMsg);
        }
        else
        {
            rv = HandleModifyingAnswer(pOaSession, strSdpMsg);
        }
        break;
    default:
        RvLogExcep(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionSetRcvdMsg(hSession=%p) - illegal state %s",
            pOaSession, OaUtilsConvertEnum2StrSessionState(pOaSession->eState)));
        return RV_ERROR_UNKNOWN;
    }

    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionSetRcvdMsg(hSession=%p) - failed to handle incoming message(rv=%d:%s). State=%s, PrevState=%s",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv),
            OaUtilsConvertEnum2StrSessionState(pOaSession->eState),
            OaUtilsConvertEnum2StrSessionState(pOaSession->ePrevState)));
        return rv;
    }

    return RV_OK;
}

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
                                    OUT RvOaStreamHandle* phOaStream)
{
    RvStatus  rv;
    OaStream* pOaStream;

    /* Allocate Stream in context of Session */
    rv = AllocateStream(pOaSession, &pOaStream);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionAddStream(hSession=%p) - failed to add stream(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Initiate the Stream object, using provided configuration */
    rv = OaStreamInitiateFromCfg(pOaStream, pOaSession, pStreamCfg);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionAddStream(hSession=%p) - failed to initiate stream(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        RemoveStream(pOaSession, (void*)pOaStream);
        return rv;
    }

    *phOaStream = (RvOaStreamHandle)pOaStream;

    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionHold(IN  OaSession*  pOaSession)
{
    RvStatus  rv;
    OaStream* pOaStream = NULL;

    if (NULL == pOaSession->pMgr->hStreamListPool)
    {
        return RV_ERROR_DESTRUCTED;
    }
    if (NULL == pOaSession->hStreamList)
    {
        return RV_ERROR_UNINITIALIZED;
    }

    /* Ensure that the session is not in the middle of Offer-Answer transaction */
    if (RVOA_SESSION_STATE_OFFER_READY == pOaSession->eState)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionHold(pOaSession=%p) - session in the middle of Offer-Answer transaction (state=OFFER_READY). Try later.",
            pOaSession));
        return RV_ERROR_TRY_AGAIN;
    }

    /* Put all streams on hold */
    RLIST_GetHead(pOaSession->pMgr->hStreamListPool, pOaSession->hStreamList,
                  (RLIST_ITEM_HANDLE*)&pOaStream);
    while (NULL != pOaStream)
    {
        rv = OaStreamHold(pOaStream);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionHold(pOaSession=%p) - failed to put on hold stream %p (rv=%d:%s)",
                pOaSession, pOaStream, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }

        RLIST_GetNext(pOaSession->pMgr->hStreamListPool,pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
    }

    /* Prepare new OFFER to be sent */
    rv = OaSessionGenerateOffer(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionHold(pOaSession=%p) - failed to generate OFFER(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionResume(IN  OaSession*  pOaSession)
{
    RvStatus  rv;
    OaStream* pOaStream = NULL;

    if (NULL == pOaSession->pMgr->hStreamListPool)
    {
        return RV_ERROR_DESTRUCTED;
    }
    if (NULL == pOaSession->hStreamList)
    {
        return RV_ERROR_UNINITIALIZED;
    }

    /* Ensure that the session is not in the middle of Offer-Answer transaction */
    if (RVOA_SESSION_STATE_OFFER_READY == pOaSession->eState)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionResume(pOaSession=%p) - session in the middle of Offer-Answer transaction (state=OFFER_READY). Try later.",
            pOaSession));
        return RV_ERROR_TRY_AGAIN;
    }

    /* Resume all streams */
    RLIST_GetHead(pOaSession->pMgr->hStreamListPool, pOaSession->hStreamList,
                  (RLIST_ITEM_HANDLE*)&pOaStream);
    while (NULL != pOaStream)
    {
        rv = OaStreamResume(pOaStream);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionResume(pOaSession=%p) - failed to resume stream %p (rv=%d:%s)",
                pOaSession, pOaStream, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }

        RLIST_GetNext(pOaSession->pMgr->hStreamListPool,pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
    }

    /* Prepare new OFFER to be sent */
    rv = OaSessionGenerateOffer(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionResume(pOaSession=%p) - failed to generate OFFER(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    return RV_OK;
}

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
RvStatus RVCALLCONV OaSessionLock(IN OaSession *pOaSession)
{
    RvStatus crv;

    crv = RvMutexLock(&pOaSession->lock, pOaSession->pMgr->pLogMgr);
    if (RV_OK != crv)
    {
        if (NULL != pOaSession->pMgr->pLogSource)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionLock - failed to lock Session %p (crv=%d:%s)",
                pOaSession, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
        }
        return OA_CRV2RV(crv);
    }

    if (RV_FALSE == RLIST_IsElementVacant(pOaSession->pMgr->hSessionListPool,
                                          pOaSession->pMgr->hSessionList,
                                          (RLIST_ITEM_HANDLE)pOaSession))
    {
        pOaSession->currThreadId = RvThreadCurrentId();
        /*The Session is valid*/
        return RV_OK;
    }
    else
    {
        RvMutexUnlock(&pOaSession->lock, pOaSession->pMgr->pLogMgr);
        RvLogWarning(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaSessionLock - Session 0x%p: RLIST_IsElementVacant returned TRUE!!!",
            pOaSession));
    }
    return RV_ERROR_INVALID_HANDLE;
}

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
void RVCALLCONV OaSessionUnlock(IN OaSession *pOaSession)
{
    RvStatus crv;

    pOaSession->currThreadId = (RvThreadId)0;

    crv = RvMutexUnlock(&pOaSession->lock, pOaSession->pMgr->pLogMgr);
    if (RV_OK != crv)
    {
        if (NULL != pOaSession->pMgr->pLogSource)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "OaSessionUnlock - failed to unlock Manager lock %p (crv=%d:%s)",
                pOaSession->pMgr->pLock, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
        }
    }
}

#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/


/*---------------------------------------------------------------------------*/
/*                              STATIC FUNCTIONS                             */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * DestructStreams
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Streams, created by the Session objects, and returns them back
 *  to the pool of Stream objects.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV DestructStreams(IN OaSession*  pOaSession)
{
    RvStatus          rv;
    RLIST_ITEM_HANDLE hListElem;
    OaMgr*            pOaMgr = pOaSession->pMgr;
    RvInt32           numOfStreams=0;

    RLIST_GetHead(pOaMgr->hStreamListPool,pOaSession->hStreamList,&hListElem);
    while (NULL != hListElem)
    {
        /* Destruct Stream */
        OaStreamDestruct((OaStream*)hListElem);

        /* Remove Stream from list of Streams*/
        rv = OaMgrLock(pOaMgr);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DestructStreams(pOaSession=%p) - failed to lock OA Manager %p(rv=%d:%s)",
                pOaSession, pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
        RLIST_Remove(pOaMgr->hStreamListPool,pOaSession->hStreamList,hListElem);
        OaMgrUnlock(pOaMgr);
        pOaSession->numOfStreams--;
        
        numOfStreams++;
        /* Get next Stream */
        RLIST_GetHead(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                      &hListElem);
    }

    RvLogDebug(pOaMgr->pLogSource, (pOaMgr->pLogSource,
        "DestructStreams(pOaSession=%p) - %d streams were destructed",
        pOaSession, numOfStreams));

    return RV_OK;
}

/******************************************************************************
 * ChangeSessionState
 * ----------------------------------------------------------------------------
 * General:
 *  Sets new state into the Session object.
 *  The current state is stored in ePrevState field of the object.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         eState     - new state.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static void RVCALLCONV ChangeSessionState(
                            IN OaSession*        pOaSession,
                            IN RvOaSessionState  eState)
{
    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "ChangeSessionState(pOaSession=%p): %s -> %s", pOaSession,
        OaUtilsConvertEnum2StrSessionState(pOaSession->eState),
        OaUtilsConvertEnum2StrSessionState(eState)));
    pOaSession->ePrevState = pOaSession->eState;
    pOaSession->eState = eState;
}

/******************************************************************************
 * CreateOffer
 * ----------------------------------------------------------------------------
 * General:
 *  Creates new OFFER message from scratch.
 *  The new OFFER is stored in msdLocal field of the Session object.
 *  The OFFER takes the session and media descriptions from the Local
 *  Capabilities that can be set by Application using
 *  RvOaSessionSetCapabilities() function.
 *  If no Local Capabilities were found, the Default Capabilities that can be
 *  set by Application using RvOaSetCapabilities() function, are used.
 *  If the Default Capabilities were not set, the empty message is created.
 *
 *  If the created message doesn't contain fields that are mandatory in
 *  accordance to RFC 2327 (SDP), they are set to default values.
 *
 *  The generated message can be retrieved by the Application using
 *  RvOaSessionGetSdpMsg() or RvOaSessionGetMsgToBeSent() API.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV CreateOffer(IN OaSession*  pOaSession)
{
    RvStatus rv;
    OaMgr*   pOaMgr = pOaSession->pMgr;

    /* If local capabilities were set, use them as an OFFER */
    if (RV_TRUE == pOaSession->capMsg.bSdpMsgConstructed)
    {
        rv = OaUtilsOaSdpMsgConstructCopy(
                pOaMgr->hMessagePool, pOaMgr->pLogSource,&pOaSession->msgLocal,
                pOaSession->capMsg.pSdpMsg);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateOffer(pOaSession=%p) - failed to copy the local capabilities message(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
        return RV_OK;
    }
    else
    /* If default capabilities were set, use them as an OFFER */
    if (RV_TRUE == pOaMgr->defCapMsg.bSdpMsgConstructed)
    {
        rv = OaUtilsOaSdpMsgConstructCopy(
                pOaMgr->hMessagePool,pOaMgr->pLogSource,&pOaSession->msgLocal,
                pOaMgr->defCapMsg.pSdpMsg);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateOffer(pOaSession=%p) - failed to copy the local capabilities message(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
        return RV_OK;
    }

    /* No capabilities were found. Create the empty OFFER message */
    rv = OaUtilsOaSdpMsgConstruct(pOaMgr->hMessagePool, pOaMgr->pLogSource,
                                  &pOaSession->msgLocal);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "CreateOffer(pOaSession=%p) - failed to construct the message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Initialize the message: set default mandatory data */
    rv=OaSdpMsgSetDefaultData(pOaSession->msgLocal.pSdpMsg,pOaMgr->pLogSource);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "CreateOffer(pOaSession=%p) - failed to set default data into message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
        "CreateOffer(pOaSession=%p) - OFFER SDP message %p was created",
        pOaSession, pOaSession->msgLocal.pSdpMsg));

    return RV_OK;
}

/******************************************************************************
 * CreateAnswer
 * ----------------------------------------------------------------------------
 * General:
 *  Creates new ANSWER message from scratch.
 *  The ANSWER message is stored in msgLocal field of the Session object.
 *  The ANSWER is created based on received OFFER (stored in msgRemote field)
 *  and capabilities, set into the Session object using
 *  RvOaSessionSetCapabilities() function or into the module using
 *  RvOaSetCapabilities() function.
 *
 *  If the created message doesn't contain fields that are mandatory in
 *  accordance to RFC 2327 (SDP), they are set to default values.
 *
 *  The generated message can be retrieved by the Application using
 *  RvOaSessionGetSdpMsg() or RvOaSessionGetMsgToBeSent() API.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV CreateAnswer(IN OaSession*  pOaSession)
{
    RvStatus  rv;
    OaMgr*                  pOaMgr = pOaSession->pMgr;
    RvSdpListIter           mdIterator; /* Media Descriptor iterator */
    const RvSdpMediaDescr*  pMediaDescr;
    RvSdpMediaDescr*        pAddedMediaDescr;
    RvSdpMsg*               pSdpMsg;

    /* Construct new message */
    rv = OaUtilsOaSdpMsgConstruct(pOaMgr->hMessagePool, pOaMgr->pLogSource,
                                  &pOaSession->msgLocal);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "CreateAnswer(pOaSession=%p) - failed to construct new message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Set Session Description into it */
    /* If local capabilities were set, take their Session Description */
    if (RV_TRUE == pOaSession->capMsg.bSdpMsgConstructed)
    {
        pSdpMsg = rvSdpMsgCopySessionDescription(
                                pOaSession->msgLocal.pSdpMsg,
                                (const RvSdpMsg*)pOaSession->capMsg.pSdpMsg);
        if (NULL == pSdpMsg)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateAnswer(pOaSession=%p) - failed to copy the session description from local capabilities",
                pOaSession));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    /* If default capabilities were set, take their Session Description */
    if (RV_TRUE == pOaMgr->defCapMsg.bSdpMsgConstructed)
    {
        pSdpMsg = rvSdpMsgCopySessionDescription(
                            pOaSession->msgLocal.pSdpMsg,
                            (const RvSdpMsg*)pOaMgr->defCapMsg.pSdpMsg);
        if (NULL == pSdpMsg)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateAnswer(pOaSession=%p) - failed to copy the session description from default capabilities",
                pOaSession));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    /* Use default Session Description */
    {
        rv = OaSdpMsgSetDefaultData(pOaSession->msgLocal.pSdpMsg,
                                    pOaMgr->pLogSource);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateAnswer(pOaSession=%p) - failed to set default session description(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }
    } /* ENDOF:  if (RV_TRUE == pOaSession->capMsg.bSdpMsgConstructed)*/

    /* Set Media Descriptions into the Session based on received descriptions*/
    memset(&mdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pOaSession->msgRemote.pSdpMsg,
                                             &mdIterator);

    while (NULL != pMediaDescr)
    {
        rv = AnswerAddDescriptor(pOaSession, (RvSdpMediaDescr*)pMediaDescr,
                                 &pAddedMediaDescr);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "CreateAnswer(pOaSession=%p) - failed to add Media Descriptor(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }

        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
    }

    RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
        "CreateAnswer(pOaSession=%p) - ANSWER SDP message %p was created",
        pOaSession, pOaSession->msgLocal.pSdpMsg));

    return RV_OK;
}

/******************************************************************************
 * AnswerAddDescriptor
 * ----------------------------------------------------------------------------
 * General:
 *  Adds new Media Descriptor to the local message.
 *  It is assumed, that the local message represents ANSWER.
 *  The new Descriptor is populated with media formats and media parameters,
 *  supported by both sides of the Session.
 *  Formats and parameters supported by local side, are taken form local
 *  capabilities, set per session or globally.
 *  Formats and parameters supported by remote side, are taken form OFFER
 *  descriptor.
 *  capabilities.
 *  After this the descriptor is updated with rest data, such as direction 
 *  of media stream (send, recv, send/recv, inactive).
 *
 * Arguments:
 * Input:  pOaSession         - pointer to the Session object.
 *         pMediaDescrOffer   - pointer to OFFER descriptor (remote descriptor)
 * Output: ppMediaDescrAnswer - pointer to the added media descriptor (local d).
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV AnswerAddDescriptor(
                                IN  OaSession*         pOaSession,
                                IN  RvSdpMediaDescr*   pMediaDescrOffer,
                                OUT RvSdpMediaDescr**  ppMediaDescrAnswer)
{
    RvStatus          rv;
    RvSdpMediaDescr*  pMediaDescrAnswer;
    RvSdpMediaType    eMediaType;
    RvSdpProtocol     eProtocol;

    /* Add empty Media Descriptor into ANSWER */
    eMediaType = rvSdpMediaDescrGetMediaType((const RvSdpMediaDescr*)pMediaDescrOffer);
    eProtocol = rvSdpMediaDescrGetProtocol((const RvSdpMediaDescr*)pMediaDescrOffer);
    pMediaDescrAnswer = rvSdpMsgAddMediaDescr(pOaSession->msgLocal.pSdpMsg,
                                              eMediaType,0/*port*/,eProtocol);
    if (NULL == pMediaDescrAnswer)
    {
        RvLogError(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
            "AnswerAddDescriptor(pOaSession=%p) - failed to add descriptor to SDP message",
            pOaSession));
        return RV_ERROR_UNKNOWN;
    }

    RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
        "AnswerAddDescriptor(pOaSession=%p) - new descriptor %p was added",
        pOaSession, pMediaDescrAnswer));

    /* Derives codecs supported by both offerer and answerer and add them
    to the newly built answer descriptor */
    rv = DeriveAndAddFormats(pOaSession, pMediaDescrOffer, pMediaDescrAnswer);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "AnswerAddDescriptor(pOaSession=%p) - failed to derive and add supported codecs(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Update Descriptor in order to form ANSWER, e.g set opposite direction */
    rv = OaSdpDescrModify(pMediaDescrAnswer, pMediaDescrOffer,
                          pOaSession->pMgr->pLogSource);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "AnswerAddDescriptor(pOaSession=%p) - failed to set ANSWER data(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    *ppMediaDescrAnswer = pMediaDescrAnswer;

    return RV_OK;
}

/******************************************************************************
 * DeriveAndAddFormats
 * ----------------------------------------------------------------------------
 * General:
 *  Goes through media formats that appear in the offer descriptor,
 *  and for each format, which presents also in the hash of supported codecs,
 *  copies it to the answer descriptor, while adding format parameters,
 *  acceptable by both sides (offerer and answerer).
 *
 *  The hash is filled with codecs from local and default capabilities.
 *  The Local Capabilities can be set by Application into the Session object
 *  using RvOaSessionSetCapabilities API function.
 *  The Default Capabilities can be set by Application into the Offer-Answer
 *  module using RvOaCapabilities API function.
 *
 * Arguments:
 * Input:  pOaSession        - pointer to the Session object.
 *         pMediaDescrOffer  - pointer to the exact copy of pMediaDescr.
 *         pMediaDescrAnswer - pointer to descriptor to be updated.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV DeriveAndAddFormats(
                                    IN OaSession*       pOaSession,
                                    IN RvSdpMediaDescr* pMediaDescrOffer,
                                    IN RvSdpMediaDescr* pMediaDescrAnswer)
{
    RvStatus                rv;
    RvSize_t                numOfFormats, i;
    const char*             strMediaType;
    OaCodecHashKey          codecHashKey;
    OaMgr*                  pOaMgr = pOaSession->pMgr;
    RvSdpMediaDescr*        pCapabilitiesMediaDescr;
    RvBool                  bStopToAdd = RV_FALSE;
    RvBool                  bCodec13 = RV_FALSE;
    const RvChar*           strFormatName;

    strMediaType = rvSdpMediaDescrGetMediaTypeStr(pMediaDescrOffer);
    numOfFormats = rvSdpMediaDescrGetNumOfFormats(pMediaDescrOffer);

    /* Go through codecs, found in OFFER Media Descriptor, and for each codec
    check if it is supported. */
    for (i=0; i<numOfFormats; i++)
    {
        OaCodecHashInitializeKey(&pOaMgr->hashCodecs, pMediaDescrOffer,
            i, NULL/*pCodecOwner*/, strMediaType, &codecHashKey);

        pCapabilitiesMediaDescr = NULL;

        /* If local capabilities were set, seek them for codec */
        if (RV_TRUE == pOaSession->capMsg.bSdpMsgConstructed)
        {
            codecHashKey.pCodecOwner = (void*)pOaSession;
            pCapabilitiesMediaDescr = OaCodecHashFindCodec(
                                        &pOaMgr->hashCodecs,&codecHashKey);
        }
        else
        /* If default capabilities were set, seek them for codec */
        if (RV_TRUE == pOaMgr->defCapMsg.bSdpMsgConstructed)
        {
            codecHashKey.pCodecOwner = (void*)pOaMgr;
            pCapabilitiesMediaDescr = OaCodecHashFindCodec(
                                        &pOaMgr->hashCodecs, &codecHashKey);
        }

        /* Log */
        strFormatName = (NULL!=codecHashKey.strMediaFormat)?
                codecHashKey.strMediaFormat : codecHashKey.strCodecName;
        RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
            "DeriveAndAddFormats(pOaSession=%p) - media format %s(%s) was%s found in Local Capabilities (offerDescr=%p, answerDescr=%p, CapDescr=%p)",
            pOaSession, 
            ((codecHashKey.strMediaFormat==NULL)? "": codecHashKey.strMediaFormat),
            strFormatName,
            ((pCapabilitiesMediaDescr!=NULL)? "": " not"),
            pMediaDescrOffer, pMediaDescrAnswer, pCapabilitiesMediaDescr));

        /* If offered codec was found in local Capabilities, add it to answer*/
        if (NULL != pCapabilitiesMediaDescr)
        {
            /* Take a special care of Media format 13:
            it can be used only in conjunction with one of following codecs:
                G.711(0), G.726(2,dynamic), G.727(d), G.728(15), G.722(9)*/
            if (NULL != codecHashKey.strMediaFormat &&
                0 == strcmp(codecHashKey.strMediaFormat, "13"))
            {
                bCodec13 = RV_TRUE;
            }

            /* Add codec, if:
                1. The application doesn't want only one codec per descriptor
                2. The application does want only one codec per descriptor but
                   no codec was added yet.
                3. This is Codec 13 */
            if (RV_FALSE == bStopToAdd  ||  RV_TRUE == bCodec13)
            {
                RvBool bFormatWasRemoved = RV_FALSE;
                RvOaMediaFormatInfo formatInfo;

                rv = OaSdpDescrDeriveFinalFormatParams(
                        pMediaDescrOffer,
                        (RvSdpMediaDescr*)pCapabilitiesMediaDescr,
                        codecHashKey.strMediaFormat,
                        codecHashKey.strCodecName,
                        pOaMgr->pLogSource,
                        (RvSdpMediaDescr*)pMediaDescrAnswer,
                        &formatInfo);
                if (RV_OK != rv)
                {
                    RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                        "DeriveAndAddFormats(pOaSession=%p) - failed to derive final parameters(rv=%d:%s)",
                        pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
                    /* Don't return here. Just remove this codec and continue with next. */
                    OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                                NULL /*strFormat*/, formatInfo.formatOffer);
                    continue;  /* for (i=0; i<numOfFormats; i++) */
                }

                /* Give the application an opportunity to inspect and
                   overwrite parameters, derived by the Offer-Answer module */
                if (NULL != pOaMgr->pfnDeriveFormatParams)
                {
                    CallbackDeriveFormatParams(pOaSession,
                        pMediaDescrOffer,
                        (RvSdpMediaDescr*)pCapabilitiesMediaDescr,
                        (RvSdpMediaDescr*)pMediaDescrAnswer,
                        &formatInfo, &bFormatWasRemoved);
                }

                if (RV_TRUE == pOaMgr->bChooseOneFormatOnly)
                {
                    /* Codec 13 is not taken in account by requirement
                    "one codec per descriptor". Therefore it should not 
                    stop loop that adds codecs. */
                    if (RV_FALSE==bCodec13  &&  RV_FALSE==bFormatWasRemoved)
                    {
                        bStopToAdd = RV_TRUE;
                        RvLogDebug(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                            "DeriveAndAddFormats(pOaSession=%p) - stop to add formats: bSetOneCodecPerMediaDescr is TRUE",
                            pOaSession));
                    }
                }
            }
        }
    } /* ENDOF:   for (i=0; i<numOfFormats; i++) */

    /* If Codec 13 was added, ensure presence of conjunction formats */
    if (RV_TRUE == bCodec13)
    {
        rv = OaSdpDescrCheckPayload13((RvSdpMediaDescr*)pMediaDescrAnswer);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DeriveAndAddFormats(pOaSession=%p) - failed to handle Payload Type 13 (rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        }
    }

    return RV_OK;
}


/******************************************************************************
 * GenerateStreamsFromMsg
 * ----------------------------------------------------------------------------
 * General:
 *  For each Media Descriptor in the provided message constructs Stream object.
 *  The new Stream object is added into the Session list of Streams.
 *  The new Stream object is initiated using data from the Media Descriptor.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         pMsg       - pointer to the message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV GenerateStreamsFromMsg(
                                        IN OaSession*  pOaSession,
                                        IN OaSdpMsg*   pOaSdpMsg)
{
    RvStatus         rv;
    RvSdpMsg*        pSdpMsg = pOaSdpMsg->pSdpMsg;
    RvSdpListIter    mdIterator; /* Media Descriptor iterator */
    RvSdpMediaDescr* pMediaDescr;
    OaStream*        pOaStream;
    OaMgr*           pOaMgr = pOaSession->pMgr;

    memset(&mdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsg, &mdIterator);
    pOaStream = NULL;
    RLIST_GetHead(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                  (RLIST_ITEM_HANDLE*)&pOaStream);
    while (NULL != pMediaDescr)
    {
        /* If Stream object correspondent to the Media Descriptor doesn't exist
           allocate it and add to the Session */
        if (NULL == pOaStream)
        {
            rv = AllocateStream(pOaSession, &pOaStream);
            if (RV_OK != rv)
            {
                RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                    "GenerateStreamsFromMsg(hSession=%p) - failed to add stream(rv=%d:%s)",
                    pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
                return rv;
            }
            /* Initiate the Stream object, using the Media Descriptor */
            OaStreamInitiateFromMediaDescr(pOaStream, pOaSession, pMediaDescr);
        }
        else
        {
            if (RVOA_STREAM_STATE_IDLE != pOaStream->eState)
            {
                pOaStream->bNewOffered = RV_FALSE;
            }
        }

        if (RVOA_STREAM_STATE_IDLE == pOaStream->eState)
        {
            OaStreamChangeState(pOaStream, RVOA_STREAM_STATE_ACTIVE);
        }

        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
        RLIST_GetNext(pOaMgr->hStreamListPool, pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
    }

    return RV_OK;
}

/******************************************************************************
 * UpdateStreamsWithRemoteMsg
 * ----------------------------------------------------------------------------
 * General:
 *  For each Stream object in the Session list of streams update it with data,
 *  contained in the remote message.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         pMsg       - pointer to the remote message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV UpdateStreamsWithRemoteMsg(
                                        IN OaSession*  pOaSession,
                                        IN OaSdpMsg*   pOaSdpMsgRemote)
{
    RvSdpMsg*        pSdpMsgRemote = pOaSdpMsgRemote->pSdpMsg;
    RvSdpListIter    mdIterator; /* Media Descriptor iterator */
    RvSdpMediaDescr* pMediaDescr;
    OaStream*        pOaStream;
    OaMgr*           pOaMgr = pOaSession->pMgr;

    /* Get the first Stream */
    RLIST_GetHead(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                  (RLIST_ITEM_HANDLE*)&pOaStream);

    /* Get the first Media Descriptor */
    memset(&mdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsgRemote, &mdIterator);

    while (NULL != pOaStream  &&  NULL != pMediaDescr)
    {
        /* Update Stream */
        OaStreamUpdateWithRemoteMsg(pOaStream, pMediaDescr);

        /* Get next Stream */
        RLIST_GetNext(pOaMgr->hStreamListPool, pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);

        /* Get next Media Descriptor */
        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
    }

    /* Sanity check: number of Streams should match number Media Descritpors */
    if (NULL != pOaStream  || NULL != pMediaDescr)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "UpdateStreamsWithRemoteMsg(pOaSession=%p) - Stream <-> Media Descriptor mismatch. pOaStream=%p, pMediaDescr=%p",
            pOaSession, pOaStream, pMediaDescr));

        /* Interoperability: there are vendors that reject streams by removing
        media descriptions (m-line) from ANSWER. Despite non compatibility
        of such behavior with the Offer-Answer RFC 3264, handle this case:
        move rejected in that way Streams to REJECTED state. */
        RejectNotAnsweredStreams(pOaSession, pOaStream);
    }

    return RV_OK;
}

/******************************************************************************
 * HandleInitialOffer
 * ----------------------------------------------------------------------------
 * General:
 *  Handles received OFFER for IDLE session.
 *  Generates matching ANSWER, creates Stream objects etc.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - SDP OFFER in string form.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HandleInitialOffer(
                                        IN OaSession*  pOaSession,
                                        IN RvChar*     strSdpMsg)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = pOaSession->pMgr;

    /* At this point no remote message should be allocated yet */
    if (NULL != pOaSession->msgRemote.pSdpMsg)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "HandleInitialOffer(pOaSession=%p) - remote message already exist",
            pOaSession));
        return RV_ERROR_UNKNOWN;
    }

    /* Create remote message that holds received OFFER */
    rv = OaUtilsOaSdpMsgConstructParse(pOaMgr->hMessagePool,pOaMgr->pLogSource,
                                       &pOaSession->msgRemote, strSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleInitialOffer(pOaSession=%p) - failed to create remote message (rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Generate ANSWER */
    rv = CreateAnswer(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleInitialOffer(pOaSession=%p) - failed to create ANSWER(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Generate Stream objects based on ANSWER */
    rv = GenerateStreamsFromMsg(pOaSession, &pOaSession->msgLocal);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleInitialOffer(pOaSession=%p) - failed to generate streams(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Fill Stream objects with data, contained in ANSWER */
    rv = UpdateStreamsWithRemoteMsg(pOaSession, &pOaSession->msgRemote);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleInitialOffer(pOaSession=%p) - failed to update streams with remote message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Move to ANSWER_READY state */
    ChangeSessionState(pOaSession, RVOA_SESSION_STATE_ANSWER_READY);

    return RV_OK;
}

/******************************************************************************
 * HandleModifyingOffer
 * ----------------------------------------------------------------------------
 * General:
 *  Handles received OFFER for not IDLE session.
 *  Generates matching ANSWER, modifies Stream objects, creates new Stream
 *  objects if need, etc.
 *  Moves the session into the ANSWER_READY state.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - SDP OFFER in string from.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HandleModifyingOffer(
                                        IN OaSession*  pOaSession,
                                        IN RvChar*     strSdpMsg)
{
    RvStatus  rv;

    rv = HandleModifyingMessage(pOaSession, strSdpMsg, RV_TRUE /* bOffer*/);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
            "HandleModifyingOffer(pOaSession=%p) - failed to handle modifying message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    ChangeSessionState(pOaSession, RVOA_SESSION_STATE_ANSWER_READY);

    return RV_OK;
}

/******************************************************************************
 * HandleModifyingAnswer
 * ----------------------------------------------------------------------------
 * General:
 *  Handles received ANSWER for modifying OFFER:
 *  modifies Stream objects.
 *  Moves the session into the ANSWER_RCVD state.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - SDP ANSWER in string from.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HandleModifyingAnswer(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg)
{
    RvStatus rv;

    rv = HandleModifyingMessage(pOaSession, strSdpMsg, RV_FALSE /* bOffer*/);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
            "HandleModifyingAnswer(pOaSession=%p) - failed to handle modifying message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    ChangeSessionState(pOaSession, RVOA_SESSION_STATE_ANSWER_RCVD);

    return RV_OK;
}

/******************************************************************************
 * HandleModifyingMessage
 * ----------------------------------------------------------------------------
 * General:
 *  Handles received message that may modify the session:
 *  for modifying OFFER modifies Stream objects and creates new Stream objects.
 *  for ANSWER for modifying OFFER modifies Stream objects only.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - received SDP message in string from.
 *         bOffer     - if RV_TRUE, the message will be handled as a modifying
 *                      OFFER. Otherwise - as an ANSWER to the modifying OFFER.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HandleModifyingMessage(
                            IN OaSession*  pOaSession,
                            IN RvChar*     strSdpMsg,
                            IN RvBool      bOffer)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = pOaSession->pMgr;
    OaSdpMsg  oaSdpMsgPreviousRemoteMsg;

    /* Backup previously received message */
    memset(&oaSdpMsgPreviousRemoteMsg, 0, sizeof(OaSdpMsg));
    rv = OaUtilsOaSdpMsgConstructCopy(pOaMgr->hMessagePool,pOaMgr->pLogSource,
                                      &oaSdpMsgPreviousRemoteMsg,
                                      pOaSession->msgRemote.pSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleModifyingMessage(pOaSession=%p) - failed to backup remote message (rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Recreate remote message that holds received OFFER or ANSWER */
    rv = OaUtilsOaSdpMsgConstructParse(pOaMgr->hMessagePool,pOaMgr->pLogSource,
                                       &pOaSession->msgRemote, strSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleModifyingMessage(pOaSession=%p) - failed to create remote message (rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaUtilsOaSdpMsgDestruct(&oaSdpMsgPreviousRemoteMsg);
        return rv;
    }

    /* Update Session with received OFFER or ANSWER.
       For OFFER: update existing Stream objects, add new Stream objects,
                  update ANSWER with changed Media Descriptions, add new Media
                  Descriptors to ANSWER, etc.
       For ANSWER: update existing Stream objects. */
    if (RV_TRUE == bOffer)
    {
        rv = ModifySessionOnRcvdOffer(pOaSession, &oaSdpMsgPreviousRemoteMsg);
    }
    else
    {
        rv = ModifySessionOnRcvdAnswer(pOaSession, &oaSdpMsgPreviousRemoteMsg);
    }
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleModifyingMessage(pOaSession=%p, bOffer=%d) - failed to modify session(rv=%d:%s)",
            pOaSession, bOffer, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaUtilsOaSdpMsgDestruct(&oaSdpMsgPreviousRemoteMsg);
        return rv;
    }

    OaUtilsOaSdpMsgDestruct(&oaSdpMsgPreviousRemoteMsg);

    return RV_OK;
}

/******************************************************************************
 * HandleAnswer
 * ----------------------------------------------------------------------------
 * General:
 *  Handles received ANSWER.
 *  Updates Stream objects with data received from remote side with ANSWER.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         strSdpMsg  - SDP ANSWER in string form.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV HandleAnswer(
                                        IN OaSession*  pOaSession,
                                        IN RvChar*     strSdpMsg)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = pOaSession->pMgr;

    /* Create remote message that holds received ANSWER */
    rv = OaUtilsOaSdpMsgConstructParse(pOaMgr->hMessagePool,pOaMgr->pLogSource,
            &pOaSession->msgRemote, strSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleAnswer(pOaSession=%p) - failed to create remote message (rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Update Stream objects using received ANSWER */
    rv = UpdateStreamsWithRemoteMsg(pOaSession, &pOaSession->msgRemote);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "HandleAnswer(pOaSession=%p) - failed to update streams with remote message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Move to ANSWER_RCVD state */
    ChangeSessionState(pOaSession, RVOA_SESSION_STATE_ANSWER_RCVD);

    return RV_OK;
}

/******************************************************************************
 * ModifySessionOnRcvdOffer
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies Session object as a result of reception of modifying OFFER.
 *  The modification includes update of existing Stream objects
 *  with data from the received OFFER, creation of new Streams, if new media
 *  descriptors were found in the OFFER, etc. The ANSWER matching the OFFER is
 *  built. The OFFER is contained in msgRemote field of the Session object,
 *  the ANSWER is contained in the msgLocal field of the Session object.
 *
 *  In order to find modified elements the message previously received from
 *  the remote side, is used. It is compared with received message.
 *
 * Arguments:
 * Input:  pOaSession             - pointer to the Session object.
 *         pOaSdpMsgPrevRemoteMsg - pointer to the message, received from
 *                                  the remote side during previous
 *                                  OFFER-ANSWER transaction.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV ModifySessionOnRcvdOffer(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsgPrevRemoteMsg)
{
    RvStatus         rv;
    RvSdpListIter    mdIterator; /* Media Descriptor iterator */
    RvSdpMediaDescr* pMediaDescr;
    OaStream*        pOaStream;
    RvSdpMediaDescr* pMediaDescrLocal;
    RvBool           bSessionWasModified = RV_FALSE;
    RvBool           bThereIsNewlyOfferedStreams = RV_FALSE;

    ModifyStreams(pOaSession, pOaSdpMsgPrevRemoteMsg->pSdpMsg, &mdIterator,
                  &pOaStream, &pMediaDescr, &bThereIsNewlyOfferedStreams,
                  &bSessionWasModified);

    /* If there are newly offered streams, add new Streams and build
    correspondent media descriptors in the local message.
    ANSWER will be generated from the local message. */
    if (RV_TRUE == bThereIsNewlyOfferedStreams)
    {
        while (NULL != pMediaDescr)
        {
            /* Allocate Stream object and add it into the Session */
            rv = AllocateStream(pOaSession, &pOaStream);
            if (RV_OK != rv)
            {
                RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                    "ModifySessionOnRcvdOffer(hSession=%p) - failed to add stream(rv=%d:%s)",
                    pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
                return rv;
            }

            /* Add Descriptor to the Local Message, used to store ANSWER */
            rv = AnswerAddDescriptor(pOaSession,pMediaDescr,&pMediaDescrLocal);
            if (RV_OK != rv)
            {
                RemoveStream(pOaSession, (void*)pOaStream);
                RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                    "ModifySessionOnRcvdOffer(hSession=%p) - failed to add Media Descriptor(rv=%d:%s)",
                    pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
                return rv;
            }

            /* Initiate the Stream object, using the Media Descriptor */
            OaStreamInitiateFromMediaDescr(pOaStream, pOaSession,
                                           pMediaDescrLocal);
            OaStreamUpdateWithRemoteMsg(pOaStream, pMediaDescr);

            OaStreamChangeState(pOaStream, RVOA_STREAM_STATE_ACTIVE);

            bSessionWasModified = RV_TRUE;

            pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
        }
    }

    if (RV_TRUE == bSessionWasModified)
    {
        rv = OaSdpMsgIncrementVersion(pOaSession->msgLocal.pSdpMsg,
                                      pOaSession->pMgr->pLogSource);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "ModifySessionOnRcvdOffer(pOaSession=%p) - failed to increment version of session description(rv=%d:%s)",
                pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
            /* don't return */
        }
    }

    RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
        "ModifySessionOnRcvdOffer(pOaSession=%p) - OFFER was processed: bThereIsNewlyOfferedStreams=%d, bSessionWasModified=%d",
        pOaSession, bThereIsNewlyOfferedStreams, bSessionWasModified));

    return RV_OK;
}

/******************************************************************************
 * ModifySessionOnRcvdAnswer
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies Session object as a result of reception of ANSWER
 *  for the modifying OFFER.
 *  The modification includes update of existing Stream objects
 *  with data from the received ANSWER. The ANSWER is contained in msgRemote
 *  field of the Session object, the OFFER is contained in the msgLocal field
 *  of the Session object.
 *
 *  In order to find modified elements the message previously received from
 *  the remote side, is used. It is compared with the received message.
 *
 * Arguments:
 * Input:  pOaSession             - pointer to the Session object.
 *         pOaSdpMsgPrevRemoteMsg - pointer to the message, received from
 *                                  the remote side during previous
 *                                  OFFER-ANSWER transaction.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV ModifySessionOnRcvdAnswer(
                            IN OaSession*  pOaSession,
                            IN OaSdpMsg*   pOaSdpMsgPrevRemoteMsg)
{
    RvSdpListIter    mdIterator; /* Media Descriptor iterator */
    RvSdpMediaDescr* pMediaDescr;
    OaStream*        pOaStream;
    OaMgr*           pOaMgr = pOaSession->pMgr;
    RvBool           bSessionWasModified = RV_FALSE;
    RvBool           bThereIsNewlyOfferedStreams = RV_FALSE;

    /* Modify Streams, that were opened before Session modification */
    ModifyStreams(pOaSession, pOaSdpMsgPrevRemoteMsg->pSdpMsg, &mdIterator,
                  &pOaStream, &pMediaDescr, &bThereIsNewlyOfferedStreams,
                  &bSessionWasModified);

    /* Updates Streams, that were added during Session modification */
    if (RV_TRUE == bThereIsNewlyOfferedStreams)
    {
        while (NULL != pOaStream && NULL != pMediaDescr)
        {
            OaStreamUpdateWithRemoteMsg(pOaStream, pMediaDescr);

            /* Get next Stream */
            RLIST_GetNext(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
            /* Get next modified Media Descriptor */
            pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
        }
    }

    /* Check, if the remote side offered streams in the ANSWER */
    if (NULL == pOaStream && NULL != pMediaDescr)
    {
        RvLogWarning(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "ModifySessionOnRcvdAnswer(pOaSession=%p) - ANSWER offers new streams. They are ignored",
            pOaSession));
    }

    /* Sanity check: number of Streams should match number Media Descritpors */
    if (NULL != pOaStream  &&  NULL == pMediaDescr)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "ModifySessionOnRcvdAnswer(pOaSession=%p) - Stream <-> Media Descriptor mismatch. pOaStream=%p, remote Media Descripor=NULL",
            pOaSession, pOaStream));

        /* Interoperability: there are vendors that reject streams by removing
        media descriptions (m-line) from ANSWER. Despite non compatibility
        of such behavior with the Offer-Answer RFC 3264, handle this case:
        move rejected in that way Streams to REJECTED state. */
        RejectNotAnsweredStreams(pOaSession, pOaStream);
    }

    RvLogDebug(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
        "ModifySessionOnRcvdAnswer(pOaSession=%p) - ANSWER was processed: bThereIsNewlyOfferedStreams=%d, bSessionWasModified=%d",
        pOaSession, bThereIsNewlyOfferedStreams, bSessionWasModified));

    return RV_OK;
}

/******************************************************************************
 * ModifyStreams
 * ----------------------------------------------------------------------------
 * General:
 *  Goes over session streams and modifies each of them.
 *
 *  In order to find modified elements the media descriptors previously
 *  received from the remote side, are used. They are compared with the media
 *  descriptors from the last received message.
 *
 * Arguments:
 * Input:  pOaSession             - pointer to the Session object.
 *         pOaSdpMsgPrevRemoteMsg - pointer to the message, received from
 *                                  the remote side during previous
 *                                  Offer-Answer transaction.
 *         pMdIterator            - iterator to be used while going over
 *                                  last received message media descriptors.
 * Output: ppOaStream -             new stream, that was offered to the remote
 *                                  side during last Offer-Answer transaction.
 *                                  NULL, if no streams were offered.
 *         ppMediaDescr -           media descriptor from the last received
 *                                  message, matching the newly offered stream.
 *         pbNewlyOfferedStreams -  RV_TRUE, if newly offered streams were
 *                                  found in the last received message.
 *         pbSessionWasModified  -  RV_TRUE, if any of existing streams was
 *                                  modified by the last received message.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static void RVCALLCONV ModifyStreams(
                            IN   OaSession*        pOaSession,
                            IN   RvSdpMsg*         pSdpMsgRemotePrev,
                            IN   RvSdpListIter*    pMdIterator,
                            OUT  OaStream**        ppOaStream,
                            OUT  RvSdpMediaDescr** ppMediaDescr,
                            OUT  RvBool*           pbNewlyOfferedStreams,
                            OUT  RvBool*           pbSessionWasModified)
{
    RvStatus         rv;
    RvSdpMsg*        pSdpMsgRemote = pOaSession->msgRemote.pSdpMsg;
    OaMgr*           pOaMgr = pOaSession->pMgr;
    OaStream*        pOaStream;
    RvSdpMediaDescr* pMediaDescr;
    RvSdpMediaDescr* pMediaDescrPrev;
    RvSdpListIter    mdIteratorPrev;

    /* Get the first Stream */
    RLIST_GetHead(pOaMgr->hStreamListPool, pOaSession->hStreamList,
        (RLIST_ITEM_HANDLE*)&pOaStream);

    /* Get the first Media Descriptor */
    memset(&mdIteratorPrev, 0, sizeof(RvSdpListIter));
    pMediaDescrPrev = rvSdpMsgGetFirstMediaDescr(
                            pSdpMsgRemotePrev, &mdIteratorPrev);
    /* Get the first modified Media Descriptor */
    memset(pMdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsgRemote, pMdIterator);

    /* Modify existing Streams and update correspondent media descriptors
    in the local message. */
    while (NULL != pOaStream && NULL != pMediaDescr && NULL != pMediaDescrPrev)
    {
        rv = OaStreamModify(pOaStream, pSdpMsgRemotePrev, pMediaDescrPrev, 
                            pSdpMsgRemote, pMediaDescr, pbSessionWasModified);
        if (RV_OK != rv)
        {
            RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
                "ModifyStreams(pOaSession=%p) - failed to modify stream pOaStream=%p",
                pOaSession, pOaStream));
            /* Don't return, just continue with the next descriptor */
        }

        /* Get next Stream */
        RLIST_GetNext(pOaMgr->hStreamListPool, pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
        /* Get next Media Descriptor */
        pMediaDescrPrev = rvSdpMsgGetNextMediaDescr(&mdIteratorPrev);
        /* Get next modified Media Descriptor */
        pMediaDescr = rvSdpMsgGetNextMediaDescr(pMdIterator);
    }

    /* Check if ANSWER was received for newly offered Stream */
    *ppOaStream   = pOaStream;
    *ppMediaDescr = pMediaDescr;
    *pbNewlyOfferedStreams = RV_FALSE;
    if (NULL != pOaStream && NULL != pMediaDescr && NULL == pMediaDescrPrev)
    {
        *pbNewlyOfferedStreams = RV_TRUE;
    }
}

/******************************************************************************
 * AllocateStream
 * ----------------------------------------------------------------------------
 * General:
 *  Takes element from the pool of Stream objects and inserts it
 *  into the Session list of streams.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: ppOaStream - allocated Stream element.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV AllocateStream(
                                    IN  OaSession* pOaSession,
                                    OUT OaStream** ppOaStream)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = pOaSession->pMgr;
    RLIST_ITEM_HANDLE hListElement;

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = RLIST_InsertTail(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                          &hListElement);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "AllocateStream(pOaSession=%p) - failed to allocate new Stream object(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    OaMgrUnlock(pOaMgr);

    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "AllocateStream(pOaSession=%p) - new Stream %p was added",
        pOaSession, hListElement));

    *ppOaStream = (OaStream*)hListElement;
    (*ppOaStream)->eState = RVOA_STREAM_STATE_IDLE;
    (*ppOaStream)->bNewOffered = RV_TRUE;
    (*ppOaStream)->pSession = pOaSession;

    pOaSession->numOfStreams++;

    return RV_OK;
}

/******************************************************************************
 * RemoveStream
 * ----------------------------------------------------------------------------
 * General:
 *  Removes Stream object from the Session list of streams and returns it back
 *  to the pool of stream objects.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: ppOaStream - allocated Stream element.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV RemoveStream(
                                    IN OaSession* pOaSession,
                                    IN void*      pOaStream)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = pOaSession->pMgr;

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    RLIST_Remove(pOaMgr->hStreamListPool, pOaSession->hStreamList,
                 (RLIST_ITEM_HANDLE)pOaStream);

    OaMgrUnlock(pOaMgr);

    RvLogDebug(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RemoveStream(pOaSession=%p) - Stream %p was removed",
        pOaSession, pOaStream));

    return RV_OK;
}

/******************************************************************************
 * CallbackDeriveFormatParams
 * ----------------------------------------------------------------------------
 * General:
 *  Calls callback, that provides the application with the data,
 *  needed in order to derive parameters of media format or codec,
 *  that are acceptable by both sides.
 *  This callback is called for each media format, supported by both sides,
 *  when the ANSWER message is built.
 *
 * Arguments:
 * Input:  pOaSession        - The pointer to the Session object.
 *         pMediaDescrOffer  - The pointer to the Media Descriptor object,
 *                             which contains offered media format and
 *                             format parameters.
 *         pMediaDescrCaps   - The pointer to the Media Descriptor object,
 *                             which contains the media format and format
 *                             parameters, set by the application as a local
 *                             capabilities.
 *         pMediaDescrAnswer - The pointer to the Media Descriptor object,
 *                             which contains the media format and
 *                             format parameters, derived by the Offer-Answer
 *                             module using pMediaDescrOffer and
 *                             pMediaDescrCaps media descriptors.
 *         pFormatInfo       - Auxiliary information about media format.
 *                             It uses the pMediaDescrCaps descriptor.
 *         strMediaFormat    - Media Format, if it uses static payload type.
 *                             Can be NULL.
 *         strCodecName      - Codec name, if format uses dynamic payload type.
 *                             Can be NULL.
 * Output:  pbRemoved        - If RV_TRUE - the application ordered to remove
 *                             the format from ANSWER.
 *****************************************************************************/
static void RVCALLCONV CallbackDeriveFormatParams(
                            IN  OaSession*            pOaSession,
                            IN  RvSdpMediaDescr*      pMediaDescrOffer,
                            IN  RvSdpMediaDescr*      pMediaDescrCaps,
                            IN  RvSdpMediaDescr*      pMediaDescrAnswer,
                            IN  RvOaMediaFormatInfo*  pFormatInfo,
                            OUT RvBool*               pbRemoved)
{
    OaMgr* pOaMgr = pOaSession->pMgr;

    RvLogDebug(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "CallbackDeriveFormatParams(hSession=%p, hAppSession=%p) - before callback",
        pOaSession, pOaSession->hAppHandle));

    pOaMgr->pfnDeriveFormatParams(
                        (RvOaSessionHandle)pOaSession, pOaSession->hAppHandle,
                        pMediaDescrOffer, pMediaDescrCaps, pMediaDescrAnswer,
                        pFormatInfo, pMediaDescrOffer->iSdpMsg,
                        pMediaDescrAnswer->iSdpMsg, pbRemoved);

    RvLogInfo(pOaMgr->pLogSource, (pOaMgr->pLogSource,
        "CallbackDeriveFormatParams(hSession=%p, hAppSession=%p) - after callback: format %d was %s",
        pOaSession, pOaSession->hAppHandle, pFormatInfo->formatOffer,
        (*pbRemoved==RV_TRUE)?"removed":"approved"));

    if (*pbRemoved == RV_TRUE)
    {
        OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                               NULL /*strFormat*/, pFormatInfo->formatOffer);
    }
}

/******************************************************************************
 * RejectNotAnsweredStreams
 * ----------------------------------------------------------------------------
 * General:
 *  Resets Stream object.
 *  This function is called for Streams, Media Description for that was not
 *  found in ANSWER.
 *  Such behavior was implemented for interoperability with some standard not
 *  compliant vendors. Offer-Answer RFC 3264 claims that answerer should never
 *  remove media descriptions from session description. Instead of this it has
 *  to set zero port in correspondent m-line, if it doesn't accept the stream.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 *         pOaStream  - the first object in list of Streams to be reset.
 *****************************************************************************/
static void RVCALLCONV RejectNotAnsweredStreams(
                            IN  OaSession*            pOaSession,
                            IN  OaStream*             pOaStream)
{
    while (pOaStream != NULL)
    {
        OaStreamReset(pOaStream);
        RLIST_GetNext(pOaSession->pMgr->hStreamListPool,pOaSession->hStreamList,
            (RLIST_ITEM_HANDLE)pOaStream, (RLIST_ITEM_HANDLE*)&pOaStream);
    }
}

/*nl for linux */


