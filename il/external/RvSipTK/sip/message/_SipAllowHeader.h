
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAllowHeader.h
 *
 * The file defines 'internal API' for Allow header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/

#ifndef SIPALLOWHEADER_H
#define SIPALLOWHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES


/***************************************************************************
 * SipAllowHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Allow object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipAllowHeaderGetPool(RvSipAllowHeaderHandle hHeader);

/***************************************************************************
 * SipAllowHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Allow object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipAllowHeaderGetPage(RvSipAllowHeaderHandle hHeader);

/***************************************************************************
 * SipAllowHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type string value from the
 *          Allow object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Allow header object.
 ***************************************************************************/
RvInt32 SipAllowHeaderGetStrMethodType(IN  RvSipAllowHeaderHandle hHeader);

/***************************************************************************
 * SipAllowHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strMethodType in the
 *          AllowHeader object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser) will
 *          call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader - Handle of the Allow header object.
 *        eMethodType - The method type to be set in the object.
 *        strMethodType - text string giving the method type to be set in the object.
 *        strMethodOffset - Offset of the method string (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAllowHeaderSetMethodType(
                                  IN RvSipAllowHeaderHandle hHeader,
                                  IN RvSipMethodType        eMethodType,
                                  IN RvChar*               strMethodType,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strMethodOffset);
/***************************************************************************
 * SipAllowHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the header object.
 ***************************************************************************/
RvInt32 SipAllowHeaderGetStrBadSyntax(IN  RvSipAllowHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif

#endif /*RVSIPALLOWHEADER_H*/
