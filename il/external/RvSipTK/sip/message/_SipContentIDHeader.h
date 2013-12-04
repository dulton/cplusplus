/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipContentIDHeader.h
 *
 * The file contains 'internal APIs' of Content-ID header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *     Mickey Farkash    Jan 2007
 ******************************************************************************/


#ifndef SIPCONTENTIDHEADER_H
#define SIPCONTENTIDHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipContentIDHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Content-ID header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hContentID - The Content-ID header to take the page from.
 ***************************************************************************/
HRPOOL SipContentIDHeaderGetPool(RvSipContentIDHeaderHandle hContentID);

/***************************************************************************
 * SipContentIDHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Content-ID header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentID - The Content-ID header to take the page from.
 ***************************************************************************/
HPAGE SipContentIDHeaderGetPage(RvSipContentIDHeaderHandle hContentID);

/***************************************************************************
* SipContentIDHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General: This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
* hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipContentIDHeaderGetStrBadSyntax(
                                     IN RvSipContentIDHeaderHandle hHeader);

#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* ifdef SIPCONTENTIDHEADER_H */
