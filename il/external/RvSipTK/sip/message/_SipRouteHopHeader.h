/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipRouteHopHeader.h
 *
 * The file contains 'internal APIs' of RouteHop header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *    Sarit Mekler       April 2001
 ******************************************************************************/


#ifndef SIPROUTEHOPHEADER_H
#define SIPROUTEHOPHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * SipRouteHopHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RouteHop header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hRoute - The RouteHop header to take the page from.
 ***************************************************************************/
HRPOOL SipRouteHopHeaderGetPool(RvSipRouteHopHeaderHandle hRoute);

/***************************************************************************
 * SipRouteHopHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RouteHop header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hRoute - The RouteHop header to take the page from.
 ***************************************************************************/
HPAGE SipRouteHopHeaderGetPage(RvSipRouteHopHeaderHandle hRoute);


#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipRouteHopHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name offset field.
 * Return Value: Display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hRoute - Handle of the RouteHop header object
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetDisplayName(IN RvSipRouteHopHeaderHandle hRoute);

/***************************************************************************
 * SipRouteHopHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          RouteHop Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader - Handle of the RouteHop header object
 *			strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipRouteHopHeaderSetDisplayName(IN    RvSipRouteHopHeaderHandle hHeader,
							           IN    RvChar *              strDisplayName,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipRouteHopHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the Other Params offset.
 * Return Value: Other Params offset or UNDEFINE if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the RouteHop header object
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetOtherParams(IN RvSipRouteHopHeaderHandle hHeader);

/***************************************************************************
 * SipRouteHopHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          RouteHop Header object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader        - Handle of the RouteHop header object
 *      	strOtherParams - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipRouteHopHeaderSetOtherParams(  IN    RvSipRouteHopHeaderHandle hHeader,
    									 IN    RvChar *              strOtherParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strOtherOffset);

/***************************************************************************
 * SipRouteHopHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRouteHopHeaderGetStrBadSyntax(
									IN RvSipRouteHopHeaderHandle hHeader);

#ifdef __cplusplus
}
#endif

#endif
