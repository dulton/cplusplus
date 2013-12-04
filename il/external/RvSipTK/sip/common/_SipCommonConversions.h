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
 *                              <_SipCommonConversions.h>
 *
 *  The SipCommon.c file contains Internal API functions used by all the stack
 *  layers.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  Oct 2003
 *********************************************************************************/



#ifndef SIP_COMMON_CONVERT_H
#define SIP_COMMON_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonTypes.h"
#include "RvSipCommonTypes.h"
#include "rvtimestamp.h"
#include "rvmemory.h"
#include "rvmutex.h"
#include "rvselect.h"
#include "rvcbase.h"
#include "rvlog.h"
#include "RvSipMsgTypes.h"
#include "RvSipTransportTypes.h"
#include "RvSipCallLegTypes.h"
#ifdef RV_SIP_IMS_ON
#include "rvimsipsec.h"
#endif
/*-----------------------------------------------------------------------*/
/*                              MACROS                                  */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CRV2RV
* ------------------------------------------------------------------------
* General: Converts a common core status to sip status
* Return Value:RvStatus
* ------------------------------------------------------------------------
* Arguments:
***************************************************************************/
#define CRV2RV(_e) SipCommonConvertStatusToSipStatus(_e)

#define ConnectionTypeToString(_t) (((_t) == RVSIP_TRANSPORT_CONN_TYPE_CLIENT) ? "Client" : "Server")
/*-----------------------------------------------------------------------*/
/*                      LOG  FUNCTIONS                                   */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SipCommonConvertSipToCoreLogMask
* ------------------------------------------------------------------------
* General: converts a sip log mask to core log mask.
* Return Value:core log mask
* ------------------------------------------------------------------------
* Arguments: sipFilters - sip log mask
***************************************************************************/
RvLogMessageType RVCALLCONV SipCommonConvertSipToCoreLogMask(
                                            IN  RvInt32    sipFilters);

/***************************************************************************
* SipCommonConvertSipToCoreLogFilter
* ------------------------------------------------------------------------
* General: converts a sip log filter to core log filter..
* Return Value:core log filter
* ------------------------------------------------------------------------
* Arguments: eSipFilter - sip log filter
***************************************************************************/
RvLogMessageType RVCALLCONV SipCommonConvertSipToCoreLogFilter(
                                            RvSipLogFilters eSipFilter);

/***************************************************************************
* SipCommonConvertCoreToSipLogFilter
* ------------------------------------------------------------------------
* General: converts a core log filter to sip log filter..
* Return Value:sip log filter
* ------------------------------------------------------------------------
* Arguments: eCoreFilter - core log filter
***************************************************************************/
RvSipLogFilters RVCALLCONV SipCommonConvertCoreToSipLogFilter(
                                             RvInt eCoreFilter);

/***************************************************************************
* SipCommonConvertStatusToSipStatus
* ------------------------------------------------------------------------
* General: Converts a core status to sip status.
*          The function retrieves the error code from the
*          core status and converts it to sip status.
* Return Value: a sip status code.
* ------------------------------------------------------------------------
* Arguments:
* Input:     coreStatus - a core status code
***************************************************************************/
RvStatus RVCALLCONV SipCommonConvertStatusToSipStatus(
                                    IN RvStatus coreStatus);

/***************************************************************************
* SipCommonConvertEnumToStrMethodType
* ------------------------------------------------------------------------
* General: Converts a SIP method type enumeration value to string.
* Return Value: a string, representing a SIP method type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     eMethodType - a SIP method type enumeration
***************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrMethodType(
                                    IN RvSipMethodType eMethodType);


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
                                    IN RvSipAuthAlgorithm eAuthAlgorithm);

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
										IN RvChar*      strAuthAlgorithm);

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
											IN RvSipAuthQopOption eAuthAlgorithm);

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
										IN RvChar*      strAuthQopOption);

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
										IN RvChar*      strMethodType);

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
                                    IN RvSipAuthScheme eAuthScheme);

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
										IN RvChar*      strAuthScheme);

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
                                    IN RvSipAuthStale eAuthStale);

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
										IN RvBool      strAuthStale);

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
											IN RvSipUserParam eUserParam);

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
										IN RvChar*      strUserParam);

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
											IN RvSipCompType eCompType);

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
										IN RvChar*      strCompType);

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
											IN RvSipDispositionType eDispType);

/***************************************************************************
* SipCommonConvertStrToEnumDispositionType
* ------------------------------------------------------------------------
* General: Converts a Content-Disposition string value to enumeration.
*          Notice: this function allows the conversion of any string into enum,
*                  s.t. strings that are not identified convert into the OTHER
*                  enumeration.
* Return Value: enumeration representing a disposition.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strDispType - Disposition string
***************************************************************************/
RvSipDispositionType RVCALLCONV SipCommonConvertStrToEnumDispositionType(
											IN RvChar*      strDispType);

/******************************************************************************
* SipCommonConvertEnumToStrDispositionHandlingType
* ------------------------------------------------------------------------
* General: Converts a Contant-Disposition handling enumeration value to string.
* Return Value: a string, representing the handling
* ------------------------------------------------------------------------
* Arguments:
* Input:     eHandlingType - Handling enumeration
******************************************************************************/
RvChar* RVCALLCONV SipCommonConvertEnumToStrDispositionHandlingType(
											IN RvSipDispositionHandling eHandlingType);

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
															IN RvChar*      strHandlingType);

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
											IN RvSipMediaType eMediaType);

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
													IN RvChar*      strMediaType);

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
											IN RvSipMediaSubType eMediaSubType);

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
													IN RvChar*      strMediaSubType);

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
											IN RvSipDateWeekDay eWeekDay);

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
													IN RvInt32      weekDay);

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
											IN RvSipDateMonth eMonth);

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
													IN RvInt32      month);

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
											IN RvSipSubscriptionSubstate eSubscriptionSubstate);

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
													IN RvChar*      strSubscriptionSubstate);

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
											IN RvSipSubscriptionReason eSubscriptionReason);

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
													IN RvChar*      strSubscriptionReason);

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
											IN RvSipReasonProtocolType eReasonProtocol);

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
													IN RvChar*      strReasonProtocol);

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
											IN RvSipContactAction eContactAction);

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
													IN RvChar*      strContactAction);

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

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
                                    IN RvSipTransport eTransportType);

/******************************************************************************
* SipCommonConvertStrToEnumTransportType
* ------------------------------------------------------------------------
* General: Converts a string to SIP transport type enumeration value.
* Return Value: a SIP transport type.
* ------------------------------------------------------------------------
* Arguments:
* Input:     strTransportType - a string with SIP transport type
*****************************************************************************/
RvSipTransport RVCALLCONV SipCommonConvertStrToEnumTransportType(IN RvChar *strTransportType);

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
											IN RvSipTelUriEnumdiType eEnumdi);

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
										IN RvBool      bEnumdi);

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
                                    IN RvSipTransportAddressType eAddrType);

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
                             IN RvSipTransportConnectionType eConnectionType);

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
                             IN RvSipTransportConnectionState eConnState);

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
            IN RvSipTransportConnectionStateChangedReason eConnStateChangeRsn);

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
                                   IN RvSipTransportConnectionStatus  eStatus);
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
                    IN RvSipCallLegSessionTimerRefresherPreference eRefresher);
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
                                IN RvSipSecurityMechanismType eSecMech);
#endif /* #ifndef RV_SIP_IMS_ON */

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
                                IN RvSipSecurityEncryptAlgorithmType eEncrAlg);
#endif /* #ifndef RV_SIP_IMS_ON */

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
                                IN RvSipSecurityAlgorithmType eIntegrAlg);
#endif /* #ifndef RV_SIP_IMS_ON */


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
RVCALLCONV SipCommonConvertCoreToSipEncrAlg(IN RvImsEncrAlg eCoreVal);
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
                                IN RvSipSecurityEncryptAlgorithmType eSipVal);
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
RVCALLCONV SipCommonConvertCoreToSipIntegrAlg(IN RvImsAuthAlg eCoreVal);
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
                                IN RvSipSecurityAlgorithmType eSipVal);
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
                                IN RvImsAuthAlg eCoreIAlg);
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
                                IN RvImsEncrAlg eCoreEAlg);
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
                                    IN RvSipTransportAddressType eSipAddrType);


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_COMMON_CONVERT_H */


