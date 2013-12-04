/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipInfoHeader.h
 *
 * The file defines 'internal API' for Info header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Aug 2005
 ******************************************************************************/

#ifndef SIPINFOHEADER_H
#define SIPINFOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 
	

/***************************************************************************
 * SipInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Info object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipInfoHeaderGetPool(RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * SipInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Info object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipInfoHeaderGetPage(RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * SipInfoHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Info header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Info header object..
 ***************************************************************************/
RvInt32 SipInfoHeaderGetOtherParams(IN RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * SipInfoHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Info Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Info header object.
 *       pOtherParams - The other-params to be set in the Info header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipInfoHeaderSetOtherParams(
							    IN  RvSipInfoHeaderHandle     hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Info header object.
 ***************************************************************************/
RvInt32 SipInfoHeaderGetStrBadSyntax(IN  RvSipInfoHeaderHandle hHeader);

/***************************************************************************
 * SipInfoHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipInfoHeaderSetStrBadSyntax(
                                  IN RvSipInfoHeaderHandle               hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);





#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*SIPINFOHEADER_H*/

