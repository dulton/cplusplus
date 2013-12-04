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
 *                              <SipCommon.c>
 *
 *  The SipCommonCore.c wraps common core function and converts type defs and
 *  status code.
 *  layers.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  Oct 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "rvstdio.h"
#include "RvSipCommonTypes.h"
#include "RvSipCallLegTypes.h"
#ifdef RV_SIP_IMS_ON
#include "rvimsipsec.h"
#endif

/*-----------------------------------------------------------------------*/
/*                              MACROS                                   */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                      LOG  FUNCTIONS                                   */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SipCommonConvertSipToCoreLogMask
* ------------------------------------------------------------------------
* General: converts a sip log mask to core log mask.
* Return Value:core log mask
* ------------------------------------------------------------------------
* Arguments: sipMask - sip log mask
***************************************************************************/
RvLogMessageType RVCALLCONV SipCommonConvertSipToCoreLogMask(
                                            IN  RvInt32    sipMask)
{
    RvLogMessageType coreFilters;

    coreFilters = RV_LOGLEVEL_NONE;
    if(sipMask & RVSIP_LOG_DEBUG_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_DEBUG;
    }
    if(sipMask & RVSIP_LOG_INFO_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_INFO;
    }
    if(sipMask & RVSIP_LOG_WARN_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_WARNING;
    }
    if(sipMask & RVSIP_LOG_ERROR_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_ERROR;
    }
    if(sipMask & RVSIP_LOG_EXCEP_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_EXCEP;
    }
    if(sipMask & RVSIP_LOG_LOCKDBG_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_SYNC;
    }
    if(sipMask & RVSIP_LOG_ENTER_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_ENTER;
    }
    if(sipMask & RVSIP_LOG_LEAVE_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_LEAVE;
    }
    return coreFilters;
}

/***************************************************************************
* SipCommonConvertSipToCoreLogFilter
* ------------------------------------------------------------------------
* General: converts a sip log filter to core log filter..
* Return Value:core log filter
* ------------------------------------------------------------------------
* Arguments: eSipFilter - sip log filter
***************************************************************************/
RvLogMessageType RVCALLCONV SipCommonConvertSipToCoreLogFilter(
                                            RvSipLogFilters eSipFilter)
{
    switch(eSipFilter)
    {
    case RVSIP_LOG_DEBUG_FILTER:
        return RV_LOGLEVEL_DEBUG;
    case RVSIP_LOG_INFO_FILTER:
        return RV_LOGLEVEL_INFO;
    case RVSIP_LOG_WARN_FILTER:
        return RV_LOGLEVEL_WARNING;
    case RVSIP_LOG_ERROR_FILTER:
        return RV_LOGLEVEL_ERROR;
    case RVSIP_LOG_EXCEP_FILTER:
        return RV_LOGLEVEL_EXCEP;
    case RVSIP_LOG_LOCKDBG_FILTER:
        return RV_LOGLEVEL_SYNC;
    case RVSIP_LOG_ENTER_FILTER:
        return RV_LOGLEVEL_ENTER;
    case RVSIP_LOG_LEAVE_FILTER:
        return RV_LOGLEVEL_LEAVE;
    default:
        return RV_LOGLEVEL_NONE;
    }
}
/***************************************************************************
* SipCommonConvertCoreToSipLogFilter
* ------------------------------------------------------------------------
* General: converts a core log filter to sip log filter..
*          Important note: The function converts log id of the common core
*          and not log level.
* Return Value:sip log filter
* ------------------------------------------------------------------------
* Arguments: eCoreFilter - core log filter
***************************************************************************/
RvSipLogFilters RVCALLCONV SipCommonConvertCoreToSipLogFilter(
                                             RvInt eCoreFilter)
{
    switch(eCoreFilter)
    {
    case RV_LOGID_DEBUG:
        return RVSIP_LOG_DEBUG_FILTER;
    case  RV_LOGID_INFO:
        return RVSIP_LOG_INFO_FILTER;
    case RV_LOGID_WARNING:
        return RVSIP_LOG_WARN_FILTER;
    case RV_LOGID_ERROR:
        return RVSIP_LOG_ERROR_FILTER;
    case RV_LOGID_EXCEP:
        return RVSIP_LOG_EXCEP_FILTER;
    case RV_LOGID_SYNC:
        return RVSIP_LOG_LOCKDBG_FILTER;
    case RV_LOGID_ENTER:
        return RVSIP_LOG_ENTER_FILTER;
    case RV_LOGID_LEAVE:
        return RVSIP_LOG_LEAVE_FILTER;
    default:
        return RVSIP_LOG_DEBUG_FILTER;
    }
}

/***************************************************************************
* SipCommonConvertStatusToSipStatus
* ------------------------------------------------------------------------
* General: Converts a core status to sip status.
*          The function gets the error code from the
*          core status and converts it to sip status.
* Return Value: a sip status code.
* ------------------------------------------------------------------------
* Arguments:
* Input:     coreStatus - a core status code
***************************************************************************/
RvStatus RVCALLCONV SipCommonConvertStatusToSipStatus(
                                    IN RvStatus coreStatus)
{
    RvStatus internalStatus = RvErrorGetCode(coreStatus);
    if(coreStatus == RV_OK)
    {
        return RV_OK;
    }

    switch(internalStatus)
    {
    case RV_ERROR_UNKNOWN:
    case RV_ERROR_OUTOFRESOURCES:
    case RV_ERROR_BADPARAM:
    case RV_ERROR_NULLPTR:
    case RV_ERROR_OUTOFRANGE:
    case RV_ERROR_DESTRUCTED:
    case RV_ERROR_NOTSUPPORTED:
    case RV_ERROR_UNINITIALIZED:
    case RV_ERROR_TRY_AGAIN:
        return internalStatus;
    default:
        return RV_ERROR_UNKNOWN;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrMethodType
* ------------------------------------------------------------------------
* General: Converts a SIP method type enumeration value to string.
* Return Value: a string, representing a SIP method type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eMethodType - a SIP method type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrMethodType(
                                    IN RvSipMethodType eMethodType)
{
    switch(eMethodType)
    {
        case RVSIP_METHOD_INVITE:    return "INVITE";
        case RVSIP_METHOD_ACK:       return "ACK";
        case RVSIP_METHOD_BYE:       return "BYE";
        case RVSIP_METHOD_REGISTER:  return "REGISTER";
		case RVSIP_METHOD_REFER:     return "REFER";
        case RVSIP_METHOD_NOTIFY:    return "NOTIFY";
        case RVSIP_METHOD_OTHER:     return "OTHER";
        case RVSIP_METHOD_PRACK:     return "PRACK";
        case RVSIP_METHOD_CANCEL:    return "CANCEL";
        case RVSIP_METHOD_SUBSCRIBE: return "SUBSCRIBE";
        case RVSIP_METHOD_UNDEFINED: return "undefined";
    }
    return NULL;
}

/******************************************************************************
* SipCommonConvertEnumToStrAuthAlgorithm
* ------------------------------------------------------------------------
* General: Converts an Auth-Algorithm enumeration value to string.
* Return Value: a string, representing an Auth-Algorithm.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eAuthAlgorithm - An Auth-Algorithm enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrAuthAlgorithm(
                                    IN RvSipAuthAlgorithm eAuthAlgorithm)
{
	switch(eAuthAlgorithm)
    {
        case RVSIP_AUTH_ALGORITHM_MD5:       return "MD5";
        case RVSIP_AUTH_ALGORITHM_OTHER:     return "other";
        case RVSIP_AUTH_ALGORITHM_UNDEFINED: return "undefined";
    }
    return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumAuthAlgorithm
* ------------------------------------------------------------------------
* General: Converts an Auth-Algorithm string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing an Auth-Algorithm.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strAuthAlgorithm - Auth-Algorithm string
***************************************************************************/
RvSipAuthAlgorithm RVCALLCONV SipCommonConvertStrToEnumAuthAlgorithm(
										IN RvChar*      strAuthAlgorithm)
{
	if (SipCommonStricmp(strAuthAlgorithm,"MD5") == RV_TRUE)
    {
        return RVSIP_AUTH_ALGORITHM_MD5;
    }
    else
    {
        return RVSIP_AUTH_ALGORITHM_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrAuthQopOption
* ------------------------------------------------------------------------
* General: Converts an Auth-Qop-Option enumeration value to string.
* Return Value: a string, representing an Auth-Qop-Option.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eAuthQopOption - An Auth-Qop-Option enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrAuthQopOption(
											IN RvSipAuthQopOption eAuthAlgorithm)
{
	switch(eAuthAlgorithm)
    {
	case RVSIP_AUTH_QOP_AUTH_ONLY:        return "auth";
	case RVSIP_AUTH_QOP_AUTHINT_ONLY:     return "auth-int";
	case RVSIP_AUTH_QOP_AUTH_AND_AUTHINT: return "auth,auth-int";
	case RVSIP_AUTH_QOP_OTHER:            return "other";
	case RVSIP_AUTH_QOP_UNDEFINED:        return "undefined";
    }
    return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumAuthQopOption
* ------------------------------------------------------------------------
* General: Converts an Auth-Qop-Option string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing an Auth-Qop-Option.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strAuthQopOption - Auth-Qop-Option string
***************************************************************************/
RvSipAuthQopOption RVCALLCONV SipCommonConvertStrToEnumAuthQopOption(
										IN RvChar*      strAuthQopOption)
{
	if (SipCommonStricmp(strAuthQopOption,"auth") == RV_TRUE)
    {
        return RVSIP_AUTH_QOP_AUTH_ONLY;
    }
	if (SipCommonStricmp(strAuthQopOption,"auth-int") == RV_TRUE)
    {
        return RVSIP_AUTH_QOP_AUTHINT_ONLY;
    }
	if (SipCommonStricmp(strAuthQopOption,"auth,auth-int") == RV_TRUE)
    {
        return RVSIP_AUTH_QOP_AUTH_AND_AUTHINT;
    }
    else
    {
        return RVSIP_AUTH_QOP_OTHER;
    }
}

#ifdef RV_SIP_JSR32_SUPPORT

/***************************************************************************
* SipCommonConvertStrToEnumMethodType
* ------------------------------------------------------------------------
* General: Converts SIP method type string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a SIP method type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strMethodType - SIP method type string
***************************************************************************/
RvSipMethodType RVCALLCONV SipCommonConvertStrToEnumMethodType(
										IN RvChar*      strMethodType)
{
	if (SipCommonStricmp(strMethodType,"INVITE") == RV_TRUE)
    {
        return RVSIP_METHOD_INVITE;
    }
    else if (SipCommonStricmp(strMethodType,"ACK") == RV_TRUE)
    {
        return RVSIP_METHOD_ACK;
    }
    else if (SipCommonStricmp(strMethodType,"BYE") == RV_TRUE)
    {
        return RVSIP_METHOD_BYE;
    }
    else if (SipCommonStricmp(strMethodType,"REGISTER") == RV_TRUE)
    {
        return RVSIP_METHOD_REGISTER;
    }
	else if (SipCommonStricmp(strMethodType,"PUBLISH") == RV_TRUE)
    {
        return RVSIP_METHOD_PUBLISH;
    }
	else if (SipCommonStricmp(strMethodType,"REFER") == RV_TRUE)
    {
        return RVSIP_METHOD_REFER;
    }
	else if (SipCommonStricmp(strMethodType,"NOTIFY") == RV_TRUE)
    {
        return RVSIP_METHOD_NOTIFY;
    }
	else if (SipCommonStricmp(strMethodType,"PRACK") == RV_TRUE)
    {
        return RVSIP_METHOD_PRACK;
    }
    else if (SipCommonStricmp(strMethodType,"CANCEL") == RV_TRUE)
    {
        return RVSIP_METHOD_CANCEL;
    }
    else if (SipCommonStricmp(strMethodType,"SUBSCRIBE") == RV_TRUE)
    {
        return RVSIP_METHOD_SUBSCRIBE;
    }
    else
    {
        return RVSIP_METHOD_OTHER;
    }

}

/******************************************************************************
* SipCommonConvertEnumToStrAuthScheme
* ------------------------------------------------------------------------
* General: Converts an Auth-Scheme enumeration value to string.
* Return Value: a string, representing an Auth-Scheme.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eAuthScheme - An Auth-Scheme enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrAuthScheme(
                                    IN RvSipAuthScheme eAuthScheme)
{
	switch(eAuthScheme)
    {
        case RVSIP_AUTH_SCHEME_DIGEST:    return "digest";
        case RVSIP_AUTH_SCHEME_OTHER:     return "other";
        case RVSIP_AUTH_SCHEME_UNDEFINED: return "undefined";
    }
    return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumAuthScheme
* ------------------------------------------------------------------------
* General: Converts an Auth-Scheme string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing an Auth-Scheme.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strAuthScheme - Auth-Scheme string
***************************************************************************/
RvSipAuthScheme RVCALLCONV SipCommonConvertStrToEnumAuthScheme(
										IN RvChar*      strAuthScheme)
{
	if (SipCommonStricmp(strAuthScheme,"digest") == RV_TRUE)
    {
        return RVSIP_AUTH_SCHEME_DIGEST;
    }
    else
    {
        return RVSIP_AUTH_SCHEME_OTHER;
    }
}

/***************************************************************************
* SipCommonConvertEnumToBoolStaleType
* ------------------------------------------------------------------------
* General: Converts an Auth-Stale type enumeration value to boolean.
* Return Value: a string, representing an Auth-Stale.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eAuthStale - Auth stale value (true, false or undefined)
***************************************************************************/
RvBool RVCALLCONV SipCommonConvertEnumToBoolAuthStale(
                                    IN RvSipAuthStale eAuthStale)
{
	switch(eAuthStale) {
	case RVSIP_AUTH_STALE_UNDEFINED:
	case RVSIP_AUTH_STALE_FALSE:
		return RV_FALSE;
	case RVSIP_AUTH_STALE_TRUE:
		return RV_TRUE;
	default:
		return RV_FALSE;
	}
}

/***************************************************************************
* SipCommonConvertBoolToEnumAuthStale
* ------------------------------------------------------------------------
* General: Converts an Auth-Stale value to enumeration.
* Return Value: enumeration representing an Auth-Stale.
* ------------------------------------------------------------------------
* Arguments:
* Input:     bAuthStale - Auth stale value (true, false or undefined)
***************************************************************************/
RvSipAuthStale RVCALLCONV SipCommonConvertBoolToEnumAuthStale(
										IN RvBool      bAuthStale)
{
	if (bAuthStale == RV_TRUE)
    {
        return RVSIP_AUTH_STALE_TRUE;
    }
    else
    {
        return RVSIP_AUTH_STALE_FALSE;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrUserParam
* ------------------------------------------------------------------------
* General: Converts a SIP-URL user-param enumeration value to string.
* Return Value: a string, representing the user-param
* ------------------------------------------------------------------------
* Arguments:
* Input:     eUserParam - User-param enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrUserParam(
											IN RvSipUserParam eUserParam)
{
	switch(eUserParam) {
	case RVSIP_USERPARAM_UNDEFINED:
		return "undefined";
	case RVSIP_USERPARAM_PHONE:
		return "phone";
	case RVSIP_USERPARAM_IP:
		return "ip";
	case RVSIP_USERPARAM_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumUserParam
* ------------------------------------------------------------------------
* General: Converts a SIP-URL user-param string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a user-param.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strUserParam - User-param string
***************************************************************************/
RvSipUserParam RVCALLCONV SipCommonConvertStrToEnumUserParam(
										IN RvChar*      strUserParam)
{
	if (SipCommonStricmp(strUserParam,"phone") == RV_TRUE)
    {
        return RVSIP_USERPARAM_PHONE;
    }
	if (SipCommonStricmp(strUserParam,"ip") == RV_TRUE)
    {
        return RVSIP_USERPARAM_IP;
    }
	else
    {
        return RVSIP_USERPARAM_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrCompType
* ------------------------------------------------------------------------
* General: Converts a SIP-URL comp-param enumeration value to string.
* Return Value: a string, representing the comp-param
* ------------------------------------------------------------------------
* Arguments:
* Input:     eCompType - Comp-param enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrCompType(
											IN RvSipCompType eCompType)
{
	switch(eCompType) {
	case RVSIP_COMP_UNDEFINED:
		return "undefined";
	case RVSIP_COMP_SIGCOMP:
		return "sigcomp";
	case RVSIP_COMP_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumCompType
* ------------------------------------------------------------------------
* General: Converts a SIP-URL comp-param string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a comp-param.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strCompType - Comp-param string
***************************************************************************/
RvSipCompType RVCALLCONV SipCommonConvertStrToEnumCompType(
										IN RvChar*      strCompType)
{
	if (SipCommonStricmp(strCompType,"sigcomp") == RV_TRUE)
    {
        return RVSIP_COMP_SIGCOMP;
    }
	else
    {
        return RVSIP_COMP_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrDispositionType
* ------------------------------------------------------------------------
* General: Converts a Contant-Disposition enumeration value to string.
* Return Value: a string, representing the disposition
* ------------------------------------------------------------------------
* Arguments:
* Input:     eDispType - Disposition enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrDispositionType(
											IN RvSipDispositionType eDispType)
{
	switch(eDispType) {
	case RVSIP_DISPOSITIONTYPE_UNDEFINED:
		return "undefined";
	case RVSIP_DISPOSITIONTYPE_RENDER:
		return "render";
	case RVSIP_DISPOSITIONTYPE_SESSION:
		return "session";
	case RVSIP_DISPOSITIONTYPE_ICON:
		return "icon";
	case RVSIP_DISPOSITIONTYPE_ALERT:
		return "alert";
	case RVSIP_DISPOSITIONTYPE_SIGNAL:
		return "signal";
	case RVSIP_DISPOSITIONTYPE_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumDispositionType
* ------------------------------------------------------------------------
* General: Converts a Content-Disposition string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a comp-param.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strDispType - Disposition string
***************************************************************************/
RvSipDispositionType RVCALLCONV SipCommonConvertStrToEnumDispositionType(
											IN RvChar*      strDispType)
{
	if (SipCommonStricmp(strDispType,"render") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONTYPE_RENDER;
    }
	if (SipCommonStricmp(strDispType,"session") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONTYPE_SESSION;
    }
	if (SipCommonStricmp(strDispType,"icon") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONTYPE_ICON;
    }
	if (SipCommonStricmp(strDispType,"alert") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONTYPE_ALERT;
    }
	if (SipCommonStricmp(strDispType,"signal") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONTYPE_SIGNAL;
    }
	else
    {
        return RVSIP_DISPOSITIONTYPE_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrDispositionHandlingType
* ------------------------------------------------------------------------
* General: Converts a Content-Disposition handling enumeration value to string.
* Return Value: a string, representing the handling
* ------------------------------------------------------------------------
* Arguments:
* Input:     eHandlingType - Handling enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrDispositionHandlingType(
											IN RvSipDispositionHandling eHandlingType)
{
	switch(eHandlingType) {
	case RVSIP_DISPOSITIONHANDLING_UNDEFINED:
		return "undefined";
	case RVSIP_DISPOSITIONHANDLING_OPTIONAL:
		return "optional";
	case RVSIP_DISPOSITIONHANDLING_REQUIRED:
		return "required";
	case RVSIP_DISPOSITIONHANDLING_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumDispositionHandlingType
* ------------------------------------------------------------------------
* General: Converts a Content-Disposition handling string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a handling.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strHandlingType - Handling string
***************************************************************************/
RvSipDispositionHandling RVCALLCONV SipCommonConvertStrToEnumDispositionHandlingType(
															IN RvChar*      strHandlingType)
{
	if (SipCommonStricmp(strHandlingType,"optional") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONHANDLING_OPTIONAL;
    }
	if (SipCommonStricmp(strHandlingType,"required") == RV_TRUE)
    {
        return RVSIP_DISPOSITIONHANDLING_REQUIRED;
    }
	else
    {
        return RVSIP_DISPOSITIONHANDLING_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrMediaType
* ------------------------------------------------------------------------
* General: Converts a media-type enumeration value to string.
* Return Value: a string, representing the media type
* ------------------------------------------------------------------------
* Arguments:
* Input:     eMediaType - Media-type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrMediaType(
											IN RvSipMediaType eMediaType)
{
	switch(eMediaType) {
	case RVSIP_MEDIATYPE_UNDEFINED:
		return "undefined";
	case RVSIP_MEDIATYPE_TEXT:
		return "text";
	case RVSIP_MEDIATYPE_IMAGE:
		return "image";
	case RVSIP_MEDIATYPE_AUDIO:
		return "audio";
	case RVSIP_MEDIATYPE_VIDEO:
		return "video";
	case RVSIP_MEDIATYPE_APPLICATION:
		return "application";
	case RVSIP_MEDIATYPE_MULTIPART:
		return "multipart";
	case RVSIP_MEDIATYPE_MESSAGE:
		return "message";
	case RVSIP_MEDIATYPE_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumMediaType
* ------------------------------------------------------------------------
* General: Converts a media-type string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a media-type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strMediaType - Media-type string
***************************************************************************/
RvSipMediaType RVCALLCONV SipCommonConvertStrToEnumMediaType(
													IN RvChar*      strMediaType)
{
	if (SipCommonStricmp(strMediaType,"text") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_TEXT;
    }
	if (SipCommonStricmp(strMediaType,"image") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_IMAGE;
    }
	if (SipCommonStricmp(strMediaType,"audio") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_AUDIO;
    }
	if (SipCommonStricmp(strMediaType,"video") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_VIDEO;
    }
	if (SipCommonStricmp(strMediaType,"application") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_APPLICATION;
    }
	if (SipCommonStricmp(strMediaType,"multipart") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_MULTIPART;
    }
	if (SipCommonStricmp(strMediaType,"message") == RV_TRUE)
    {
        return RVSIP_MEDIATYPE_MESSAGE;
    }
	else
    {
        return RVSIP_MEDIATYPE_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrMediaSubType
* ------------------------------------------------------------------------
* General: Converts a media sub-type enumeration value to string.
* Return Value: a string, representing the media sub-type
* ------------------------------------------------------------------------
* Arguments:
* Input:     eMediaSubType - Media sub-type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrMediaSubType(
											IN RvSipMediaSubType eMediaSubType)
											
{
	switch(eMediaSubType) {
	case RVSIP_MEDIASUBTYPE_UNDEFINED:
		return "undefined";
	case RVSIP_MEDIASUBTYPE_PLAIN:
		return "plain";
	case RVSIP_MEDIASUBTYPE_SDP:
		return "sdp";
	case RVSIP_MEDIASUBTYPE_ISUP:
		return "ISUP";
	case RVSIP_MEDIASUBTYPE_QSIG:
		return "QSIG";
	case RVSIP_MEDIASUBTYPE_MIXED:
		return "mixed";
	case RVSIP_MEDIASUBTYPE_ALTERNATIVE:
		return "alternative";
	case RVSIP_MEDIASUBTYPE_DIGEST:
		return "digest";
	case RVSIP_MEDIASUBTYPE_RFC822:
		return "rfc822";
	case RVSIP_MEDIASUBTYPE_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumMediaSubType
* ------------------------------------------------------------------------
* General: Converts a media sub-type string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a media-type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strMediaSubType - Media sub-type string
***************************************************************************/
RvSipMediaSubType RVCALLCONV SipCommonConvertStrToEnumMediaSubType(
													IN RvChar*      strMediaSubType)
{
	if (SipCommonStricmp(strMediaSubType,"plain") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_PLAIN;
    }
	if (SipCommonStricmp(strMediaSubType,"sdp") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_SDP;
    }
	if (SipCommonStricmp(strMediaSubType,"ISUP") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_ISUP;
    }
	if (SipCommonStricmp(strMediaSubType,"QSIG") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_QSIG;
    }
	if (SipCommonStricmp(strMediaSubType,"mixed") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_MIXED;
    }
	if (SipCommonStricmp(strMediaSubType,"alternative") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_ALTERNATIVE;
    }
	if (SipCommonStricmp(strMediaSubType,"digest") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_DIGEST;
    }
	if (SipCommonStricmp(strMediaSubType,"rfc822") == RV_TRUE)
    {
        return RVSIP_MEDIASUBTYPE_RFC822;
    }
	else
    {
        return RVSIP_MEDIASUBTYPE_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToIntWeekDay
* ------------------------------------------------------------------------
* General: Converts a week-day enumeration value to string.
* Return Value: a string, representing the week-day
* ------------------------------------------------------------------------
* Arguments:
* Input:     eWeekDay - Week-day enumeration
******************************************************************************/
RvInt32 RVCALLCONV SipCommonConvertEnumToIntWeekDay(
											IN RvSipDateWeekDay eWeekDay)
											
{
	return (RvInt32)eWeekDay;
}

/***************************************************************************
* SipCommonConvertIntToEnumWeekDay
* ------------------------------------------------------------------------
* General: Converts a week-day integer value to enumeration.
* Return Value: enumeration representing a week-day.
* ------------------------------------------------------------------------
* Arguments:
* Input:     weekDay - Week-day string
***************************************************************************/
RvSipDateWeekDay RVCALLCONV SipCommonConvertIntToEnumWeekDay(
													IN RvInt32      weekDay)
{
	if (0 == weekDay)
	{
		return RVSIP_WEEKDAY_SUN;
	}
	if (1 == weekDay)
	{
		return RVSIP_WEEKDAY_MON;
	}
	if (2 == weekDay)
	{
		return RVSIP_WEEKDAY_TUE;
	}
	if (3 == weekDay)
	{
		return RVSIP_WEEKDAY_WED;
	}
	if (4 == weekDay)
	{
		return RVSIP_WEEKDAY_THU;
	}
	if (5 == weekDay)
	{
		return RVSIP_WEEKDAY_FRI;
	}
	if (6 == weekDay)
	{
		return RVSIP_WEEKDAY_SAT;
	}

	return RVSIP_WEEKDAY_UNDEFINED;
}

/******************************************************************************
* SipCommonConvertEnumToIntMonth
* ------------------------------------------------------------------------
* General: Converts a month enumeration value to string.
* Return Value: a string, representing the month
* ------------------------------------------------------------------------
* Arguments:
* Input:     eMonth - Month enumeration
******************************************************************************/
RvInt32 RVCALLCONV SipCommonConvertEnumToIntMonth(
											IN RvSipDateMonth eMonth)
											
{
	return (RvInt32)eMonth;
}

/***************************************************************************
* SipCommonConvertIntToEnumMonth
* ------------------------------------------------------------------------
* General: Converts a month integer value to enumeration.
* Return Value: enumeration representing a month.
* ------------------------------------------------------------------------
* Arguments:
* Input:     month - Month string
***************************************************************************/
RvSipMediaSubType RVCALLCONV SipCommonConvertIntToEnumMonth(
													IN RvInt32      month)
{
	if (0 == month)
	{
		return RVSIP_MONTH_JAN;
	}
	if (1 == month)
	{
		return RVSIP_MONTH_FEB;
	}
	if (2 == month)
	{
		return RVSIP_MONTH_MAR;
	}
	if (3 == month)
	{
		return RVSIP_MONTH_APR;
	}
	if (4 == month)
	{
		return RVSIP_MONTH_MAY;
	}
	if (5 == month)
	{
		return RVSIP_MONTH_JUN;
	}
	if (6 == month)
	{
		return RVSIP_MONTH_JUL;
	}
	if (7 == month)
	{
		return RVSIP_MONTH_AUG;
	}
	if (8 == month)
	{
		return RVSIP_MONTH_SEP;
	}
	if (9 == month)
	{
		return RVSIP_MONTH_OCT;
	}
	if (10 == month)
	{
		return RVSIP_MONTH_NOV;
	}
	if (11 == month)
	{
		return RVSIP_MONTH_DEC;
	}
	
	return RVSIP_MONTH_UNDEFINED;
}

/******************************************************************************
* SipCommonConvertEnumToStrSubscriptionSubstate
* ------------------------------------------------------------------------
* General: Converts a Subscription-Substate enumeration value to string.
* Return Value: a string, representing the Subscription-Substate
* ------------------------------------------------------------------------
* Arguments:
* Input:     eSubscriptionSubstate - Subscription-Substate enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrSubscriptionSubstate(
											IN RvSipSubscriptionSubstate eSubscriptionSubstate)
{
	switch(eSubscriptionSubstate) {
	case RVSIP_SUBSCRIPTION_SUBSTATE_UNDEFINED:
		return "undefined";
	case RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE:
		return "active";
	case RVSIP_SUBSCRIPTION_SUBSTATE_PENDING:
		return "pending";
	case RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED:
		return "terminated";
	case RVSIP_SUBSCRIPTION_SUBSTATE_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumSubscriptionSubstate
* ------------------------------------------------------------------------
* General: Converts a Subscription-Substate string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a Subscription-Substate.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strSubscriptionSubstate - Subscription-Substate string
***************************************************************************/
RvSipSubscriptionSubstate RVCALLCONV SipCommonConvertStrToEnumSubscriptionSubstate(
													IN RvChar*      strSubscriptionSubstate)
{
	if (SipCommonStricmp(strSubscriptionSubstate,"active") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE;
    }
	if (SipCommonStricmp(strSubscriptionSubstate,"pending") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_SUBSTATE_PENDING;
    }
	if (SipCommonStricmp(strSubscriptionSubstate,"terminated") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED;
    }
	else
    {
        return RVSIP_SUBSCRIPTION_SUBSTATE_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrSubscriptionReason
* ------------------------------------------------------------------------
* General: Converts a Subscription-Reason enumeration value to string.
* Return Value: a string, representing the Subscription-Reason
* ------------------------------------------------------------------------
* Arguments:
* Input:     eSubscriptionReason - Subscription-Reason enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrSubscriptionReason(
											IN RvSipSubscriptionReason eSubscriptionReason)
{
	switch(eSubscriptionReason) {
	case RVSIP_SUBSCRIPTION_REASON_UNDEFINED:
		return "undefined";
	case RVSIP_SUBSCRIPTION_REASON_DEACTIVATED:
		return "deactivated";
	case RVSIP_SUBSCRIPTION_REASON_PROBATION:
		return "probation";
	case RVSIP_SUBSCRIPTION_REASON_REJECTED:
		return "rejected";
	case RVSIP_SUBSCRIPTION_REASON_TIMEOUT:
		return "timeout";
	case RVSIP_SUBSCRIPTION_REASON_GIVEUP:
		return "giveup";
	case RVSIP_SUBSCRIPTION_REASON_NORESOURCE:
		return "noresource";
	case RVSIP_SUBSCRIPTION_REASON_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumSubscriptionReason
* ------------------------------------------------------------------------
* General: Converts a Subscription-Reason string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a Subscription-Reason.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strSubscriptionReason - Subscription-Reason string
***************************************************************************/
RvSipSubscriptionReason RVCALLCONV SipCommonConvertStrToEnumSubscriptionReason(
													IN RvChar*      strSubscriptionReason)
{
	if (SipCommonStricmp(strSubscriptionReason,"deactivated") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_DEACTIVATED;
    }
	if (SipCommonStricmp(strSubscriptionReason,"probation") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_PROBATION;
    }
	if (SipCommonStricmp(strSubscriptionReason,"rejected") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_REJECTED;
    }
	if (SipCommonStricmp(strSubscriptionReason,"timeout") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_TIMEOUT;
    }
	if (SipCommonStricmp(strSubscriptionReason,"giveup") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_GIVEUP;
    }
	if (SipCommonStricmp(strSubscriptionReason,"noresource") == RV_TRUE)
    {
        return RVSIP_SUBSCRIPTION_REASON_NORESOURCE;
    }
	else
    {
        return RVSIP_SUBSCRIPTION_REASON_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrReasonProtocol
* ------------------------------------------------------------------------
* General: Converts a Reason-Protocol enumeration value to string.
* Return Value: a string, representing the Reason-Protocol
* ------------------------------------------------------------------------
* Arguments:
* Input:     eReasonProtocol - Reason-Protocol enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrReasonProtocol(
											IN RvSipReasonProtocolType eReasonProtocol)
{
	switch(eReasonProtocol) {
	case RVSIP_REASON_PROTOCOL_UNDEFINED:
		return "undefined";
	case RVSIP_REASON_PROTOCOL_SIP:
		return "SIP";
	case RVSIP_REASON_PROTOCOL_Q_850:
		return "Q.850";
	case RVSIP_REASON_PROTOCOL_OTHER:
		return "other";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumReasonProtocol
* ------------------------------------------------------------------------
* General: Converts a Reason-Protocol string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a Reason-Protocol.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strReasonProtocol - Reason-Protocol string
***************************************************************************/
RvSipReasonProtocolType RVCALLCONV SipCommonConvertStrToEnumReasonProtocol(
													IN RvChar*      strReasonProtocol)
{
	if (SipCommonStricmp(strReasonProtocol,"SIP") == RV_TRUE)
    {
        return RVSIP_REASON_PROTOCOL_SIP;
    }
	if (SipCommonStricmp(strReasonProtocol,"Q.850") == RV_TRUE)
    {
        return RVSIP_REASON_PROTOCOL_Q_850;
    }
	else
    {
        return RVSIP_REASON_PROTOCOL_OTHER;
    }
}

/******************************************************************************
* SipCommonConvertEnumToStrContactAction
* ------------------------------------------------------------------------
* General: Converts a Contact-Action enumeration value to string.
* Return Value: a string, representing the Contact-Action
* ------------------------------------------------------------------------
* Arguments:
* Input:     eContactAction - Contact-Action enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrContactAction(
											IN RvSipContactAction eContactAction)
{
	switch(eContactAction) {
	case RVSIP_CONTACT_ACTION_UNDEFINED:
		return "undefined";
	case RVSIP_CONTACT_ACTION_PROXY:
		return "proxy";
	case RVSIP_CONTACT_ACTION_REDIRECT:
		return "redirect";
	}
	return NULL;
}

/***************************************************************************
* SipCommonConvertStrToEnumContactAction
* ------------------------------------------------------------------------
* General: Converts a Contact-Action string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a Contact-Action.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strContactAction - Contact-Action string
***************************************************************************/
RvSipContactAction RVCALLCONV SipCommonConvertStrToEnumContactAction(
													IN RvChar*      strContactAction)
{
	if (SipCommonStricmp(strContactAction,"proxy") == RV_TRUE)
    {
        return RVSIP_CONTACT_ACTION_PROXY;
    }
	if (SipCommonStricmp(strContactAction,"redirect") == RV_TRUE)
    {
        return RVSIP_CONTACT_ACTION_REDIRECT;
    }
	else
    {
        return RVSIP_CONTACT_ACTION_UNDEFINED;
    }
}

#endif /* #ifdef RVSIP_JSR32_SUPPORT */

/******************************************************************************
* SipCommonConvertEnumToStrTransportType
* ------------------------------------------------------------------------
* General: Converts a SIP transport type enumeration value to string.
* Return Value: a string, representing a SIP transport type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eTransportType - a SIP transport type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrTransportType(
                                    IN RvSipTransport eTransportType)
{
    switch(eTransportType)
    {
        case RVSIP_TRANSPORT_UDP:       return "UDP";
        case RVSIP_TRANSPORT_TCP:       return "TCP";
        case RVSIP_TRANSPORT_SCTP:      return "SCTP";
        case RVSIP_TRANSPORT_TLS:       return "TLS";
        case RVSIP_TRANSPORT_OTHER:     return "OTHER";
        case RVSIP_TRANSPORT_UNDEFINED: return "undefined";
    }
    return NULL;
}

/******************************************************************************
* SipCommonConvertStrToEnumTransportType
* ------------------------------------------------------------------------
* General: Converts a string to SIP transport type enumeration value.
* Return Value: a SIP transport type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strTransportType - a string with SIP transport type
*****************************************************************************/
RvSipTransport RVCALLCONV SipCommonConvertStrToEnumTransportType(IN RvChar *strTransportType)
{
    if (SipCommonStricmp(strTransportType,"UDP") == RV_TRUE)
    {
        return RVSIP_TRANSPORT_UDP;
    }
    else if (SipCommonStricmp(strTransportType,"TCP") == RV_TRUE)
    {
        return RVSIP_TRANSPORT_TCP;
    }
    else if (SipCommonStricmp(strTransportType,"SCTP") == RV_TRUE)
    {
        return RVSIP_TRANSPORT_SCTP;
    }
    else if (SipCommonStricmp(strTransportType,"TLS") == RV_TRUE)
    {
        return RVSIP_TRANSPORT_TLS;
    }
    else
    {
        return RVSIP_TRANSPORT_OTHER;
    }
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/******************************************************************************
* SipCommonConvertEnumToBoolEnumdi
* ------------------------------------------------------------------------
* General: Converts an enumdi enumeration value to boolean.
* Return Value: boolean representing an enumdi..
* ------------------------------------------------------------------------
* Arguments:
* Input:     eEnumdi - the enumdi value
******************************************************************************/
RvBool RVCALLCONV SipCommonConvertEnumToBoolEnumdi(
											IN RvSipTelUriEnumdiType eEnumdi)
											
{
    if (eEnumdi == RVSIP_ENUMDI_TYPE_EXISTS_EMPTY)
    {
        return RV_TRUE;
    }
    else
    {
        return RV_FALSE;
    }
}

/***************************************************************************
* SipCommonConvertBoolToEnumEnumdi
* ------------------------------------------------------------------------
* General: Converts an enumdi value to enumeration.
* Return Value: enumeration representing an enumdi.
* ------------------------------------------------------------------------
* Arguments:
* Input:     bEnumdi - the boolean value of enumdi  parameter
***************************************************************************/
RvSipTelUriEnumdiType RVCALLCONV SipCommonConvertBoolToEnumEnumdi(
										IN RvBool      bEnumdi)
{
	if (bEnumdi == RV_TRUE) 
	{
		return RVSIP_ENUMDI_TYPE_EXISTS_EMPTY;
	}
	else
	{
		return RVSIP_ENUMDI_TYPE_UNDEFINED;
	}
}

#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

/******************************************************************************
* SipCommonConvertEnumToStrTransportAddressType
* ------------------------------------------------------------------------
* General: Converts a SIP transport address type enumeration value to string.
* Return Value: a string, representing a SIP transport type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eAddrType - a SIP transport address type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrTransportAddressType(
                                    IN RvSipTransportAddressType eAddrType)
{
    switch(eAddrType)
    {
        case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:        return "IP";
        case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:       return "IP6";
        case RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED: return "undefined";
    }
    return NULL;
}
/******************************************************************************
* SipCommonConvertEnumToStrConnectionType
* ------------------------------------------------------------------------
* General: Converts a SIP transport connection type enumeration value to string.
* Return Value: a string, representing a transport connection type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eTransportType - a SIP transport connection type enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrConnectionType(
                             IN RvSipTransportConnectionType eConnectionType)
{
    switch(eConnectionType)
    {
    case RVSIP_TRANSPORT_CONN_TYPE_CLIENT:       return "client";
    case RVSIP_TRANSPORT_CONN_TYPE_SERVER:       return "server";
    case RVSIP_TRANSPORT_CONN_TYPE_MULTISERVER:  return "listening";
    case RVSIP_TRANSPORT_CONN_TYPE_UNDEFINED:    return "undefined";
    }
    return "";
}

/******************************************************************************
* SipCommonConvertEnumToStrConnectionState
* -----------------------------------------------------------------------------
* General: Converts a SIP transport connection state enumeration value to strin
* Return Value: a string, representing a transport connection state.
* -----------------------------------------------------------------------------
* Arguments:
* Input:     eConnState - a SIP transport connection state enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrConnectionState(
                             IN RvSipTransportConnectionState eConnState)
{
    switch(eConnState)
    {
    case RVSIP_TRANSPORT_CONN_STATE_UNDEFINED:      return "UNDEFINED";
    case RVSIP_TRANSPORT_CONN_STATE_IDLE:           return "IDLE";
    case RVSIP_TRANSPORT_CONN_STATE_READY:          return "READY";
    case RVSIP_TRANSPORT_CONN_STATE_CONNECTING:     return "CONNECTING";
    case RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED: return "CONNECTED";
    case RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED:  return "TCP_CONNECTED";
    case RVSIP_TRANSPORT_CONN_STATE_CLOSING:        return "CLOSING";
    case RVSIP_TRANSPORT_CONN_STATE_CLOSED:         return "CLOSED";
    case RVSIP_TRANSPORT_CONN_STATE_TERMINATED:     return "TERMINATED";
    }
    return "";
}

/******************************************************************************
* SipCommonConvertEnumToStrConnectionStateChangeRsn
* -----------------------------------------------------------------------------
* General: Converts a SIP transport connection state chang ereason enumeration
*          value to string.
* Return Value: a string, representing a connection state change reason.
* -----------------------------------------------------------------------------
* Arguments:
* Input:     eConnStateChangeRsn - a SIP transport connection state change
*                                  reason enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrConnectionStateChangeRsn(
            IN RvSipTransportConnectionStateChangedReason eConnStateChangeRsn)
{
    switch(eConnStateChangeRsn)
    {
    case RVSIP_TRANSPORT_CONN_REASON_UNDEFINED:
        return "UNDEFINED";
    case RVSIP_TRANSPORT_CONN_REASON_ERROR:
        return "ERROR";
    case RVSIP_TRANSPORT_CONN_REASON_CLIENT_CONNECTED:
        return "CLIENT_CONNECTED";
    case RVSIP_TRANSPORT_CONN_REASON_SERVER_CONNECTED:
        return "SERVER_CONNECTED";
    case RVSIP_TRANSPORT_CONN_REASON_TLS_POST_CONNECTION_ASSERTION_FAILED:
        return "TLS_POST_CONNECTION_ASSERTION_FAILED";
    case RVSIP_TRANSPORT_CONN_REASON_DISCONNECTED:
        return "DISCONNECTED";
    }
    return "";
}

/******************************************************************************
* SipCommonConvertEnumToStrConnectionStateChangeRsn
* -----------------------------------------------------------------------------
* General: Converts a SIP transport connection status enumeration value to str.
* Return Value: a string, representing a connection status.
* -----------------------------------------------------------------------------
* Arguments:
* Input:     eStatus - a SIP transport connection status enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrConnectionStatus(
                                    IN RvSipTransportConnectionStatus  eStatus)
{
    switch(eStatus)
    {
    case RVSIP_TRANSPORT_CONN_STATUS_UNDEFINED:
        return "UNDEFINED";
    case RVSIP_TRANSPORT_CONN_STATUS_ERROR:
        return "ERROR";
    case RVSIP_TRANSPORT_CONN_STATUS_MSG_SENT:
        return "MSG_SENT";
    case RVSIP_TRANSPORT_CONN_STATUS_MSG_NOT_SENT:
        return "MSG_NOT_SENT";
    case RVSIP_TRANSPORT_CONN_STATUS_SCTP_PEER_ADDR_CHANGED:
        return "SCTP_PEER_ADDR_CHANGED";
    case RVSIP_TRANSPORT_CONN_STATUS_SCTP_REMOTE_ERROR:
        return "SCTP_REMOTE_ERROR";
    }
    return "";
}

#ifndef RV_SIP_PRIMITIVES
/******************************************************************************
* SipCommonConvertEnumToStrTimerRefresher
* ------------------------------------------------------------------------
* General: Converts a session timer refresher preference enumeration value to
*          string.
* Return Value: a string, representing a timer refresher preference.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eRefresher - a timer refresher preference enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrTimerRefresher(
                    IN RvSipCallLegSessionTimerRefresherPreference eRefresher)
{
    switch(eRefresher)
    {
        case RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_NONE:      return "none";
        case RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL:     return "local";
        case RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE:    return "remote";
        case RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE: return "dont care";
    }
    return "";
}
#endif /* #ifndef RV_SIP_PRIMITIVES */ 

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipCommonConvertEnumToStrSecMechanism
 * ----------------------------------------------------------------------------
 * General: returns string, representing name of the Security Mechanism.
 *
 * Return Value: security mechanism name
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eSecMech - mechanism to be converted
 *****************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrSecMechanism(
                                        IN RvSipSecurityMechanismType eSecMech)
{
    switch(eSecMech)
    {
        case RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED:   return "UNDEFINED";
        case RVSIP_SECURITY_MECHANISM_TYPE_DIGEST:      return "Digest";
        case RVSIP_SECURITY_MECHANISM_TYPE_TLS:         return "TLS";
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE:   return "IPSEC-IKE";
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN:   return "IPSEC-MAN";
        case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:  return "IPSEC-3GPP";
        case RVSIP_SECURITY_MECHANISM_TYPE_OTHER:       return "Other";
    }
    return "";
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipCommonConvertEnumToStrSecEncryptAlg
 * ----------------------------------------------------------------------------
 * General: returns string, representing name of the Encryption Algorithm.
 *
 * Return Value: algorithm name
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eEncrAlg - algorithm to be converted
 *****************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrSecEncryptAlg(
                                    IN RvSipSecurityEncryptAlgorithmType eEncrAlg)
{
    switch(eEncrAlg)
    {
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED:    return "UNDEFINED";
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC: return "DES-EDE3-CBC";
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC:      return "AES-CBC";
        case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL:         return "NULL";
    }
    return "";
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipCommonConvertEnumToStrSecAlg
 * ----------------------------------------------------------------------------
 * General: returns string, representing name of the Integrity Algorithm.
 *
 * Return Value: algorithm name
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     eIntegrAlg - algorithm to be converted
 *****************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrSecAlg(
                                    IN RvSipSecurityAlgorithmType eIntegrAlg)
{
    switch(eIntegrAlg)
    {
        case RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED:       return "UNDEFINED";
        case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96:     return "MD5-96";
        case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96:   return "SHA-1-96";
    }
    return "";
}
#endif /* #ifdef RV_SIP_IMS_ON */

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertCoreToSipEncrAlg
* -----------------------------------------------------------------------------
* General: converts a core encryption algorithm to sip algoritm.
* Return Value: sip algorithm.
* -----------------------------------------------------------------------------
* Arguments: eCoreVal - core value to be converted
******************************************************************************/
RvSipSecurityEncryptAlgorithmType
RVCALLCONV SipCommonConvertCoreToSipEncrAlg(IN RvImsEncrAlg eCoreVal)
{
    switch(eCoreVal)
    {
    case RvImsEncrNull:
        return RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL;
    case RvImsEncr3Des:
        return RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC;
    case RvImsEncrAes:
        return RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC;
    case RvImsEncrUndefined:
    default:
        return RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertSipToCoreEncrAlg
* -----------------------------------------------------------------------------
* General: converts a SIP Toolkit encryption algorithm to core algoritm.
* Return Value: core algorithm.
* -----------------------------------------------------------------------------
* Arguments: eSipVal - SIP TK value to be converted
******************************************************************************/
RvImsEncrAlg RVCALLCONV SipCommonConvertSipToCoreEncrAlg(
                                IN RvSipSecurityEncryptAlgorithmType eSipVal)
{
    switch(eSipVal)
    {
    case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL:
        return RvImsEncrNull;
    case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC:
        return RvImsEncr3Des;
    case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC:
        return RvImsEncrAes;
    case RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED:
    default:
        return RvImsEncrUndefined;
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertCoreToSipIntegrAlg
* -----------------------------------------------------------------------------
* General: converts a core integrity algorithm to sip algoritm.
* Return Value: sip algorithm.
* -----------------------------------------------------------------------------
* Arguments: eCoreVal - core value to be converted
******************************************************************************/
RvSipSecurityAlgorithmType
RVCALLCONV SipCommonConvertCoreToSipIntegrAlg(IN RvImsAuthAlg eCoreVal)
{
    switch(eCoreVal)
    {
    case RvImsAuthMd5:
        return RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96;
    case RvImsAuthSha1:
        return RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96;
    case RvImsAuthUndefined:
    default:
        return RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertSipToCoreIntegrAlg
* -----------------------------------------------------------------------------
* General: converts a SIP Toolkit integrity algorithm to core algoritm.
* Return Value: core algorithm.
* -----------------------------------------------------------------------------
* Arguments: eSipVal - SIP TK value to be converted
******************************************************************************/
RvImsAuthAlg RVCALLCONV SipCommonConvertSipToCoreIntegrAlg(
                                IN RvSipSecurityAlgorithmType eSipVal)
{
    switch(eSipVal)
    {
    case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96:
        return RvImsAuthMd5;
    case RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96:
        return RvImsAuthSha1;
    case RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED:
    default:
        return RvImsAuthUndefined;
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertEnumToStrCoreIntegrAlg
* -----------------------------------------------------------------------------
* General: converts a SIP Toolkit Common Core integrity algorithm to string.
* Return Value: string.
* -----------------------------------------------------------------------------
* Arguments: eCoreIAlg - SIP TK Common Core value to be converted
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrCoreIntegrAlg(
                                IN RvImsAuthAlg eCoreIAlg)
{
    switch(eCoreIAlg)
    {
    case RvImsAuthMd5:
        return "MD5";
    case RvImsAuthSha1:
        return "SHA1";
    default:
        return "undefined";
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/

#if (RV_IMS_IPSEC_ENABLED==RV_YES)
/******************************************************************************
* SipCommonConvertEnumToStrCoreEncrAlg
* -----------------------------------------------------------------------------
* General: converts a SIP Toolkit Common Core encryption algorithm to string.
* Return Value: string.
* -----------------------------------------------------------------------------
* Arguments: eCoreIAlg - SIP TK Common Core value to be converted
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrCoreEncrAlg(
                                IN RvImsEncrAlg eCoreEAlg)
{
    switch(eCoreEAlg)
    {
    case RvImsEncrNull:
        return "NULL";
    case RvImsEncr3Des:
        return "3DES";
    case RvImsEncrAes:
        return "AES";
    default:
        return "undefined";
    }
}
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
/******************************************************************************
* SipCommonConvertSipToCoreAddrType
* -----------------------------------------------------------------------------
* General: converts a SIP Toolkit address type to core address type.
* Return Value: core address type.
* -----------------------------------------------------------------------------
* Arguments: eSipAddrType - SIP TK value to be converted
******************************************************************************/
RvInt RVCALLCONV SipCommonConvertSipToCoreAddrType(
                                    IN RvSipTransportAddressType eSipAddrType)
{
    switch(eSipAddrType)
    {
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:
        return RV_ADDRESS_TYPE_IPV4;
    case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:
        return RV_ADDRESS_TYPE_IPV6;
    default:
        return UNDEFINED;
    }
}

