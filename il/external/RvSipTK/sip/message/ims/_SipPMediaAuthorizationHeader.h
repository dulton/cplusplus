 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPMediaAuthorizationHeader.h
 *
 * The file contains 'internal API' for P-Media-Authorization header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPMEDIAAUTHORIZATIONHEADER_H
#define SIPPMEDIAAUTHORIZATIONHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PMediaAuthorization object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPMediaAuthorizationHeaderGetPool(RvSipPMediaAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PMediaAuthorization object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPMediaAuthorizationHeaderGetPage(RvSipPMediaAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetToken
 * ------------------------------------------------------------------------
 * General:This method gets the Token in the PMediaAuthorization header object.
 * Return Value: Token offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PMediaAuthorization header object..
 ***************************************************************************/
RvInt32 SipPMediaAuthorizationHeaderGetToken(IN RvSipPMediaAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipPMediaAuthorizationHeaderSetToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Token in the
 *          PMediaAuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PMediaAuthorization header object.
 *            pToken - The Token to be set in the PMediaAuthorization header.
 *                          If NULL, the existing Token string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPMediaAuthorizationHeaderSetToken(
                                     IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                     IN    RvChar *                pToken,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPMediaAuthorizationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the BadSyntax type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PMediaAuthorization header object.
 ***************************************************************************/
RvInt32 SipPMediaAuthorizationHeaderGetStrBadSyntax(
                                    IN RvSipPMediaAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipPMediaAuthorizationHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipPMediaAuthorizationHeaderSetStrBadSyntax(
                                  IN RvSipPMediaAuthorizationHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPMEDIAAUTHORIZATIONHEADER_H */


