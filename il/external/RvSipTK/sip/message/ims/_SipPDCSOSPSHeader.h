 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPDCSOSPSHeader.h
 *
 * The file contains 'internal API' for P-DCS-OSPS header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Jan.2006
 ******************************************************************************/
#ifndef SIPPDCSOSPSHEADER_H
#define SIPPDCSOSPSHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

/***************************************************************************
 * SipPDCSOSPSHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSOSPS object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPDCSOSPSHeaderGetPool(RvSipPDCSOSPSHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSOSPSHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSOSPS object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSOSPSHeaderGetPage(RvSipPDCSOSPSHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSOSPSHeaderGetStrOSPSTag
 * ------------------------------------------------------------------------
 * General:This method gets the StrOSPSTag embedded in the PDCSOSPS header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSOSPS header object..
 ***************************************************************************/
RvInt32 SipPDCSOSPSHeaderGetStrOSPSTag(
                                    IN RvSipPDCSOSPSHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSOSPSHeaderSetOSPSTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pOSPSTag in the
 *          PDCSOSPSHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PDCSOSPS header object.
 *         pOSPSTag - The Access Type to be set in the PDCSOSPS header - If
 *                NULL, the existing Access Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSOSPSHeaderSetOSPSTag(
                         IN    RvSipPDCSOSPSHeaderHandle	hHeader,
						 IN	   RvSipOSPSTagType				eOSPSTag,
                         IN    RvChar*						pOSPSTag,
                         IN    HRPOOL						hPool,
                         IN    HPAGE						hPage,
                         IN    RvInt32						strOffset);

/***************************************************************************
 * SipPDCSOSPSHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSOSPSHeaderGetStrBadSyntax(
								IN RvSipPDCSOSPSHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSOSPSHeaderSetStrBadSyntax
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
RvStatus SipPDCSOSPSHeaderSetStrBadSyntax(
                              IN RvSipPDCSOSPSHeaderHandle	hHeader,
                              IN RvChar*					strBadSyntax,
                              IN HRPOOL						hPool,
                              IN HPAGE						hPage,
                              IN RvInt32					strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPDCSOSPSHEADER_H */


