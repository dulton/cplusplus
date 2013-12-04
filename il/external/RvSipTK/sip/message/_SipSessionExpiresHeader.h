/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipSessionExpiresHeader.h
 *
 * The file contains 'internal APIs' of SessionExpires header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Michal Mashiach       April 2001
 ******************************************************************************/


#ifndef SIPSESSIONEXPIRESHEADER_H
#define SIPSESSIONEXPIRESHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipSessionExpiresHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Session Expires object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipSessionExpiresHeaderGetPool(RvSipSessionExpiresHeaderHandle hHeader);

/***************************************************************************
 * SipSessionExpiresHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Session Expires object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipSessionExpiresHeaderGetPage(RvSipSessionExpiresHeaderHandle hHeader);

/***************************************************************************
 * SipSessionExpiresHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipSessionExpiresHeaderGetOtherParams(
                                     IN RvSipSessionExpiresHeaderHandle hHeader);
/***************************************************************************
 * SipSessionExpiresHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader        - Handle of the Content-Type header object
 *          strOtherParams - The OtherParams string to be set - if Null,
 *                     the exist strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipSessionExpiresHeaderSetOtherParams(
                             IN    RvSipSessionExpiresHeaderHandle hHeader,
                             IN    RvChar *                    strOtherParams,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             IN    RvInt32                     strOffset);

/***************************************************************************
 * SipSessionExpiresHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSessionExpiresHeaderGetStrBadSyntax(
                                    IN RvSipSessionExpiresHeaderHandle hHeader);

#endif /* RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif


