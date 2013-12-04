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
 *                              <OaSdpMsg.h>
 *
 * The OaSdpMsg.h file defines functions that manipulate objects, defined in
 * RADVISION SDP Stack, using the RADVISION SDP Stack API.
 * These functions form layer between Offer-Answer module and SDP Stack.
 *
 * The functions can be divided in following groups:
 *  1. Functions that operates on SDP Message object
 *  2. Functions that operates on Media Descriptor object
 *
 * Each group contains functions for creation and destruction of correspondent
 * object and for inspection & modifying the object parameters.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

#ifndef _OA_SDPMSG_H
#define _OA_SDPMSG_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                              TYPE DEFINITIONS                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaSdpMsgSetDefaultData
 * ----------------------------------------------------------------------------
 * General:
 *  Set parameters into the message that are mandatory in accordance to RFC.
 *  Following parameters are mandatory: t-line ("t=0 0"), s-line (s=-), o-line.
 *  In addition, sets the default values for username, Session ID and Session
 *  Description Version, if they were not set yet.
 *  Username is "RADVISION". Session ID is pSdpMsg, Version is pSdpMsg.
 *
 * Arguments:
 * Input:  pSdpMsg    - pointer to the Message object.
 *         pLogSource - log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpMsgSetDefaultData(
                            IN RvSdpMsg*    pSdpMsg,
                            IN RvLogSource* pLogSource);

/******************************************************************************
 * OaSdpMsgIncrementVersion
 * ----------------------------------------------------------------------------
 * General:
 *  Increments version of Session Description.
 *  The version is a part of o-line.
 *  It should be incremented each time the modifies OFFER/ANSWER is sent.
 *
 * Arguments:
 * Input:  pSdpMsg    - pointer to the Message object.
 *         pLogSource - log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpMsgIncrementVersion(
                            IN RvSdpMsg*    pSdpMsg,
                            IN RvLogSource* pLogSource);

/******************************************************************************
 * OaSdpMsgCopyMediaDescriptors
 * ----------------------------------------------------------------------------
 * General:
 *  Copies descriptions of media (m-line blocks).
 *
 * Arguments:
 * Input:  pSdpMsgDst - message object where the media descriptors is copied.
 *         pSdpMsgSrc - message object from which media descriptors is taken.
 *         pLogSource - log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpMsgCopyMediaDescriptors(
                            IN RvSdpMsg*    pSdpMsgDst,
                            IN RvSdpMsg*    pSdpMsgSrc,
                            IN RvLogSource* pLogSource);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * OaSdpMsgLogCapabilities
 * ----------------------------------------------------------------------------
 * General:
 *  Prints media formats, found in message into log.
 *
 * Arguments:
 * Input:  pSdpMsg    - message object containing capabilities.
 *         pLogSource - log source to be used for logging.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaSdpMsgLogCapabilities(
                            IN RvSdpMsg*     pSdpMsg,
                            IN RvLogSource*  pLogSource);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * OaSdpDescrDeriveFinalFormatParams
 * ----------------------------------------------------------------------------
 * General:
 *  Given parameters of media format that are acceptable by local site and
 *  parameters that are acceptable by remote side, derives parameters,
 *  acceptable by both sides and sets them into media descriptor.
 *
 * Arguments:
 * Input:  pMediaDescrOffer      - media parameters acceptable by remote side.
 *         pMediaDescrCaps       - media parameters acceptable by local side.
 *         strMediaFormat        - static payload type.
 *         strCodecName          - codec name, if dynamic payload type is used.
 *         pLogSource            - log source to be used for logging.
 * Output: pMediaDescrAnswer     - media parameters acceptable by both sides.
 *         pFormatInfo           - various info about media format, collected
 *                                 while deriving format parameters.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpDescrDeriveFinalFormatParams(
                            IN    RvSdpMediaDescr*       pMediaDescrOffer,
                            IN    RvSdpMediaDescr*       pMediaDescrCaps,
                            IN    const RvChar*          strMediaFormat,
                            IN    const RvChar*          strCodecName,
                            IN    RvLogSource*           pLogSource,
                            OUT   RvSdpMediaDescr*       pMediaDescrAnswer,
                            OUT   RvOaMediaFormatInfo*   pFormatInfo);

/******************************************************************************
 * OaSdpDescrModify
 * ----------------------------------------------------------------------------
 * General:
 *  Changes data in media description in order to form ANSWER to the provided
 *  media description, which represents OFFER.
 *  This data includes direction of media transmission.
 *
 * Arguments:
 * Input:  pSdpMediaDescr      - ANSWER media description.
 *         pSdpMediaDescrOffer - OFFER media description.
 *         pLogSource          - log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpDescrModify(
                            IN  RvSdpMediaDescr*   pSdpMediaDescr,
                            IN  RvSdpMediaDescr*   pSdpMediaDescrOffer,
                            IN  RvLogSource*       pLogSource);

/******************************************************************************
 * OaSdpDescrClean
 * ----------------------------------------------------------------------------
 * General:
 *  Removes data from Media Descriptor, that is requested to be removed by
 *  Stream removal procedure described in RFC 3264:
 *  removes any content from stream description and resets port to zero.
 *
 * Arguments:
 * Input:  pSdpMediaDescr - media description.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaSdpDescrClean(IN  RvSdpMediaDescr*   pSdpMediaDescr);

/******************************************************************************
 * OaSdpDescrCheckPayload13
 * ----------------------------------------------------------------------------
 * General:
 *  Checks if the media descriptor includes at least one media format,
 *  that can be used in conjunction with Dynamic Payload of type 13
 *  (Comfort Noise, RFC 3389). If it doesn't, remove parameters of payload 13 
 *  from media descriptor.
 *  Formats, that can be used in conjunction with Dynamic Payload of type 13:
 *      G.711(static type 0), G.726(2, dynamic), G.727(d), G.728(15), G.722(9)
 * Arguments:
 * Input:  pMediaDescr - media description.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSdpDescrCheckPayload13(IN RvSdpMediaDescr*  pMediaDescr);

/******************************************************************************
 * OaSdpDescrRemoveFormat
 * ----------------------------------------------------------------------------
 * General:
 *  Remove any data from Media Descriptor that is related to media format.
 *  Media format can be represented by strFormatName parameter for
 *  static payload types, and it can be represented by strCodecName parameter
 *  for dynamic payload types.
 *  Note strFormatName has higly priority than strCodecName.
 *
 * Arguments:
 * Input:  pMediaDescr   - media description.
 *         strFormatName - format to be removed from descriptor in string form.
 *         format        - format (payload type) to be removed from descriptor.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaSdpDescrRemoveFormat(
                                IN RvSdpMediaDescr* pMediaDescr,
                                IN const RvChar*    strFormatName,
                                IN RvInt32          format);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _OA_SDPMSG_H */


/*nl for linux */

