
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           _SipAllowEventsHeader.h
 *
 * The file defines 'internal API' for Allow-Event header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             May 2002
 ******************************************************************************/

#ifndef SIPALLOW_EVENTS_HEADER_H
#define SIPALLOW_EVENTS_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipAllowEventsHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Allow-Event object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipAllowEventsHeaderGetPool(RvSipAllowEventsHeaderHandle hHeader);

/***************************************************************************
 * SipAllowEventsHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Allow-Event object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipAllowEventsHeaderGetPage(RvSipAllowEventsHeaderHandle hHeader);

/***************************************************************************
 * SipAllowEventsHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the event-package string in the
 *          AllowEvents Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strEventPackage- Text string giving the event-package to be set in the object.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strEventPackageOffset - Offset of the event-package string (if relevant).
 ***************************************************************************/
RvStatus SipAllowEventsHeaderSetEventPackage(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar*               strEventPackage,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventPackageOffset);

/***************************************************************************
 * SipAllowEventsHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General: This method retrieves the event-package string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetEventPackage(IN  RvSipAllowEventsHeaderHandle hHeader);

/***************************************************************************
 * SipAllowEventsHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the event-template string in the
 *          AllowEvents Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strEventTemplate - Text string giving the event-template to be set in the object.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strEventTemplateOffset - Offset of the event-template string (if relevant).
 ***************************************************************************/
RvStatus SipAllowEventsHeaderSetEventTemplate(
                                  IN RvSipAllowEventsHeaderHandle hHeader,
                                  IN RvChar*               strEventTemplate,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventTemplateOffset);

/***************************************************************************
 * SipAllowEventsHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General: This method retrieves the event-template string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetEventTemplate(IN  RvSipAllowEventsHeaderHandle hHeader);

/***************************************************************************
 * SipAllowEventsHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          Allow-Events object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Allow-events header object.
 ***************************************************************************/
RvInt32 SipAllowEventsHeaderGetStrBadSyntax(IN  RvSipAllowEventsHeaderHandle hHeader);

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /*SIPALLOW_EVENTS_HEADER_H*/
