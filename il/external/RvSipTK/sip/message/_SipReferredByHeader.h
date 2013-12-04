
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipReferredByHeader.h                                *
 *                                                                            *
 * The file contains 'internal APIs' of Refer-To header.
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Apr.2001                                              *
 ******************************************************************************/


#ifndef SIPREFERREDBYHEADER_H
#define SIPREFERREDBYHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSipReferredByHeader.h"


/***************************************************************************
 * SipReferredByHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Referred-By header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReferredBy - The Refered-By header to take the page from.
 ***************************************************************************/
HRPOOL SipReferredByHeaderGetPool(RvSipReferredByHeaderHandle hReferredBy);

/***************************************************************************
 * SipReferredByHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Referred-By header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReferred-By - The Referred-By header to take the page from.
 ***************************************************************************/
HPAGE SipReferredByHeaderGetPage(RvSipReferredByHeaderHandle hReferredBy);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReferredByHeaderGetReferrerDispName
 * ------------------------------------------------------------------------
 * General: This function retrieves the referrer display-name field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetReferrerDispName(
                                       IN RvSipReferredByHeaderHandle hHeader);

/***************************************************************************
 * SipReferredByHeaderSetReferrerDispName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strReferrerDisplayName
 *          in the Referred-By Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Referred-by header object
 *            strDispName  - The display-name string to be set - if Null, the exist
 *                         display-name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strStringOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetReferrerDispName(
                               IN    RvSipReferredByHeaderHandle hHeader,
                               IN    RvChar *                   strDispName,
                               IN    HRPOOL                      hPool,
                               IN    HPAGE                       hPage,
                               IN    RvInt32                    strStringOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipReferredByHeaderGetCidParam
 * ------------------------------------------------------------------------
 * General: This function retrieves the cid parameter.
 * Return Value: string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetCidParam(
                                       IN RvSipReferredByHeaderHandle hHeader);

/***************************************************************************
 * SipReferredByHeaderSetCidParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strCidParam
 *          in the Referred-By Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Referred-by header object
 *            strCidParam  - The cid-param string to be set - if Null, the exist
 *                         cid-param in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strStringOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetCidParam(
                               IN    RvSipReferredByHeaderHandle hHeader,
                               IN    RvChar *                   strCidParam,
                               IN    HRPOOL                      hPool,
                               IN    HPAGE                       hPage,
                               IN    RvInt32                    strStringOffset);

/***************************************************************************
 * SipReferredByHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the other params.
 * Return Value: other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Referred-By header object
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetOtherParams(
                                        IN RvSipReferredByHeaderHandle hHeader);

/***************************************************************************
 * SipReferredByHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the other params in the
 *          Referred-By Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 *         Note that signature params are kept in the object in one concatenated
 *         string in the form-"name=value;name=value..." (the string you set
 *         should follow these instructions).
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the Referred-By header object
 *          strOtherParams - The Referred-By other Params string to be set
 *                           - if Null, the exist other Params in the object
 *                           will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the params string.
 ***************************************************************************/
RvStatus SipReferredByHeaderSetOtherParams(
                                IN RvSipReferredByHeaderHandle hHeader,
                                IN RvChar *                   strOtherParams,
                                IN HRPOOL                      hPool,
                                IN HPAGE                       hPage,
                                IN RvInt32                    strOffset);

/***************************************************************************
 * SipReferredByHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReferredByHeaderGetStrBadSyntax(
                                    IN RvSipReferredByHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* SIPREFERTOHEADER_H */
