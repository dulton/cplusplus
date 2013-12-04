/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAcceptLanguageHeader.h
 *
 * The file defines 'internal API' for AcceptLanguage header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPACCEPTLANGUAGEHEADER_H
#define SIPACCEPTLANGUAGEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 

/***************************************************************************
 * SipAcceptLanguageHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AcceptLanguage object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptLanguageHeaderGetPool(RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AcceptLanguage object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptLanguageHeaderGetPage(RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderGetLanguage
 * ------------------------------------------------------------------------
 * General:This method gets the Language string from the Accept-Language header.
 * Return Value: Language offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Language header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetLanguage(IN RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderSetLanguage
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Language string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pLanguage      - The Language string to be set in the AcceptLanguage header - If
 *                      NULL, the exist Language string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetLanguage(IN  RvSipAcceptLanguageHeaderHandle   hHeader,
										  IN  RvChar                           *pLanguage,
										  IN  HRPOOL                            hPool,
										  IN  HPAGE                             hPage,
										  IN  RvInt32                           strOffset);

/***************************************************************************
 * SipAcceptLanguageHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept-Language header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Language header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetQVal(IN RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pQVal        - The QVal string to be set in the AcceptLanguage header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetQVal(IN  RvSipAcceptLanguageHeaderHandle   hHeader,
										IN  RvChar                           *pQVal,
										IN  HRPOOL                            hPool,
										IN  HPAGE                             hPage,
										IN  RvInt32                           strOffset);

/***************************************************************************
 * SipAcceptLanguageHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the AcceptLanguage header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptLanguage header object..
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetOtherParams(IN RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          AcceptLanguage Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptLanguage header object.
 *       pOtherParams - The other-params to be set in the AcceptLanguage header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptLanguageHeaderSetOtherParams(
							    IN  RvSipAcceptLanguageHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipAcceptLanguageHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptLanguage header object.
 ***************************************************************************/
RvInt32 SipAcceptLanguageHeaderGetStrBadSyntax(IN  RvSipAcceptLanguageHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptLanguageHeaderSetStrBadSyntax
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
RvStatus SipAcceptLanguageHeaderSetStrBadSyntax(
                                  IN RvSipAcceptLanguageHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);
	
	
#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
	
#ifdef __cplusplus
}
#endif

#endif /*SIPACCEPTLANGUAGEHEADER_H*/

