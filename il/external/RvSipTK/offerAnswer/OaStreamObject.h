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
 *                              <OaStreamObject.h>
 *
 * The OaStreamObject.h file defines interface for Stream object.
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

#ifndef _OA_STREAM_OBJECT_H
#define _OA_STREAM_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "RvOaTypes.h"
#include "OaSessionObject.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * OaStream
 * ----------------------------------------------------------------------------
 * OaStream structure represents Stream object.
 * Description of media, transferred over the stream, is contained in SDP
 * messages. The description can be accessed via Media Descriptors objects,
 * that are part of RADVISION SDP Stack Message object.
 *****************************************************************************/
typedef struct
{
    OaSession*           pSession;      /* Session that created this object */
    RvOaAppStreamHandle  hAppHandle;
        /* Reference to the Application object, associated with this stream. */

    RvSdpMediaDescr* pMediaDescrLocal;  /* Media Descriptor in local message*/
    RvSdpMediaDescr* pMediaDescrRemote; /* Media Descriptor in remote message*/

    RvOaStreamState eState;

    RvBool bNewOffered;
        /* If RV_TRUE, the stream was added by the remote side.
           This flag is set only for newly offered streams.
           It is reset on reception/generation of stream modification OFFER. */

    RvBool bWasModified;
        /* If RV_TRUE, some parameters of stream were modified by other side.
           This flag doesn't take in account HOLD/RESUME parameters. */

    RvBool bWasClosed;
        /* If RV_TRUE, stream was closed / rejected by the remotre side.
           This flag is reset to RV_FALSE when the stream is closed locally. */

    RvBool bAddressWasModified;
        /* If RV_TRUE, address for media reception was modified by the remote
           side. The address can be contained in session level c-line or in
           media level c-line.
           The address modification in media level c-line sets also
           the bWasModified flag to RV_TRUE. */

    RvSdpConnectionMode eConnModeBeforeHold;
        /* Connection mode that was found in the Stream local Media Descriptor.
           The connection mode may be changed by RvOaSessionHold API function.
           In this case the RvOaSessionResume API function will revert
           the connection mode to the value, stored in this field. */

} OaStream;

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
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
                                        IN RvOaStreamCfg* pStreamCfg);

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
                                        IN RvSdpMediaDescr* pMediaDescr);
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
void RVCALLCONV OaStreamDestruct(IN OaStream*  pOaStream);

/******************************************************************************
 * OaStreamModify
 * ----------------------------------------------------------------------------
 * General:
 *  Modifies Stream object in accordance to data received in modifying OFFER.
 *  Takes a care of HOLD / RESUME, updates parameters of Stream status,
 *  updates local media descriptor based on remote modified descriptor and
 *  remote descriptor before modification, etc.
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
                                OUT RvBool*          pbSessionWasModified);

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
 * Return Value: RV_OK on success, error - otherwise.
 *****************************************************************************/
void RVCALLCONV OaStreamUpdateWithRemoteMsg(
                                IN  OaStream*        pOaStream,
                                IN  RvSdpMediaDescr* pMediaDescrRemote);

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
RvStatus RVCALLCONV OaStreamHold(IN OaStream*  pOaStream);

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
RvStatus RVCALLCONV OaStreamResume(IN OaStream*  pOaStream);

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
void RVCALLCONV OaStreamReset(IN OaStream*  pOaStream);

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
                            IN RvOaStreamState  eState);

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef _OA_STREAM_OBJECT_H */
