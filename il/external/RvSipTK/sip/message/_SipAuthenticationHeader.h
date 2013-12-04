/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAuthenticationHeader.h
 *
 * The file defines 'internal API' for Authentication header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Oren Libis         Jan.2001
 ******************************************************************************/

#ifndef SIPAUTHENTICATIONHEADER_H
#define SIPAUTHENTICATIONHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "RvSipMsgTypes.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/


/***************************************************************************
 * SipAuthenticationHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Authentication object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAuthenticationHeaderGetPool(RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Authentication object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAuthenticationHeaderGetPage(RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderGetStrAuthScheme
 * ------------------------------------------------------------------------
 * General: This method retrieves the authentication scheme string value from the
 *          Authentication object.
 * Return Value: text string giving the authentication scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrAuthScheme(
                                            IN  RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetAuthScheme
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthScheme in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader       - Handle of the Authentication header object.
 *        eAuthScheme   - The authentication scheme to be set in the object.
 *          strAuthScheme - text string giving the method type to be set in the object.
 *        strOffset - Offset of a string
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetAuthScheme(
                                  IN    RvSipAuthenticationHeaderHandle hHeader,
                                  IN    RvSipAuthScheme                 eAuthScheme,
                                  IN    RvChar*                        strAuthScheme,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        strOffset);

/***************************************************************************
 * SipAuthenticationHeaderGetRealm
 * ------------------------------------------------------------------------
 * General:This method gets the realm string from the authentication header.
 * Return Value: realm offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetRealm(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetRealm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the realm string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pRealm - The realm to be set in the Authentication header - If
 *                      NULL, the exist realm string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetRealm(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pRealm,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderGetDomain
 * ------------------------------------------------------------------------
 * General:This method gets the domain string from the authentication header.
 * Return Value: domain offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetDomain(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetDomain
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Domain string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pDomain - The domain to be set in the Authentication header - If
 *                      NULL, the exist Domain string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetDomain(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pDomain,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);


/***************************************************************************
 * SipAuthenticationHeaderGetNonce
 * ------------------------------------------------------------------------
 * General:This method gets the nonce string from the authentication header.
 * Return Value: nonce offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetNonce(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetNonce
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the nonce string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pNonce - The nonce to be set in the Authentication header - If
 *                      NULL, the exist Nonce string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetNonce(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pNonce,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);


/***************************************************************************
 * SipAuthenticationHeaderGetOpaque
 * ------------------------------------------------------------------------
 * General:This method gets the opaque string from the authentication header.
 * Return Value: opaque offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetOpaque(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetOpaque
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the opaque string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pOpaque - The opaque to be set in the Authentication header - If
 *                      NULL, the exist opaque string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetOpaque(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pOpaque,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);


/***************************************************************************
 * SipAuthenticationHeaderGetStrQop
 * ------------------------------------------------------------------------
 * General:This method gets the qop string from the authentication header.
 * Return Value: qop or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrQop(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetStrQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the qop string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pQop         - The qop to be set in the Authentication header - If
 *                      NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrQop(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pQop,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the AuthParam string from the authentication header.
 * Return Value: AuthParam offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetOtherParams(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the AuthParam string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pAuthParam - The AuthParam to be set in the Authentication header - If
 *                      NULL, the exist AuthParam string in the header will be removed.
 *       offset       - Offset of a string (if relevant).
 *       hPool        - The pool on which the string lays (if relevant).
 *       hPage        - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetOtherParams(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pAuthParam,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);


/***************************************************************************
 * SipAuthenticationHeaderGetStrAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This method retrieves the authentication algorithm string value from the
 *          Authentication object.
 * Return Value: text string giving the authentication scheme
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrAuthAlgorithm(
                                            IN  RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strAuthAlgorithm in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Authentication header object.
 *        eAuthAlgorithm   - The authentication algorithm to be set in the object.
 *          strAuthAlgorithm - text string giving the algorithm type to be set in the object.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetAuthAlgorithm(
                                  IN    RvSipAuthenticationHeaderHandle hHeader,
                                  IN    RvSipAuthAlgorithm              eAuthAlgorithm,
                                  IN    RvChar*                        strAuthAlgorithm,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvInt32                        offset);


/***************************************************************************
 * SipAuthenticationHeaderGetUserName
 * ------------------------------------------------------------------------
 * General:This method gets the username offset from the authentication header.
 * Return Value: username offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 RVCALLCONV SipAuthenticationHeaderGetUserName(
                                           IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetUserName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the UserName string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pUserName - The UserName to be set in the Authentication header - If
 *                      NULL, the exist UserName string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetUserName(
                                     INOUT RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pUserName,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderGetPassword
 * ------------------------------------------------------------------------
 * General:This method gets the password offset from the authentication header.
 * Return Value: password offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object..
 ***************************************************************************/
RvInt32 RVCALLCONV SipAuthenticationHeaderGetPassword(
                                           IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetPassword
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Password string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader      - Handle of the Authentication header object.
 *         pPassword - The Password to be set in the Authentication header - If
 *                      NULL, the exist Password string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetPassword(
                                     INOUT RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pPassword,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderClearPassword
 * ------------------------------------------------------------------------
 * General: This is an internal function that overrides the Password string
 *          with zeroes in order to ensure hijacking by memory dump.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Authentication header object.
 ***************************************************************************/
void SipAuthenticationHeaderClearPassword(
                                IN    RvSipAuthenticationHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationHeaderGetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General:This method gets the IntegrityKey string from the authentication header.
 * Return Value: IntegrityKey or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrIntegrityKey(
                                    IN RvSipAuthenticationHeaderHandle hHeader);

/***************************************************************************
 * SipAuthenticationHeaderSetStrIntegrityKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IntegrityKey string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader		- Handle of the Authentication header object.
 *        pIntegrityKey - The qop to be set in the Authentication header - If
 *						  NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrIntegrityKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pIntegrityKey,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderGetStrCipherKey
 * ------------------------------------------------------------------------
 * General:This method gets the CipherKey string from the authentication header.
 * Return Value: CipherKey or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authentication header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrCipherKey(
                                    IN RvSipAuthenticationHeaderHandle hHeader);


/***************************************************************************
 * SipAuthenticationHeaderSetStrCipherKey
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the CipherKey string in the
 *          AuthenticationHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader	 - Handle of the Authentication header object.
 *        pCipherKey - The qop to be set in the Authentication header - If
 *						NULL, the exist qop string in the header will be removed.
 *        offset - Offset of a string on the page (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAuthenticationHeaderSetStrCipherKey(
                                     IN    RvSipAuthenticationHeaderHandle hHeader,
                                     IN    RvChar                         *pCipherKey,
                                     IN    HRPOOL                          hPool,
                                     IN    HPAGE                           hPage,
                                     IN    RvInt32                        offset);

/***************************************************************************
 * SipAuthenticationHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the header object.
 ***************************************************************************/
RvInt32 SipAuthenticationHeaderGetStrBadSyntax(
                                    IN RvSipAuthenticationHeaderHandle hHeader);

#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPAUTHENTICATIONHEADER_H*/

