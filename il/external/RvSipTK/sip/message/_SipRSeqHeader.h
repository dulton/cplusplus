
/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          SipRSeqHeader.h
 *
 * This file includes the 'internal APIs' for RSeq header.
 *
 *      Author           Date
 *     ------           ------------
 *      Sarit             Aug.2001
 ******************************************************************************/

#ifndef SIPRSEQHEADER_H
#define SIPRSEQHEADER_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * SipRSeqHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the RSeq object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipRSeqHeaderGetPool(RvSipRSeqHeaderHandle hHeader);

/***************************************************************************
 * SipRSeqHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the RSeq object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipRSeqHeaderGetPage(RvSipRSeqHeaderHandle hHeader);

/***************************************************************************
 * SipRSeqHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipRSeqHeaderGetStrBadSyntax(
                                    IN RvSipRSeqHeaderHandle hHeader);


#endif /*RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif
#endif
