/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipReplyToHeader.h
 *
 * The file defines 'internal API' for ReplyTo header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPREPLYTOHEADER_H
#define SIPREPLYTOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 
	
	
/***************************************************************************
 * SipReplyToHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the ReplyTo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipReplyToHeaderGetPool(RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * SipReplyToHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the ReplyTo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipReplyToHeaderGetPage(RvSipReplyToHeaderHandle hHeader);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReplyToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetDisplayName(IN RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * SipReplyToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          ReplyToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the ReplyTo header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipReplyToHeaderSetDisplayName(IN   RvSipReplyToHeaderHandle hHeader,
                                        IN   RvChar *                 strDisplayName,
                                        IN   HRPOOL                   hPool,
                                        IN   HPAGE                    hPage,
                                        IN   RvInt32                  strOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipReplyToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the ReplyTo header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object..
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetOtherParams(IN RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * SipReplyToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          ReplyTo Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the ReplyTo header object.
 *       pOtherParams - The other-params to be set in the ReplyTo header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReplyToHeaderSetOtherParams(
							    IN  RvSipReplyToHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                     hPool,
                                IN  HPAGE                      hPage,
                                IN  RvInt32                    strOffset);

/***************************************************************************
 * SipReplyToHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReplyTo header object.
 ***************************************************************************/
RvInt32 SipReplyToHeaderGetStrBadSyntax(IN  RvSipReplyToHeaderHandle hHeader);

/***************************************************************************
 * SipReplyToHeaderSetStrBadSyntax
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
RvStatus SipReplyToHeaderSetStrBadSyntax(
                                  IN RvSipReplyToHeaderHandle            hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);

	
#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */
	
#ifdef __cplusplus
}
#endif

#endif /*SIPREPLYTOHEADER_H*/

