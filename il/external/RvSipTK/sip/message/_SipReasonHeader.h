/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipReasonHeader.h
 *
 * The file defines 'internal API' for Reason header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPREASONHEADER_H
#define SIPREASONHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT


/***************************************************************************
 * SipReasonHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Reason object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipReasonHeaderGetPool(RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Reason object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipReasonHeaderGetPage(RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderGetText
 * ------------------------------------------------------------------------
 * General:This method gets the reason-text string from the Reason header.
 * Return Value: reason-text offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object..
 ***************************************************************************/
RvInt32 SipReasonHeaderGetText(IN RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderSetText
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the reason-text string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Reason header object.
 *       pText        - The reason text to be set in the Reason header - If
 *                      NULL, the exist reason text string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetText(IN  RvSipReasonHeaderHandle   hHeader,
                                IN  RvChar                    *pText,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipReasonHeaderGetStrProtocol
 * ------------------------------------------------------------------------
 * General:This method gets the protocol string from the Reason header.
 * Return Value: protocol or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object.
 ***************************************************************************/
RvInt32 SipReasonHeaderGetStrProtocol(IN RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderSetProtocol
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the protocol string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader   - Handle of the Reason header object.
 *       eProtocol - The protocol enumeration value to be set in the object.
 *       pProtocol - The protocol to be set in the Reason header - If
 *                   NULL, the exist protocol string in the header will be removed.
 *       offset    - Offset of a string on the page (if relevant).
 *       hPool     - The pool on which the string lays (if relevant).
 *       hPage     - The page on which the stringlays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetProtocol(IN RvSipReasonHeaderHandle  hHeader,
                                    IN RvSipReasonProtocolType  eProtocol,
									IN RvChar                  *pProtocol,
                                    IN HRPOOL                   hPool,
                                    IN HPAGE                    hPage,
                                    IN RvInt32                  offset);

/***************************************************************************
 * SipReasonHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the other-params string from the Reason header.
 * Return Value: other-params offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object..
 ***************************************************************************/
RvInt32 SipReasonHeaderGetOtherParams(IN RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other-params string in the
 *          Reason Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Reason header object.
 *       pOtherParams - The other-params to be set in the Reason header - If
 *                      NULL, the exist other-params string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipReasonHeaderSetOtherParams(
							    IN  RvSipReasonHeaderHandle   hHeader,
                                IN  RvChar                    *pOtherParams,
                                IN  HRPOOL                    hPool,
                                IN  HPAGE                     hPage,
                                IN  RvInt32                   strOffset);

/***************************************************************************
 * SipReasonHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Reason header object.
 ***************************************************************************/
RvInt32 SipReasonHeaderGetStrBadSyntax(IN  RvSipReasonHeaderHandle hHeader);

/***************************************************************************
 * SipReasonHeaderSetStrBadSyntax
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
RvStatus SipReasonHeaderSetStrBadSyntax(
                                  IN RvSipReasonHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);





#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*SIPREASONHEADER_H*/

