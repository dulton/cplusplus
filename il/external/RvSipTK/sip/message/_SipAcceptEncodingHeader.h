/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAcceptEncodingHeader.h
 *
 * The file defines 'internal API' for AcceptEncoding header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPACCEPTENCODINGHEADER_H
#define SIPACCEPTENCODINGHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 

/***************************************************************************
 * SipAcceptEncodingHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AcceptEncoding object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptEncodingHeaderGetPool(RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AcceptEncoding object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptEncodingHeaderGetPage(RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderGetCoding
 * ------------------------------------------------------------------------
 * General:This method gets the Coding string from the Accept-Encoding header.
 * Return Value: Coding offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Encoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetCoding(IN RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderSetCoding
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Coding string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pCoding      - The Coding string to be set in the AcceptEncoding header - If
 *                      NULL, the exist Coding string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetCoding(IN  RvSipAcceptEncodingHeaderHandle   hHeader,
										  IN  RvChar                           *pCoding,
										  IN  HRPOOL                            hPool,
										  IN  HPAGE                             hPage,
										  IN  RvInt32                           strOffset);

/***************************************************************************
 * SipAcceptEncodingHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept-Encoding header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept-Encoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetQVal(IN RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pQVal        - The QVal string to be set in the AcceptEncoding header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetQVal(IN  RvSipAcceptEncodingHeaderHandle   hHeader,
										IN  RvChar                           *pQVal,
										IN  HRPOOL                            hPool,
										IN  HPAGE                             hPage,
										IN  RvInt32                           strOffset);

/***************************************************************************
 * SipAcceptEncodingHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the AcceptEncoding header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptEncoding header object..
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetOtherParams(IN RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          AcceptEncoding Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AcceptEncoding header object.
 *       pOtherParams - The other-params to be set in the AcceptEncoding header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptEncodingHeaderSetOtherParams(
							    IN  RvSipAcceptEncodingHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipAcceptEncodingHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AcceptEncoding header object.
 ***************************************************************************/
RvInt32 SipAcceptEncodingHeaderGetStrBadSyntax(IN  RvSipAcceptEncodingHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptEncodingHeaderSetStrBadSyntax
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
RvStatus SipAcceptEncodingHeaderSetStrBadSyntax(
                                  IN RvSipAcceptEncodingHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);
	
	
#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
	
#ifdef __cplusplus
}
#endif

#endif /*SIPACCEPTENCODINGHEADER_H*/

