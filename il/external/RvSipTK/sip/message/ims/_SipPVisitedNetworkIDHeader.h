/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPVisitedNetworkIDHeader.h
 *
 * The file contains 'internal API' for P-Visited-Network-ID header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPVISITEDNETWORKIDHEADER_H
#define SIPPVISITEDNETWORKIDHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PVisitedNetworkID object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPVisitedNetworkIDHeaderGetPool(RvSipPVisitedNetworkIDHeaderHandle hHeader);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PVisitedNetworkID object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPVisitedNetworkIDHeaderGetPage(RvSipPVisitedNetworkIDHeaderHandle hHeader);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetVNetworkSpec
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the PVisitedNetworkID header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PVisitedNetworkID header object..
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetVNetworkSpec(
                                    IN RvSipPVisitedNetworkIDHeaderHandle hHeader);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetVNetworkSpec
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pVNetworkSpec in the
 *          PVisitedNetworkIDHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PVisitedNetworkID header object.
 *       pVNetworkSpec - The display name to be set in the PVisitedNetworkID header - If
 *                NULL, the exist VNetworkSpec string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPVisitedNetworkIDHeaderSetVNetworkSpec(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar *                pVNetworkSpec,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Visited-Network-ID Params in the PVisitedNetworkID header object.
 * Return Value: PVisitedNetworkID param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PVisitedNetworkID header object..
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetOtherParams(
                                            IN RvSipPVisitedNetworkIDHeaderHandle hHeader);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PVisitedNetworkIDParam in the
 *          PVisitedNetworkIDHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PVisitedNetworkID header object.
 *          pPVisitedNetworkIDParam - The P-Visited-Network-ID Params to be set in the PVisitedNetworkID header.
 *                          If NULL, the exist P-Visited-Network-IDParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPVisitedNetworkIDHeaderSetOtherParams(
                                     IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                     IN    RvChar *                pPVisitedNetworkIDParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPVisitedNetworkIDHeaderGetStrBadSyntax(
                                    IN RvSipPVisitedNetworkIDHeaderHandle hHeader);

/***************************************************************************
 * SipPVisitedNetworkIDHeaderSetStrBadSyntax
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
RvStatus SipPVisitedNetworkIDHeaderSetStrBadSyntax(
                                  IN RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPVISITEDNETWORKIDHEADER_H */


