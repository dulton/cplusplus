/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPProfileKeyHeader.h
 *
 * The file contains 'internal API' for P-Profile-Key header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPPROFILEKEYHEADER_H
#define SIPPPROFILEKEYHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPProfileKeyHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PProfileKey object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPProfileKeyHeaderGetPool(RvSipPProfileKeyHeaderHandle hHeader);

/***************************************************************************
 * SipPProfileKeyHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PProfileKey object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPProfileKeyHeaderGetPage(RvSipPProfileKeyHeaderHandle hHeader);

/***************************************************************************
 * SipPProfileKeyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PProfileKey header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PProfileKey header object..
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetDisplayName(
                                    IN RvSipPProfileKeyHeaderHandle hHeader);

/***************************************************************************
 * SipPProfileKeyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          PProfileKeyHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PProfileKey header object.
 *       pDisplayName - The display name to be set in the PProfileKey header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPProfileKeyHeaderSetDisplayName(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPProfileKeyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Profile-Key Params in the PProfileKey header object.
 * Return Value: PProfileKey param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PProfileKey header object..
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetOtherParams(
                                            IN RvSipPProfileKeyHeaderHandle hHeader);

/***************************************************************************
 * SipPProfileKeyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PProfileKeyParam in the
 *          PProfileKeyHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PProfileKey header object.
 *          pPProfileKeyParam - The P-Profile-Key Params to be set in the PProfileKey header.
 *                          If NULL, the exist P-Profile-KeyParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPProfileKeyHeaderSetOtherParams(
                                     IN    RvSipPProfileKeyHeaderHandle hHeader,
                                     IN    RvChar *                pPProfileKeyParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPProfileKeyHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPProfileKeyHeaderGetStrBadSyntax(
                                    IN RvSipPProfileKeyHeaderHandle hHeader);

/***************************************************************************
 * SipPProfileKeyHeaderSetStrBadSyntax
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
RvStatus SipPProfileKeyHeaderSetStrBadSyntax(
                                  IN RvSipPProfileKeyHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif


