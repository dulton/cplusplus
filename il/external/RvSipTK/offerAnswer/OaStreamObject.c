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
 *                              <OaStreamObject.c>
 *
 * The OaStreamObject.h file defines Stream object.
 * The Stream is stateless object, which enable the Application to inspect,
 * to modify and to track modifications made by the remote side in Media
 * Descriptions.
 * The Stream object keeps references to the media descriptor of local and
 * remote messages.
 * The Stream object provides functions for inspection data in remote
 * descriptor and for set new or modification data in local descriptor.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "OaStreamObject.h"
#include "OaSdpMsg.h"
#include "OaUtils.h"

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTIONS DEFINITIONS                      */
/*---------------------------------------------------------------------------*/
static void RVCALLCONV CheckHoldResume(
                                  IN OaStream* pOaStream,
                                  IN RvSdpConnectionMode eConnModePrev,
                                  IN RvSdpConnectionMode eConnModeCurr);

static void RVCALLCONV CheckRemoteAddress(
                                IN OaStream*        pOaStream,
                                IN RvSdpMsg*        pSdpMsgRemoteBefore,
                                IN RvSdpMediaDescr* pMediaDescrRemoteBefore,
                                IN RvSdpMsg*        pSdpMsgRemoteModified,
                                IN RvSdpMediaDescr* pMediaDescrRemoteModified);

/*---------------------------------------------------------------------------*/
/*                              MODULE FUNCTIONS                             */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaStreamInitiateFromCfg
 * ----------------------------------------------------------------------------
 * General:
 *  Initiates Stream object using data from configuration.
 *  Builds new Media Descriptor in the local message and populate it with data
 *  from the configuration.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 *         pOaSession  - pointer to the Session object.
 *         pStreamCfg  - configuration.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaStreamInitiateFromCfg(
                                        IN OaStream*      pOaStream,
                                        IN OaSession*     pOaSession,
                                        IN RvOaStreamCfg* pStreamCfg)
{
    RvSdpMediaDescr* pMediaDescr;

    /* Create descriptor in the message */
    pMediaDescr = rvSdpMsgAddMediaDescr(pOaSession->msgLocal.pSdpMsg,
                        pStreamCfg->eMediaType, pStreamCfg->port,
                        pStreamCfg->eProtocol);
    if (NULL == pMediaDescr)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "OaStreamInitiateFromCfg(pOaStream=%p, pOaSession=%p) - failed to add new media descriptor",
            pOaStream, pOaSession));
        return RV_ERROR_UNKNOWN;
    }

    OaStreamInitiateFromMediaDescr(pOaStream, pOaSession, pMediaDescr);

    return RV_OK;
}

/******************************************************************************
 * OaStreamInitiateFromMediaDescr
 * ----------------------------------------------------------------------------
 * General:
 *  Initiates Stream object using data from Media Descriptor.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 *         pOaSession  - pointer to the Session object.
 *         pMediaDescr - pointer to the message.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaStreamInitiateFromMediaDescr(
                                        IN OaStream*        pOaStream,
                                        IN OaSession*       pOaSession,
                                        IN RvSdpMediaDescr* pMediaDescr)
{
    pOaStream->pSession = pOaSession;
    pOaStream->hAppHandle = NULL;
    pOaStream->pMediaDescrLocal = pMediaDescr;
    pOaStream->pMediaDescrRemote = NULL;
    pOaStream->bWasModified = RV_FALSE;
    pOaStream->bWasClosed = RV_FALSE;
    pOaStream->bAddressWasModified = RV_FALSE;
    return;
}

/******************************************************************************
 * OaStreamDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Stream object data and frees resources consumed by the Stream.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaStreamDestruct(IN OaStream*  pOaStream)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvLogSource* pLogSource = pOaStream->pSession->pMgr->pLogSource;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    memset((void*)pOaStream, 0, sizeof(OaStream));
    pOaStream->eState = RVOA_STREAM_STATE_UNDEFINED;

    RvLogDebug(pLogSource, (pLogSource,
        "OaStreamDestruct(pOaStream=%p) - stream was destructed", pOaStream));
}

/******************************************************************************
 * OaStreamModify
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies Stream object in accordance to data received in modifying OFFER.
 *  Takes a care of HOLD / RESUME, updates parameters of Stream status,
 *  updates local media descriptor based on remote modified descriptor and
 *  remote descriptor before modification.
 *
 * Arguments:
 * Input:  pOaStream                 - pointer to the Stream object.
 *         pSdpMsgRemotePrev         - remote message before modification.
 *         pMediaDescrRemoteBefore   - remote media descriptor before
 *                                     modification.
 *         pSdpMsgRemoteModified     - modified remote message modification
 *                                     (received with last OFFER).
 *         pMediaDescrRemoteModified - modified remote media descriptor
 *                                     (received with last OFFER).
 * Output: pbSessionWasModified - RV_TRUE, if the modification of Stream
 *                                impacts on Session Modification status.
 *                                Never is set to RV_FALSE.
 *
 * Return Value: RV_OK on success, error - otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV OaStreamModify(
                                IN  OaStream*        pOaStream,
                                IN  RvSdpMsg*        pSdpMsgRemotePrev,
                                IN  RvSdpMediaDescr* pMediaDescrRemoteBefore,
                                IN  RvSdpMsg*        pSdpMsgRemoteModified,
                                IN  RvSdpMediaDescr* pMediaDescrRemoteModified,
                                OUT RvBool*          pbSessionWasModified)
{
    RvStatus  rv;
    RvBool    bDescrAreEqual;
    RvSdpConnectionMode eConnModeBefore;
    RvSdpConnectionMode eConnModeModified;
    RvInt32   remotePort;
    RvBool    bConnModeWasModified;

    /* Update Stream */
    pOaStream->pMediaDescrRemote = pMediaDescrRemoteModified;
    pOaStream->bNewOffered = RV_FALSE;
    pOaStream->bWasModified = RV_FALSE;
    pOaStream->bWasClosed = RV_FALSE;
    pOaStream->bAddressWasModified = RV_FALSE;

    /* Do nothing for removed streams */
    if (RVOA_STREAM_STATE_REMOVED == pOaStream->eState)
    {
        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamModify(pOaStream=%p): stream was removed. Ignore it.",
            pOaStream));
        return RV_OK;
    }

    /* Check if the Stream was closed */
    remotePort = rvSdpMediaDescrGetPort(pMediaDescrRemoteModified);
    if (remotePort == 0)
    {
        pOaStream->bWasClosed = RV_TRUE;
        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamModify - STATUS(pOaStream=%p): stream was closed by the remote side",
            pOaStream));
    }

    eConnModeBefore   = rvSdpMediaDescrGetConnectionMode(
                            (const RvSdpMediaDescr*)pMediaDescrRemoteBefore);
    eConnModeModified = rvSdpMediaDescrGetConnectionMode(
                            (const RvSdpMediaDescr*)pMediaDescrRemoteModified);

    /* Perform various checks */
    if (pOaStream->bWasClosed == RV_FALSE)
    {
        /* Check if the Stream was held or resumed */
        CheckHoldResume(pOaStream, eConnModeBefore, eConnModeModified);

        /* Check if remote address for media reception was modified */
        CheckRemoteAddress(pOaStream,pSdpMsgRemotePrev,  pMediaDescrRemoteBefore,
                           pSdpMsgRemoteModified, pMediaDescrRemoteModified);
    }

    /* Check if any of description parameters were changed,
       excepting the connection mode */

    /* In order to ignore the possible change of connection mode,
       reset the connection mode parameters in both descriptors. */
    if (eConnModeModified != eConnModeBefore)
    {
        rvSdpMediaDescrSetConnectionMode(pMediaDescrRemoteBefore,
                                         RV_SDPCONNECTMODE_INACTIVE);
        rvSdpMediaDescrSetConnectionMode(pMediaDescrRemoteModified,
                                         RV_SDPCONNECTMODE_INACTIVE);
    }

    /* Compare previous and modified remote media descriptors */
    bDescrAreEqual = rvSdpMediaDescrCompare(
                            (const RvSdpMediaDescr*)pMediaDescrRemoteModified,
                            (const RvSdpMediaDescr*)pMediaDescrRemoteBefore);
    if (RV_FALSE == bDescrAreEqual)
    {
        pOaStream->bWasModified = RV_TRUE;
    }

    /* Restore connection mode parameters if they were overwritten */
    bConnModeWasModified = RV_FALSE;
    if (eConnModeModified != eConnModeBefore)
    {
        rvSdpMediaDescrSetConnectionMode(pMediaDescrRemoteBefore,eConnModeBefore);
        rvSdpMediaDescrSetConnectionMode(pMediaDescrRemoteModified,eConnModeModified);
        bConnModeWasModified = RV_TRUE;
    }

    /* Update local Media Descriptor in accordance to the new description. */
    if (RV_TRUE == pOaStream->bWasModified || RV_TRUE == bConnModeWasModified)
    {
        rv = OaSdpDescrModify(pOaStream->pMediaDescrLocal,
                              pOaStream->pMediaDescrRemote,
                              pOaStream->pSession->pMgr->pLogSource);
        if (RV_OK != rv)
        {
            RvLogError(pOaStream->pSession->pMgr->pLogSource,
                (pOaStream->pSession->pMgr->pLogSource,
                "OaStreamModify(pOaStream=%p) - failed to modify local descriptor(rv=%d:%s)",
                pOaStream, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return rv;
        }

        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamModify - STATUS(pOaStream=%p): stream parameters were modified by the remote side",
            pOaStream));
    }
    else
    {
        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamModify - STATUS(pOaStream=%p): stream parameters were not modified by the remote side",
            pOaStream));
    }

    /* Update the requested SessionWasModified flag */
    if (RV_TRUE == pOaStream->bWasModified || RV_TRUE == bConnModeWasModified)
    {
        *pbSessionWasModified = RV_TRUE;
    }

    return RV_OK;
}

/******************************************************************************
 * OaStreamUpdateWithRemoteMsg
 * ----------------------------------------------------------------------------
 * General:
 *  Update Stream object witht the data, contained in correspondent media
 *  descriptor in the remote message.
 *
 * Arguments:
 * Input:  pOaStream         - pointer to the Stream object.
 *         pMediaDescrRemote - remote media descriptor.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaStreamUpdateWithRemoteMsg(
                                IN  OaStream*        pOaStream,
                                IN  RvSdpMediaDescr* pMediaDescrRemote)
{
    RvInt32          remotePort;

    /* Update Stream */
    pOaStream->pMediaDescrRemote = pMediaDescrRemote;

    /* Check, if the stream was rejected by the remote side.
    If port in the remote descriptor is 0, than the stream was rejected.
    In this case, remove the stream */
    remotePort = rvSdpMediaDescrGetPort(pMediaDescrRemote);
    if (0 == remotePort && RVOA_STREAM_STATE_REMOVED != pOaStream->eState)
    {
        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamUpdateWithRemoteMsg(pOaStream=%p) - zero port was found => remove the stream",
            pOaStream));
        OaStreamReset(pOaStream);
    }
}

/******************************************************************************
 * OaStreamHold
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies local Media Descriptor of the Stream object in order
 *  to put other side on hold, as it is described by put on hold procedure
 *  in RFC 3264.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK on success, error - otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV OaStreamHold(IN OaStream*  pOaStream)
{
    RvSdpStatus         srv;
    RvSdpConnectionMode eConnModeHold;

    if (NULL == pOaStream->pMediaDescrLocal)
    {
        RvLogError(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamHold(pOaStream=%p) - no local Media Descriptor was found",
            pOaStream));
        return RV_ERROR_UNKNOWN;
    }
    if (pOaStream->eState == RVOA_STREAM_STATE_REMOVED)
    {
        RvLogWarning(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamHold(pOaStream=%p) - can't hold removed stream",
            pOaStream));
        return RV_OK;
    }

    pOaStream->eConnModeBeforeHold = rvSdpMediaDescrGetConnectionMode(
                        (const RvSdpMediaDescr*)pOaStream->pMediaDescrLocal);
    switch(pOaStream->eConnModeBeforeHold)
    {
        case RV_SDPCONNECTMODE_SENDRECV:
        case RV_SDPCONNECTMODE_SENDONLY:
        case RV_SDPCONNECTMODE_NOTSET:
            eConnModeHold = RV_SDPCONNECTMODE_SENDONLY;
            break;
        default:
            eConnModeHold = RV_SDPCONNECTMODE_INACTIVE;
            break;
    }

    srv = rvSdpMediaDescrSetConnectionMode(
                pOaStream->pMediaDescrLocal, eConnModeHold);
    if (RV_OK != srv)
    {
        RvLogError(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamHold(pOaStream=%p) - failed to set connection mode: Media Desr=%p, Conn Mode=%d(status=%d:%s)",
            pOaStream, pOaStream->pMediaDescrLocal, eConnModeHold,
            srv, OaUtilsConvertEnum2StrSdpStatus(srv)));
        return OA_SRV2RV(srv);
    }

    RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
        (pOaStream->pSession->pMgr->pLogSource,
        "OaStreamHold(pOaStream=%p) - the remote side was put on hold",
        pOaStream));

    return RV_OK;
}

/******************************************************************************
 * OaStreamResume
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies local Media Descriptor of the Stream object in order
 *  to resume other side from hold, as it is described by put on hold procedure
 *  in RFC 3264.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK on success, error - otherwise.
 *****************************************************************************/
RvStatus RVCALLCONV OaStreamResume(IN OaStream*  pOaStream)
{
    RvSdpStatus         srv;
    RvSdpConnectionMode eConnModeCurr;

    if (NULL == pOaStream->pMediaDescrLocal)
    {
        RvLogError(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamResume(pOaStream=%p) - no local Media Descriptor was found",
            pOaStream));
        return RV_ERROR_UNKNOWN;
    }
    if (pOaStream->eState == RVOA_STREAM_STATE_REMOVED)
    {
        RvLogWarning(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamResume(pOaStream=%p) - can't resume removed stream",
            pOaStream));
        return RV_OK;
    }

    /* Sanity check */
    eConnModeCurr = rvSdpMediaDescrGetConnectionMode(
                        (const RvSdpMediaDescr*)pOaStream->pMediaDescrLocal);
    switch(eConnModeCurr)
    {
        case RV_SDPCONNECTMODE_INACTIVE:
        case RV_SDPCONNECTMODE_SENDONLY:
            break;
        default:
            RvLogError(pOaStream->pSession->pMgr->pLogSource,
                (pOaStream->pSession->pMgr->pLogSource,
                "OaStreamResume(pOaStream=%p) - wrong connection mode %d",
                pOaStream, eConnModeCurr));
            return RV_ERROR_UNKNOWN;
    }

    srv = rvSdpMediaDescrSetConnectionMode(
                pOaStream->pMediaDescrLocal, pOaStream->eConnModeBeforeHold);
    if (RV_OK != srv)
    {
        RvLogError(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "OaStreamResume(pOaStream=%p) - failed to set connection mode: Media Desr=%p, Conn Mode=%d(status=%d:%s)",
            pOaStream, pOaStream->pMediaDescrLocal, pOaStream->eConnModeBeforeHold,
            srv, OaUtilsConvertEnum2StrSdpStatus(srv)));
        return OA_SRV2RV(srv);
    }

    RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
        (pOaStream->pSession->pMgr->pLogSource,
        "OaStreamResume(pOaStream=%p) - the remote side was resumed from being held",
        pOaStream));

    return RV_OK;
}

/******************************************************************************
 * OaStreamReset
 * ----------------------------------------------------------------------------
 * General:
 *  Removes all attributes from local Media Descriptor of the Stream object
 *  and moves it into REMOVED state.
 *  After this the Stream can't be used any more.
 *
 * Arguments:
 * Input:  pOaStream   - pointer to the Stream object.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaStreamReset(IN OaStream*  pOaStream)
{
    RvSdpStatus    srv;
    RvSdpMediaType eMediaType;
    const char*    strMediaType;
    RvSize_t       numOfFormats;
    const char*    strMediaFormat;


    OaSdpDescrClean(pOaStream->pMediaDescrLocal);
    pOaStream->bWasClosed = RV_FALSE;
    OaStreamChangeState(pOaStream, RVOA_STREAM_STATE_REMOVED);

    /* Ensure that cleaned descriptor conforms Offer-Answer RFC 3264:
       contains same media type, as the remote descriptor, if exists,
       and at least one media format in m-line */
    /* 1. Check media type */
    eMediaType = rvSdpMediaDescrGetMediaType((const RvSdpMediaDescr*)pOaStream->pMediaDescrLocal);
    if (eMediaType == RV_SDPMEDIATYPE_UNKNOWN  &&  pOaStream->pMediaDescrRemote != NULL)
    {
        strMediaType = rvSdpMediaDescrGetMediaTypeStr(pOaStream->pMediaDescrRemote);
        srv = rvSdpMediaDescrSetMediaTypeStr(pOaStream->pMediaDescrLocal, strMediaType);
        if (srv != RV_SDPSTATUS_OK)
        {
            RvLogError(pOaStream->pSession->pMgr->pLogSource,
                (pOaStream->pSession->pMgr->pLogSource,
                "OaStreamReset(pOaStream=%p) - failed to copy Media Type %s(srv=%s)",
                pOaStream, strMediaType, OaUtilsConvertEnum2StrSdpStatus(srv)));
        }
    }
    /* 2. Check existence of media formats in m-line */
    numOfFormats = rvSdpMediaDescrGetNumOfFormats(pOaStream->pMediaDescrLocal);
    if (numOfFormats == 0  &&  pOaStream->pMediaDescrRemote != NULL)
    {
        strMediaFormat = rvSdpMediaDescrGetFormat(
                           (const RvSdpMediaDescr*)pOaStream->pMediaDescrRemote,
                           0 /* Use the first format */);
        srv = rvSdpMediaDescrAddFormat(pOaStream->pMediaDescrLocal, strMediaFormat);
        if (RV_SDPSTATUS_OK != srv)
        {
            RvLogError(pOaStream->pSession->pMgr->pLogSource,
                (pOaStream->pSession->pMgr->pLogSource,
                "OaStreamReset(pOaStream=%p) - failed to copy media Format %s(srv=%s)",
                pOaStream, strMediaFormat, OaUtilsConvertEnum2StrSdpStatus(srv)));
        }
    }
}

/******************************************************************************
 * OaStreamChangeState
 * ----------------------------------------------------------------------------
 * General:
 *  Sets new state into the Stream object.
 *
 * Arguments:
 * Input:  pOaStream - pointer to the Stream object.
 *         eState    - new state.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaStreamChangeState(
                            IN OaStream*        pOaStream,
                            IN RvOaStreamState  eState)
{
    RvLogInfo(pOaStream->pSession->pMgr->pLogSource,
        (pOaStream->pSession->pMgr->pLogSource,
        "OaStreamChangeState - STATUS(pOaStream=%p): %s -> %s", pOaStream,
        OaUtilsConvertEnum2StrStreamState(pOaStream->eState),
        OaUtilsConvertEnum2StrStreamState(eState)));
    pOaStream->eState = eState;
}

/*---------------------------------------------------------------------------*/
/*                              STATIC FUNCTIONS                             */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * CheckHoldResume
 * ----------------------------------------------------------------------------
 * General:
 *  Deduces HELD/RESUMED state of stream based on connection mode parameter,
 *  currently received from remote end, and connection mode parameter, received
 *  from remote end during previous OFFER-ANSWER transaction.
 *
 * Arguments:
 * Input:  pOaStream     - pointer to the Stream object.
 *         eConnModePrev - previously received mode of connection.
 *         eConnModeCurr - currently received mode of connection.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV CheckHoldResume(
                                  IN OaStream* pOaStream,
                                  IN RvSdpConnectionMode eConnModePrev,
                                  IN RvSdpConnectionMode eConnModeCurr)
{
    if ((RV_SDPCONNECTMODE_SENDRECV == eConnModePrev &&
         RV_SDPCONNECTMODE_SENDONLY == eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_NOTSET   == eConnModePrev &&
         RV_SDPCONNECTMODE_SENDONLY == eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_RECVONLY == eConnModePrev &&
         RV_SDPCONNECTMODE_INACTIVE == eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_SENDRECV == eConnModePrev &&
         RV_SDPCONNECTMODE_INACTIVE == eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_NOTSET   == eConnModePrev &&
         RV_SDPCONNECTMODE_INACTIVE == eConnModeCurr)
       )
    {
        OaStreamChangeState(pOaStream, RVOA_STREAM_STATE_HELD);
    }
    else
    if ((RV_SDPCONNECTMODE_SENDONLY==eConnModePrev &&
         RV_SDPCONNECTMODE_SENDRECV==eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_SENDONLY==eConnModePrev &&
         RV_SDPCONNECTMODE_RECVONLY==eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_SENDONLY==eConnModePrev &&
         RV_SDPCONNECTMODE_NOTSET==eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_INACTIVE==eConnModePrev &&
         RV_SDPCONNECTMODE_RECVONLY==eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_INACTIVE==eConnModePrev &&
         RV_SDPCONNECTMODE_SENDRECV==eConnModeCurr)
        ||
        (RV_SDPCONNECTMODE_INACTIVE==eConnModePrev &&
         RV_SDPCONNECTMODE_NOTSET==eConnModeCurr)
       )
    {
        OaStreamChangeState(pOaStream, RVOA_STREAM_STATE_RESUMED);
    }
}

/******************************************************************************
 * CheckRemoteAddress
 * ----------------------------------------------------------------------------
 * General:
 *  Deduces if the address, used by the remote address for media reception,
 *  was modified. If it did, set the bAddressWasModified flag to RV_TRUE.
 *
 * Arguments:
 * Input:  pOaStream                 - pointer to the Stream object.
 *         pSdpMsgRemoteBefore       - previously received SDP message.
 *         pMediaDescrRemoteBefore   - media descriptor from previously received
 *                                     message that describes the Stream.
 *         pSdpMsgRemoteModified     - currently received SDP message.
 *         pMediaDescrRemoteModified - media descriptor from currently received
 *                                     message that describes the Stream.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV CheckRemoteAddress(
                                IN OaStream*        pOaStream,
                                IN RvSdpMsg*        pSdpMsgRemoteBefore,
                                IN RvSdpMediaDescr* pMediaDescrRemoteBefore,
                                IN RvSdpMsg*        pSdpMsgRemoteModified,
                                IN RvSdpMediaDescr* pMediaDescrRemoteModified)
{
    RvSdpConnection*  pSdpConnBefore;
    RvSdpConnection*  pSdpConnModified;
    const char*       strAddressBefore;
    const char*       strAddressModified;

    /* Get previous address */
    strAddressBefore = NULL;
    pSdpConnBefore = rvSdpMediaDescrGetConnection(pMediaDescrRemoteBefore);
    if (NULL == pSdpConnBefore)
    {
        pSdpConnBefore = rvSdpMediaDescrGetConnection(pSdpMsgRemoteBefore);
        if (NULL != pSdpConnBefore)
        {
            strAddressBefore = rvSdpConnectionGetAddress(pSdpConnBefore);
        }
    }

    /* Get current address */
    strAddressModified = NULL;
    pSdpConnModified = rvSdpMediaDescrGetConnection(pMediaDescrRemoteModified);
    if (NULL == pSdpConnModified)
    {
        pSdpConnModified = rvSdpMediaDescrGetConnection(pSdpMsgRemoteModified);
        if (NULL != pSdpConnModified)
        {
            strAddressModified = rvSdpConnectionGetAddress(pSdpConnModified);
        }
    }

    if (NULL == strAddressBefore  &&  NULL == strAddressModified)
    {
        return;
    }
    else
    if (NULL != strAddressBefore  &&  NULL != strAddressModified)
    {
        if (0 != strcmp(strAddressBefore, strAddressModified))
        {
            pOaStream->bAddressWasModified = RV_TRUE;
        }
    }
    else
    {
        pOaStream->bAddressWasModified = RV_TRUE;
    }

    if (RV_TRUE == pOaStream->bAddressWasModified)
    {
        RvLogDebug(pOaStream->pSession->pMgr->pLogSource,
            (pOaStream->pSession->pMgr->pLogSource,
            "CheckRemoteAddress - STATUS(pOaStream=%p): remote address was modified",
            pOaStream));
    }

    return;
}


/*nl for linux */

