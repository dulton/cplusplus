/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipExpiresHeader.h
 *
 * The file contains 'internal API' for expires header.
 *
 *      Author           Date
 *     ------           ------------
 *     Tamar Barzuza    Jan 2001
 ******************************************************************************/
#ifndef SIPEXPIRESHEADER_H
#define SIPEXPIRESHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipExpiresHeader.h"


/***************************************************************************
 * SipExpiresHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Expires object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipExpiresHeaderGetPool(RvSipExpiresHeaderHandle hHeader);


/***************************************************************************
 * SipExpiresHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Expires object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipExpiresHeaderGetPage(RvSipExpiresHeaderHandle hHeader);

/***************************************************************************
 * SipExpiresHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipExpiresHeaderGetStrBadSyntax(
                                    IN RvSipExpiresHeaderHandle hHeader);

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIPEXPIRESHEADER_H */
