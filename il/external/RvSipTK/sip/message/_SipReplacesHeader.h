
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     _SipReplacesHeader.h                                    *
 *                                                                            *
 * The file contains 'internal APIs' of Replaces header.
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ----------                                          *
 *     Shiri Mergel      Jun.2002                                             *
 ******************************************************************************/

#ifndef SIPREPLACESHEADER_H
#define SIPREPLACESHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES


/***************************************************************************
 * SipReplacesHeaderGetToTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the To Tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetToTag(IN RvSipReplacesHeaderHandle hHeader);

/***************************************************************************
 * SipReplacesHeaderGetFromTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the From Tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetFromTag(IN RvSipReplacesHeaderHandle hHeader);

/***************************************************************************
 * SipReplacesHeaderGetCallID
 * ------------------------------------------------------------------------
 * General: This method retrieves the Call-ID field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetCallID(IN RvSipReplacesHeaderHandle hHeader);

/***************************************************************************
 * SipReplacesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the Call-ID field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Replaces header object
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetOtherParams(IN RvSipReplacesHeaderHandle hHeader);

/***************************************************************************
 * SipReplacesHeaderSetToTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strToTag in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strToTag  - The to tag string to be set - if Null, the exist to tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strToTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetToTag(IN    RvSipReplacesHeaderHandle hHeader,
                                   IN    RvChar *              strToTag,
                                   IN    HRPOOL                 hPool,
                                   IN    HPAGE                  hPage,
                                   IN    RvInt32               strToTagOffset);

/***************************************************************************
 * SipReplacesHeaderSetFromTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strFromTag in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strFromTag  - The from tag string to be set - if Null, the exist from tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strFromTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetFromTag(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strFromTag,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strFromTagOffset);

/***************************************************************************
 * SipReplacesHeaderSetCallID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strCallID in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strCallID  - The Call-ID string to be set - if Null, the exist Call-ID in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strCallIDOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetCallID(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strCallID,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strCallIDOffset);

/***************************************************************************
 * SipReplacesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          Replaces Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the replaces header object
 *            strOtherParams  - The OtherParams string to be set - if Null, the exist Call-ID in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOtherParamsOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipReplacesHeaderSetOtherParams(IN    RvSipReplacesHeaderHandle hHeader,
                                       IN    RvChar *              strOtherParams,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOtherParamsOffset);

/***************************************************************************
 * SipReplacesHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Replaces header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hReplaces - The Replaces header to take the page from.
 ***************************************************************************/
HRPOOL SipReplacesHeaderGetPool(RvSipReplacesHeaderHandle hReplaces);

/***************************************************************************
 * SipReplacesHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Replaces header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hReplaces - The Replaces header to take the page from.
 ***************************************************************************/
HPAGE SipReplacesHeaderGetPage(RvSipReplacesHeaderHandle hReplaces);

/***************************************************************************
 * SipReplacesHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipReplacesHeaderGetStrBadSyntax(
                                    IN RvSipReplacesHeaderHandle hHeader);

/***************************************************************************
 * SipReplacesHeaderParseFromAddrSpec
 * ------------------------------------------------------------------------
 * General: Parse a replaces header from address spec.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:    hPool - Handle to the pool where the Replaces header in the address
 *                    spec is saved.
 *            hPage - Handle to the page where the Replaces header in the address
 *                    spec is saved.
 *            offset - The offset of the Replaces header in the address spec.
 *                     We will parse the Replaces header till we reach the '&' or '\0'.
 ***************************************************************************/
RvStatus SipReplacesHeaderParseFromAddrSpec(IN  HRPOOL                     hPool,
                                             IN  HPAGE                      hPage,
                                             IN  RvInt32                   offset,
                                             OUT RvSipReplacesHeaderHandle *hReplacesHeader);

/***************************************************************************
 * SipReplacesHeaderConstructInReferToHeader
 * ------------------------------------------------------------------------
 * General: Constructs a Replaces header inside a given Refer-To
 *          header. The address handle is returned.
 *          This is an internal function. the difference between this function,
 *          and the regular API function, is that it doesn't call to
 *          SipReferToRemoveOldReplacesFromHeadersList().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Refer-To header.
 * output: phReplacesHeader  - Handle to the newly constructed Replaces header.
 ***************************************************************************/
RvStatus RVCALLCONV SipReplacesHeaderConstructInReferToHeader(
                                          IN  RvSipReferToHeaderHandle   hHeader,
                                          OUT RvSipReplacesHeaderHandle *phReplacesHeader);



#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* SIPREPLACESHEADER_H */

