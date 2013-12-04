/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipContactHeader.h
 *
 * The file contains 'internal API' for contact header.
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/
#ifndef SIPCONTACTHEADER_H
#define SIPCONTACTHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

/***************************************************************************
 * SipContactHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Contact object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipContactHeaderGetPool(RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * SipContactHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Contact object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipContactHeaderGetPage(RvSipContactHeaderHandle hHeader);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipContactHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the Contact header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetDisplayName(
                                    IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * SipContactHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *       pDisplayName - The display name to be set in the Contact header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetDisplayName(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipContactHeaderGetContactParam
 * ------------------------------------------------------------------------
 * General:This method gets the contact Params in the Contact header object.
 * Return Value: Contact param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetContactParam(
                                            IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * SipContactHeaderSetContactParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ContactParam in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Contact header object.
 *          pContactParam - The contact Params to be set in the Contact header.
 *                          If NULL, the exist contactParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetContactParam(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pContactParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipContactHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *       pQVal - The 'q' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetQVal(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pQVal,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipContactHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'q'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'q' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetQVal(
                                    IN RvSipContactHeaderHandle hHeader);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * SipContactHeaderGetPubGruu
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'pub-gruu'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'pub-gruu' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetPubGruu(
                                    IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * SipContactHeaderSetPubGruu
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PubGruu in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pPubGruu - The 'pub-gruu' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetPubGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pPubGruu,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipContactHeaderGetTempGruu
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'temp-gruu'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'temp-gruu' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetTempGruu(
                                    IN RvSipContactHeaderHandle hHeader);

/***************************************************************************
 * SipContactHeaderSetTempGruu
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the TempGruu in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pTempGruu - The 'temp-gruu' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetTempGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pTempGruu,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipContactHeaderGetStrFeatureTag
 * ------------------------------------------------------------------------
 * General:This method gets one of the Feature Tag embedded in the Contact header.
 * Return Value: Feature Tag offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetStrFeatureTag(
                                    IN RvSipContactHeaderHandle hHeader,
                                    IN RvSipContactFeatureTagType eType);

/***************************************************************************
 * SipContactHeaderSetStrFeatureTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pFeatureTag in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *       pFeatureTag - The Feature Tag to be set in the Contact header - If NULL, 
 *                     the exist displayName string in the header will be removed.
 *       strOffset   - Offset of a string on the page  (if relevant).
 *       hPool       - The pool on which the string lays (if relevant).
 *       hPage       - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetStrFeatureTag(
                                     IN    RvSipContactHeaderHandle   hHeader,
                                     IN    RvSipContactFeatureTagType eType,
                                     IN    RvChar*                    pFeatureTag,
                                     IN    HRPOOL                     hPool,
                                     IN    HPAGE                      hPage,
                                     IN    RvInt32                    strOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/***************************************************************************
 * SipContactHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetStrBadSyntax(
                                    IN RvSipContactHeaderHandle hHeader);

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif


