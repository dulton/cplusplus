/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPUserDatabaseHeader.h
 *
 * The file contains 'internal API' for P-User-Database header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPUSERDATABASEHEADER_H
#define SIPPUSERDATABASEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPUserDatabaseHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PUserDatabase object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPUserDatabaseHeaderGetPool(RvSipPUserDatabaseHeaderHandle hHeader);

/***************************************************************************
 * SipPUserDatabaseHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PUserDatabase object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPUserDatabaseHeaderGetPage(RvSipPUserDatabaseHeaderHandle hHeader);

/***************************************************************************
 * SipPUserDatabaseHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPUserDatabaseHeaderGetStrBadSyntax(
                                    IN RvSipPUserDatabaseHeaderHandle hHeader);

/***************************************************************************
 * SipPUserDatabaseHeaderSetStrBadSyntax
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
RvStatus SipPUserDatabaseHeaderSetStrBadSyntax(
                                  IN RvSipPUserDatabaseHeaderHandle hHeader,
                                  IN RvChar*                        strBadSyntax,
                                  IN HRPOOL                         hPool,
                                  IN HPAGE                          hPage,
                                  IN RvInt32                        strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif


