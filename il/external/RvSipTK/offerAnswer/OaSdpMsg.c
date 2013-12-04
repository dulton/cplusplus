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
 *                              <OaSdpMsg.c>
 *
 * The OaSdpMsg.c file implements functions that manipulate objects, defined in
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

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rvlog.h"
#include "rvstrutils.h"
#include "rv64ascii.h"

#include "RvOaTypes.h"
#include "OaUtils.h"
#include "OaSdpMsg.h"
#include "rvsdpcodecs.h"

/*---------------------------------------------------------------------------*/
/*                         TYPE DEFINITION                                   */
/*---------------------------------------------------------------------------*/
#define RVOA_FMTP_PARAM_MAX_LEN 256
#define RVOA_UNDEFINED_MIN      0x0
#define RVOA_UNDEFINED_MAX      0x0fffffff

/******************************************************************************
 * Type: CodecParamPtrs
 * ----------------------------------------------------------------------------
 * Auxiliary structure that unites pointers to the fields of the SDP Media
 * Descriptor object, that are used while processing codecs.
 *****************************************************************************/
typedef struct {
    RvUint32        format;        /* Payload type, defined for codec */
    RvChar*         strFormatName; /* Payload type as a string */
    RvUint32        formatIdx;     /* 0-based index of payload type in m-line*/
    RvSdpRtpMap*    pRtpmap;
    RvSdpAttribute* pFmtp;
    RvSdpListIter   iteratorRtpmap;
    RvSdpListIter   iteratorFmtp;
} CodecParamPtrs;

/******************************************************************************
 * Type: DeriveParameter
 * ----------------------------------------------------------------------------
 * Auxiliary structure that unites pointers to the same attribute, located
 * in offer, local and answer Media Descriptor objects.
 * This structure is used by the functions, that derive codec parameters,
 * acceptable by both sides of SDP session.
 *****************************************************************************/
typedef struct {
    RvChar*         strOffer;
    RvSdpAttribute* pAttrOffer;
    RvChar*         strLocal;
    RvSdpAttribute* pAttrLocal;
    RvChar*         strAnswer;
    RvSdpAttribute* pAttrAnswer;
} DeriveParameter;

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTION DECLARATION                       */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void RVCALLCONV LogCodecParams(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  int              payloadType,
                            IN  const char*      strPayloadType,
                            IN  RvUint32         buffLen,
                            OUT RvChar*          buffer);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static void RVCALLCONV GetCodecParamPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  const RvChar*    strFormatName,
                            IN  const RvChar*    strCodecName,
                            OUT CodecParamPtrs*  pCodecParams);

static RvStatus RVCALLCONV CopyCodecParams(
                            IN  RvSdpMediaDescr* pMediaDescrSrc,
                            IN  RvUint32         format,
                            IN  RvChar*          strFormatName,
                            IN  CodecParamPtrs*  pCodecParams,
                            IN  RvLogSource*     pLogSource,
                            OUT RvSdpMediaDescr* pMediaDescrDst);

static RvStatus RVCALLCONV CopyFmtpParam(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  CodecParamPtrs*  pCodecParams,
                            IN  RvSdpMediaDescr* pMediaDescrSrc,
                            IN  CodecParamPtrs*  pCodecParamsSrc,
                            IN  RvSdpAttribute*  pFmtpCaps,
                            IN  const char*      strParamName,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DeriveBasicParamsG7XX(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DeriveBasicParamsH26X(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DeriveParamsH261(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DeriveParamsH263(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DerivePtime(
                            IN  DeriveParameter* pPtime,
                            OUT RvChar**         pstrPtimeResult);

static RvBool RVCALLCONV DeriveFramerate(
                            IN  DeriveParameter* pFramerate,
                            OUT RvChar**         pstrFramerateResult);

static RvBool RVCALLCONV DeriveSilenceSupp(
                            IN  DeriveParameter*  pSilenceSupp,
                            OUT RvChar**          pstrSilenceSuppResult);

static RvBool RVCALLCONV DeriveBandwidth(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  RvLogSource*     pLogSource);

static RvBool RVCALLCONV DeriveMaxBitRate(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource);

static RvStatus RVCALLCONV SetMediaDescrFormat(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  RvUint32         format,
                            IN  RvChar*          strFormatName,
                            IN  RvSdpRtpMap*     pRtpmap,
                            IN  RvLogSource*     pLogSource);

static RvSdpAttribute* RVCALLCONV SetMediaDescrFmtp(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  RvUint32         format,
                            IN  RvSdpAttribute*  pFmtpSrc,
                            IN  RvLogSource*     pLogSource);

static RvSdpStatus RVCALLCONV SetMediaDescrAttr(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  RvChar*          strAttrName,
                            IN  RvChar*          strAttrVal,
                            IN  RvSdpAttribute*  pAttr);

static void RVCALLCONV GetBasicParamG7XXPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            OUT RvChar**         pstrPtime,
                            OUT RvSdpAttribute** ppAttrPtime,
                            OUT RvChar**         pstrSilenceSupp,
                            OUT RvSdpAttribute** ppAttrSilenceSupp);

static void RVCALLCONV GetBasicParamH26XPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            OUT RvChar**         pstrFramerate,
                            OUT RvSdpAttribute** ppAttrFramerate);

static RvBool RVCALLCONV FmtpParamDeriveMinimalValue(
                            IN  RvSdpMediaDescr*  pMediaDescrOffer,
                            IN  CodecParamPtrs*   pCodecParamsOffer,
                            IN  RvSdpMediaDescr*  pMediaDescrCaps,
                            IN  CodecParamPtrs*   pCodecParamsCaps,
                            IN  RvSdpMediaDescr*  pMediaDescrAnswer,
                            IN  CodecParamPtrs*   pCodecParamsAnswer,
                            IN  const char*       strParamName,
                            IN  RvLogSource*      pLogSource,
                            OUT RvSdpMediaDescr** ppMediaDescr,
                            OUT CodecParamPtrs**  ppCodecParams);

static RvBool RVCALLCONV FmtpParamDeriveMaximalValue(
                            IN  RvSdpMediaDescr*  pMediaDescrOffer,
                            IN  CodecParamPtrs*   pCodecParamsOffer,
                            IN  RvSdpMediaDescr*  pMediaDescrCaps,
                            IN  CodecParamPtrs*   pCodecParamsCaps,
                            IN  RvSdpMediaDescr*  pMediaDescrAnswer,
                            IN  CodecParamPtrs*   pCodecParamsAnswer,
                            IN  const char*       strParamName,
                            IN  RvLogSource*      pLogSource,
                            OUT RvSdpMediaDescr** ppMediaDescr,
                            OUT CodecParamPtrs**  ppCodecParams);

static RvSdpStatus RVCALLCONV CheckQcifCifOrder(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer);

/*---------------------------------------------------------------------------*/
/*                          FUNCTION IMPLEMENTATIONS                         */
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
                                        IN RvSdpMsg*     pSdpMsg,
                                        IN RvLogSource*  pLogSource)
{
    const char*        strLine;
    RvSdpOrigin*       pOrigin;
    RvSdpSessionTime*  pTime;
    RvSdpListIter      timeIterator;
    RvChar             strSessionId[RV_64TOASCII_BUFSIZE];
	RvUint64           sessionId;
    RvSdpStatus        srv;
    RvStatus           rv;
    RvSdpConnection*   pConn;

    sessionId = RvUint64FromRvSize_t((RvSize_t)pSdpMsg);
	Rv64UtoA(sessionId, strSessionId);

    /* Set o-line if it is not set yet */
    pOrigin = rvSdpMsgGetOrigin((const RvSdpMsg*)pSdpMsg);
    if (NULL == pOrigin)
    {
        srv = rvSdpMsgSetOrigin(pSdpMsg, "RADVISION", strSessionId,
                                strSessionId /*version*/,
                                RV_SDPNETTYPE_IN, RV_SDPADDRTYPE_IP4,
                                "0.0.0.0"); /* 0.0.0.0 prevents parser error */
        if (RV_OK != srv)
        {
            rv = OA_SRV2RV(srv);
            RvLogError(pLogSource ,(pLogSource,
                "OaSdpMsgSetDefaultData - failed to set origin(srv=%s)",
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            return rv;
        }
    }
    else
    {
        strLine = rvSdpOriginGetUsername(pOrigin);
        if (NULL == strLine)
        {
            srv = rvSdpOriginSetUsername(pOrigin, "RADVISION");
            if (RV_OK != srv)
            {
                rv = OA_SRV2RV(srv);
                RvLogError(pLogSource ,(pLogSource,
                    "OaSdpMsgSetDefaultData - failed to set username(srv=%s)",
                    OaUtilsConvertEnum2StrSdpStatus(srv)));
                return rv;
            }
        }
        strLine = rvSdpOriginGetVersion(pOrigin);
        if (NULL == strLine)
        {
            srv = rvSdpOriginSetVersion(pOrigin, strSessionId);
            if (RV_OK != srv)
            {
                rv = OA_SRV2RV(srv);
                RvLogError(pLogSource ,(pLogSource,
                    "OaSdpMsgSetDefaultData - failed to set version(srv=%s)",
                    OaUtilsConvertEnum2StrSdpStatus(srv)));
                return rv;
            }
        }
        strLine = rvSdpOriginGetSessionId(pOrigin);
        if (NULL == strLine  ||  0 == strcmp(strLine, "0"))
        {
            srv = rvSdpOriginSetSessionId(pOrigin, strSessionId/*Version*/);
            if (RV_OK != srv)
            {
                rv = OA_SRV2RV(srv);
                RvLogError(pLogSource ,(pLogSource,
                    "OaSdpMsgSetDefaultData - failed to set session id(srv=%s)",
                    OaUtilsConvertEnum2StrSdpStatus(srv)));
                return rv;
            }
        }
    } /* ENDOF: if (NULL == pOrigin) else */

    /* Set s-line if it is not set yet */
    strLine = rvSdpMsgGetSessionName((const RvSdpMsg*)pSdpMsg);
    if (NULL == strLine  ||  strLine[0]=='\0')
    {
        srv = rvSdpMsgSetSessionName(pSdpMsg, "-");
        if (RV_OK != srv)
        {
            rv = OA_SRV2RV(srv);
            RvLogError(pLogSource ,(pLogSource,
                "OaSdpMsgSetDefaultData - failed to set Session Name(srv=%s)",
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            return rv;
        }
    }

    /* Set t-line if it is not set yet */
    pTime = rvSdpMsgGetFirstSessionTime((const RvSdpMsg*)pSdpMsg,&timeIterator);
    if (NULL == pTime)
    {
        pTime = rvSdpMsgAddSessionTime(pSdpMsg, 0, 0);
        if (NULL == pTime)
        {
            RvLogError(pLogSource ,(pLogSource,
                "OaSdpMsgSetDefaultData - failed to set Session Time"));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Set c-line */
    pConn = rvSdpMsgGetConnection(pSdpMsg);
    if (NULL == pConn)
    {
        RvSdpMediaDescr*   pMediaDescr;
        RvSdpListIter      mdIterator;
        RvBool             bNoCline = RV_FALSE;

        /* If at least one media descriptor doesn't contain c-line, build
        zero c-line in order to prevent parser errors */
        pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsg, &mdIterator);
        if (NULL == pMediaDescr)
        {
            bNoCline = RV_TRUE;
        }

        while (NULL != pMediaDescr  &&  RV_FALSE == bNoCline)
        {
            if (NULL == rvSdpMediaDescrGetConnection(pMediaDescr))
            {
                bNoCline = RV_TRUE;
            }
            pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
        }

        if (RV_TRUE == bNoCline)
        {
            srv = rvSdpMsgSetConnection(pSdpMsg,
                    RV_SDPNETTYPE_IN, RV_SDPADDRTYPE_IP4, "0.0.0.0");
            if (RV_OK != srv)
            {
                rv = OA_SRV2RV(srv);
                RvLogError(pLogSource ,(pLogSource,
                    "OaSdpMsgSetDefaultData - failed to set connection(srv=%s)",
                    OaUtilsConvertEnum2StrSdpStatus(srv)));
                return rv;
            }
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    return RV_OK;
}

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
                            IN RvLogSource* pLogSource)
{
    const char*   strFoundVersion;
    RvSdpOrigin*  pOrigin;
    RvSdpStatus   srv;
    RvStatus      rv;
    RvChar        strNewVersion[RV_64TOASCII_BUFSIZE];

    /* Set o-line if it is not set yet */
    pOrigin = rvSdpMsgGetOrigin((const RvSdpMsg*)pSdpMsg);
    if (NULL == pOrigin)
    {
        RvLogError(pLogSource ,(pLogSource,
            "OaSdpMsgIncrementVersion - there is no origin"));
        return RV_ERROR_NOT_FOUND;
    }

    strFoundVersion = rvSdpOriginGetVersion(pOrigin);

    /* If no version was set yet, use the default value */
    if (NULL == strFoundVersion)
    {
		RvUint64 version;
		version = RvUint64FromRvSize_t((RvSize_t)pSdpMsg);
		Rv64UtoA(version, strNewVersion);
    }
    else /* Increment the found value */
    {
        RvInt64 version;
        RvAto64((RvChar*)strFoundVersion, &version);
        version = RvInt64Add(version, RvInt64Const2(1));

        /* Don't enable zero version
           since this value is reserved for new sessions only. */
        if (RvInt64IsEqual(version, RvInt64Const(0,0,0)))
        {
            version = RvInt64Add(version, RvInt64Const2(1));
        }

        Rv64toA(version, strNewVersion);
    }

    srv = rvSdpOriginSetVersion(pOrigin, strNewVersion);
    if (RV_OK != srv)
    {
        rv = OA_SRV2RV(srv);
        RvLogError(pLogSource ,(pLogSource,
            "OaSdpMsgIncrementVersion - failed to set version %s (srv=%s)",
            strNewVersion, OaUtilsConvertEnum2StrSdpStatus(srv)));
        return rv;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return RV_OK;
}

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
                            IN RvLogSource* pLogSource)
{
    RvSdpStatus srv;
    RvSdpListIter    mdIterator;
    RvSdpMediaDescr* pMediaDescr;

    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsgSrc, &mdIterator);
    while (NULL != pMediaDescr)
    {
        srv = rvSdpMsgInsertMediaDescr(pSdpMsgDst, pMediaDescr);
        if (RV_OK != srv)
        {
            RvStatus rv = OA_SRV2RV(srv);
            RvLogError(pLogSource ,(pLogSource,
                "OaSdpMsgCopyMediaDescriptors - failed to insert media descriptor(srv=%s)",
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            return rv;
        }

        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    return RV_OK;
}

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
                            IN RvLogSource*  pLogSource)
{
    RvSdpMediaDescr* pMediaDescr;
    RvSdpListIter    mdIterator;
    const char*      strMediaType;
    const char*      strMediaFormat;
    RvChar           strLogOutput[RVOA_MAX_LOG_LINE];
    RvUint32         numOfMediaFormats, i;
    int              payloadType;
    RvUint32         lenOfOutput;
    RvChar*          pStrFormatName;

    /* Set Media Descriptions into the Session based on received descriptions*/
    memset(&mdIterator, 0, sizeof(RvSdpListIter));
    pMediaDescr = rvSdpMsgGetFirstMediaDescr(pSdpMsg, &mdIterator);
    while (NULL != pMediaDescr)
    {
        strLogOutput[0] = '\0';

        /* Print prefix, e.g. "audio:" */
        strMediaType = rvSdpMediaDescrGetMediaTypeStr(pMediaDescr);
        strcpy(strLogOutput, strMediaType);
        strcat(strLogOutput, ":");
        pStrFormatName = strLogOutput + strlen(strLogOutput);

        /* Print formats, e.g. of output to log - "audio: 0 97(G729) 8" */
        numOfMediaFormats = (RvUint32)rvSdpMediaDescrGetNumOfFormats(pMediaDescr);
        for (i=0; i<numOfMediaFormats; i++)
        {
            /* Add format name. Result: "audio:97" */
            strMediaFormat = rvSdpMediaDescrGetFormat(pMediaDescr, i);
            strcpy(pStrFormatName, strMediaFormat);

            /* If the format represents dynamic payload type, add also
            codec parameters from RTPMAP and FMTP attributes.
            Result: "audio: 97(G729/20000)" */
            payloadType = (RvUint32)rvSdpMediaDescrGetPayload(pMediaDescr, i);
            lenOfOutput = (RvUint32)strlen(strLogOutput);
            LogCodecParams(pMediaDescr, payloadType, strMediaFormat,
                (RVOA_MAX_LOG_LINE-lenOfOutput), &strLogOutput[lenOfOutput]);

            /* Print the line into log */
            RvLogDebug(pLogSource,(pLogSource,"%s", strLogOutput));
        }
        pMediaDescr = rvSdpMsgGetNextMediaDescr(&mdIterator);
    }
    
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * OaSdpDescrDeriveFinalFormatParams
 * ----------------------------------------------------------------------------
 * General:
 *  Given media parameters for codec that are acceptable by local side and
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
                            OUT   RvOaMediaFormatInfo*   pFormatInfo)
{
    RvStatus        rv = RV_OK;
    CodecParamPtrs  codecParamsOffer;
    CodecParamPtrs  codecParamsCaps;
    CodecParamPtrs  codecParamsAnswer;
    RvBool          bDerivationSuccess;
    RvOaCodec       eCodec;

    /* Prevent Lint warning */
    if (pMediaDescrOffer == NULL  ||  pMediaDescrCaps == NULL ||
        pMediaDescrAnswer == NULL ||  pFormatInfo == NULL)
    {
        RvLogError(pLogSource ,(pLogSource,
            "OaSdpDescrDeriveFinalFormatParams: bad params: pMediaDescrOffer=%p,pMediaDescrCaps=%p,pMediaDescrAnswer=%p,pFormatInfo=%p",
            pMediaDescrOffer, pMediaDescrCaps, pMediaDescrAnswer,pFormatInfo));
        return RV_ERROR_BADPARAM;
    }

    /* Find out the Codec enumeration value */
    if (NULL != strMediaFormat)
    {
        eCodec = OaUtilsConvertStr2EnumCodec((RvChar*)strMediaFormat);
    }
    else
    {
        eCodec = OaUtilsConvertStr2EnumCodec((RvChar*)strCodecName);
    }

    /* Initialize auxiliary structures */
    memset(&codecParamsCaps, 0, sizeof(CodecParamPtrs));
    GetCodecParamPtrs(pMediaDescrCaps, strMediaFormat, strCodecName, &codecParamsCaps);
    memset(&codecParamsOffer, 0, sizeof(CodecParamPtrs));
    GetCodecParamPtrs(pMediaDescrOffer, strMediaFormat, strCodecName, &codecParamsOffer);
    if (eCodec == RVOA_CODEC_H261  ||  eCodec == RVOA_CODEC_H263_2190)
    {
        memset(&codecParamsAnswer, 0, sizeof(CodecParamPtrs));
        GetCodecParamPtrs(pMediaDescrAnswer, strMediaFormat, strCodecName,
                          &codecParamsAnswer);
    }

    /* Derive format parameter in accordance to the codec */
    switch (eCodec)
    {
        case RVOA_CODEC_G711:
        case RVOA_CODEC_G722:
        case RVOA_CODEC_G729:
        case RVOA_CODEC_G7231:
            {
                bDerivationSuccess = DeriveBasicParamsG7XX(
                                            pMediaDescrOffer, pMediaDescrCaps,
                                            pMediaDescrAnswer, pLogSource);
                if (RV_TRUE == bDerivationSuccess)
                {
                    /* Add format and RTPMAP attribute if exist */
                    rv = SetMediaDescrFormat(pMediaDescrAnswer,
                                        codecParamsOffer.format,
                                        codecParamsOffer.strFormatName,
                                        codecParamsCaps.pRtpmap, pLogSource);
                    if (RV_OK != rv)
                    {
                        RvLogError(pLogSource ,(pLogSource,
                            "OaSdpDescrDeriveFinalFormatParams: failed to set format %s (codec %s) into ANSWER %p",
                            codecParamsCaps.strFormatName,
                            (strCodecName==NULL ? "" : strCodecName),
                            pMediaDescrAnswer));
                        return rv;
                    }
                    /* Add FMTP attribute if exist */
                    if (NULL != codecParamsCaps.pFmtp)
                    {
                        codecParamsAnswer.pFmtp = SetMediaDescrFmtp(
                                    pMediaDescrAnswer, codecParamsOffer.format,
                                    codecParamsCaps.pFmtp, pLogSource);
                        if (NULL == codecParamsAnswer.pFmtp)
                        {
                            RvLogError(pLogSource ,(pLogSource,
                                "OaSdpDescrDeriveFinalFormatParams - failed to add FMTP to descriptor %p",
                                pMediaDescrAnswer));
                            /* don't return */
                        }
                    }
                }
            }
            break;

        case RVOA_CODEC_H261:
            {
                bDerivationSuccess = DeriveParamsH261(
                                        pMediaDescrOffer,  &codecParamsOffer,
                                        pMediaDescrCaps,   &codecParamsCaps,
                                        pMediaDescrAnswer, &codecParamsAnswer,
                                        pLogSource);
            }
            break;

        case RVOA_CODEC_H263_2190:
            {
                bDerivationSuccess = DeriveParamsH263(
                                        pMediaDescrOffer,  &codecParamsOffer,
                                        pMediaDescrCaps,   &codecParamsCaps,
                                        pMediaDescrAnswer, &codecParamsAnswer,
                                        pLogSource);
            }
            break;

        default:
            /* For not supported codecs just copy the format parameters from
               Capabilities into ANSWER. */
            {
                rv = CopyCodecParams(pMediaDescrCaps, codecParamsOffer.format,
                                     codecParamsOffer.strFormatName,
                                     &codecParamsCaps, pLogSource,
                                     pMediaDescrAnswer);
                if (RV_OK != rv)
                {
                    RvLogError(pLogSource ,(pLogSource,
                        "OaSdpDescrDeriveFinalFormatParams: failed to copy format %s (codec %s) parameters into ANSWER, mediaDesc=0x%p",
                        codecParamsCaps.strFormatName,
                        (strCodecName==NULL ? "" : strCodecName),
                        pMediaDescrAnswer));
                    return rv;
                }
                bDerivationSuccess = RV_TRUE;
            }
    }

    RvLogDebug(pLogSource ,(pLogSource,
        "OaSdpDescrDeriveFinalFormatParams: format %s (codec %s) was handled (bDerivationSuccess=%d)",
        codecParamsCaps.strFormatName, ( (strCodecName==NULL)? "" : strCodecName),
        bDerivationSuccess));

    pFormatInfo->formatOffer = codecParamsOffer.format;
    pFormatInfo->formatIdxOffer = codecParamsOffer.formatIdx;
    pFormatInfo->formatCaps = codecParamsCaps.format;
    pFormatInfo->formatIdxCaps = codecParamsCaps.formatIdx;
    pFormatInfo->eCodec = eCodec;

    return RV_OK;
}

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
                                IN  RvLogSource*       pLogSource)
{
    RvSdpStatus         srv;
    RvSdpConnectionMode eModeOffer, eMode=RV_SDPCONNECTMODE_NOTSET;

    /* Change direction to opposite */
    eModeOffer = rvSdpMediaDescrGetConnectionMode(
                    (const RvSdpMediaDescr*)pSdpMediaDescrOffer);
    switch (eModeOffer)
    {
        case RV_SDPCONNECTMODE_SENDONLY:eMode=RV_SDPCONNECTMODE_RECVONLY;break;
        case RV_SDPCONNECTMODE_RECVONLY:eMode=RV_SDPCONNECTMODE_SENDONLY;break;
        default:
            eMode = eModeOffer;
            break;

    }

    srv = rvSdpMediaDescrSetConnectionMode(pSdpMediaDescr, eMode);
    if (RV_OK != srv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "OaSdpDescrModify - failed to set direction(srv=%s)",
            OaUtilsConvertEnum2StrSdpStatus(srv)));
        /* don't return */
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    return RV_OK;
}

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
void RVCALLCONV OaSdpDescrClean(IN  RvSdpMediaDescr*  pSdpMediaDescr)
{
    rvSdpMediaDescrSetPort(pSdpMediaDescr, 0);
    /* RFC 3264: The stream description MAY omit all attributes present
                 previously, and MAY list just a single media format.
       => don't clean media formats
    */
    /*
    rvSdpMediaDescrClearFormat(pSdpMediaDescr);
    */
    rvSdpMediaDescrClearRtpMap(pSdpMediaDescr);
    rvSdpMediaDescrClearFmtp(pSdpMediaDescr);
    rvSdpMediaDescrClearBandwidth(pSdpMediaDescr);
    rvSdpMediaDescrClearConnection(pSdpMediaDescr);
    rvSdpMediaDescrClearCrypto(pSdpMediaDescr);
    rvSdpMediaDescrClearKeyMgmt(pSdpMediaDescr);
    rvSdpMediaDescrClearAttr2(pSdpMediaDescr);
}

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
RvStatus RVCALLCONV OaSdpDescrCheckPayload13(IN RvSdpMediaDescr*  pMediaDescr)
{
    RvBool      bWasFound = RV_FALSE;
    RvSize_t    numOfFormats, i;
    const char*        strFormatName;
    const RvSdpRtpMap* pRtpMap;
    RvSdpListIter      rmIterator; /* RTPMAP iterator */

    /* Try to find requested static payload */
    numOfFormats = rvSdpMediaDescrGetNumOfFormats(pMediaDescr);
    for (i=0; i<numOfFormats && RV_FALSE==bWasFound; i++)
    {
        strFormatName = rvSdpMediaDescrGetFormat(
                            (const RvSdpMediaDescr*)pMediaDescr, i);
        if (0==strcmp(strFormatName,"0")  ||  0==strcmp(strFormatName,"2") ||
            0==strcmp(strFormatName,"9")  ||  0==strcmp(strFormatName,"15"))
        {
            bWasFound = RV_TRUE;
        }
    }

    /* Try to find requested dynamic payload */
    if (RV_FALSE == bWasFound)
    {
        pRtpMap = rvSdpMediaDescrGetFirstRtpMap(
                    (RvSdpMediaDescr*)pMediaDescr, &rmIterator);
        while (NULL != pRtpMap  &&  RV_FALSE==bWasFound)
        {
            strFormatName = rvSdpRtpMapGetEncodingName(pRtpMap);
            if (NULL != strstr(strFormatName,"728") ||
                NULL != strstr(strFormatName,"727") ||
                NULL != strstr(strFormatName,"726") ||
                NULL != strstr(strFormatName,"722") ||
                NULL != strstr(strFormatName,"711"))
            {
                bWasFound = RV_TRUE;
            }
            pRtpMap = rvSdpMediaDescrGetNextRtpMap(&rmIterator);
        }
    }

    /* If no compatible codec was found, remove parameters of payload type 13*/
    if (RV_FALSE == bWasFound)
    {
        OaSdpDescrRemoveFormat(pMediaDescr, "13", RVOA_UNDEFINED);
    }

    return RV_OK;
}

/******************************************************************************
 * OaSdpDescrRemoveFormat
 * ----------------------------------------------------------------------------
 * General:
 *  Remove any data from Media Descriptor that is related to media format.
 *  Media format can be represented by string (strFormatName parameter)
 *  or by integer (format parameter).
 *  These representations are intended to be exlusive.
 *  But if both of them are provided, the format value represented by string
 *  has higher priority, than the format, represented by integer.
 *
 * Arguments:
 * Input:  pMediaDescr   - media description.
 *         strFormatName - format to be removed from descriptor in string form.
 *                         It has a higher priority than the format parameter.
 *                         Can be NULL.
 *         format        - format (payload type) to be removed from descriptor.
 *                         Can be RVOA_UNDEFINED.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
void RVCALLCONV OaSdpDescrRemoveFormat(
                                IN RvSdpMediaDescr* pMediaDescr,
                                IN const RvChar*    strFormatName,
                                IN RvInt32          format)
{
    const char*        strCurrFormatName;
    const char*        strCurrFmtpVal;
    const RvSdpRtpMap* pCurrRtpMap;
    RvSdpAttribute*    pCurrFmtp;
    RvSdpListIter      iter; /* iterator to be used with lists */
    RvInt32            formatIdx;
    RvInt32            numOfFormats, i;

    /* Overwrite format with integer of strFormatName, if provided */
    if (NULL != strFormatName)
    {
        format = atoi(strFormatName);
    }

    /* Find index of format to be removed */
    formatIdx = RVOA_UNDEFINED;
    numOfFormats = (RvInt32)rvSdpMediaDescrGetNumOfFormats(pMediaDescr);
    for (i=0; i<numOfFormats; i++)
    {
        strCurrFormatName = rvSdpMediaDescrGetFormat(
                                    (const RvSdpMediaDescr*)pMediaDescr, i);
        if (format == atoi(strCurrFormatName))
        {
            formatIdx = i;
            break;
        }
    }
    if (RVOA_UNDEFINED == formatIdx)
    {
        return;
    }

    /* Remove the format from m-line */
    rvSdpMediaDescrRemoveFormat(pMediaDescr, formatIdx);

    /* Remove the format's RTPMAP line */
    pCurrRtpMap = rvSdpMediaDescrGetFirstRtpMap(pMediaDescr, &iter);
    while (NULL != pCurrRtpMap)
    {
        if (format == rvSdpRtpMapGetPayload(pCurrRtpMap))
        {
            rvSdpMediaDescrRemoveCurrentRtpMap(&iter);
            break;
        }
        pCurrRtpMap = rvSdpMediaDescrGetNextRtpMap(&iter);
    }

    /* Remove the format's FMTP line */
    pCurrFmtp = rvSdpMediaDescrGetFirstFmtp(pMediaDescr, &iter);
    while (NULL != pCurrFmtp)
    {
        strCurrFmtpVal = rvSdpAttributeGetValue(pCurrFmtp);
        if (format ==  strtol(strCurrFmtpVal,NULL/*endchar pointer*/, 10))
        {
            rvSdpMediaDescrRemoveCurrentFmtp(&iter);
            break;
        }
        pCurrFmtp = rvSdpMediaDescrGetNextFmtp(&iter);
    }
}

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTION IMPLEMENTATION                    */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * LogCodecParams
 * ----------------------------------------------------------------------------
 * General:
 *  Extract parameters from RTPMAP and FMTP attributes that describe codec,
 *  which matches requested type of payload, and prints them into the buffer
 *  in following format: "(RTPMAP data, FMTP data)".
 *  Keeps track of buffer overflowing.
 *
 * Arguments:
 * Input:  pSdpMediaDescr - media description.
 *         payloadType    - payload type as integer number.
 *         strPayloadType - payload type in string form.
 *         buffLen        - length of buffer.
 * Output: buffer         - buffer.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV LogCodecParams(
                                IN   RvSdpMediaDescr* pMediaDescr,
                                IN   int              payloadType,
                                IN   const char*      strPayloadType,
                                IN   RvUint32         buffLen,
                                OUT  RvChar*          buffer)
{
    const RvSdpRtpMap*  pRtpmap;
    RvSdpAttribute*     pFmtp;
    RvSdpListIter       iter;
    int                 intParam;
    const RvChar*       strPtr;
    RvChar              strBuf[32];
    RvUint32            len;

    /* Find RTPMAP that matches the requested format */
    pRtpmap = (const RvSdpRtpMap*)
        rvSdpMediaDescrGetFirstRtpMap(pMediaDescr, &iter);
    while (NULL != pRtpmap)
    {
        if (payloadType == rvSdpRtpMapGetPayload(pRtpmap))
        {
            break;
        }
        pRtpmap = rvSdpMediaDescrGetNextRtpMap(&iter);
    }
    /* Sanity check */
    if (NULL == pRtpmap)
    {
        return;
    }

    /* Append open bracket */
    if (buffLen < 2) /* 1 - for bracket, 1 - for '\0' */
    {
        return;
    }
    strcat(buffer, "("); buffLen--;

    /* Append codec name */
    strPtr = rvSdpRtpMapGetEncodingName(pRtpmap);
    len = (RvUint32)strlen(strPtr);
    if (buffLen < len + 1) /* 1 - for '\0' */
    {
        return;
    }
    strcat(buffer, strPtr); buffLen -= len;

    /* Append clock rate */
    intParam = rvSdpRtpMapGetClockRate(pRtpmap);
    if (0 < intParam)
    {
        sprintf(strBuf, "%d", intParam); 
        len = (RvUint32)strlen(strBuf);
        if (buffLen < len + 2)  /* 1 - for delimeter, 1 - for '\0' */
        {
            return;
        }
        strcat(buffer, "/");
        strcat(buffer, strBuf);
        buffLen = buffLen - (len + 1);
    }

    /* Append number of channels */
    intParam = rvSdpRtpMapGetChannels(pRtpmap);
    if (0 < intParam)
    {
        sprintf(strBuf, "%d", intParam); 
        len = (RvUint32)strlen(strBuf);
        if (buffLen < len + 2)  /* 1 - for delimeter, 1 - for '\0' */
        {
            return;
        }
        strcat(buffer, "/");
        strcat(buffer, strBuf);
        buffLen = buffLen - (len + 1);
    }

    /* Append codec parameters */
    strPtr = rvSdpRtpMapGetEncodingParameters(pRtpmap);
    if (NULL != strPtr  &&  0!=strcmp(strPtr,""))
    {
        len = (RvUint32)strlen(strPtr);
        if (buffLen < len + 2)  /* 1 - for delimeter, 1 - for '\0' */
        {
            return;
        }
        strcat(buffer, "/");
        strcat(buffer, strBuf);
        buffLen = buffLen - (len + 1);
    }
    
    /* Get codec parameters from FMTP */
    pFmtp = rvSdpMediaDescrGetFirstFmtp(pMediaDescr, &iter);
    while (NULL != pFmtp)
    {
        strPtr = rvSdpAttributeGetValue(pFmtp);
        if (0 == memcmp(strPtr, strPayloadType, strlen(strPayloadType)))
        {
            strPtr += strlen(strPayloadType);
            len = (RvUint32)strlen(strPtr);
            if (buffLen < len + 7)  /* 6 - for delimeter " fmtp:", 1 - for '\0' */
            {
                return;
            }
            strcat(buffer, " fmtp:");
            strcat(buffer, strPtr);
            buffLen = buffLen - (len + 6);
            break;
        }
        pFmtp = rvSdpMediaDescrGetNextFmtp(&iter);
    }

    /* Add closing bracket */
    if (buffLen < 2)  /* 1 - for delimeter, 1 - for '\0' */
    {
        return;
    }
    strcat(buffer, ")");
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * GetCodecParamPtrs
 * ----------------------------------------------------------------------------
 * General:
 *  Retrieves pointers to Format, RTPMAP and FMTP structures of
 *  Media Descriptor object, provided by SDP Stack.
 *  The retrieved elements contain data that relates to specified Media Format.
 *  The Media Format can be specified by strFormatName for static payload types
 *  and by strCodecName for dynamic payload types.
 *  Update Iterator objects to point to the found structures.
 *
 * Arguments:
 * Input:  pMediaDescr   - Media Descriptor object.
 *         strFormatName - Media format for static payload types.
 *         strCodecName  - Name of codec for dynamic payload types.
 * Output: pFormatIdx    - Index of format in list of formats of m-line.
 *         pCodecParams  - Requested pointers.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV GetCodecParamPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  const RvChar*    strFormatName,
                            IN  const RvChar*    strCodecName,
                            OUT CodecParamPtrs*  pCodecParams)
{
    const char*         strCurrFormatName;
    const char*         strCurrCodecName;
    const char*         strCurrFmtpVal;
    const RvSdpRtpMap*  pCurrRtpMap;
    RvSdpAttribute*     pCurrFmtp;
    RvUint32            numOfFormats, i;

    numOfFormats = (RvUint32)rvSdpMediaDescrGetNumOfFormats(pMediaDescr);

    pCodecParams->strFormatName = NULL;

    /* 1. Find index of format to be removed */
    /* Find it in accordance to format name (static payload type) */
    if (NULL != strFormatName)
    {
        for (i=0; i<numOfFormats; i++)
        {
            strCurrFormatName = rvSdpMediaDescrGetFormat(
                                    (const RvSdpMediaDescr*)pMediaDescr, i);
            if (0==strcmp(strFormatName, strCurrFormatName))
            {
                pCodecParams->formatIdx = i;
                break;
            }
        }
        pCodecParams->strFormatName = (RvChar*)strFormatName;
        pCodecParams->format = atoi(strFormatName);
    }
    /* Find it in accordance to codec name (dynamic payload type) */
    else if (NULL != strCodecName)
    {
        pCurrRtpMap = rvSdpMediaDescrGetFirstRtpMap(
                            pMediaDescr, &pCodecParams->iteratorRtpmap);
        while (NULL != pCurrRtpMap)
        {
            strCurrCodecName = rvSdpRtpMapGetEncodingName(pCurrRtpMap);
            if (RV_TRUE == OaUtilsCompareCodecNames(strCodecName, strCurrCodecName))
            {
                int payloadType; 
                
                pCodecParams->pRtpmap = (RvSdpRtpMap*)pCurrRtpMap;
                
                /* Find the index of format, matching the codec payload type */
                payloadType = rvSdpRtpMapGetPayload(pCurrRtpMap);
                for (i=0; i<numOfFormats; i++)
                {
                    strCurrFormatName = rvSdpMediaDescrGetFormat(
                                            (const RvSdpMediaDescr*)pMediaDescr, i);
                    if (payloadType == atoi(strCurrFormatName))
                    {
                        pCodecParams->strFormatName = (RvChar*)strCurrFormatName;
                        pCodecParams->formatIdx = i;
                        pCodecParams->format = payloadType;
                        break;
                    }
                }
                break;
            }
            pCurrRtpMap = rvSdpMediaDescrGetNextRtpMap(&pCodecParams->iteratorRtpmap);
        }
    }
    else
    {
        return;
    }

    /* If no format was found, just return */
    if (NULL == pCodecParams->strFormatName)
    {
        return;
    }

    /* 2. Find RTPMAP for static payload type to be removed (interoperability).
          RTPMAP for dynamic payload type was found earlier. */
    if (NULL != strFormatName)
    {
        pCurrRtpMap = rvSdpMediaDescrGetFirstRtpMap(
                            pMediaDescr, &pCodecParams->iteratorRtpmap);
        while (NULL != pCurrRtpMap)
        {
            if (atoi(strFormatName) == rvSdpRtpMapGetPayload(pCurrRtpMap))
            {
                pCodecParams->pRtpmap = (RvSdpRtpMap*)pCurrRtpMap;
                break;
            }
            pCurrRtpMap = rvSdpMediaDescrGetNextRtpMap(&pCodecParams->iteratorRtpmap);
        }
    }

    /* 3. Find FMTP */
    pCurrFmtp = rvSdpMediaDescrGetFirstFmtp(
                        pMediaDescr, &pCodecParams->iteratorFmtp);
    while (NULL != pCurrFmtp)
    {
        strCurrFmtpVal = rvSdpAttributeGetValue(pCurrFmtp);
        if (0 == memcmp(strCurrFmtpVal, pCodecParams->strFormatName,
                        strlen(pCodecParams->strFormatName)))
        {
            pCodecParams->pFmtp = pCurrFmtp;
            break;
        }
        pCurrFmtp = rvSdpMediaDescrGetNextFmtp(&pCodecParams->iteratorFmtp);
    }
}

/******************************************************************************
 * CopyCodecParams
 * ----------------------------------------------------------------------------
 * General:
 *  Add provided format name to the destination Media Descriptor and
 *  copies into the Desriptor the provided RTPMAP and FMTP objects and
 *  general and special attributes, pointed by the provided pointers structure.
 *
 * Arguments:
 * Input:  pMediaDescrSrc  - Source Media Descriptor, where the parameters
 *                           tp be copied are located.
 *         format          - media format to be set into destination Media Desc
 *         strFormat       - media format in string format to be set.
 *         pCodecParamsSrc - Pointers to the codec parameters to be copied.
 * Output: pMediaDescrDst  - Destination Media Descriptor object.
 *
 * Return Value: RV_OK on success, error code otherwise.
 *****************************************************************************/
static RvStatus RVCALLCONV CopyCodecParams(
                            IN  RvSdpMediaDescr* pMediaDescrSrc,
                            IN  RvUint32         format,
                            IN  RvChar*          strFormatName,
                            IN  CodecParamPtrs*  pCodecParamsSrc,
                            IN  RvLogSource*     pLogSource,
                            OUT RvSdpMediaDescr* pMediaDescrDst)
{
    RvSdpStatus     srv;
    RvSdpAttribute* pFmtp;
    RvSdpAttribute* pAttr;
    RvSdpListIter   attrIterator;
    RvSdpListIter   attrIteratorSrc;
    RvSdpAttribute* pAttrSrc;
    RvStatus        rv = RV_OK;

    /* 1. Add media format & RTPMAP, if exist */
    srv = SetMediaDescrFormat(pMediaDescrDst, format, strFormatName,
                              pCodecParamsSrc->pRtpmap, pLogSource);
    if (RV_SDPSTATUS_OK != srv)
    {
        RvStatus rv = OA_SRV2RV(srv);
        RvLogError(pLogSource ,(pLogSource,
            "CopyCodecParams - failed to add format %s to descriptor %p (srv=%s)",
            pCodecParamsSrc->strFormatName, pMediaDescrDst,
            OaUtilsConvertEnum2StrSdpStatus(srv)));
        return rv;
    }

    /* 2. Copy FMTP parameters */
    if (NULL != pCodecParamsSrc->pFmtp)
    {
        pFmtp = SetMediaDescrFmtp(pMediaDescrDst, format,
                                  pCodecParamsSrc->pFmtp, pLogSource);
        if (NULL == pFmtp)
        {
            RvLogError(pLogSource ,(pLogSource,
                "CopyCodecParams - failed to add FMTP to descriptor %p",
                pMediaDescrDst));
            rv = RV_ERROR_UNKNOWN;
            /* don't return */
        }
    }

    /* 3. Copy general attributes.
    Don't copy special attributes for now. Wait for request from customer. */
    pAttrSrc = rvSdpMediaDescrGetFirstAttribute(pMediaDescrSrc, &attrIteratorSrc);
    while (NULL != pAttrSrc)
    {
        const char* strAttrName = rvSdpAttributeGetName(pAttrSrc);
        const char* strAttrVal = rvSdpAttributeGetValue(pAttrSrc);
        RvBool bDontCopy = RV_FALSE;

        if (NULL != strAttrName)
        {
            /* Check if attribute with same name already exist.
            If it does, don't add the attribute again.
            More complex policy will be implemented on demand. */
            pAttr = rvSdpMediaDescrGetFirstAttribute(
                                    pMediaDescrDst, &attrIterator);
            while (NULL != pAttr)
            {
                const char* strAttrNameAnswer = rvSdpAttributeGetName(pAttr);
                if (0 == strcmp(strAttrName, strAttrNameAnswer))
                {
                    bDontCopy = RV_TRUE;
                    break;
                }
                pAttr = rvSdpMediaDescrGetNextAttribute(&attrIterator);
            }

            /* Copy attribute */
            if (RV_FALSE == bDontCopy)
            {
                pAttr = rvSdpMediaDescrAddAttr(pMediaDescrDst, strAttrName, strAttrVal);
                if (NULL == pAttr)
                {
                    RvLogError(pLogSource ,(pLogSource,
                        "CopyCodecParams - failed to add general attribute to descriptor %p",
                        pMediaDescrDst));
                    rv = RV_ERROR_UNKNOWN;
                    /* don't return */
                }
            }
        }
        pAttrSrc = rvSdpMediaDescrGetNextAttribute(&attrIteratorSrc);
    }
    /* ENDOF:   while (NULL != pAttrSrc) */

    return rv;
}

/******************************************************************************
 * CopyFmtpParam
 * ----------------------------------------------------------------------------
 * General:
 *  Set the specified FMTP parameter to the value, found for the same parameter
 *  in the source Media Descriptor.
 *  If no FMTP line presents in the destination Media Descriptor,
 *  the FMTP line form Capabilities is copied into it before the value is set.
 *
 * Arguments:
 * Input:  pMediaDescr      - Media Descriptor, where FMTP parameter
 *                            should be set.
 *         pCodecParams     - Various pointers to the Media Descriptor.
 *         pMediaDescrSrc   - Media Descriptor, the FMTP parameter of which
 *                            should be set into pMediaDescr.
 *         pCodecParamsSrcs - Various pointers to the pMediaDescrSrc.
 *         pFmtpCaps        - Pointer to the FMTP line, that should be copied
 *                            into pMediaDescr, if it doesn't contain FMTP line
 *         strParamName     - Name of parameter to be added to the FMTP line.
 *         pLogSource       - Log source to be used while logging.
 * Output: none.
 *
 * Return Value: RV_OK on success, error code otherwise.
 *****************************************************************************/
static RvStatus RVCALLCONV CopyFmtpParam(
                            IN   RvSdpMediaDescr* pMediaDescr,
                            IN   CodecParamPtrs*  pCodecParams,
                            IN   RvSdpMediaDescr* pMediaDescrSrc,
                            IN   CodecParamPtrs*  pCodecParamsSrc,
                            IN   RvSdpAttribute*  pFmtpCaps,
                            IN   const char*      strParamName,
                            IN   RvLogSource*     pLogSource)
{
    RvStatus    rv;
    RvSdpStatus srv;
    RvChar      strParamVal[RVOA_FMTP_PARAM_MAX_LEN];

    /* If no FMTP exist, add it, using pCodecParamsSrc as a copy constructor */
    if (NULL == pCodecParams->pFmtp  &&  NULL != pFmtpCaps)
    {
        pCodecParams->pFmtp = SetMediaDescrFmtp(pMediaDescr,
                                pCodecParams->format, pFmtpCaps, pLogSource);
        if (NULL == pCodecParams->pFmtp)
        {
            RvLogError(pLogSource ,(pLogSource,
                "CopyFmtpParam - failed to add new FMTP into descriptor %p",
                pMediaDescr));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Fetch the value from the source Media Desriptor */
    srv  = RvSdpCodecFmtpParamGetByName(pMediaDescrSrc, strParamName,
                pCodecParamsSrc->format, ' ' /*separator*/, strParamVal,
                RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
    rv = OA_SRV2RV(srv);
	if(RV_OK != rv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "CopyFmtpParam - failed to fetch %s FMTP param from descriptor %p (srv=%s)",
            strParamName, pMediaDescrSrc, OaUtilsConvertEnum2StrSdpStatus(srv)));
        return rv;
    }

    /* Update the FMTP with the provided parameter */
    srv = RvSdpCodecFmtpParamSet(pMediaDescr, strParamName, strParamVal,
                RV_TRUE  /*addFmtpAttr*/, pCodecParams->format,
                ' '/*separator*/, pMediaDescr->iSdpMsg->iAllocator);
    rv = OA_SRV2RV(srv);
	if(RV_OK != rv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "CopyFmtpParam - failed to update descriptor %p with FMTP parameter %s=%s (srv=%s)",
            pMediaDescr, strParamName, strParamVal,
            OaUtilsConvertEnum2StrSdpStatus(srv)));
        return rv;
    }

    return RV_OK;
}

/******************************************************************************
 * DeriveBasicParamsG7XX
 * ----------------------------------------------------------------------------
 * General:
 *  Derives basic parameters of G7XX family codecs, acceptable by both sides of
 *  the session, and set them into the ANSWER.
 *  Offer and Capability descriptors provide parameters acceptable by remote and
 *  local side correspondently.
 *  Takes in account parameters, that were set into the ANSWER previously,
 *  while handling other formats.
 *
 *  For now the basic parameters include Packetization Time and Silence
 *  Suppresions only.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pMediaDescrAnswer - ANSWER.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveBasicParamsG7XX(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  RvLogSource*     pLogSource)
{
    RvSdpStatus     srv;
    DeriveParameter ptime;
    DeriveParameter silenceSupp;
    RvChar*         strPtimeAnswer = NULL;
    RvChar*         strSilenceSuppAnswer = NULL;
    RvBool          bWasDerived;

    GetBasicParamG7XXPtrs(pMediaDescrOffer, &ptime.strOffer, &ptime.pAttrOffer,
                          &silenceSupp.strOffer, &silenceSupp.pAttrOffer);
    GetBasicParamG7XXPtrs(pMediaDescrCaps,  &ptime.strLocal, &ptime.pAttrLocal,
                          &silenceSupp.strLocal, &silenceSupp.pAttrLocal);
    GetBasicParamG7XXPtrs(pMediaDescrAnswer,&ptime.strAnswer,&ptime.pAttrAnswer,
                          &silenceSupp.strAnswer, &silenceSupp.pAttrAnswer);

    /* 1. Agree about ptime */
    bWasDerived = DerivePtime(&ptime, &strPtimeAnswer);
    if (RV_FALSE == bWasDerived)
    {
        RvLogDebug(pLogSource ,(pLogSource,
            "DeriveBasicParamsG7XX - failed to derive ptime, acceptable by both sides (offer=%s, local=%s)",
            (ptime.strOffer != NULL ? ptime.strOffer : ""),
            (ptime.strLocal != NULL ? ptime.strLocal : "")));
        return RV_FALSE;
    }

    /* 2. If Silence Suppresion was offered, agree about it */
    if (NULL != silenceSupp.strOffer)
    {
        /* Agree */
        bWasDerived = DeriveSilenceSupp(&silenceSupp, &strSilenceSuppAnswer);
        if (RV_FALSE == bWasDerived)
        {
            RvLogDebug(pLogSource ,(pLogSource,
                "DeriveBasicParamsG7XX - failed to derive silenceSupp, acceptable by both sides (offer=%s, local=%s)",
                (silenceSupp.strOffer != NULL ? silenceSupp.strOffer : ""),
                (silenceSupp.strLocal != NULL ? silenceSupp.strLocal : "")));
            return RV_FALSE;
        }
    }

    /* 3. Set agreed parameters into the ANSWER */
    if (NULL != strPtimeAnswer)
    {
        srv = SetMediaDescrAttr(pMediaDescrAnswer,
                                "ptime", strPtimeAnswer, ptime.pAttrAnswer);
        if (RV_SDPSTATUS_OK != srv)
        {
            RvLogError(pLogSource ,(pLogSource,
                "DeriveBasicParamsG7XX - failed to set ptime attribute in descriptor %p, attr=%p (srv=%s)",
                pMediaDescrAnswer, silenceSupp.pAttrAnswer,
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            /* don't return */
        }
    }
    if (NULL != strSilenceSuppAnswer)
    {
        srv = SetMediaDescrAttr(pMediaDescrAnswer, "silenceSupp",
                        strSilenceSuppAnswer, silenceSupp.pAttrAnswer);
        if (RV_SDPSTATUS_OK != srv)
        {
            RvLogError(pLogSource ,(pLogSource,
                "DeriveBasicParamsG7XX - failed to set silenceSupp attribute in descriptor %p, attr=%p (srv=%s)",
                pMediaDescrAnswer, silenceSupp.pAttrAnswer,
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            /* don't return */
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    return RV_TRUE;
}

/******************************************************************************
 * GetBasicParamG7XXPtrs
 * ----------------------------------------------------------------------------
 * General:
 *  Gets pointers to the attributes, that describe basic parameters of G7XX
 *  family codecs, and the the attribute values.
 *
 * Arguments:
 * Input:  pMediaDescr       - Media Descriptor, where attributes are located.
 * Output: pstrPtime         - Value of ptime atribute.
 *         ppAttrPtime       - Attribute that describes Packetization Time.
 *         pstrSilenceSupp   - Value of silenceSupp atribute.
 *         ppAttrSilenceSupp - Attribute that describes Silence Suppression.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV GetBasicParamG7XXPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            OUT RvChar**         pstrPtime,
                            OUT RvSdpAttribute** ppAttrPtime,
                            OUT RvChar**         pstrSilenceSupp,
                            OUT RvSdpAttribute** ppAttrSilenceSupp)
{
    RvSdpAttribute* pAttr;
    RvSdpListIter   attrIterator;
    const RvChar*   strAttrName;

    *pstrPtime = NULL; *pstrSilenceSupp = NULL;
    *ppAttrPtime = NULL; *ppAttrSilenceSupp = NULL;
    pAttr = rvSdpMediaDescrGetFirstAttribute(pMediaDescr, &attrIterator);
    while (NULL != pAttr)
    {
        strAttrName = rvSdpAttributeGetName(pAttr);
        if (0 == strcmp("ptime", strAttrName))
        {
            *pstrPtime = (RvChar*)rvSdpAttributeGetValue(pAttr);
            *ppAttrPtime = pAttr;
        }
        else
        if (0 == strcmp("silenceSupp", strAttrName))
        {
            *pstrSilenceSupp = (RvChar*)rvSdpAttributeGetValue(pAttr);
            *ppAttrSilenceSupp = pAttr;
        }
        pAttr = rvSdpMediaDescrGetNextAttribute(&attrIterator);
    }
}

/******************************************************************************
 * DerivePtime
 * ----------------------------------------------------------------------------
 * General:
 *  Given offered and local values of Paketization Time derives the value
 *  to be set into the ANSWER: the minimal of values should be set.
 *  Takes a care of the value, already set into the ANSWER.
 *
 * Arguments:
 * Input:  pPtime          - Container of offered, local and answer values.
 * Output: pstrPtimeResult - The result value to be used in ANSWER.
 *
 * Return Value: RV_TRUE, if the value acceptable by both side was found,
 *               RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DerivePtime(
                            IN  DeriveParameter*  pPtime,
                            OUT RvChar**          pstrPtimeResult)
{
    RvInt32 ptime;
    RvInt32 ptimeFinal = RVOA_UNDEFINED;

    *pstrPtimeResult = NULL;

    if (NULL != pPtime->strOffer)
    {
        ptimeFinal = atoi(pPtime->strOffer);
        *pstrPtimeResult = pPtime->strOffer;
    }
    if (NULL != pPtime->strLocal)
    {
        ptime = atoi(pPtime->strLocal);
        if (ptimeFinal == RVOA_UNDEFINED)
        {
            ptimeFinal = ptime;
        }
        else
        if (ptime < ptimeFinal  &&  ptime != 0)
        {
            ptimeFinal = (ptimeFinal < ptime);
            *pstrPtimeResult = pPtime->strLocal;
        }
    }
    if (NULL != pPtime->strAnswer)
    {
        ptime = atoi(pPtime->strAnswer);
        if (ptimeFinal == RVOA_UNDEFINED)
        {
            ptimeFinal = ptime;
        }
        else
        if (ptime < ptimeFinal  &&  ptime != 0)
        {
            ptimeFinal = (ptimeFinal < ptime);
            *pstrPtimeResult = pPtime->strAnswer;
        }
    }

    /* For now the conditions, where the disacgree about ptime should cause
       stream rejection, are unknown. Therefore return RV_TRUE always. */
    return RV_TRUE;
}

/******************************************************************************
 * DeriveFramerate
 * ----------------------------------------------------------------------------
 * General:
 *  Given offered and local values of Frame Rates derives the value
 *  to be set into the ANSWER: the minimal of values should be set.
 *  Takes a care of the value, already set into the ANSWER.
 *
 * Arguments:
 * Input:  pFramerate          - Container of offered, local and answer values.
 * Output: pstrFramerateResult - The result value to be used in ANSWER.
 *
 * Return Value: RV_TRUE, if the value acceptable by both side was found,
 *               RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveFramerate(
                            IN  DeriveParameter* pFramerate,
                            OUT RvChar**         pstrFramerateResult)
{
    RvInt32 framerate;
    RvInt32 framerateFinal = RVOA_UNDEFINED;

    *pstrFramerateResult = NULL;

    if (NULL != pFramerate->strOffer)
    {
        framerateFinal = atoi(pFramerate->strOffer);
        *pstrFramerateResult = pFramerate->strOffer;
    }
    if (NULL != pFramerate->strLocal)
    {
        framerate = atoi(pFramerate->strLocal);
        if (framerateFinal == RVOA_UNDEFINED)
        {
            framerateFinal = framerate;
        }
        else
        if (framerate < framerateFinal  &&  framerate != 0)
        {
            framerateFinal = (framerateFinal < framerate);
            *pstrFramerateResult = pFramerate->strLocal;
        }
    }
    if (NULL != pFramerate->strAnswer)
    {
        framerate = atoi(pFramerate->strAnswer);
        if (framerateFinal == RVOA_UNDEFINED)
        {
            framerateFinal = framerate;
        }
        else
        if (framerate < framerateFinal  &&  framerate != 0)
        {
            framerateFinal = (framerateFinal < framerate);
            *pstrFramerateResult = pFramerate->strAnswer;
        }
    }

    /* For now the conditions, where the disacgree about framerate should cause
       stream rejection, are unknown. Therefore return RV_TRUE always. */
    return RV_TRUE;
}


/******************************************************************************
 * DeriveSilenceSupp
 * ----------------------------------------------------------------------------
 * General:
 *  Given offered and local values of Silence Suppresion derives the value
 *  to be set into the ANSWER: the "off" value has the higher priority.
 *  Takes a care of the value, already set into the ANSWER.
 *
 * Arguments:
 * Input:  pSilenceSupp    - Container of offered, local and answer values.
 * Output: pstrSilenceSuppResult - The result value to be used in ANSWER.
 *
 * Return Value: RV_TRUE, if the value acceptable by both side was found,
 *               RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveSilenceSupp(
                            IN  DeriveParameter*  pSilenceSupp,
                            OUT RvChar**          pstrSilenceSuppResult)
{
    RvBool bSilenceSupp = RV_TRUE;

    *pstrSilenceSuppResult = NULL;

    if (NULL != pSilenceSupp->strOffer)
    {
        if (0 != memcmp(&pSilenceSupp->strOffer, "on", 2 /*strlen("on")*/))
        {
            bSilenceSupp = RV_FALSE;
        }
    }
    if (RV_TRUE == bSilenceSupp  &&  NULL != pSilenceSupp->strLocal)
    {
        if (0 != memcmp(&pSilenceSupp->strLocal, "on", 2 /*strlen("on")*/))
        {
            bSilenceSupp = RV_FALSE;
        }
        else
        {
            *pstrSilenceSuppResult = pSilenceSupp->strLocal;
        }
    }
    if (RV_TRUE == bSilenceSupp  &&  NULL != pSilenceSupp->strAnswer)
    {
        if (0 == memcmp(&pSilenceSupp->strAnswer, "on", 2 /*strlen("on")*/))
        {
            *pstrSilenceSuppResult = pSilenceSupp->strAnswer;
        }
    }

    /* For now the conditions, where the disacgree about Silence Suppresion
    should cause stream rejection, are unknown.
    Therefore return RV_TRUE always. */
    return RV_TRUE;
}

/******************************************************************************
 * DeriveParamsH261
 * ----------------------------------------------------------------------------
 * General:
 *  Derives parameters of H261 codec, acceptable by both sides of the session,
 *  and set them into the ANSWER.
 *  Offer and Capability descriptors provide parameters acceptable by remote and
 *  local side correspondently.
 *  Takes in account parameters, that were set into the ANSWER previously,
 *  while handling other formats.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsCaps  - Various pointers to ANSWER descriptor.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveParamsH261(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource)
{
    RvBool   bDerivationSuccess;

    /* Derive basic video parameters */
    bDerivationSuccess = DeriveBasicParamsH26X(
                                pMediaDescrOffer,  pCodecParamsOffer,
                                pMediaDescrCaps,   pCodecParamsCaps,
                                pMediaDescrAnswer, pCodecParamsAnswer,
                                pLogSource);
    if (RV_FALSE == bDerivationSuccess)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveParamsH261 - failed to derive basic parameters for format %s, descriptor %p",
            pCodecParamsCaps->strFormatName, pMediaDescrAnswer));

        OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                               NULL /*strFormat*/, pCodecParamsOffer->format);
        return RV_FALSE;
    }

    /* Derive H261 specific parameters */
    bDerivationSuccess = DeriveBandwidth(pMediaDescrOffer, pMediaDescrCaps,
                                         pMediaDescrAnswer, pLogSource);
    if (RV_FALSE == bDerivationSuccess)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveParamsH261 - failed to derive bandwidth for format %s, descriptor %p",
            pCodecParamsCaps->strFormatName, pMediaDescrAnswer));

        OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                               NULL /*strFormat*/, pCodecParamsOffer->format);
        return RV_FALSE;
    }

    return RV_TRUE;
}

/******************************************************************************
 * DeriveParamsH263
 * ----------------------------------------------------------------------------
 * General:
 *  Derives parameters of H263 (RFC 2190) codec, acceptable by both sides of
 *  the session, and set them into the ANSWER.
 *  Offer and Capability descriptors provide parameters acceptable by remote and
 *  local side correspondently.
 *  Takes in account parameters, that were set into the ANSWER previously,
 *  while handling other formats.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveParamsH263(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource)
{
    RvBool   bDerivationSuccess;

    /* Derive basic video parameters */
    bDerivationSuccess = DeriveBasicParamsH26X(
                                pMediaDescrOffer,  pCodecParamsOffer,
                                pMediaDescrCaps,   pCodecParamsCaps,
                                pMediaDescrAnswer, pCodecParamsAnswer,
                                pLogSource);
    if (RV_FALSE == bDerivationSuccess)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveParamsH263 - failed to derive basic parameters for format %s, descriptor %p",
            pCodecParamsCaps->strFormatName, pMediaDescrAnswer));

        OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                               NULL /*strFormat*/, pCodecParamsOffer->format);
        return RV_FALSE;
    }

    /* Derive H263 specific parameters */
    bDerivationSuccess = DeriveMaxBitRate(
                                pMediaDescrOffer, pCodecParamsOffer,
                                pMediaDescrCaps,  pCodecParamsCaps,
                                pMediaDescrAnswer,pCodecParamsAnswer,
                                pLogSource);
    if (RV_FALSE == bDerivationSuccess)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveParamsH263 - failed to derive Maximal Bit Rate for format %s, descriptor %p",
            pCodecParamsCaps->strFormatName, pMediaDescrAnswer));

        OaSdpDescrRemoveFormat(pMediaDescrAnswer,
                               NULL /*strFormat*/, pCodecParamsOffer->format);
        return RV_FALSE;
    }

    return RV_TRUE;
}

/******************************************************************************
 * DeriveBasicParamsH26X
 * ----------------------------------------------------------------------------
 * General:
 *  Derives basic parameters of H26X family codecs, acceptable by both sides of
 *  the session, and set them into the ANSWER.
 *  Offer and Capability descriptors provide parameters acceptable by remote and
 *  local side correspondently.
 *  Takes in account parameters, that were set into the ANSWER previously,
 *  while handling other formats.
 *
 *  For now the basic parameters include Packetization Time and Silence
 *  Suppresions only.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveBasicParamsH26X(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrCaps,
                            IN  CodecParamPtrs*  pCodecParamsCaps,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer,
                            IN  RvLogSource*     pLogSource)
{
    RvStatus         rv;
    RvSdpStatus      srv;
    DeriveParameter  framerate;
    RvChar*          strFramerateAnswer;
    RvBool           bWasDerived;
    RvSdpMediaDescr* pMediaDescrToUseForQcif  = NULL;
    CodecParamPtrs*  pCodecParamsToUseForQcif = NULL;
    RvSdpMediaDescr* pMediaDescrToUseForCif   = NULL;
    CodecParamPtrs*  pCodecParamsToUseForCif  = NULL;
    RvBool           bLocalParamQcifExist;
    RvBool           bLocalParamCifExist;


    GetBasicParamH26XPtrs(pMediaDescrOffer,
                          &framerate.strOffer, &framerate.pAttrOffer);
    GetBasicParamH26XPtrs(pMediaDescrCaps,
                          &framerate.strLocal, &framerate.pAttrLocal);
    GetBasicParamH26XPtrs(pMediaDescrAnswer,
                          &framerate.strAnswer, &framerate.pAttrAnswer);

    /* 1. Agree about framerate */
    strFramerateAnswer = NULL;
    bWasDerived = DeriveFramerate(&framerate, &strFramerateAnswer);
    if (RV_FALSE == bWasDerived)
    {
        RvLogDebug(pLogSource ,(pLogSource,
            "DeriveBasicParamsH26X - failed to derive framerate, acceptable by both sides (offer=%s, local=%s)",
            (framerate.strOffer != NULL ? framerate.strOffer : ""),
            (framerate.strLocal != NULL ? framerate.strLocal : "")));
        return RV_FALSE;
    }

    /* 2. Agree about QCIF */
    bLocalParamQcifExist = FmtpParamDeriveMaximalValue(
                            pMediaDescrOffer, pCodecParamsOffer,
                            pMediaDescrCaps,  pCodecParamsCaps,
                            pMediaDescrAnswer,pCodecParamsAnswer,
                            "QCIF", pLogSource,
                            &pMediaDescrToUseForQcif, &pCodecParamsToUseForQcif);
    if (RV_FALSE == bLocalParamQcifExist)
    {
        RvLogDebug(pLogSource ,(pLogSource,
            "DeriveBasicParamsH26X - failed to derive QCIF: local QCIF was not found, format %s",
            pCodecParamsCaps->strFormatName));
        return RV_FALSE;
    }

    /* 3. Agree about CIF */
    bLocalParamCifExist = FmtpParamDeriveMaximalValue(
                            pMediaDescrOffer, pCodecParamsOffer,
                            pMediaDescrCaps,  pCodecParamsCaps,
                            pMediaDescrAnswer,pCodecParamsAnswer,
                            "CIF", pLogSource,
                            &pMediaDescrToUseForCif, &pCodecParamsToUseForCif);
    if (RV_FALSE == bLocalParamCifExist)
    {
        RvLogDebug(pLogSource ,(pLogSource,
            "DeriveBasicParamsH26X - failed to derive CIF: local CIF was not found, format %s",
            pCodecParamsCaps->strFormatName));
        return RV_FALSE;
    }

    /* 4. Add Media Format to the ANSWER */
    rv = SetMediaDescrFormat(pMediaDescrAnswer, pCodecParamsOffer->format,
                             pCodecParamsOffer->strFormatName,
                             pCodecParamsCaps->pRtpmap, pLogSource);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveBasicParamsH26X - failed to add format %d into descriptor %p (rv=%d:%s)",
            pCodecParamsOffer->format, pMediaDescrAnswer,
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return RV_FALSE;
    }
    pCodecParamsAnswer->format = pCodecParamsOffer->format;

    /* 5. Set agreed parameters into the ANSWER */
    /* Set frame rate */
    if (NULL != strFramerateAnswer)
    {
        srv = SetMediaDescrAttr(pMediaDescrAnswer, "framerate",
                                strFramerateAnswer, framerate.pAttrAnswer);
        if (RV_SDPSTATUS_OK != srv)
        {
            RvLogError(pLogSource ,(pLogSource,
                "DeriveBasicParamsH26X - failed to set framerate attribute in descriptor %p, attr=%p (srv=%s)",
                pMediaDescrAnswer, framerate.pAttrAnswer,
                OaUtilsConvertEnum2StrSdpStatus(srv)));
            return RV_FALSE;
        }
    }
    /* Set QCIF */
    if (NULL != pMediaDescrToUseForQcif  &&  NULL != pCodecParamsToUseForQcif)
    {
        rv = CopyFmtpParam(pMediaDescrAnswer, pCodecParamsAnswer,
                           pMediaDescrToUseForQcif, pCodecParamsToUseForQcif,
                           pCodecParamsCaps->pFmtp, "QCIF", pLogSource);
	    if(RV_OK != rv)
        {
            RvLogError(pLogSource ,(pLogSource,
                "DeriveBasicParamsH26X - failed to set the derived QCIF in descriptor %p (rv=%d:%s)",
                pMediaDescrAnswer, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_FALSE;
        }
    }
    /* Set CIF */
    if (NULL != pMediaDescrToUseForCif  &&  NULL != pCodecParamsToUseForCif)
    {
        rv = CopyFmtpParam(pMediaDescrAnswer, pCodecParamsAnswer,
                           pMediaDescrToUseForCif, pCodecParamsToUseForCif,
                           pCodecParamsCaps->pFmtp, "CIF", pLogSource);
	    if(RV_OK != rv)
        {
            RvLogError(pLogSource ,(pLogSource,
                "DeriveBasicParamsH26X - failed to set the derived CIF in descriptor %p (rv=%d:%s)",
                pMediaDescrAnswer, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_FALSE;
        }
    }

    /* 5. Ensure that order of QCIF and CIF in ANSWER is same as in OFFER */
    srv = CheckQcifCifOrder(pMediaDescrOffer, pCodecParamsOffer,
                            pMediaDescrAnswer, pCodecParamsAnswer);
    if (RV_SDPSTATUS_OK != srv  &&  RV_SDPSTATUS_NOT_EXISTS != srv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveBasicParamsH26X - failed to change order of QCIF and CIF parameters in ANSWER, descr=%p(srv=%s)",
            pMediaDescrAnswer, OaUtilsConvertEnum2StrSdpStatus(srv)));
        /* don't return */
    }

    return RV_TRUE;
}

/******************************************************************************
 * GetBasicParamH26XPtrs
 * ----------------------------------------------------------------------------
 * General:
 *  Gets pointers to the attributes, that describe basic parameters of H26X
 *  family codecs, and the the attribute values.
 *
 * Arguments:
 * Input:  pMediaDescr     - Media Descriptor, where attributes are located.
 * Output: pstrFramerate   - Value of framerate atribute.
 *         ppAttrFramerate - Attribute that describes Rate of Frames.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV GetBasicParamH26XPtrs(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            OUT RvChar**         pstrFramerate,
                            OUT RvSdpAttribute** ppAttrFramerate)
{
    RvSdpAttribute* pAttr;
    RvSdpListIter   attrIterator;
    const RvChar*   strAttrName;

    *pstrFramerate = NULL; *ppAttrFramerate = NULL;
    pAttr = rvSdpMediaDescrGetFirstAttribute2(pMediaDescr, &attrIterator);
    while (NULL != pAttr)
    {
        strAttrName = rvSdpAttributeGetName(pAttr);
        if (0 == strcmp("framerate", strAttrName))
        {
            *pstrFramerate = (RvChar*)rvSdpAttributeGetValue(pAttr);
            *ppAttrFramerate = pAttr;
        }
        pAttr = rvSdpMediaDescrGetNextAttribute2(&attrIterator);
    }
}

/******************************************************************************
 * DeriveBandwidth
 * ----------------------------------------------------------------------------
 * General:
 *  Given offered and local values of Bandwidth derives the value
 *  to be set into the ANSWER: the minimal of values should be set.
 *  Takes a care of the value, already set into the ANSWER.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pMediaDescrAnswer - ANSWER.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveBandwidth(
                        IN RvSdpMediaDescr* pMediaDescrOffer,
                        IN RvSdpMediaDescr* pMediaDescrCaps,
                        IN RvSdpMediaDescr* pMediaDescrAnswer,
                        IN RvLogSource*     pLogSource)
{
    RvSdpBandwidth* pBwOffer; 
    RvSdpBandwidth* pBwLocal; 
    RvSdpBandwidth* pBwAnswer; 
    RvUint32        bw;
    RvSdpListIter   iterOffer;
    RvSdpListIter   iteratorBw;

    pBwOffer = rvSdpMediaDescrGetFirstBandwidth(pMediaDescrOffer, &iterOffer);
    while (NULL != pBwOffer)
    {
        /* Find the local bandwidth of same type as in pBwOffer */
        pBwLocal = rvSdpMediaDescrGetFirstBandwidth(pMediaDescrCaps, &iteratorBw);
        while (NULL != pBwLocal)
        {
            if (0 == RvStrcasecmp(pBwOffer->iBWType, pBwLocal->iBWType))
                break;
            pBwLocal = rvSdpMediaDescrGetNextBandwidth(&iteratorBw);
        }

        /* Find the answer bandwidth of same type as in pBwOffer */
        pBwAnswer = rvSdpMediaDescrGetFirstBandwidth(pMediaDescrAnswer, &iteratorBw);
        while (NULL != pBwAnswer)
        {
            if (0 == RvStrcasecmp(pBwOffer->iBWType, pBwAnswer->iBWType))
                break;
            pBwAnswer = rvSdpMediaDescrGetNextBandwidth(&iteratorBw);
        }

        /* Derive the minimal value from the offered, local and answer bws */
        bw = 0xffffffff;
        if (NULL != pBwLocal)
        {
            bw = pBwLocal->iBWValue;
        }
        if (NULL != pBwAnswer  &&  pBwAnswer->iBWValue < bw)
        {
            bw = pBwAnswer->iBWValue;
        }
        bw = (pBwOffer->iBWValue < bw) ? pBwOffer->iBWValue : bw;

        /* Set the derived value into the answer */
        if (NULL != pBwAnswer)
        {
            rvSdpBandwidthSetValue(pBwAnswer, bw);
        }
        else
        {
            pBwAnswer = rvSdpMediaDescrAddBandwidth(
                                pMediaDescrAnswer, pBwOffer->iBWType, bw);
            if (NULL == pBwAnswer)
            {
                RvLogError(pLogSource ,(pLogSource,
                    "DeriveBandwidth - failed to add new BANDWIDTH into descriptor %p",
                    pMediaDescrAnswer));
                /* don't return */
            }
        }

        /* Handle next type of bandwidth */
        pBwOffer = rvSdpMediaDescrGetNextBandwidth(&iterOffer);

    } /* while (NULL != pBwOffer) */


#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

    /* For now the conditions, where the disagree about bandwidth should cause
    stream rejection, are unknown. Therefore return RV_TRUE always. */
    return RV_TRUE;
}

/******************************************************************************
 * DeriveMaxBitRate
 * ----------------------------------------------------------------------------
 * General:
 *  Given offered and local values of Maximum Bit Rate derives the value
 *  to be set into the ANSWER: the minimal of values should be set.
 *  Takes a care of the value, already set into the ANSWER.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         pLogSource        - Log source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the parameters accepted by both sides were derived
 *               successfully. RV_FALSE - otherwise.
 *****************************************************************************/
static RvBool RVCALLCONV DeriveMaxBitRate(
                            IN RvSdpMediaDescr* pMediaDescrOffer,
                            IN CodecParamPtrs*  pCodecParamsOffer,
                            IN RvSdpMediaDescr* pMediaDescrCaps,
                            IN CodecParamPtrs*  pCodecParamsCaps,
                            IN RvSdpMediaDescr* pMediaDescrAnswer,
                            IN CodecParamPtrs*  pCodecParamsAnswer,
                            IN RvLogSource*     pLogSource)
{
    RvStatus         rv;
    RvSdpMediaDescr* pMediaDescrToUse;
    CodecParamPtrs*  pCodecParamsToUse;

    /* Find out the Media Descriptor that contain the minimal value of MBR*/
    FmtpParamDeriveMinimalValue(pMediaDescrOffer, pCodecParamsOffer,
                                pMediaDescrCaps,  pCodecParamsCaps,
                                pMediaDescrAnswer,pCodecParamsAnswer,
                                "MaxBR", pLogSource,
                                &pMediaDescrToUse, &pCodecParamsToUse);
    if (NULL == pMediaDescrToUse)
    {
        return RV_TRUE;
    }

    /* Update the Answer FMTP with the derived parameter */
    rv = CopyFmtpParam(pMediaDescrAnswer, pCodecParamsAnswer,
                       pMediaDescrToUse, pCodecParamsToUse,
                       pCodecParamsCaps->pFmtp, "MaxBR", pLogSource);
	if(RV_OK != rv)
    {
        RvLogError(pLogSource ,(pLogSource,
            "DeriveMaxBitRate - failed to set the derived MaxBR into descriptor %p (rv=%d:%s)",
            pMediaDescrAnswer, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return RV_FALSE;
    }

    /* For now the conditions, where the disacgree about bandwidth should cause
    stream rejection, are unknown. Therefore return RV_TRUE always. */
    return RV_TRUE;
}

/******************************************************************************
 * SetMediaDescrAttr
 * ----------------------------------------------------------------------------
 * General:
 *  Adds attribute to the provided Media Descriptor.
 *  If Attribute object is provided, uses it: sets it value to the provided one
 *
 * Arguments:
 * Input:  pMediaDescr - Media Descriptor, where attribute should be set.
 *         strAttrName - Name of the attribute.
 *         strAttrVal  - Name of the attribute.
 *         pAttr       - Attribute object. Can be NULL.
 * Output: none.
 *
 * Return Value: RV_SDPSTATUS_OK on success, error code otherwise.
 *****************************************************************************/
static RvSdpStatus RVCALLCONV SetMediaDescrAttr(
                    IN RvSdpMediaDescr* pMediaDescr,
                    IN RvChar*          strAttrName,
                    IN RvChar*          strAttrVal,
                    IN RvSdpAttribute*  pAttr)
{
    /* If attribute object was provided, use it  */
    if (NULL != pAttr)
    {
        return rvSdpAttributeSetValue(pAttr, strAttrVal);
    }
    /* Otherwise - add new attribute */
    pAttr = rvSdpMediaDescrAddAttr(pMediaDescr, strAttrName, strAttrVal);
    if (NULL == pAttr)
    {
        return RV_SDPSTATUS_SET_FAILED;
    }
    return RV_SDPSTATUS_OK;
}

/******************************************************************************
 * SetMediaDescrFmtp
 * ----------------------------------------------------------------------------
 * General:
 *  Adds value of FMTP attribute into the Media Descriptor,
 *  while using specified format as payload type for FMTP.
 *
 * Arguments:
 * Input:  pMediaDescr - Media Descriptor, where FMTP should be set.
 *         format      - media format to be used as a payload type.
 *         pFmtpSrc    - FMTP attribute, copy of which should be added.
 *         pLogSource  - Log source to be used while logging.
 * Output: none.
 *
 * Return Value: newly added FMTP attribute on success, NULL otherwise.
 *****************************************************************************/
static RvSdpAttribute* RVCALLCONV SetMediaDescrFmtp(
                            IN  RvSdpMediaDescr* pMediaDescr,
                            IN  RvUint32         format,
                            IN  RvSdpAttribute*  pFmtpSrc,
                            IN  RvLogSource*     pLogSource)
{
    RvSdpAttribute* pFmtp;
    const char* strFmtpVal = rvSdpAttributeGetValue(pFmtpSrc);
    char*       pstrFmtpVal;
    RvChar      strPayloadType[32];
    RvChar      strFmtp[RVOA_FMTP_PARAM_MAX_LEN];
    RvSize_t    fmtpSize;

    if (NULL == strFmtpVal)
    {
        return NULL;
    }

    /* Skip the existing payload type */
    pstrFmtpVal = strchr(strFmtpVal, ' ');
    if (NULL == pstrFmtpVal)
    {
        return NULL;
    }
    while (*pstrFmtpVal == ' ')
        pstrFmtpVal++;

    /* Prepare FMTP with required payload type */
    sprintf(strPayloadType, "%d", format);
    fmtpSize = strlen(strPayloadType) + strlen(pstrFmtpVal);
    if (fmtpSize >= RVOA_FMTP_PARAM_MAX_LEN)
    {
        RvLogError(pLogSource ,(pLogSource,
            "SetMediaDescrFmtp - buffer is not big enough (%d) to contain FMTP line (%d), descr=%p",
            RVOA_FMTP_PARAM_MAX_LEN, fmtpSize, pMediaDescr));
        return NULL;
    }
    strFmtp[0] = '\0';
    strcat(strFmtp, strPayloadType);
    strcat(strFmtp, " ");
    strcat(strFmtp, pstrFmtpVal);

    pFmtp = rvSdpMediaDescrAddFmtp(pMediaDescr, strFmtp);
 
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return pFmtp;
}

/******************************************************************************
 * SetMediaDescrFormat
 * ----------------------------------------------------------------------------
 * General:
 *  Adds provided name of media format to the Media Descriptor object.
 *  If RTPMAP parameter is provided, copies it into the Media Descriptor.
 *
 * Arguments:
 * Input:  pMediaDescr - Media Descriptor, where attribute should be set.
 *         format      - media format to be set.
 *         strFormat   - media format in string format to be set.
 *         pRtpmap     - RTPMAP attribute to be used.
 *         pLogSource  - Log source to be used while logging.
 * Output: none.
 *
 * Return Value: RV_OK on success, error code otherwise.
 *****************************************************************************/
static RvStatus RVCALLCONV SetMediaDescrFormat(
                                    IN  RvSdpMediaDescr* pMediaDescr,
                                    IN  RvUint32         format,
                                    IN  RvChar*          strFormatName,
                                    IN  RvSdpRtpMap*     pRtpmap,
                                    IN  RvLogSource*     pLogSource)
{
    RvStatus     rv;
    RvSdpStatus  srv;
    RvSdpRtpMap* pRtpmapNew;

    /* 1. Add format name */
    srv = rvSdpMediaDescrAddFormat(pMediaDescr, strFormatName);
    if (RV_SDPSTATUS_OK != srv)
    {
        rv = OA_SRV2RV(srv);
        RvLogError(pLogSource ,(pLogSource,
            "SetMediaDescrFormat - failed to add Format %s to descriptor %p (srv=%s)",
            strFormatName, pMediaDescr,
            OaUtilsConvertEnum2StrSdpStatus(srv)));
        return rv;
    }

    /* 2. Copy RTPMAP parameters */
    if (NULL != pRtpmap)
    {
        pRtpmapNew = rvSdpMediaDescrAddRtpMap(pMediaDescr,
                            0 /*payload*/, "" /*encoding_name*/, 0 /*rate*/);
        if (NULL == pRtpmapNew)
        {
            RvLogError(pLogSource ,(pLogSource,
                "SetMediaDescrFormat - failed to add new RTPMAP into descriptor %p",
                pMediaDescr));
            return srv;
        }
        pRtpmapNew = rvSdpRtpMapCopy(
                        pRtpmapNew, (const RvSdpRtpMap*)pRtpmap);
        if (NULL == pRtpmapNew)
        {
            RvLogError(pLogSource ,(pLogSource,
                "SetMediaDescrFormat - failed to copy RTPMAP %p into RTPMAP %p, descriptor %p",
                pRtpmap, pRtpmapNew, pMediaDescr));
            return RV_ERROR_UNKNOWN;
        }
        /* Ensure that RTPMAP contains payload type that matches the format */
        rvSdpRtpMapSetPayload(pRtpmapNew, format);
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return RV_OK;
}

/******************************************************************************
 * FmtpParamDeriveMinimalValue
 * ----------------------------------------------------------------------------
 * General:
 *  Finds the Media Desriptor with minimal value of specified FMTP parameter.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         strParamName      - Name of the FMTP parameter.
 *         pLogSource        - Log Source to be used for logging.
 * Output: ppMediaDescr      - Media Descriptor with minimal value.
 *                             OFFER or ANSWER or Capabilities.
 *         pCodecParamsOffer - Various ppMediaDescr to OFFER descriptor.
 *
 * Return Value: RV_TRUE - if the parameter is defined in local capabilties.
 *****************************************************************************/
static RvBool RVCALLCONV FmtpParamDeriveMinimalValue(
                            IN  RvSdpMediaDescr*  pMediaDescrOffer,
                            IN  CodecParamPtrs*   pCodecParamsOffer,
                            IN  RvSdpMediaDescr*  pMediaDescrCaps,
                            IN  CodecParamPtrs*   pCodecParamsCaps,
                            IN  RvSdpMediaDescr*  pMediaDescrAnswer,
                            IN  CodecParamPtrs*   pCodecParamsAnswer,
                            IN  const char*       strParamName,
                            IN  RvLogSource*      pLogSource,
                            OUT RvSdpMediaDescr** ppMediaDescr,
                            OUT CodecParamPtrs**  ppCodecParams)
{
    RvSdpStatus srv;
    RvChar      strOffer[RVOA_FMTP_PARAM_MAX_LEN];
    RvChar      strLocal[RVOA_FMTP_PARAM_MAX_LEN];
    RvChar      strAnswer[RVOA_FMTP_PARAM_MAX_LEN];
    RvInt32     offer, local, answer;
    RvInt32     minValue;
    RvBool      bLocalParamExist;

    /* Get the offered, local and answer values */

    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrOffer, strParamName,
            pCodecParamsOffer->format, ' ' /*separator*/, strOffer,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        offer = atoi((const char*)strOffer);
    }
    else
    {
        offer = RVOA_UNDEFINED_MAX;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrCaps, strParamName,
            pCodecParamsCaps->format, ' ' /*separator*/, strLocal,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        local = atoi((const char*)strLocal);
    }
    else
    {
        local = RVOA_UNDEFINED_MAX;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrAnswer, strParamName,
            pCodecParamsAnswer->format, ' ' /*separator*/, strAnswer,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        answer = atoi((const char*)strAnswer);
    }
    else
    {
        answer = RVOA_UNDEFINED_MAX;
    }

    /* Find the descriptor, that contain the minimal value */
    minValue = RVOA_UNDEFINED_MAX; *ppMediaDescr = NULL; *ppCodecParams = NULL;
    /* Firstly, choose minimum of Local and ANSWER values */
    if (RVOA_UNDEFINED_MAX != local  ||  RVOA_UNDEFINED_MAX != answer)
    {
        minValue = (RVOA_UNDEFINED_MAX == local) ? answer : RvMin(local, answer);
        *ppMediaDescr  =   (minValue == local) ? pMediaDescrCaps  : pMediaDescrAnswer;
        *ppCodecParams =   (minValue == local) ? pCodecParamsCaps : pCodecParamsAnswer;
    }
    /* Now choose minimum of choosed previously and offered values */
    if (RVOA_UNDEFINED_MAX != offer  ||  RVOA_UNDEFINED_MAX != minValue)
    {
        minValue = (RVOA_UNDEFINED_MAX == offer) ? minValue : RvMin(minValue, offer);
        if (minValue == offer)
        {
            *ppMediaDescr  = pMediaDescrOffer;
            *ppCodecParams = pCodecParamsOffer;
        }
    }

    RvLogDebug(pLogSource ,(pLogSource,
        "FmtpParamDeriveMinimalValue - value of %s was %s derived, descr=%p",
        strParamName, (*ppMediaDescr==NULL)?"not":"", pMediaDescrAnswer));

    if (RVOA_UNDEFINED_MAX == local  &&  RVOA_UNDEFINED_MAX == answer)
    {
        bLocalParamExist = RV_FALSE;
    }
    else
    {
        bLocalParamExist = RV_TRUE;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return bLocalParamExist;
}

/******************************************************************************
 * FmtpParamDeriveMaximalValue
 * ----------------------------------------------------------------------------
 * General:
 *  Finds the Media Descriptor with maximal value of specified FMTP parameter.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrCaps   - Capabilities (contains local parameters).
 *         pCodecParamsCaps  - Various pointers to Capabilities descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         strParamName      - Name of the FMTP parameter.
 *         pLogSource        - Log Source to be used for logging.
 * Output: ppMediaDescr      - Media Descriptor with minimal value.
 *                             OFFER or ANSWER or Capabilities.
 *         pCodecParamsOffer - Various ppMediaDescr to OFFER descriptor.
 *
 * Return Value: RV_TRUE - if the parameter is defined in local capabilties.
 *****************************************************************************/
static RvBool RVCALLCONV FmtpParamDeriveMaximalValue(
                            IN  RvSdpMediaDescr*  pMediaDescrOffer,
                            IN  CodecParamPtrs*   pCodecParamsOffer,
                            IN  RvSdpMediaDescr*  pMediaDescrCaps,
                            IN  CodecParamPtrs*   pCodecParamsCaps,
                            IN  RvSdpMediaDescr*  pMediaDescrAnswer,
                            IN  CodecParamPtrs*   pCodecParamsAnswer,
                            IN  const char*       strParamName,
                            IN  RvLogSource*      pLogSource,
                            OUT RvSdpMediaDescr** ppMediaDescr,
                            OUT CodecParamPtrs**  ppCodecParams)
{
    RvSdpStatus srv;
    RvChar      strOffer[RVOA_FMTP_PARAM_MAX_LEN];
    RvChar      strLocal[RVOA_FMTP_PARAM_MAX_LEN];
    RvChar      strAnswer[RVOA_FMTP_PARAM_MAX_LEN];
    RvInt32     offer, local, answer;
    RvInt32     maxValue;
    RvBool      bLocalParamExist;

    /* Get the offered, local and answer values */
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrOffer, strParamName,
            pCodecParamsOffer->format, ' ' /*separator*/, strOffer,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        offer = atoi((const char*)strOffer);
    }
    else
    {
        offer = RVOA_UNDEFINED_MIN;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrCaps, strParamName,
            pCodecParamsCaps->format, ' ' /*separator*/, strLocal,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        local = atoi((const char*)strLocal);
    }
    else
    {
        local = RVOA_UNDEFINED_MIN;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrAnswer, strParamName,
            pCodecParamsAnswer->format, ' ' /*separator*/, strAnswer,
            RVOA_FMTP_PARAM_MAX_LEN, NULL/*pIndex*/);
	if(RV_SDPSTATUS_OK == srv)
    {
        answer = atoi((const char*)strAnswer);
    }
    else
    {
        answer = RVOA_UNDEFINED_MIN;
    }


    /* Find the descriptor, that contain the maximal value */
    maxValue = RVOA_UNDEFINED_MIN; *ppMediaDescr = NULL; *ppCodecParams = NULL;
    /* Firstly, choose minimum of Local and ANSWER values */
    if (RVOA_UNDEFINED_MIN != local  ||  RVOA_UNDEFINED_MIN != answer)
    {
        maxValue = (RVOA_UNDEFINED_MIN == local) ? answer : RvMax(local, answer);
        *ppMediaDescr  =   (maxValue == local) ? pMediaDescrCaps  : pMediaDescrAnswer;
        *ppCodecParams =   (maxValue == local) ? pCodecParamsCaps : pCodecParamsAnswer;
    }
    /* Now choose maximum of choosed previously and offered values */
    if (RVOA_UNDEFINED_MIN != offer  ||  RVOA_UNDEFINED_MIN != maxValue)
    {
        maxValue = (RVOA_UNDEFINED_MIN == offer) ? maxValue : RvMax(maxValue, offer);
        if (maxValue == offer)
        {
            *ppMediaDescr  = pMediaDescrOffer;
            *ppCodecParams = pCodecParamsOffer;
        }
    }

    RvLogDebug(pLogSource ,(pLogSource,
        "FmtpParamDeriveMaximalValue - value of %s was %s derived, descr=%p",
        strParamName, (*ppMediaDescr==NULL)?"not":"", pMediaDescrAnswer));

    if (RVOA_UNDEFINED_MIN == local  &&  RVOA_UNDEFINED_MIN == answer)
    {
        bLocalParamExist = RV_FALSE;
    }
    else
    {
        bLocalParamExist = RV_TRUE;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return bLocalParamExist;
}

/******************************************************************************
 * CheckQcifCifOrder
 * ----------------------------------------------------------------------------
 * General:
 *  Checks if the order of QCIF in CIF parameters in FMTP line of ANSWER is
 *  identical to this of OFFER.
 *  If it is not, fix it.
 *
 * Arguments:
 * Input:  pMediaDescrOffer  - OFFER (contains remote parameters).
 *         pCodecParamsOffer - Various pointers to OFFER descriptor.
 *         pMediaDescrAnswer - ANSWER.
 *         pCodecParamsAnser - Various pointers to ANSWER descriptor.
 *         pLogSource        - Log Source to be used for logging.
 * Output: none.
 *
 * Return Value: RV_SDPSTATUS_OK on success, error code otherwise.
 *****************************************************************************/
static RvSdpStatus RVCALLCONV CheckQcifCifOrder(
                            IN  RvSdpMediaDescr* pMediaDescrOffer,
                            IN  CodecParamPtrs*  pCodecParamsOffer,
                            IN  RvSdpMediaDescr* pMediaDescrAnswer,
                            IN  CodecParamPtrs*  pCodecParamsAnswer)
{
    RvSdpStatus srv;
    RvChar  strQcif[RVOA_FMTP_PARAM_MAX_LEN];
    RvChar  strCif[RVOA_FMTP_PARAM_MAX_LEN];
    RvInt   idxQcifOffer, idxCifOffer, idxQcifAnswer, idxCifAnswer;

    if (NULL == pCodecParamsOffer->pFmtp || NULL == pCodecParamsAnswer->pFmtp)
    {
        return RV_SDPSTATUS_OK;
    }

    /* Get indexes of QCIF and CIF parameters in OFFER */
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrOffer, "QCIF",
            pCodecParamsOffer->format, ' ' /* parameter separator */,
            NULL /*buff*/, 0 /*buff len*/, &idxQcifOffer);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrOffer, "CIF",
            pCodecParamsOffer->format, ' ' /* parameter separator */,
            NULL /*buff*/, 0 /*buff len*/, &idxCifOffer);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }
    /* Get values and indexes of QCIF and CIF parameters in ANSWER */
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrAnswer, "QCIF",
            pCodecParamsAnswer->format, ' ' /* parameter separator */,
            strQcif, RVOA_FMTP_PARAM_MAX_LEN, &idxQcifAnswer);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }
    srv = RvSdpCodecFmtpParamGetByName(pMediaDescrAnswer, "CIF",
            pCodecParamsAnswer->format, ' ' /* parameter separator */,
            strCif, RVOA_FMTP_PARAM_MAX_LEN, &idxCifAnswer);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }

    /* Return if order of QCIF and CIF in OFFER is identical to ANSWER */
    if ((idxQcifOffer < idxCifOffer  &&  idxQcifAnswer < idxCifAnswer) ||
        (idxQcifOffer > idxCifOffer  &&  idxQcifAnswer > idxCifAnswer))
    {
        return RV_SDPSTATUS_OK;
    }

    /* Switch QCIF and CIF in ANSWER */
    /* 1. Remove them */
    srv = RvSdpCodecFmtpParamRemoveByIndex(pMediaDescrAnswer, idxQcifAnswer,
            RV_FALSE /* don't remove FMTP line */, pCodecParamsAnswer->format,
            ' ' /* parameter separator */);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }
    srv = RvSdpCodecFmtpParamRemoveByIndex(pMediaDescrAnswer, idxCifAnswer,
            RV_FALSE /* don't remove FMTP line */, pCodecParamsAnswer->format,
            ' ' /* parameter separator */);
    if (RV_SDPSTATUS_OK != srv)
    {
        return srv;
    }
    /* 2. Adds them in opposite order */
    if (idxQcifAnswer < idxCifAnswer)
    {
        srv = RvSdpCodecFmtpParamSet(pMediaDescrAnswer, "CIF", strCif,
                RV_TRUE /* add if not exist */, pCodecParamsAnswer->format,
                ' ' /* parameter separator */,
                pMediaDescrAnswer->iSdpMsg->iAllocator);
        if (RV_SDPSTATUS_OK != srv)
        {
            return srv;
        }
        srv = RvSdpCodecFmtpParamSet(pMediaDescrAnswer, "QCIF", strQcif,
                RV_TRUE /* add if not exist */, pCodecParamsAnswer->format,
                ' ' /* parameter separator */,
                pMediaDescrAnswer->iSdpMsg->iAllocator);
        if (RV_SDPSTATUS_OK != srv)
        {
            return srv;
        }
    }
    else
    {
        srv = RvSdpCodecFmtpParamSet(pMediaDescrAnswer, "QCIF", strQcif,
                RV_TRUE /* add if not exist */, pCodecParamsAnswer->format,
                ' ' /* parameter separator */,
                pMediaDescrAnswer->iSdpMsg->iAllocator);
        if (RV_SDPSTATUS_OK != srv)
        {
            return srv;
        }
        srv = RvSdpCodecFmtpParamSet(pMediaDescrAnswer, "CIF", strCif,
                RV_TRUE /* add if not exist */, pCodecParamsAnswer->format,
                ' ' /* parameter separator */,
                pMediaDescrAnswer->iSdpMsg->iAllocator);
        if (RV_SDPSTATUS_OK != srv)
        {
            return srv;
        }
    }
    return RV_SDPSTATUS_OK;
}

/*nl for linux */

