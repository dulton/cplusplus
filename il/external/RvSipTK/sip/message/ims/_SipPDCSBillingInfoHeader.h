 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPDCSBillingInfoHeader.h
 *
 * The file contains 'internal API' for P-DCS-Billing-Info header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Jan. 2006
 ******************************************************************************/
#ifndef SIPPDCSBILLINGINFOHEADER_H
#define SIPPDCSBILLINGINFOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSBillingInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPDCSBillingInfoHeaderGetPool(RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSBillingInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSBillingInfoHeaderGetPage(RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General:This method gets the BillingCorrelationID in the PDCSBillingInfo header object.
 * Return Value: BillingCorrelationID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrBillingCorrelationID(IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrBillingCorrelationID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BillingCorrelationID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pBillingCorrelationID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrBillingCorrelationID(
                         IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                         IN    RvChar *							pBillingCorrelationID,
                         IN    HRPOOL							hPool,
                         IN    HPAGE							hPage,
                         IN    RvInt32							strOffset);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrFEID
 * ------------------------------------------------------------------------
 * General:This method gets the FEID in the PDCSBillingInfo header object.
 * Return Value: FEID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrFEID(
                                            IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrFEID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FEID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pFEID - The FEID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing FEID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrFEID(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pFEID,
                             IN    HRPOOL							hPool,
                             IN    HPAGE							hPage,
                             IN    RvInt32							strOffset);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrFEIDHost
 * ------------------------------------------------------------------------
 * General:This method gets the FEIDHost in the PDCSBillingInfo header object.
 * Return Value: FEIDHost offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrFEIDHost(IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrFEIDHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FEIDHost in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pBillingCorrelationID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrFEIDHost(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pFEIDHost,
                             IN    HRPOOL							hPool,
                             IN    HPAGE							hPage,
                             IN    RvInt32							strOffset);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General:This method gets the RKSGroupID in the PDCSBillingInfo header object.
 * Return Value: BillingCorrelationID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrRKSGroupID(IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrRKSGroupID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BillingCorrelationID in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSBillingInfo header object.
 *            pRKSGroupID - The BillingCorrelationID to be set in the PDCSBillingInfo header.
 *                          If NULL, the existing BillingCorrelationID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetStrRKSGroupID(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pRKSGroupID,
                             IN    HRPOOL							hPool,
                             IN    HPAGE							hPage,
                             IN    RvInt32							strOffset);


/***************************************************************************
 * SipPDCSBillingInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-DCS-Billing-Info Params in the PDCSBillingInfo header object.
 * Return Value: PDCSBillingInfo param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PDCSBillingInfo header object..
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetOtherParams(
                                            IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSBillingInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PDCSBillingInfo header object.
 *          pOtherParams - The P-DCS-Billing-Info Params to be set in the PDCSBillingInfo header.
 *                          If NULL, the exist P-DCS-Billing-InfoParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSBillingInfoHeaderSetOtherParams(
                             IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                             IN    RvChar *							pPDCSBillingInfoParam,
                             IN    HRPOOL							hPool,
                             IN    HPAGE							hPage,
                             IN    RvInt32							strOffset);

/***************************************************************************
 * SipPDCSBillingInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSBillingInfoHeaderGetStrBadSyntax(
                                    IN RvSipPDCSBillingInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSBillingInfoHeaderSetStrBadSyntax
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
RvStatus SipPDCSBillingInfoHeaderSetStrBadSyntax(
                          IN RvSipPDCSBillingInfoHeaderHandle	hHeader,
                          IN RvChar*							strBadSyntax,
                          IN HRPOOL								hPool,
                          IN HPAGE								hPage,
                          IN RvInt32							strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPDCSBILLINGINFOHEADER_H */


