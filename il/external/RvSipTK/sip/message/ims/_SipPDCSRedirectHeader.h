 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPDCSRedirectHeader.h
 *
 * The file contains 'internal API' for P-DCS-Redirect header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Jan. 2006
 ******************************************************************************/
#ifndef SIPPDCSREDIRECTHEADER_H
#define SIPPDCSREDIRECTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

/***************************************************************************
 * SipPDCSRedirectHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSRedirect object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPDCSRedirectHeaderGetPool(RvSipPDCSRedirectHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSRedirectHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSRedirect object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSRedirectHeaderGetPage(RvSipPDCSRedirectHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSRedirectHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-DCS-Redirect Params in the PDCSRedirect header object.
 * Return Value: PDCSRedirect param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PDCSRedirect header object..
 ***************************************************************************/
RvInt32 SipPDCSRedirectHeaderGetOtherParams(
                                            IN RvSipPDCSRedirectHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSRedirectHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSRedirectHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PDCSRedirect header object.
 *          pOtherParams - The P-DCS-Redirect Params to be set in the PDCSRedirect header.
 *                          If NULL, the exist P-DCS-RedirectParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSRedirectHeaderSetOtherParams(
                             IN    RvSipPDCSRedirectHeaderHandle hHeader,
                             IN    RvChar *							pPDCSRedirectParam,
                             IN    HRPOOL							hPool,
                             IN    HPAGE							hPage,
                             IN    RvInt32							strOffset);

/***************************************************************************
 * SipPDCSRedirectHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSRedirectHeaderGetStrBadSyntax(
                                    IN RvSipPDCSRedirectHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSRedirectHeaderSetStrBadSyntax
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
RvStatus SipPDCSRedirectHeaderSetStrBadSyntax(
                          IN RvSipPDCSRedirectHeaderHandle	hHeader,
                          IN RvChar*							strBadSyntax,
                          IN HRPOOL								hPool,
                          IN HPAGE								hPage,
                          IN RvInt32							strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPDCSREDIRECTHEADER_H */


