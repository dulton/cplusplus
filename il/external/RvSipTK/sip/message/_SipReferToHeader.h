
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipReferToHeader.h                                    *
 *                                                                            *
 * The file contains 'internal APIs' of Refer-To header.
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Apr.2001                                             *
 ******************************************************************************/

#ifndef SIPREFERTOHEADER_H
#define SIPREFERTOHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "RvSipReferToHeader.h"


/***************************************************************************
 * SipReferToHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Refer-To header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReferTo - The Refer-To header to take the page from.
 ***************************************************************************/
HRPOOL SipReferToHeaderGetPool(RvSipReferToHeaderHandle hReferTo);

/***************************************************************************
 * SipReferToHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Refer-To header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReferTo - The Refer-To header to take the page from.
 ***************************************************************************/
HPAGE SipReferToHeaderGetPage(RvSipReferToHeaderHandle hReferTo);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipReferToHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Refer-To header object
 ***************************************************************************/
RvInt32 SipReferToHeaderGetDisplayName(IN RvSipReferToHeaderHandle hHeader);

/***************************************************************************
 * SipReferToHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          ReferToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Refer-To header object
 *            strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipReferToHeaderSetDisplayName(
                                       IN    RvSipReferToHeaderHandle hHeader,
                                       IN    RvChar *                strDisplayName,
                                       IN    HRPOOL                   hPool,
                                       IN    HPAGE                    hPage,
                                       IN    RvInt32                 strOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipReferToHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the header OtherParams.
 * Return Value: Offset of other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the ReferTo header object
 ***************************************************************************/
RvInt32 SipReferToHeaderGetOtherParams(IN RvSipReferToHeaderHandle hHeader);

/***************************************************************************
 * SipReferToHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          ReferToHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader        - Handle of the ReferTo header object
 *          strDisplayName - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool          - The pool on which the string lays (if relevant).
 *          hPage          - The page on which the string lays (if relevant).
 *          strOffset      - the offset of the other-params string.
 ***************************************************************************/
RvStatus SipReferToHeaderSetOtherParams(IN    RvSipReferToHeaderHandle hHeader,
                                         IN    RvChar *              strOtherParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strOtherOffset);

/***************************************************************************
 * SipReferToHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReferToHeaderGetStrBadSyntax(
                                    IN RvSipReferToHeaderHandle hHeader);

/***************************************************************************
 * SipReferToRemoveOldReplacesFromHeadersList
 * ------------------------------------------------------------------------
 * General: This function is called from the 'ReferToSetReplaces' function. It checks if the
 *          headers in the address spec has Replaces header and removes the Replaces header from
 *          the address spec if it exists.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 ***************************************************************************/
RvStatus SipReferToRemoveOldReplacesFromHeadersList(IN RvSipReferToHeaderHandle  hHeader);

/***************************************************************************
 * SipReferToHeaderSetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Sets the replaces header in the Refer-To header object.
 *          This is an internal function, the function just copy the
 *          replaces header, without checking the url headers parameter,
 *          whoever uses this function, must remove the old replaces from
 *          the url headers parameter.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader            - Handle to the Refer-To header object.
 *  hReplacesHeader - Handle to the Replaces header to be set in the
 *                      Refer-To header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipReferToHeaderSetReplacesHeader(
                               IN  RvSipReferToHeaderHandle  hHeader,
                               IN  RvSipReplacesHeaderHandle hReplacesHeader);

/***************************************************************************
 * SipReferToHeaderGetReplacesHeader
 * ------------------------------------------------------------------------
 * General: Gets the replaces header from the Refer-To header object.
 *          This is an internal function, the function just returns the
 *          replaces header, without checking the url headers parameter,
 *
 * Return Value: Returns hReplacesHeader.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hHeader            - Handle to the Refer-To header object.
 ***************************************************************************/
RvSipReplacesHeaderHandle RVCALLCONV SipReferToHeaderGetReplacesHeader(
                                    IN  RvSipReferToHeaderHandle   hHeader);


#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* SIPREFERTOHEADER_H */
