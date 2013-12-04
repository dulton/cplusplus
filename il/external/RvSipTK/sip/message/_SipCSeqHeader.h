/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          SipCSeqHeader.h
 *
 * This file includes the 'internal APIs' for CSeq header.
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/

#ifndef SIPCSEQHEADER_H
#define SIPCSEQHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RV_SIP_LIGHT
/***************************************************************************
 * SipCSeqHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the CSeq object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipCSeqHeaderGetPool(RvSipCSeqHeaderHandle hHeader);

/***************************************************************************
 * SipCSeqHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the CSeq object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipCSeqHeaderGetPage(RvSipCSeqHeaderHandle hHeader);

/***************************************************************************
 * SipCSeqHeaderGetStrMethodType
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type string value from the CSeq object.
 * Return Value: string offset of the method type, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle of the CSeq header object.
 ***************************************************************************/
RvInt32 SipCSeqHeaderGetStrMethodType(IN  RvSipCSeqHeaderHandle hHeader);

/***************************************************************************
 * SipCSeqHeaderSetMethodType
 * ------------------------------------------------------------------------
 * General:This method sets the method type in the CSeq object. If eMethodType
 *         is RVSIP_METHOD_OTHER, the pMethodTypeStr will be copied to the header,
 *         otherwise it will be ignored.
 *         the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle of the CSeq header object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - text string giving the method type to be set in the object.
 *                  This argument is needed when eMethodType is RVSIP_METHOD_OTHER.
 *                  otherwise it may be NULL.
 *  strMethodOffset - Offset of a string on the page  (if relevant).
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipCSeqHeaderSetMethodType(IN    RvSipCSeqHeaderHandle hHeader,
                                     IN    RvSipMethodType       eMethodType,
                                     IN    RvChar*              strMethodType,
                                     IN    HRPOOL                hPool,
                                     IN    HPAGE                 hPage,
                                     IN    RvInt32              strMethodOffset);
/***************************************************************************
* SipCSeqHeaderGetStrBadSyntax
* ------------------------------------------------------------------------
* General:This method retrieves the bad-syntax string value from the
*          header object.
* Return Value: text string giving the method type
* ------------------------------------------------------------------------
* Arguments:
*    hHeader - Handle of the Authorization header object.
***************************************************************************/
RvInt32 SipCSeqHeaderGetStrBadSyntax(IN  RvSipCSeqHeaderHandle hHeader);

/***************************************************************************
 * SipCSeqHeaderGetInitializedCSeq
 * ------------------------------------------------------------------------
 * General: Gets the step value from the CSeq header object and check if
 *			it was already initialized. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the CSeq header object.
 *	  pStep       - Pointer to the value of the retrieved CSeq
 ***************************************************************************/
RvStatus RVCALLCONV SipCSeqHeaderGetInitializedCSeq(
										IN  RvSipCSeqHeaderHandle hHeader,
										OUT RvInt32				 *pStep); 
#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif
#endif
