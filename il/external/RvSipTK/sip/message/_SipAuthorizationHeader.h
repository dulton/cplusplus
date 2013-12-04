/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAuthorizationHeader.h
 *
 * The file defines 'internal API' for Authorization header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Oren Libis         Jan.2001
 ******************************************************************************/

#ifndef SIPAUTHORIZATIONNHEADER_H
#define SIPAUTHORIZATIONNHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * SipAuthorizationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Authorization object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthorizationHeaderGetPool(RvSipAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipAuthorizationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Authorization object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthorizationHeaderGetPage(RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: This method retrieves the Authorization scheme string value from the
 *          Authorization object.
 * Return Value: text string offset giving the Authorization scheme, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuthScheme(
                                            IN  RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthScheme in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader       - Handle of the Authorization header object.
 *        eAuthScheme   - The Authorization scheme to be set in the object.
 *        strAuthScheme - text string giving the method type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetAuthScheme(
                                  IN    RvSipAuthorizationHeaderHandle  hHeader,
                                  IN    RvSipAuthScheme                 eAuthScheme,
                                  IN    RvChar*                        strAuthScheme,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset);


/***************************************************************************
 * SipAuthorizationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General:This method gets the realm string from the Authorization header.
 * Return Value: realm offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetRealm(
                                    IN RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the realm string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pRealm - The realm to be set in the Authorization header - If
 *                      NULL, the exist realm string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetRealm(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pRealm,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);



/***************************************************************************
 * SipAuthorizationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the Authorization header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetNonce(
                                    IN RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pNonce - The nonce to be set in the Authorization header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetNonce(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General:This method gets the opaque string from the Authorization header.
 * Return Value: opaque offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOpaque(
                                    IN RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the opaque string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pOpaque - The opaque to be set in the Authorization header - If
 *                      NULL, the exist opaque string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOpaque(
                                     IN    RvSipAuthorizationHeaderHandle hHeader,
                                     IN    RvChar                       *pOpaque,
                                     IN    HRPOOL                         hPool,
                                     IN    HPAGE                          hPage,
                                     IN    RvInt32                       strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetUserName
 * ------------------------------------------------------------------------
 * General:This method gets the user name string from the Authorization header.
 * Return Value: user name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetUserName(
                                    IN RvSipAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipAuthorizationHeaderSetUserName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the user name string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pUserName    - The user name to be set in the Authorization header - If
 *                      NULL, the exist user name string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetUserName(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pUserName,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the AuthParam string from the Authorization header.
 * Return Value: AuthParam offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOtherParams(
                                    IN RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the AuthParam string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pAuthParam - The AuthParam to be set in the Authorization header - If
 *                      NULL, the exist AuthParam string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOtherParams(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pAuthParam,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This method retrieves the Authorization algorithm string value from the
 *          Authorization object.
 * Return Value: text string giving the Authorization scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuthAlgorithm(
                                            IN  RvSipAuthorizationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthorizationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthAlgorithm in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Authorization header object.
 *        eAuthAlgorithm   - The Authorization algorithm to be set in the object.
 *        strAuthAlgorithm - text string giving the algorithm type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetAuthAlgorithm(
                                  IN    RvSipAuthorizationHeaderHandle  hHeader,
                                  IN    RvSipAuthAlgorithm              eAuthAlgorithm,
                                  IN    RvChar*                        strAuthAlgorithm,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset);



/***************************************************************************
 * SipAuthorizationHeaderGetResponse
 * ------------------------------------------------------------------------
 * General:This method gets the Response string from the Authorization header.
 * Return Value: Response offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetResponse(
                                    IN RvSipAuthorizationHeaderHandle hHeader);



/***************************************************************************
 * SipAuthorizationHeaderSetResponse
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Response string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pResponse - The Response to be set in the Authorization header - If
 *                      NULL, the exist Response string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetResponse(
                                     IN RvSipAuthorizationHeaderHandle  hHeader,
                                     IN RvChar                         *pResponse,
                                     IN HRPOOL                          hPool,
                                     IN HPAGE                           hPage,
                                     IN RvInt32                        strOffset);




/***************************************************************************
 * SipAuthorizationHeaderGetCNonce
 * ------------------------------------------------------------------------
 * General:This method gets the CNonce string from the Authorization header.
 * Return Value: CNonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetCNonce(
                                    IN RvSipAuthorizationHeaderHandle hHeader);




/***************************************************************************
 * SipAuthorizationHeaderSetCNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the CNonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *       pCNonce - The CNonce to be set in the Authorization header - If
 *                      NULL, the exist CNonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetCNonce(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pCNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetStrAuts
 * ------------------------------------------------------------------------
 * General:This method gets the Auts string from the Authorization header.
 * Return Value: Auts offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrAuts(
                                    IN RvSipAuthorizationHeaderHandle hHeader);

/***************************************************************************
 * SipAuthorizationHeaderSetStrAuts
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Auts string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader    - Handle of the Authorization header object.
 *         pAuts	- The Auts to be set in the Authorization header - If
 *                    NULL, the exist Auts string in the header will be removed.
 *        strOffset - Offset of a string on the page  (if relevant).
 *        hPool		- The pool on which the string lays (if relevant).
 *        hPage		- The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetStrAuts(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pAuts,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetStrBadSyntax(
                                            IN  RvSipAuthorizationHeaderHandle hHeader);

/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
/***************************************************************************
 * SipAuthorizationHeaderSetOrigUri
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthorizationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authorization header object.
 *         pDisplayName - The nonce to be set in the Authorization header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthorizationHeaderSetOrigUri(
                                     IN    RvSipAuthorizationHeaderHandle  hHeader,
                                     IN    RvChar                         *pOrigUri,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthorizationHeaderGetOrigUri
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the Authorization header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object..
 ***************************************************************************/
RvInt32 SipAuthorizationHeaderGetOrigUri(
                                    IN RvSipAuthorizationHeaderHandle hHeader);

/*#endif*/ /*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif /*SIPAUTHORIZATIONNHEADER_H*/

