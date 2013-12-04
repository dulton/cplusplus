/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAuthenticationInfoHeader.h
 *
 * The file defines 'internal API' for Authentication-Info header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Tamar Barzuza      Mar 2005
 ******************************************************************************/

#ifndef SIPAUTHENTICATIONINFOHEADER_H
#define SIPAUTHENTICATIONINFOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_AUTH_ON


/***************************************************************************
 * SipAuthenticationInfoHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the AuthenticationInfo object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthenticationInfoHeaderGetPool(RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the AuthenticationInfo object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthenticationInfoHeaderGetPage(RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetNextNonce
 * ------------------------------------------------------------------------
 * General:This method gets the next-nonce string from the AuthenticationInfo header.
 * Return Value: next-nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetNextNonce(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderSetNextNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the next-nonce string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the AuthenticationInfo header object.
 *       pNextNonce   - The next-nonce to be set in the AuthenticationInfo header - If
 *                      NULL, the exist next-nonce string in the header will be removed.
 *       strOffset    - Offset of a string on the page  (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetNextNonce(
                                     IN  RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN  RvChar                              *pNextNonce,
                                     IN  HRPOOL                               hPool,
                                     IN  HPAGE                                hPage,
                                     IN  RvInt32                              strOffset);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General:This method gets the CNonce string from the AuthenticationInfo header.
 * Return Value: CNonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetCNonce(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the cnonce string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader   - Handle of the AuthenticationInfo header object.
 *       pCNonce   - The cnonce to be set in the AuthenticationInfo header - If
 *                   NULL, the exist cnonce string in the header will be removed.
 *       strOffset - Offset of a string on the page  (if relevant).
 *       hPool     - The pool on which the string lays (if relevant).
 *       hPage     - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetCNonce(
                                     IN RvSipAuthenticationInfoHeaderHandle  hHeader,
                                     IN RvChar                              *pCNonce,
                                     IN HRPOOL                               hPool,
                                     IN HPAGE                                hPage,
                                     IN RvInt32                              strOffset);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetResponseAuth
 * ------------------------------------------------------------------------
 * General:This method gets the response-auth string from the AuthenticationInfo header.
 * Return Value: response-auth offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object..
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetResponseAuth(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderSetResponseAuth
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the response-auth string in the
 *          AuthenticationInfoHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader        - Handle of the AuthenticationInfo header object.
 *       pResponseAuth  - The response-auth to be set in the AuthenticationInfo header - If
 *                        NULL, the exist response-auth string in the header will be removed.
 *       strOffset      - Offset of a string on the page  (if relevant).
 *       hPool			- The pool on which the string lays (if relevant).
 *       hPage			- The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetResponseAuth(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvChar                             *pResponseAuth,
                                     IN HRPOOL                              hPool,
                                     IN HPAGE                               hPage,
                                     IN RvInt32                             strOffset);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General:This method gets the qop string from the authentication-info header.
 * Return Value: qop or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetStrQop(
                                    IN RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderSetStrQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the qop string in the
 *          Authentication-Info Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Authentication-Info header object.
 *       eQop     - The qop option enumeration value to be set in the object.
 *       pQop    - The qop to be set in the Authentication-Info header - If
 *                 NULL, the exist qop string in the header will be removed.
 *       offset  - Offset of a string on the page (if relevant).
 *       hPool   - The pool on which the string lays (if relevant).
 *       hPage   - The page on which the stringlays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationInfoHeaderSetStrQop(
                                     IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                     IN RvSipAuthQopOption                  eQop,
									 IN RvChar                             *pQop,
                                     IN HRPOOL                              hPool,
                                     IN HPAGE                               hPage,
                                     IN RvInt32                             offset);

/***************************************************************************
 * SipAuthenticationInfoHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the AuthenticationInfo header object.
 ***************************************************************************/
RvInt32 SipAuthenticationInfoHeaderGetStrBadSyntax(
                                            IN  RvSipAuthenticationInfoHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationInfoHeaderSetStrBadSyntax
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
RvStatus SipAuthenticationInfoHeaderSetStrBadSyntax(
                                  IN RvSipAuthenticationInfoHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);


#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif /*SIPAUTHENTICATIONINFOHEADER_H*/

