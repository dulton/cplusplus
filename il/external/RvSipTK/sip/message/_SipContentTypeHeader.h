/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipContentTypeHeader.h
 *
 * The file contains 'internal APIs' of Content-Type header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *     Tamar Barzuza    Aug 2001
 ******************************************************************************/


#ifndef SIPCONTENTTYPEHEADER_H
#define SIPCONTENTTYPEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipContentTypeHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-Type header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
HRPOOL SipContentTypeHeaderGetPool(RvSipContentTypeHeaderHandle hContentType);

/***************************************************************************
 * SipContentTypeHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-Type header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
HPAGE SipContentTypeHeaderGetPage(RvSipContentTypeHeaderHandle hContentType);

/***************************************************************************
 * SipContentTypeHeaderGetMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetMediaType(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Type header object
 *          strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetMediaType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipContentTypeHeaderGetMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetMediaSubType(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Type header object
 *          strMediaSubType  - The media sub type string to be set - if Null,
 *                     the exist media sub type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetMediaSubType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipContentTypeHeaderGetBoundary
 * ------------------------------------------------------------------------
 * General: This method retrieves the boundary field.
 * Return Value: Boundary value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetBoundary(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetBoundary
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strBoundary in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Type header object
 *          strBoundary  - The boundary string to be set - if Null,
 *                           the exist boundary in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetBoundary(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strBoundary,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipContentTypeHeaderGetVersion
 * ------------------------------------------------------------------------
 * General: This method retrieves the version field.
 * Return Value: Version value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetVersion(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetVersion
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strVersion in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Type header object
 *          strVersion  - The version string to be set - if Null,
 *                           the exist version in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetVersion(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strVersion,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipContentTypeHeaderGetBase
 * ------------------------------------------------------------------------
 * General: This method retrieves the base field.
 * Return Value: Version value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetBase(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetBase
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strBase in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the Content-Type header object
 *          strBase  - The base string to be set - if Null,
 *                     the exist base in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetBase(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strBase,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

/***************************************************************************
 * SipContentTypeHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams string.
 * Return Value: OtherParams string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderGetOtherParams(
                                     IN RvSipContentTypeHeaderHandle hHeader);

/***************************************************************************
 * SipContentTypeHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Content-Type header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader        - Handle of the Content-Type header object
 *          strOtherParams - The OtherParams string to be set - if Null,
 *                     the exist strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderSetOtherParams(
                                IN    RvSipContentTypeHeaderHandle hHeader,
                                IN    RvChar *                    strOtherParams,
                                IN    HRPOOL                       hPool,
                                IN    HPAGE                        hPage,
                                IN    RvInt32                     strOffset);
/***************************************************************************
* SipContentTypeHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
* hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipContentTypeHeaderGetStrBadSyntax(
                                     IN RvSipContentTypeHeaderHandle hHeader);


/*-----------------------------------------------------------------------*/
/*                         MULTIPART/RELATED FUNCTIONS                   */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipContentTypeHeaderTypeParamGetMediaType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media type field of the 'type' 
 *		    parameter
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderTypeParamGetMediaType(
                                     IN RvSipContentTypeHeaderHandle hHeader);


/***************************************************************************
 * SipContentTypeHeaderTypeParamGetMediaSubType
 * ------------------------------------------------------------------------
 * General: This method retrieves the media sub type field of the 'type' 
 *			parameter.
 * Return Value: Media type value string or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Content-Type header object
 ***************************************************************************/
RvInt32 SipContentTypeHeaderTypeParamGetMediaSubType(
                                     IN RvSipContentTypeHeaderHandle hHeader);




/***************************************************************************
 * SipContentTypeHeaderTypeParamSetMediaType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaType in the
 *          'type' parameter of the Content-Type header object. 
 *			The API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid un-needed coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Content-Type header object
 *            strMediaType  - The media type string to be set - if Null, the
 *                          exist media type in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipContentTypeHeaderTypeParamSetMediaType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);


/***************************************************************************
 * SipContentTypeHeaderTypeParamSetMediaSubType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMediaSubType in the
 *          'type' parameter of the Content-Type header object. the API will 
 *			call it with NULL pool
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
RvStatus SipContentTypeHeaderTypeParamSetMediaSubType(
                               IN  RvSipContentTypeHeaderHandle hHeader,
                               IN  RvChar *                    strMediaSubType,
                               IN  HRPOOL                       hPool,
                               IN  HPAGE                        hPage,
                               IN  RvInt32                     strOffset);

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* ifdef SIPCONTENTTYPEHEADER_H */
