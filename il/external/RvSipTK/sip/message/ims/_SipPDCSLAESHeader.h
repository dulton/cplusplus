 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPDCSLAESHeader.h
 *
 * The file contains 'internal API' for P-DCS-LAES header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Jan.2006
 ******************************************************************************/
#ifndef SIPPDCSLAESHEADER_H
#define SIPPDCSLAESHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT

/***************************************************************************
 * SipPDCSLAESHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PDCSLAES object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPDCSLAESHeaderGetPool(RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PDCSLAES object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPDCSLAESHeaderGetPage(RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderGetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General:This method gets the LaesSigHost in the PDCSLAES header object.
 * Return Value: LaesSigHost offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesSigHost(IN RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesSigHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesSigHost in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesSigHost - The LaesSigHost to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesSigHost string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesSigHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesSigHost,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPDCSLAESHeaderGetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General:This method gets the LaesContentHost in the PDCSLAES header object.
 * Return Value: LaesContentHost offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesContentHost(
                                            IN RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesContentHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesContentHost in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesContentHost - The LaesContentHost to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesContentHost string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesContentHost(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesContentHost,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPDCSLAESHeaderGetStrLaesKey
 * ------------------------------------------------------------------------
 * General:This method gets the LaesKey in the PDCSLAES header object.
 * Return Value: LaesKey offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrLaesKey(IN RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderSetStrLaesKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the LaesKey in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PDCSLAES header object.
 *            pLaesSigHost - The LaesSigHost to be set in the PDCSLAES header.
 *                          If NULL, the existing LaesSigHost string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetStrLaesKey(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pLaesKey,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPDCSLAESHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-DCS-LAES Params in the PDCSLAES header object.
 * Return Value: PDCSLAES param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PDCSLAES header object..
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetOtherParams(
                                            IN RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PDCSLAESHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PDCSLAES header object.
 *          pOtherParams - The P-DCS-LAES Params to be set in the PDCSLAES header.
 *                          If NULL, the exist P-DCS-LAESParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPDCSLAESHeaderSetOtherParams(
                                     IN    RvSipPDCSLAESHeaderHandle hHeader,
                                     IN    RvChar *                pPDCSLAESParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPDCSLAESHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPDCSLAESHeaderGetStrBadSyntax(
                                    IN RvSipPDCSLAESHeaderHandle hHeader);

/***************************************************************************
 * SipPDCSLAESHeaderSetStrBadSyntax
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
RvStatus SipPDCSLAESHeaderSetStrBadSyntax(
                                  IN RvSipPDCSLAESHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPDCSLAESHEADER_H */


