
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          _SipSubscriptionStateHeader.h
 *
 * This file includes the 'internal APIs' for Subscription-State header.
 *
 *      Author           Date
 *     ------           ------------
 *   Ofra Wachsman        May 2002
 ******************************************************************************/

#ifndef SIPSUBSSTATEHEADER_H
#define SIPSUBSSTATEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipSubscriptionStateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Contact object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipSubscriptionStateHeaderGetPool(RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SipSubscriptionStateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Contact object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipSubscriptionStateHeaderGetPage(RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SipSubscriptionStateHeaderGetSubstate
 * ------------------------------------------------------------------------
 * General:This method gets the substate string from Subscroption-State header.
 * Return Value: substate offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrSubstate(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SipSubscriptionStateHeaderSetSubstate
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the substate in the
 *           SubscriptionState header object.
 *           User can set a substate enum value (in eSubstate) and no string,
 *           or to set eSubstate to be RVSIP_SUBSCRIPTION_SUBSTATE_OTHER, and
 *           then put the substate value in strSubstate.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle of the Suibscription-State header object.
 *    eSubstate   - The enum substate value to be set in the object.
 *  strSubstate - String of substate, in case eSubstate is
 *                  RVSIP_SUBSCRIPTION_SUBSTATE_OTHER. may be NULL.
 *  hPool       - The pool on which the string lays (if relevant).
 *  hPage       - The page on which the string lays (if relevant).
 *  strOffset   - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetSubstate(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionSubstate          eSubstate,
                             IN  RvChar                            *strSubstate,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                           strOffset);

/***************************************************************************
 * SipSubscriptionStateHeaderGetStrReason
 * ------------------------------------------------------------------------
 * General:This method gets the reason string from Subscroption-State header.
 * Return Value: reason offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrReason(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SipSubscriptionStateHeaderSetReason
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the reaon in the
 *           SubscriptionState header object.
 *           User can set a reason enum value (in eReason) and no string,
 *           or to set eReason to be RVSIP_SUBSCRIPTION_REASON_OTHER, and
 *           then put the reason value in strReason.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle of the Suibscription-State header object.
 *    eReason     - The enum reason value to be set in the object.
 *  strReason   - String of reson, in case eSubstate is
 *                RVSIP_SUBSCRIPTION_REASON_OTHER. may be NULL.
 *  hPool       - The pool on which the string lays (if relevant).
 *  hPage       - The page on which the string lays (if relevant).
 *  strOffset   - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetReason(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvSipSubscriptionReason            eReason,
                             IN  RvChar                            *strReason,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                           strOffset);

/***************************************************************************
 * SipSubscriptionStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the other Params value.
 * Return Value: other params string offset or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Subscription-State header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetOtherParams(
                               IN RvSipSubscriptionStateHeaderHandle hHeader);

/***************************************************************************
 * SipSubscriptionStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the other params in the
 *           SubscriptionState header object.
 *           the API will call it with NULL pool and page, to make a real
 *           allocation and copy. internal modules (such as parser)
 *           will call directly to this function, with valid
 *           pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - Handle of the Suibscription-State header object.
 *    strOtherParams - The other parameter to be set in the object.
 *                   If Null, the exist ViaParam string in the
 *                   object will be removed.
 *  hPool          - The pool on which the string lays (if relevant).
 *  hPage          - The page on which the string lays (if relevant).
 *  strOffset      - the offset of the string.
 ***************************************************************************/
RvStatus SipSubscriptionStateHeaderSetOtherParams(
                             IN  RvSipSubscriptionStateHeaderHandle hHeader,
                             IN  RvChar                            *strOtherParams,
                             IN  HRPOOL                             hPool,
                             IN  HPAGE                              hPage,
                             IN  RvInt32                           strOffset);
/***************************************************************************
 * SipSubscriptionStateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSubscriptionStateHeaderGetStrBadSyntax(
                                    IN RvSipSubscriptionStateHeaderHandle hHeader);

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif
#endif /*SIPSUBSSTATEHEADER_H*/
