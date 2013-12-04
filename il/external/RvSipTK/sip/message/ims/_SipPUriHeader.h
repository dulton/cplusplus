/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPUriHeader.h
 *
 * The file contains 'internal API' for P-Uri header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPURIHEADER_H
#define SIPPURIHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPUriHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PUri object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPUriHeaderGetPool(RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * SipPUriHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PUri object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPUriHeaderGetPage(RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * SipPUriHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PUri header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PUri header object..
 ***************************************************************************/
RvInt32 SipPUriHeaderGetDisplayName(
                                    IN RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * SipPUriHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          PUriHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PUri header object.
 *       pDisplayName - The display name to be set in the PUri header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPUriHeaderSetDisplayName(
                                     IN    RvSipPUriHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPUriHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Uri Params in the PUri header object.
 * Return Value: PUri param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PUri header object..
 ***************************************************************************/
RvInt32 SipPUriHeaderGetOtherParams(
                                            IN RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * SipPUriHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PUriParam in the
 *          PUriHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PUri header object.
 *          pPUriParam - The P-Uri Params to be set in the PUri header.
 *                          If NULL, the exist P-UriParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPUriHeaderSetOtherParams(
                                     IN    RvSipPUriHeaderHandle hHeader,
                                     IN    RvChar *                pPUriParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPUriHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPUriHeaderGetStrBadSyntax(
                                    IN RvSipPUriHeaderHandle hHeader);

/***************************************************************************
 * SipPUriHeaderSetStrBadSyntax
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
RvStatus SipPUriHeaderSetStrBadSyntax(
                                  IN RvSipPUriHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif


