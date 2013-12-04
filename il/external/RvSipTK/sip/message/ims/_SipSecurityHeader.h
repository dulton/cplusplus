/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipSecurityHeader.h
 *
 * The file contains 'internal API' for P-Uri header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPSECURITYHEADER_H
#define SIPSECURITYHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipSecurityHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Security object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipSecurityHeaderGetPool(RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Security object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipSecurityHeaderGetPage(RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two event header objects.
 *         Security headers considered equal if all parameters, excluding extension parameters,
 *         are equal (case insensitive).
 *         The string parameters are compared byte-by-byte,
 *         A header containing a parameter never matches a header without that parameter.
 * Return Value: Returns RV_TRUE if the Security header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - A handle to the Security Header object.
 *    hOtherHeader - Handle to the Security header object with which a comparison is being made.
 *    bCompareType - If RV_FALSE, the header types (client, server or verify) will not 
 *                   be considered in comparison.
 *    bCompareDigestVerify - If RV_FALSE, the digest-verify parameter will not
 *                           be considered in comparison.
 ***************************************************************************/
RvBool RVCALLCONV SipSecurityHeaderIsEqual(
                                 IN  const RvSipSecurityHeaderHandle  hHeader,
                                 IN  const RvSipSecurityHeaderHandle  hOtherHeader,
								 IN  RvBool                           bCompareType,
								 IN  RvBool                           bCompareDigestVerify);

/***************************************************************************
 * SipSecurityHeaderGetStrMechanismType
 * ------------------------------------------------------------------------
 * General:This method gets the StrMechanismType embedded in the Security header.
 * Return Value: StrMechanismType offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrMechanismType(
                                    IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetMechanismType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pMechanismType in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Security header object.
 *         pMechanismType - The Mechanism Type to be set in the Security header - If
 *                NULL, the existing Mechanism Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetMechanismType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityMechanismType	eMechanismType,
                                     IN    RvChar *						pMechanismType,
                                     IN    HRPOOL						hPool,
                                     IN    HPAGE						hPage,
                                     IN    RvInt32						strOffset);

/***************************************************************************
 * SipSecurityHeaderGetStrPreference
 * ------------------------------------------------------------------------
 * General:This method gets the Preference in the Security header object.
 * Return Value: Preference offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrPreference(
										  IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetStrPreference
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Preference in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pPreference - The Preference to be set in the Security header.
 *                          If NULL, the existing Preference string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrPreference(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pPreference,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipSecurityHeaderGetStrDigestAlgorithm
 * ------------------------------------------------------------------------
 * General:This method gets the DigestAlgorithm in the Security header object.
 * Return Value: DigestAlgorithm offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestAlgorithm(
                                            IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strDigestAlgorithm in the
 *          Security Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Security header object.
 *        eDigestAlgorithm   - The Security algorithm to be set in the object.
 *        strDigestAlgorithm - text string giving the algorithm type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetDigestAlgorithm(
                                  IN    RvSipSecurityHeaderHandle	hHeader,
                                  IN    RvSipAuthAlgorithm			eDigestAlgorithm,
                                  IN    RvChar*						strDigestAlgorithm,
                                  IN    HRPOOL						hPool,
                                  IN    HPAGE						hPage,
                                  IN    RvInt32						strOffset);

/***************************************************************************
 * SipSecurityHeaderGetStrDigestQop
 * ------------------------------------------------------------------------
 * General:This method gets the DigestQop in the Security header object.
 * Return Value: DigestQop offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestQop(
                                            IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetStrDigestQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DigestQop in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pDigestQop - The DigestQop to be set in the Security header.
 *                          If NULL, the existing DigestQop string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrDigestQop(
                                     IN    RvSipSecurityHeaderHandle hHeader,
									 IN    RvSipAuthQopOption		 eDigestQop,
                                     IN    RvChar *					 pDigestQop,
                                     IN    HRPOOL					 hPool,
                                     IN    HPAGE					 hPage,
                                     IN    RvInt32					 strOffset);

/***************************************************************************
 * SipSecurityHeaderGetStrDigestVerify
 * ------------------------------------------------------------------------
 * General:This method gets the DigestVerify in the Security header object.
 * Return Value: DigestVerify offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestVerify(
                                            IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetStrDigestVerify
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DigestVerify in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pDigestVerify - The DigestVerify to be set in the Security header.
 *                          If NULL, the existing DigestVerify string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrDigestVerify(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pDigestVerify,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipSecurityHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the Security header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetOtherParams(
                                            IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pOtherParams - The Other Params to be set in the Security header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetOtherParams(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipSecurityHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrBadSyntax(
                                    IN RvSipSecurityHeaderHandle hHeader);

/***************************************************************************
 * SipSecurityHeaderSetStrBadSyntax
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
RvStatus SipSecurityHeaderSetStrBadSyntax(
                                  IN RvSipSecurityHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif


