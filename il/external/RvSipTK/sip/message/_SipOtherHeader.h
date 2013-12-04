/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipOtherHeader.h
 *
 * contains the 'internal APIs' for Other header.
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/
#ifndef SIPOTHERHEADER_H
#define SIPOTHERHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * SipOtherHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Other object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HRPOOL SipOtherHeaderGetPool(RvSipOtherHeaderHandle hHeader);

/***************************************************************************
 * SipOtherHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Other object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipOtherHeaderGetPage(RvSipOtherHeaderHandle hHeader);

/***************************************************************************
 * SipOtherHeaderGetName
 * ------------------------------------------------------------------------
 * General: This method retreives the name of the header.
 * Return Value: offset of the name string, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Other header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetName(IN RvSipOtherHeaderHandle hHeader);

/***************************************************************************
 * SipOtherHeaderSetName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pName in the
 *          OtherHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader - Handle of the Other header object.
 *	        pName   - The name of the header to be set in the Other header.
 *                    if NULL - the exist name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          NameOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipOtherHeaderSetName(IN    RvSipOtherHeaderHandle hHeader,
								IN    RvChar*               pName,
                                IN    HRPOOL                 hPool,
                                IN    HPAGE                  hPage,
                                IN    RvInt32               NameOffset);

/***************************************************************************
 * SipOtherHeaderGetValue
 * ------------------------------------------------------------------------
 * General: This method gets the value of the header.
 * Return Value: offset of the value string, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Other header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetValue(IN RvSipOtherHeaderHandle hHeader);

/***************************************************************************
 * SipOtherHeaderSetValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pValue in the
 *          OtherHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader - Handle of the Other header object.
 *	        pValue  - pointer to the value of the header to be set in the Other header.
 *                    if NULL - the exist name in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          valueOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipOtherHeaderSetValue(IN    RvSipOtherHeaderHandle hHeader,
								 IN    RvChar*               pValue,
                                 IN    HRPOOL                 hPool,
                                 IN    HPAGE                  hPage,
                                 IN    RvInt32               valueOffset);

/***************************************************************************
 * SipOtherHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipOtherHeaderGetStrBadSyntax(
									IN RvSipOtherHeaderHandle hHeader);

#ifdef __cplusplus
}
#endif
#endif
