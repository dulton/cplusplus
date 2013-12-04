/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipTimestampHeader.h
 *
 * The file defines 'internal API' for Timestamp header.
 *
 *
 *
 *      Author                 Date                                           
 *     ------               ------------                                      
 *    Galit Edri Domani      May 2005                                         
 *
 ******************************************************************************/

#ifndef SIPTIMESTAMPHEADER_H
#define SIPTIMESTAMPHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT 
	

/***************************************************************************
 * SipTimestampHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Timestamp object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipTimestampHeaderGetPool(RvSipTimestampHeaderHandle hHeader);

/***************************************************************************
 * SipTimestampHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Timestamp object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipTimestampHeaderGetPage(RvSipTimestampHeaderHandle hHeader);


/***************************************************************************
 * SipTimestampHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Timestamp header object.
 ***************************************************************************/
RvInt32 SipTimestampHeaderGetStrBadSyntax(IN  RvSipTimestampHeaderHandle hHeader);

/***************************************************************************
 * SipTimestampHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipTimestampHeaderSetStrBadSyntax(
                                  IN RvSipTimestampHeaderHandle hHeader,
                                  IN RvChar*							 strBadSyntax,
                                  IN HRPOOL								 hPool,
                                  IN HPAGE								 hPage,
                                  IN RvInt32							 strBadSyntaxOffset);





#endif /* #ifdef RVSIP_ENHANCED_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /*SIPAUTHENTICATIONINFOHEADER_H*/

