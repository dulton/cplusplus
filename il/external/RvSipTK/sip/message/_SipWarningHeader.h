/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipWarningHeader.h
 *
 * The file defines 'internal API' for Warning header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPWARNINGHEADER_H
#define SIPWARNINGHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT


/***************************************************************************
 * SipWarningHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Warning object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipWarningHeaderGetPool(RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * SipWarningHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Warning object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipWarningHeaderGetPage(RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * SipWarningHeaderGetAgentHost
 * ------------------------------------------------------------------------
 * General:This method gets the warn-agent string from the Warning header.
 * Return Value: host offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object..
 ***************************************************************************/
RvInt32 SipWarningHeaderGetWarnAgent(IN RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * SipWarningHeaderSetWarnAgent
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the warn-agent's host string in
 *          the Warning Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Warning header object.
 *       pHost        - The host string to be set in the Warning header - If
 *                      NULL, the existing host string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipWarningHeaderSetWarnAgent(IN  RvSipWarningHeaderHandle   hHeader,
                                      IN  RvChar                    *pHost,
                                      IN  HRPOOL                     hPool,
                                      IN  HPAGE                      hPage,
                                      IN  RvInt32                    strOffset);

/***************************************************************************
 * SipWarningHeaderGetWarnText
 * ------------------------------------------------------------------------
 * General:This method gets the warn-text string from the Warning header.
 * Return Value: warn-text offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object..
 ***************************************************************************/
RvInt32 SipWarningHeaderGetWarnText(IN RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * SipWarningHeaderSetWarnText
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the warn-text string
 *          in the Warning Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Warning header object.
 *       pWarnText    - The warn-text string to be set in the Warning header - If
 *                      NULL, the existing warn-text string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipWarningHeaderSetWarnText(IN  RvSipWarningHeaderHandle   hHeader,
                                     IN  RvChar                    *pWarnText,
                                     IN  HRPOOL                     hPool,
                                     IN  HPAGE                      hPage,
                                     IN  RvInt32                    strOffset);

/***************************************************************************
 * SipWarningHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Warning header object.
 ***************************************************************************/
RvInt32 SipWarningHeaderGetStrBadSyntax(IN  RvSipWarningHeaderHandle hHeader);

/***************************************************************************
 * SipWarningHeaderSetStrBadSyntax
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
RvStatus SipWarningHeaderSetStrBadSyntax(
                                  IN RvSipWarningHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*SIPWARNINGHEADER_H*/

