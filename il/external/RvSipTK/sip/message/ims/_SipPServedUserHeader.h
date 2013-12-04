/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPServedUserHeader.h
 *
 * The file contains 'internal API' for P-Served-User header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPSERVEDUSERHEADER_H
#define SIPPSERVEDUSERHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPServedUserHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PServedUser object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPServedUserHeaderGetPool(RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * SipPServedUserHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PServedUser object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPServedUserHeaderGetPage(RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * SipPServedUserHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PServedUser header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PServedUser header object..
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetDisplayName(
                                    IN RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * SipPServedUserHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          PServedUserHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PServedUser header object.
 *       pDisplayName - The display name to be set in the PServedUser header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPServedUserHeaderSetDisplayName(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPServedUserHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Served-User Params in the PServedUser header object.
 * Return Value: PServedUser param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PServedUser header object..
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetOtherParams(
                                            IN RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * SipPServedUserHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PServedUserParam in the
 *          PServedUserHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PServedUser header object.
 *          pPServedUserParam - The P-Served-User Params to be set in the PServedUser header.
 *                          If NULL, the exist P-Served-UserParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPServedUserHeaderSetOtherParams(
                                     IN    RvSipPServedUserHeaderHandle hHeader,
                                     IN    RvChar *                pPServedUserParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPServedUserHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPServedUserHeaderGetStrBadSyntax(
                                    IN RvSipPServedUserHeaderHandle hHeader);

/***************************************************************************
 * SipPServedUserHeaderSetStrBadSyntax
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
RvStatus SipPServedUserHeaderSetStrBadSyntax(
                                  IN RvSipPServedUserHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif


