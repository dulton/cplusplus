/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipContentDispositionHeader.h
 *
 * The file contains 'internal APIs' of Content-Disposition header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *     Tamar Barzuza    Aug 2001
 ******************************************************************************/


#ifndef SIPCONTENTDISPOSITIONHEADER_H
#define SIPCONTENTDISPOSITIONHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipContentDispositionHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-Disposition header
 *          object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentDisposition - The Content-Disposition header to take t
 *            he page from.
 ***************************************************************************/
HRPOOL SipContentDispositionHeaderGetPool(
                    RvSipContentDispositionHeaderHandle hContentDisposition);

/***************************************************************************
 * SipContentDispositionHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-Disposition header
 *          object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentDisposition - The Content-Disposition header to take
 *            the page from.
 ***************************************************************************/
HPAGE SipContentDispositionHeaderGetPage(
                    RvSipContentDispositionHeaderHandle hContentDisposition);

/***************************************************************************
 * SipContentDispositionHeaderGetType
 * ------------------------------------------------------------------------
 * General: This method retrieves the disposition type field.
 * Return Value: Disposition type string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetType(
                              IN RvSipContentDispositionHeaderHandle hHeader);

/***************************************************************************
 * SipContentDispositionHeaderSetType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strType in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Disposition header object
 *          strType - The disposition type string to be set - if Null, the
 *                    exist disposition type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetType(
                             IN  RvSipContentDispositionHeaderHandle hHeader,
                             IN  RvChar *                           strType,
                             IN  HRPOOL                              hPool,
                             IN  HPAGE                               hPage,
                             IN  RvInt32                            strOffset);

/***************************************************************************
 * SipContentDispositionHeaderGetHandling
 * ------------------------------------------------------------------------
 * General: This method retrieves the handling field.
 * Return Value: Handling string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetHandling(
                             IN RvSipContentDispositionHeaderHandle hHeader);

/***************************************************************************
 * SipContentDispositionHeaderSetHandling
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strHandling in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Disposition header object
 *          strHandling  - The handling string to be set - if Null,
 *                     the exist handling type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetHandling(
                          IN  RvSipContentDispositionHeaderHandle hHeader,
                          IN  RvChar *                           strHandling,
                          IN  HRPOOL                              hPool,
                          IN  HPAGE                               hPage,
                          IN  RvInt32                            strOffset);

/***************************************************************************
 * SipContentDispositionHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Disposition header object
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetOtherParams(
                            IN RvSipContentDispositionHeaderHandle hHeader);

/***************************************************************************
 * SipContentDispositionHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Content-Disposition header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader        - Handle of the Content-Disposition header object
 *          strOtherParams - The OtherParams string to be set - if Null,
 *                     the exist strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipContentDispositionHeaderSetOtherParams(
                      IN    RvSipContentDispositionHeaderHandle hHeader,
                      IN    RvChar *                           strOtherParams,
                      IN    HRPOOL                              hPool,
                      IN    HPAGE                               hPage,
                      IN    RvInt32                            strOffset);

/***************************************************************************
 * SipContentDispositionHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipContentDispositionHeaderGetStrBadSyntax(
                            IN RvSipContentDispositionHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* ifdef SIPCONTENTDISPOSITIONHEADER_H */
