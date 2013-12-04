 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPAccessNetworkInfoHeader.h
 *
 * The file contains 'internal API' for P-Access-Network-Info header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPACCESSNETWORKINFOHEADER_H
#define SIPPACCESSNETWORKINFOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PAccessNetworkInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPAccessNetworkInfoHeaderGetPool(RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PAccessNetworkInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPAccessNetworkInfoHeaderGetPage(RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrAccessType
 * ------------------------------------------------------------------------
 * General:This method gets the StrAccessType embedded in the PAccessNetworkInfo header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrAccessType(
                                    IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetAccessType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pAccessType in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PAccessNetworkInfo header object.
 *         pAccessType - The Access Type to be set in the PAccessNetworkInfo header - If
 *                NULL, the existing Access Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetAccessType(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
									 IN	   RvSipPAccessNetworkInfoAccessType eAccessType,
                                     IN    RvChar *                pAccessType,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General:This method gets the Cgi3gpp in the PAccessNetworkInfo header object.
 * Return Value: Cgi3gpp offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrCgi3gpp(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrCgi3gpp
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Cgi3gpp in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pCgi3gpp - The Cgi3gpp to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing Cgi3gpp string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrCgi3gpp(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pCgi3gpp,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General:This method gets the UtranCellId3gpp in the PAccessNetworkInfo header object.
 * Return Value: UtranCellId3gpp offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrUtranCellId3gpp(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the UtranCellId3gpp in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pUtranCellId3gpp - The UtranCellId3gpp to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing UtranCellId3gpp string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pUtranCellId3gpp,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General:This method gets the IWlanNodeID in the PAccessNetworkInfo header object.
 * Return Value: IWlanNodeID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrIWlanNodeID(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrIWlanNodeID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IWlanNodeID in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pIWlanNodeID - The IWlanNodeID to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing IWlanNodeID string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrIWlanNodeID(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pIWlanNodeID,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrDslLocation
 * ------------------------------------------------------------------------
 * General:This method gets the DslLocation in the PAccessNetworkInfo header object.
 * Return Value: DslLocation offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrDslLocation(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrDslLocation
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DslLocation in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pDslLocation - The DslLocation to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing DslLocation string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrDslLocation(
                             IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                             IN    RvChar *								pDslLocation,
                             IN    HRPOOL								hPool,
                             IN    HPAGE								hPage,
                             IN    RvInt32								strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General:This method gets the Ci3gpp2 in the PAccessNetworkInfo header object.
 * Return Value: Ci3gpp2 offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrCi3gpp2(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrCi3gpp2
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Ci3gpp2 in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PAccessNetworkInfo header object.
 *            pCi3gpp2 - The Ci3gpp2 to be set in the PAccessNetworkInfo header.
 *                          If NULL, the existing Ci3gpp2 string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetStrCi3gpp2(
                         IN    RvSipPAccessNetworkInfoHeaderHandle	hHeader,
                         IN    RvChar *								pCi3gpp2,
                         IN    HRPOOL								hPool,
                         IN    HPAGE								hPage,
                         IN    RvInt32								strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Access-Network-Info Params in the PAccessNetworkInfo header object.
 * Return Value: PAccessNetworkInfo param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PAccessNetworkInfo header object..
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetOtherParams(
                                            IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PAccessNetworkInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PAccessNetworkInfo header object.
 *          pOtherParams - The P-Access-Network-Info Params to be set in the PAccessNetworkInfo header.
 *                          If NULL, the exist P-Access-Network-InfoParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAccessNetworkInfoHeaderSetOtherParams(
                                     IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                     IN    RvChar *                pPAccessNetworkInfoParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPAccessNetworkInfoHeaderGetStrBadSyntax(
                                    IN RvSipPAccessNetworkInfoHeaderHandle hHeader);

/***************************************************************************
 * SipPAccessNetworkInfoHeaderSetStrBadSyntax
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
RvStatus SipPAccessNetworkInfoHeaderSetStrBadSyntax(
                                  IN RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPACCESSNETWORKINFOHEADER_H */


