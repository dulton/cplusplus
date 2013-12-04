/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAcceptHeader.h
 *
 * The file defines 'internal API' for Accept header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPACCEPTHEADER_H
#define SIPACCEPTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 

/***************************************************************************
 * SipAcceptHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Accept object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAcceptHeaderGetPool(RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Accept object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAcceptHeaderGetPage(RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderGetStrMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrMediaType(
                                     IN RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader   - Handle of the Content-Type header object
 *          strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool     - The pool on which the string lays (if relevant).
 *          hPage     - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipAcceptHeaderSetMediaType(
                               IN  RvSipAcceptHeaderHandle     hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                      hPool,
                               IN  HPAGE                       hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipAcceptHeaderGetStrMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrMediaSubType(
                                     IN RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaSubType  - The media sub type string to be set - if Null,
 *                     the exist media sub type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipAcceptHeaderSetMediaSubType(
                               IN  RvSipAcceptHeaderHandle     hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                      hPool,
                               IN  HPAGE                       hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipAcceptHeaderGetQVal
 * ------------------------------------------------------------------------
 * General:This method gets the QVal string from the Accept header.
 * Return Value: QVal offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object..
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetQVal(IN RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal string in the
 *          Accept Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Accept header object.
 *       pQVal        - The QVal string to be set in the Accept header - If
 *                      NULL, the exist QVal string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptHeaderSetQVal(IN  RvSipAcceptHeaderHandle   hHeader,
								IN  RvChar                    *pQVal,
								IN  HRPOOL                    hPool,
								IN  HPAGE                     hPage,
								IN  RvInt32                   strOffset);

/***************************************************************************
 * SipAcceptHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Accept header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object..
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetOtherParams(IN RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Accept Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Accept header object.
 *       pOtherParams - The other-params to be set in the Accept header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAcceptHeaderSetOtherParams(
							    IN  RvSipAcceptHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipAcceptHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Accept header object.
 ***************************************************************************/
RvInt32 SipAcceptHeaderGetStrBadSyntax(IN  RvSipAcceptHeaderHandle hHeader);

/***************************************************************************
 * SipAcceptHeaderSetStrBadSyntax
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
RvStatus SipAcceptHeaderSetStrBadSyntax(
                                  IN RvSipAcceptHeaderHandle        hHeader,
                                  IN RvChar*						strBadSyntax,
                                  IN HRPOOL							hPool,
                                  IN HPAGE							hPage,
                                  IN RvInt32						strBadSyntaxOffset);

#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
	
#ifdef __cplusplus
}
#endif

#endif /*SIPACCEPTHEADER_H*/

