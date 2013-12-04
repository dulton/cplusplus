
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          SipEventHeader.h
 *
 * This file includes the 'internal APIs' for event header.
 *
 *      Author           Date
 *     ------           ------------
 *   Ofra Wachsman       May 2002
 ******************************************************************************/

#ifndef SIPEVENTHEADER_H
#define SIPEVENTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipEventHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Event header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipEventHeaderGetPool(RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Event header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipEventHeaderGetPage(RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderGetEventPackage
 * ------------------------------------------------------------------------
 * General: This method retrieves the event string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventPackage(IN  RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderSetEventPackage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventPackage in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventPackage       - Text string giving the event to be set in the object.
 *        strEventPackageOffset - Offset of the event-package string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventPackage(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventPackage,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventPackageOffset);

/***************************************************************************
 * SipEventHeaderGetEventTemplate
 * ------------------------------------------------------------------------
 * General: This method retrieves the event template string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventTemplate(IN  RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderSetEventTemplate
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventTemplate in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventTemplate       - Text string giving the event to be set in the object.
 *        strEventTemplateOffset - Offset of the event-template string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventTemplate(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventTemplate,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventTemplateOffset);

/***************************************************************************
 * SipEventHeaderGetEventId
 * ------------------------------------------------------------------------
 * General: This method retrieves the event id string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventId(IN  RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderSetEventId
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventId in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle to the Event header object.
 *        strEventId       - Text string giving the event id to be set in the object.
 *        strEventIdOffset - Offset of the event-id string (if relevant).
 *        hPool            - The pool on which the string lays (if relevant).
 *        hPage            - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventId(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventId,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventIdOffset);

/***************************************************************************
 * SipEventHeaderGetEventParam
 * ------------------------------------------------------------------------
 * General: This method retrieves the event string value from the
 *          Event object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Event header object.
 ***************************************************************************/
RvInt32 SipEventHeaderGetEventParam(IN  RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderSetEventParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strEventParam in the
 *          Event Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader               - Handle to the Event header object.
 *        strEventParam         - Text string giving the event to be set in the object.
 *        strEventParamOffset   - Offset of the event-param string (if relevant).
 *        hPool                 - The pool on which the string lays (if relevant).
 *        hPage                 - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipEventHeaderSetEventParam(
                                  IN RvSipEventHeaderHandle hHeader,
                                  IN RvChar*               strEventParam,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strEventParamOffset);

/***************************************************************************
* SipEventHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipEventHeaderGetStrBadSyntax(IN  RvSipEventHeaderHandle hHeader);

/***************************************************************************
 * SipEventHeaderIsReferEventPackage
 * ------------------------------------------------------------------------
 * General: Is the header package is refer
 * Return Value: RV_TRUE of refer. RV_FALSE if not.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 ***************************************************************************/
RvBool RVCALLCONV SipEventHeaderIsReferEventPackage(
                                  IN RvSipEventHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif
#endif /*SIPEVENTHEADER_H*/
