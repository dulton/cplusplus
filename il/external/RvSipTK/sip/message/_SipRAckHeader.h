
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          SipRAckHeader.h
 *
 * This file includes the 'internal APIs' for RAck header.
 *
 *      Author           Date
 *     ------           ------------
 *   Sarit Mekler        Aug.2001
 ******************************************************************************/

#ifndef SIPRACKHEADER_H
#define SIPRACKHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipRAckHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RAck object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipRAckHeaderGetPool(RvSipRAckHeaderHandle hHeader);

/***************************************************************************
 * SipRAckHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RAck object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipRAckHeaderGetPage(RvSipRAckHeaderHandle hHeader);

/***************************************************************************
 * SipRAckHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRAckHeaderGetStrBadSyntax(
                                    IN RvSipRAckHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /*SIPRACKHEADER_H*/

