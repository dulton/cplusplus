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
 *                              <OaUtils.c>
 *
 * The OaUtils.c file implements various functions that are used by different
 * components of the Offer-Answer module.
 * The utility functions can be divided in following groups:
 *  1. Offer-Answer SDP Message.
 *  2. Conversions.
 *
 * Offer-Answer SDP Message represents the wrapper for the Message object,
 * defined in RADVISION SDP Stack. The Offer-Answer SDP Message provides
 * memory to hold SDP Stack Message itself and keeps the Allocator to be used
 * with the SDP Stack Message. Also the Offer-Answer SDP Message holds
 * reference to pool that feeds the Allocator object.
 *
 * Conversion functions convert mainly different enumeration values to string
 * and via versa, or SDP Stack enumeration values to Offer-Answer module
 * enumerations and via versa. The first type of function is used for logging
 * the second is used for wrappers of SDp Stack functions, provided
 * by the Offer-Answer module to the Application.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include <string.h>
#include "rvlog.h"
#include "RvOaTypes.h"
#include "OaUtils.h"



/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTION DEFINITIONS                      */
/*---------------------------------------------------------------------------*/
static RvStatus RVCALLCONV OaSdpMsgReallocate(
                                        IN    HRPOOL        hPool,
                                        IN    RvLogSource*  pLogSource,
                                        OUT   OaSdpMsg*     pOaSdpMsg);

static RvStatus RVCALLCONV OaPSdpMsgReallocate(
                                        IN    HRA           hRaAllocators,
                                        IN    HRA           hRaMsgs,
                                        IN    HRPOOL        hPool,
                                        IN    RvLogSource*  pLogSource,
                                        IN    void*         pMsgOwner,
                                        INOUT OaPSdpMsg*    pOaSdpMsg);


/*---------------------------------------------------------------------------*/
/*                          FUNCTION IMPLEMENTATIONS                         */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaUtilsConvertEnum2StrStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts RvStatus enumeration value into string.
 *
 * Arguments:
 * Input:  crv - value of RvStatus enumeration.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrStatus(RvStatus status)
{
    switch (status)
    {
        case RV_OK:                         return (char *)"OK";
        case RV_ERROR_UNKNOWN:              return (char *)"UNKNOWN";
        case RV_ERROR_BADPARAM:             return (char *)"BADPARAM";
        case RV_ERROR_NULLPTR:              return (char *)"NULLPTR";
        case RV_ERROR_OUTOFRANGE:           return (char *)"OUTOFRANGE";
        case RV_ERROR_DESTRUCTED:           return (char *)"DESTRUCTED";
        case RV_ERROR_NOTSUPPORTED:         return (char *)"NOTSUPPORTED";
        case RV_ERROR_UNINITIALIZED:        return (char *)"UNINITIALIZED";
        case RV_ERROR_TRY_AGAIN:            return (char *)"TRY_AGAIN";

        case RV_ERROR_ILLEGAL_ACTION:       return (char *)"ILLEGAL_ACTION";
        case RV_ERROR_INVALID_HANDLE:       return (char *)"INVALID_HANDLE";
        case RV_ERROR_NOT_FOUND:            return (char *)"NOT_FOUND";
        case RV_ERROR_INSUFFICIENT_BUFFER:  return (char *)"INSUFFICIENT_BUFFER";
        case RV_ERROR_OUTOFRESOURCES:       return (char *)"OUTOFRESOURCES";
        default:                            return (char *)"-Unknown status-";
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrLogMask
 * ----------------------------------------------------------------------------
 * General:
 *  Converts mask of log filters into string.
 *  String representation of filters set in the mask, is concatenated into
 *  the returned string.
 *
 * Arguments:
 * Input:  logMask - RvStatus enumeration value.
 * Output: strMask - concatenation of log mask filters in string form.
 *                   This parameter should point to the buffer large enough
 *                   to contain all possible filters.
 *                   Use buffers of at least RVOA_LOGMASK_STRLEN size.
 *
 * Return Value: strMask parameter.
 *****************************************************************************/
RvChar* RVCALLCONV OaUtilsConvertEnum2StrLogMask(
                                        IN  RvInt32 logMask,
                                        OUT RvChar* strMask)
{
    RvSize_t lenOfStr;

    *strMask = '\0';

    if(logMask & RVOA_LOG_DEBUG_FILTER)
    {
        strcat(strMask, "DEBUG,");
    }
    if(logMask & RVOA_LOG_INFO_FILTER)
    {
        strcat(strMask, "INFO,");
    }
    if(logMask & RVOA_LOG_WARN_FILTER)
    {
        strcat(strMask, "WARN,");
    }
    if(logMask & RVOA_LOG_ERROR_FILTER)
    {
        strcat(strMask, "ERROR,");
    }
    if(logMask & RVOA_LOG_EXCEP_FILTER)
    {
        strcat(strMask, "EXCEP,");
    }
    if(logMask & RVOA_LOG_LOCKDBG_FILTER)
    {
        strcat(strMask, "LOCKDBG");
    }

    if ('\0' == *strMask)
    {
        strcat(strMask, "NONE");
    }

    lenOfStr = strlen(strMask);
    if (',' == strMask[lenOfStr-1])
    {
        strMask[lenOfStr-1] = '\0';
    }

    return strMask;
}

/******************************************************************************
 * OaUtilsConvertOa2CcLogMask
 * ----------------------------------------------------------------------------
 * General:
 *  Converts log mask in Offer-Answer format into log mask in Common Core
 *  format.
 *
 * Arguments:
 * Input:  oaLogMask - log mask in Offer-Answer format.
 * Output: none.
 *
 * Return Value: log mask in Common Core format.
 *****************************************************************************/
RvLogMessageType RVCALLCONV OaUtilsConvertOa2CcLogMask(IN RvInt32 oaLogMask)
{
    RvLogMessageType ccLogMask = RV_LOGLEVEL_NONE;

    if(oaLogMask & RVOA_LOG_DEBUG_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_DEBUG;
    }
    if(oaLogMask & RVOA_LOG_INFO_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_INFO;
    }
    if(oaLogMask & RVOA_LOG_WARN_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_WARNING;
    }
    if(oaLogMask & RVOA_LOG_ERROR_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_ERROR;
    }
    if(oaLogMask & RVOA_LOG_EXCEP_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_EXCEP;
    }
    if(oaLogMask & RVOA_LOG_LOCKDBG_FILTER)
    {
        ccLogMask |= RV_LOGLEVEL_SYNC;
    }

    return ccLogMask;
}

/******************************************************************************
 * OaUtilsConvertCc2OaLogFilter
 * ----------------------------------------------------------------------------
 * General:
 *  Converts log filter in Common Core format into log filter in Offer-Answer
 *  format.
 *
 * Arguments:
 * Input:  ccLogFilter - log filter in Offer-Answer format.
 * Output: none.
 *
 * Return Value: log filter in Common Core format.
 *****************************************************************************/
RvOaLogFilter RVCALLCONV OaUtilsConvertCc2OaLogFilter(IN RvInt ccLogFilter)
{
    switch (ccLogFilter)
    {
    case RV_LOGID_DEBUG:
        return RVOA_LOG_DEBUG_FILTER;
    case  RV_LOGID_INFO:
        return RVOA_LOG_INFO_FILTER;
    case RV_LOGID_WARNING:
        return RVOA_LOG_WARN_FILTER;
    case RV_LOGID_ERROR:
        return RVOA_LOG_ERROR_FILTER;
    case RV_LOGID_EXCEP:
        return RVOA_LOG_EXCEP_FILTER;
    case RV_LOGID_SYNC:
        return RVOA_LOG_LOCKDBG_FILTER;
    default:
        return RVOA_LOG_DEBUG_FILTER;
    } /* end of switch block */
}

/******************************************************************************
 * OaUtilsConvertEnum2StrSdpParseStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of SDP Stack parse status enumeration into string.
 *
 * Arguments:
 * Input:  eSdpParseStatus - RvSdpParseStatus enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSdpParseStatus(
                                        IN RvSdpParseStatus eSdpParseStatus)
{
    switch (eSdpParseStatus)
    {
    case RV_SDPPARSER_STOP_ZERO:        return "ZERO";
    case RV_SDPPARSER_STOP_BLANKLINE:   return "BLANKLINE";
    case RV_SDPPARSER_STOP_DOTLINE:     return "DOTLINE";
    case RV_SDPPARSER_STOP_CLOSEBRACE:  return "CLOSEBRACKET";
    case RV_SDPPARSER_STOP_ALLOCFAIL:   return "ALLOCFAIL";
    case RV_SDPPARSER_STOP_ERROR:       return "ERROR";
    case RV_SDPPARSER_STOP_NOT_SET:     return "NOT_SET";
    default: return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertSdp2OaStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts status, returned by SDP Stack API into status, returned by
 *  Offer-Answer API.
 *
 * Arguments:
 * Input:  eSdpStatus - SDP Stack value.
 * Output: none.
 *
 * Return Value: Offer-Answer value.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsConvertSdp2OaStatus(IN RvSdpStatus eSdpStatus)
{
    switch (eSdpStatus)
    {
    case RV_SDPSTATUS_OK:               return RV_OK;
    case RV_SDPSTATUS_ALLOCFAIL:        return RV_ERROR_OUTOFRESOURCES;
    case RV_SDPSTATUS_ILLEGAL_SET:      return RV_ERROR_BADPARAM;
    case RV_SDPSTATUS_NULL_PTR:         return RV_ERROR_INVALID_HANDLE;

    case RV_SDPSTATUS_ENCODEFAILBUF:
    case RV_SDPSTATUS_PARSEFAIL:
    case RV_SDPSTATUS_SET_FAILED:
    default: return RV_ERROR_UNKNOWN;
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrSdpStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvSdpStatus enumeration into string.
 *
 * Arguments:
 * Input:  eSdpStatus - RvSdpStatus enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSdpStatus(
                                                    IN RvSdpStatus eSdpStatus)
{
    switch (eSdpStatus)
    {
    case RV_SDPSTATUS_OK:               return "OK";
    case RV_SDPSTATUS_ENCODEFAILBUF:    return "ENCODEFAILBUF";
    case RV_SDPSTATUS_ALLOCFAIL:        return "ALLOCFAIL";
    case RV_SDPSTATUS_PARSEFAIL:        return "PARSEFAIL";
    case RV_SDPSTATUS_ILLEGAL_SET:      return "ILLEGAL_SET";
    case RV_SDPSTATUS_NULL_PTR:         return "NULL_PTR";
    case RV_SDPSTATUS_SET_FAILED:       return "SET_FAILED";
    default:                            return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrSessionState
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaSessionState enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaSessionState enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSessionState(
                                                IN RvOaSessionState  eState)
{
    switch (eState)
    {
    case RVOA_SESSION_STATE_UNDEFINED:      return "UNDEFINED";
    case RVOA_SESSION_STATE_IDLE:           return "IDLE";
    case RVOA_SESSION_STATE_OFFER_READY:    return "OFFER_READY";
    case RVOA_SESSION_STATE_ANSWER_RCVD:    return "ANSWER_RCVD";
    case RVOA_SESSION_STATE_ANSWER_READY:   return "ANSWER_READY";
    default:                                return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrStreamState
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaStreamState enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaStreamState enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrStreamState(
                                        IN RvOaStreamState  eState)
{
    switch (eState)
    {
    case RVOA_STREAM_STATE_UNDEFINED:   return "UNDEFINED";
    case RVOA_STREAM_STATE_IDLE:        return "IDLE";
    case RVOA_STREAM_STATE_ACTIVE:      return "ACTIVE";
    case RVOA_STREAM_STATE_HELD:        return "HELD";
    case RVOA_STREAM_STATE_RESUMED:     return "RESUMED";
    case RVOA_STREAM_STATE_REMOVED:     return "REMOVED";
    default:                            return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrLogSource
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaLogSource enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaLogSource enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrLogSource(
                                                IN RvOaLogSource  eLogSource)
{
    switch (eLogSource)
    {
    case RVOA_LOG_SRC_GENERAL: return "OA_GENERAL";
    case RVOA_LOG_SRC_CAPS:    return "OA_CAPS";
    case RVOA_LOG_SRC_MSGS:    return "OA_MSGS";
    default:                   return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertEnum2StrCodec
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaCodec enumeration to string.
 *
 * Arguments:
 * Input:  eCodec - enumeration value to be converted.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrCodec(IN RvOaCodec eCodec)
{
    switch (eCodec)
    {
        case RVOA_CODEC_G711:       return "G711";
        case RVOA_CODEC_G722:       return "G722";
        case RVOA_CODEC_G729:       return "G729";
        case RVOA_CODEC_G7231:      return "G7231";
        case RVOA_CODEC_H261:       return "H261";
        case RVOA_CODEC_H263_2190:  return "H263(RFC2190)";
        default:
            return "UNKNOWN";
    }
}

/******************************************************************************
 * OaUtilsConvertStr2EnumCodec
 * ----------------------------------------------------------------------------
 * General:
 *  Converts string to value of RvOaCodec enumeration.
 *
 * Arguments:
 * Input:  strCodec - string to be converted.
 * Output: none.
 *
 * Return Value: enumeration value.
 *****************************************************************************/
RvOaCodec RVCALLCONV OaUtilsConvertStr2EnumCodec(IN const RvChar* strCodec)
{
    RvOaCodec eCodec = RVOA_CODEC_UNKNOWN;

    if (NULL == strCodec)
    {
        return RVOA_CODEC_UNKNOWN;
    }

    if (0==strcmp(strCodec, "0") || NULL!=strstr(strCodec,"711"))
    {
        eCodec = RVOA_CODEC_G711;
    }
    else if (0==strcmp(strCodec, "9") || NULL!=strstr(strCodec,"722"))
    {
        eCodec = RVOA_CODEC_G722;
    }
    else if (0==strcmp(strCodec, "18") || NULL!=strstr(strCodec,"729"))
    {
        eCodec = RVOA_CODEC_G729;
    }
    else if (NULL != strstr(strCodec,"7231"))
    {
        eCodec = RVOA_CODEC_G7231;
    }
    else if (NULL != strstr(strCodec,"261"))
    {
        eCodec = RVOA_CODEC_H261;
    }
    else if (NULL != strstr(strCodec,"263"))
    {
        eCodec = RVOA_CODEC_H263_2190;
    }
    return eCodec;
}

/******************************************************************************
 * OaUtilsOaSdpMsgConstruct
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 * Output: pOaSdpMsg  - constructed Offer-Answer SDP message.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstruct(
                                    IN  HRPOOL        hPool,
                                    IN  RvLogSource*  pLogSource,
                                    OUT OaSdpMsg*     pOaSdpMsg)
{
    RvStatus rv;

    /* Allocate new SDP Message object and Allocator object to serve it */
    rv = OaSdpMsgReallocate(hPool,pLogSource,pOaSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaSdpMsgConstruct - failed to reallocate SDP message(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Construct message */
    pOaSdpMsg->pSdpMsg = rvSdpMsgConstructA(&pOaSdpMsg->sdpMsg,
                                            pOaSdpMsg->pSdpAllocator);
    if (NULL == pOaSdpMsg->pSdpMsg)
    {
        RvLogError(pLogSource,(pLogSource,
            "OaUtilsOaSdpMsgConstruct - failed to construct message"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/******************************************************************************
 * OaUtilsOaSdpMsgConstructParse
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure. Parses provided SDP message in string form
 *  into the constructed message.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 *         pOaSdpMsg  - Offer-Answer SDP message.
 *         strSdpMsg  - SDP message in string form.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstructParse(
                                    IN HRPOOL        hPool,
                                    IN RvLogSource*  pLogSource,
                                    IN OaSdpMsg*     pOaSdpMsg,
                                    IN RvChar*       strSdpMsg)
{
    RvStatus         rv;
    RvInt            lenOfStrSdpMsg;
    RvSdpParseStatus eParseStatus;

    /* Allocate new SDP Message object and Allocator object to serve it */
    rv = OaSdpMsgReallocate(hPool,pLogSource,pOaSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaSdpMsgConstructParse - failed to reallocate SDP message(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Parse the SDP message string into the allocated Message */
    lenOfStrSdpMsg = (RvInt)strlen(strSdpMsg);
    pOaSdpMsg->pSdpMsg = rvSdpMsgConstructParseA(
                            &pOaSdpMsg->sdpMsg, strSdpMsg, &lenOfStrSdpMsg,
                            &eParseStatus, pOaSdpMsg->pSdpAllocator);
    if (NULL == pOaSdpMsg->pSdpMsg)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaSdpMsgConstructParse - failed to parse SDP message string(status=%d:%s)",
            eParseStatus,
            OaUtilsConvertEnum2StrSdpParseStatus(eParseStatus)));
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/******************************************************************************
 * OaUtilsOaSdpMsgConstructCopy
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure. Copies SDP message in form of SDP Stack message into
 *  the constructed message.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 *         pOaSdpMsg  - Offer-Answer SDP message.
 *         pSdpMsg    - SDP message in form of SDP Stack Message object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstructCopy(
                                    IN HRPOOL        hPool,
                                    IN RvLogSource*  pLogSource,
                                    IN OaSdpMsg*     pOaSdpMsg,
                                    IN RvSdpMsg*     pSdpMsg)
{
    RvStatus   rv;

    /* Allocate new SDP Message object and Allocator object to serve it */
    rv = OaSdpMsgReallocate(hPool,pLogSource,pOaSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaSdpMsgConstructCopy - failed to reallocate SDP message(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Copy the message into the newly allocated message */
    pOaSdpMsg->pSdpMsg = rvSdpMsgConstructCopyA(&pOaSdpMsg->sdpMsg, pSdpMsg,
                                                pOaSdpMsg->pSdpAllocator);
    if (NULL == pOaSdpMsg->pSdpMsg)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaSdpMsgConstructCopy - failed to Copy-Construct SDP message"));
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/******************************************************************************
 * OaUtilsOaSdpMsgDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Offer-Answer SDP Message object:
 *  destructs contained SDP Stack message and SDP Stack allocator.
 *
 * Arguments:
 * Input:  pOaSdpMsg  - Offer-Answer SDP message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaUtilsOaSdpMsgDestruct(IN OaSdpMsg* pOaSdpMsg)
{
    if (NULL != pOaSdpMsg->pSdpMsg)
    {
        rvSdpMsgDestruct(pOaSdpMsg->pSdpMsg);
        pOaSdpMsg->pSdpMsg = NULL;
    }
    /* Destruct the allocator serving the message */
    if (NULL != pOaSdpMsg->pSdpAllocator)
    {
        RvSdpAllocDestruct(pOaSdpMsg->pSdpAllocator);
        pOaSdpMsg->pSdpAllocator = NULL;
    }
}

/******************************************************************************
 * OaUtilsOaPSdpMsgConstructParse
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer Pool SDP Message object, while freeing object
 *  resources, if the object was already in use. Constructs SDP Stack Allocator
 *  and SDP Stack Message objects in the memory, provided by external pools.
 *  Parses provided SDP message in string form into the constructed message.
 *
 * Arguments:
 * Input:  hRaAllocators - pool to be used while allocating Allocator object.
 *                         This pool provides memory for the SDP Stack
 *                         Allocator object used by Pool SDP Message.
 *         hRaMsgs       - pool to be used while constructing SDP Message
 *                         object. This pool provides memory for the SDP Stack
 *                         Message object used by Pool SDP Message.
 *         hPool         - pool to be used while constructing Allocator.
 *                         This pool provides memory for strings, kept by SDP
 *                         Message object.
 *         pOaSdpMsg     - Offer-Answer SDP message.
 *         strSdpMsg     - SDP message in string form.
 *         pLogSource    - log source to be used for log printing.
 *         pMsgOwner     - owner of the message, which is printed into log.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaPSdpMsgConstructParse(
                                    IN HRA           hRaAllocators,
                                    IN HRA           hRaMsgs,
                                    IN HRPOOL        hPool,
                                    IN OaPSdpMsg*    pOaSdpMsg,
                                    IN RvChar*       strSdpMsg,
                                    IN RvLogSource*  pLogSource,
                                    IN void*         pMsgOwner)
{
    RvStatus         rv;
    RvInt            lenOfStrSdpMsg;
    RvSdpParseStatus eParseStatus;
    RvSdpMsg*        pConstructedSdpMsg;

    /* Allocate new SDP Message object and Allocator object to serve it */
    rv = OaPSdpMsgReallocate(hRaAllocators, hRaMsgs, hPool,
                             pLogSource, pMsgOwner, pOaSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaPSdpMsgConstructParse - failed to reallocate SDP message(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Parse the SDP message string into the allocated Message */
    lenOfStrSdpMsg = (RvInt)strlen(strSdpMsg);
    pConstructedSdpMsg = rvSdpMsgConstructParseA(
                            pOaSdpMsg->pSdpMsg, strSdpMsg, &lenOfStrSdpMsg,
                            &eParseStatus, pOaSdpMsg->pSdpAllocator);
    if (NULL == pConstructedSdpMsg)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaPSdpMsgConstructParse - failed to parse SDP message string(status=%d:%s)",
            eParseStatus,
            OaUtilsConvertEnum2StrSdpParseStatus(eParseStatus)));
        return RV_ERROR_UNKNOWN;
    }

    pOaSdpMsg->bSdpMsgConstructed = RV_TRUE;

    return RV_OK;
}

/******************************************************************************
 * OaUtilsOaPSdpMsgConstructCopy
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer Pool SDP Message object, while freeing object
 *  resources, if the object was already in use. Constructs SDP Stack Allocator
 *  and SDP Stack Message objects in the memory, provided by external pools.
 *  Copies provided SDP message in form of SDP Stack Message object into
 *  the constructed message.
 *
 * Arguments:
 * Input:  hRaAllocators - pool to be used while allocating Allocator object.
 *                         This pool provides memory for the SDP Stack
 *                         Allocator object used by Pool SDP Message.
 *         hRaMsgs       - pool to be used while constructing SDP Message
 *                         object. This pool provides memory for the SDP Stack
 *                         Message object used by Pool SDP Message.
 *         hPool         - pool to be used while constructing Allocator.
 *                         This pool provides memory for strings, kept by SDP
 *                         Message object.
 *         pOaSdpMsg     - Offer-Answer SDP message.
 *         strSdpMsg     - SDP message in string form.
 *         pLogSource    - log source to be used for log printing.
 *         pMsgOwner     - owner of the message, which is printed into log.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaPSdpMsgConstructCopy(
                                    IN HRA           hRaAllocators,
                                    IN HRA           hRaMsgs,
                                    IN HRPOOL        hPool,
                                    IN OaPSdpMsg*    pOaSdpMsg,
                                    IN RvSdpMsg*     pSdpMsg,
                                    IN RvLogSource*  pLogSource,
                                    IN void*         pMsgOwner)
{
    RvStatus   rv;
    RvSdpMsg*  pConstructedSdpMsg;

    /* Allocate new SDP Message object and Allocator object to serve it */
    rv = OaPSdpMsgReallocate(hRaAllocators, hRaMsgs, hPool,
                             pLogSource, pMsgOwner, pOaSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaPSdpMsgConstructCopy - failed to reallocate SDP message(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Copy the message into the newly allocated message */
    pConstructedSdpMsg = rvSdpMsgConstructCopyA(pOaSdpMsg->pSdpMsg, pSdpMsg,
                                                pOaSdpMsg->pSdpAllocator);
    if (NULL == pConstructedSdpMsg)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaUtilsOaPSdpMsgConstructCopy - failed to Copy-Construct SDP message"));
        return RV_ERROR_UNKNOWN;
    }

    pOaSdpMsg->bSdpMsgConstructed = RV_TRUE;

    return RV_OK;
}

/******************************************************************************
 * OaUtilsOaPSdpMsgDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Offer-Answer Pool SDP Message object:
 *  destructs contained SDP Stack Message object and SDP Stack Allocator object,
 *  frees memory, used by these objects.
 *
 * Arguments:
 * Input:  pOaSdpMsg - Offer-Answer Pool SDP message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaUtilsOaPSdpMsgDestruct(IN OaPSdpMsg*  pOaSdpMsg)
{
    if (RV_TRUE == pOaSdpMsg->bSdpMsgConstructed)
    {
        rvSdpMsgDestruct(pOaSdpMsg->pSdpMsg);
        pOaSdpMsg->bSdpMsgConstructed = RV_FALSE;
    }
    if (NULL != pOaSdpMsg->pSdpMsg)
    {
        RA_DeAlloc(pOaSdpMsg->hRaMsgs, (RA_Element)pOaSdpMsg->pSdpMsg);
        pOaSdpMsg->pSdpMsg = NULL;
    }
    if (NULL != pOaSdpMsg->pSdpAllocator)
    {
        RvSdpAllocDestruct(pOaSdpMsg->pSdpAllocator);
        RA_DeAlloc(pOaSdpMsg->hRaAllocators,(RA_Element)pOaSdpMsg->pSdpAllocator);
    }
}

/******************************************************************************
 * OaUtilsCompareCodecNames
 * ----------------------------------------------------------------------------
 * General:
 *  Compares two strings that contain names of codecs:
 *  1. performs case insensitive comparison of strings
 *  2. ignores dots while comparing
 *
 * Arguments:
 * Input:  strName1 - the codec name.
 *         strName2 - the codec name.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the strings describe same codec,
 *               RV_FALSE - otherwise.
 *****************************************************************************/
RvBool RVCALLCONV OaUtilsCompareCodecNames(IN const RvChar* strName1,
                                           IN const RvChar* strName2)
{
    const RvChar* p1 = strName1;
    const RvChar* p2 = strName2;
    RvChar  ch1, ch2;
    RvBool  bEqual = RV_TRUE;
    RvChar  deltaLowUpperCase = 'a'-'A';

    while (*p1 != '\0'  &&  *p2 != '\0')
    {
        /* Skip dots on string 1 */
        while (*p1 == '.')
            p1++;
        /* Skip dots on string 2 */
        while (*p2 == '.')
            p2++;

        /* Compare current characters */
        if (*p1 == *p2)
        {
            p1++; p2++;
            continue;
        }

        /* If the current characters are not equal, try to normalize them
        and check again. Use temporary variables ch1 and ch2 in order to
        prevent modification of source strings. */
        if (*p1 > 'Z')
        {
            ch1 = (RvChar)(*p1 - deltaLowUpperCase);
        }
        else
        {
            ch1 = *p1;
        }
        if (*p2 > 'Z')
        {
            ch2 = (RvChar)(*p2 - deltaLowUpperCase);
        }
        else
        {
            ch2 = *p2;
        }
        if (ch1 != ch2)
        {
            bEqual = RV_FALSE;
            break;
        }
        p1++; p2++;
    } /* ENDOF:  while (*p1 != '\0'  &&  *p2 != '\0') */

    /* Check for string lengths */
    /* before this skip the tail dots */
    while (*p1 == '.')
        p1++;
    while (*p2 == '.')
        p2++;
    if (RV_TRUE == bEqual && (*p1 != '\0'  ||  *p2 != '\0') )
    {
        bEqual = RV_FALSE;
    }

    return bEqual;
}

/*---------------------------------------------------------------------------*/
/*                        STATIC FUNCTION IMPLEMENTATIONS                    */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaSdpMsgReallocate
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs SDP Message and SDP Allocator contained in Offer-Answer SDP
 *  Message object, if they were constructed.
 *  Constructs SDP Message and SDP Allocator objects in memory, provided by
 *  Offer-Answer SDP Message object.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 * Output: pOaSdpMsg  - Offer-Answer SDP message.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV OaSdpMsgReallocate(
                                    IN    HRPOOL        hPool,
                                    IN    RvLogSource*  pLogSource,
                                    OUT   OaSdpMsg*     pOaSdpMsg)
{
    RvStatus rv;

    /* Destruct the message */
    OaUtilsOaSdpMsgDestruct(pOaSdpMsg);

    /* Construct the Allocator */
    rv = RvSdpAllocConstruct(hPool, &pOaSdpMsg->sdpAllocator);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaSdpMsgReallocate - failed to construct Allocator(status=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }
    pOaSdpMsg->pSdpAllocator = &pOaSdpMsg->sdpAllocator;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSource);
#endif

	return RV_OK;
}

/******************************************************************************
 * OaPSdpMsgReallocate
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs SDP Message and SDP Allocator contained in Offer-Answer SDP
 *  Message object, and frees memory allocated for them.
 *  Allocates memory for SDP Allocator and SDP Message object from external
 *  pool. Constructs SDP Message and SDP Allocator objects in this memory.
 *
 * Arguments:
 * Input:  hRaAllocators - pool to be used while allocating Allocator object.
 *         hRaMsgs       - pool to be used while allocating SDP Message object.
 *         hPool         - pool to be used while constructing SDP Message.
 *                         This pool provides memory for strings and subobjects
 *                         kept by SDP Message object.
 *         pLogSource    - log source to be used for log printing.
 *         pMsgOwner     - owner of the message that is printed into log.
 * Output: pOaSdpMsg     - Offer-Answer SDP message.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV OaPSdpMsgReallocate(
                                    IN    HRA           hRaAllocators,
                                    IN    HRA           hRaMsgs,
                                    IN    HRPOOL        hPool,
                                    IN    RvLogSource*  pLogSource,
                                    IN    void*         pMsgOwner,
                                    INOUT OaPSdpMsg*    pOaSdpMsg)
{
    RvStatus rv;

    /* Destruct the message and free its memory */
    OaUtilsOaPSdpMsgDestruct(pOaSdpMsg);

    /* Allocate a new Allocator */
    rv = RA_Alloc(hRaAllocators, (RA_Element*)&pOaSdpMsg->pSdpAllocator);
    if (rv != RV_OK)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaPSdpMsgReallocate - failed to allocate Allocator(status=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }
    pOaSdpMsg->hRaAllocators = hRaAllocators;

    /* Construct the Allocator */
    rv = RvSdpAllocConstruct(hPool, pOaSdpMsg->pSdpAllocator);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaPSdpMsgReallocate - failed to construct Allocator(status=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Allocate a new SDP Message object */
    rv = RA_Alloc(hRaMsgs, (RA_Element*)&pOaSdpMsg->pSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pLogSource, (pLogSource,
            "OaPSdpMsgReallocate - failed to allocate SDP Message(status=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }
    pOaSdpMsg->hRaMsgs = hRaMsgs;

    RvLogDebug(pLogSource, (pLogSource,
        "OaPSdpMsgReallocate - object %p uses RA elements: %p for allocator, %p for message object",
        pMsgOwner, pOaSdpMsg->pSdpAllocator, pOaSdpMsg->pSdpMsg));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARGS((pMsgOwner,pLogSource));
#endif

    return RV_OK;
}

/*nl for linux */

