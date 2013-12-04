 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPAnswerStateHeader.h
 *
 * The file contains 'internal API' for P-Answer-State header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Jul.2007
 ******************************************************************************/
#ifndef SIPPANSWERSTATEHEADER_H
#define SIPPANSWERSTATEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPAnswerStateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PAnswerState object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPAnswerStateHeaderGetPool(RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * SipPAnswerStateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PAnswerState object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPAnswerStateHeaderGetPage(RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * SipPAnswerStateHeaderGetStrAnswerType
 * ------------------------------------------------------------------------
 * General:This method gets the StrAnswerType embedded in the PAnswerState header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PAnswerState header object..
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetStrAnswerType(
                                    IN RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * SipPAnswerStateHeaderSetAnswerType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pAnswerType in the
 *          PAnswerStateHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the PAnswerState header object.
 *         pAnswerType - The Answer Type to be set in the PAnswerState header - If
 *                NULL, the existing Answer Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAnswerStateHeaderSetAnswerType(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
									 IN	   RvSipPAnswerStateAnswerType   eAnswerType,
                                     IN    RvChar *                      pAnswerType,
                                     IN    HRPOOL                        hPool,
                                     IN    HPAGE                         hPage,
                                     IN    RvInt32                       strOffset);

/***************************************************************************
 * SipPAnswerStateHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Answer-State Params in the PAnswerState header object.
 * Return Value: PAnswerState param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PAnswerState header object..
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetOtherParams(
                                            IN RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * SipPAnswerStateHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PAnswerStateHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PAnswerState header object.
 *          pOtherParams - The P-Answer-State Params to be set in the PAnswerState header.
 *                          If NULL, the exist P-Answer-StateParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPAnswerStateHeaderSetOtherParams(
                                     IN    RvSipPAnswerStateHeaderHandle hHeader,
                                     IN    RvChar *                pPAnswerStateParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPAnswerStateHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPAnswerStateHeaderGetStrBadSyntax(
                                    IN RvSipPAnswerStateHeaderHandle hHeader);

/***************************************************************************
 * SipPAnswerStateHeaderSetStrBadSyntax
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
RvStatus SipPAnswerStateHeaderSetStrBadSyntax(
                                  IN RvSipPAnswerStateHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPANSWERSTATEHEADER_H */


