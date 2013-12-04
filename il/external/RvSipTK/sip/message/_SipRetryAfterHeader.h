/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             _SipRetryAfter.h
 *
 * The file contains 'internal API' for RetryAfter header.
 *
 *      Author           Date
 *     ------           ------------
 *     Tamar Barzuza    Jan 2001
 ******************************************************************************/
#ifndef RETRYAFTER_HEADER_H
#define RETRYAFTER_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipRetryAfterHeader.h"

/***************************************************************************
 * SipRetryAfterHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Retry-After object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipRetryAfterHeaderGetPool(RvSipRetryAfterHeaderHandle hHeader);

/***************************************************************************
 * SipRetryAfterHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Retry-After object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipRetryAfterHeaderGetPage(RvSipRetryAfterHeaderHandle hHeader);

/***************************************************************************
 * SipRetryAfterHeaderGetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the RetryAfterParams.
 * Return Value: RetryAfterParams offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RetryAfter header object
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetRetryAfterParams(IN RvSipRetryAfterHeaderHandle hHeader);

/***************************************************************************
 * SipRetryAfterHeaderGetStrComment
 * ------------------------------------------------------------------------
 * General: This method retrieves the strComment.
 * Return Value: strComment offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the RetryAfter header object
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetStrComment(IN RvSipRetryAfterHeaderHandle hHeader);

/***************************************************************************
 * SipRetryAfterHeaderSetRetryAfterParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strRetryAfterParams in the
 *          RetryAfterHeader object. The API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader             - Handle of the RetryAfter header object
 *          strRetryAfterParams - The RetryAfterParams string to be set - if Null,
 *                                the exist strRetryAfterParams in the object will be removed.
 *          hPool               - The pool on which the string lays (if relevant).
 *          hPage               - The page on which the string lays (if relevant).
 *          strRetryAfterOffset - the offset of the RetryAfter string.
 ***************************************************************************/
RvStatus SipRetryAfterHeaderSetRetryAfterParams(
                                         IN    RvSipRetryAfterHeaderHandle hHeader,
                                         IN    RvChar *              strRetryAfterParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strRetryAfterOffset);

/***************************************************************************
 * SipRetryAfterHeaderSetStrComment
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strComment in the
 *          RetryAfterHeader object. The API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader             - Handle of the RetryAfter header object
 *          strComment          - The strComment string to be set - if Null,
 *                                the exist strComment in the object will be removed.
 *          hPool               - The pool on which the string lays (if relevant).
 *          hPage               - The page on which the string lays (if relevant).
 *          strCommentOffset - the offset of the strComment string.
 ***************************************************************************/
RvStatus SipRetryAfterHeaderSetStrComment(
                                         IN    RvSipRetryAfterHeaderHandle hHeader,
                                         IN    RvChar *              strComment,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strCommentOffset);

/***************************************************************************
 * SipRetryAfterHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRetryAfterHeaderGetStrBadSyntax(
                                    IN RvSipRetryAfterHeaderHandle hHeader);


#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RETRYAFTER_HEADER_H */
