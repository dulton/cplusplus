 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPChargingFunctionAddressesHeader.h
 *
 * The file contains 'internal API' for P-Charging-Function-Addresses header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPCHARGINGFUNCTIONADDRESSESHEADER_H
#define SIPPCHARGINGFUNCTIONADDRESSESHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PChargingFunctionAddresses object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPChargingFunctionAddressesHeaderGetPool(RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PChargingFunctionAddresses object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPChargingFunctionAddressesHeaderGetPage(RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Charging-Function-Addresses Params in the PChargingFunctionAddresses header object.
 * Return Value: PChargingFunctionAddresses param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PChargingFunctionAddresses header object..
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesHeaderGetOtherParams(
                                            IN RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PChargingFunctionAddressesHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PChargingFunctionAddresses header object.
 *          pOtherParams - The P-Charging-Function-Addresses Params to be set in the PChargingFunctionAddresses header.
 *                          If NULL, the exist P-Charging-Function-AddressesParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingFunctionAddressesHeaderSetOtherParams(
                                     IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                     IN    RvChar *                pPChargingFunctionAddressesParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesHeaderGetStrBadSyntax(
                                    IN RvSipPChargingFunctionAddressesHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingFunctionAddressesHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the bad-syntax string in the
 *          Header object. the API will call it with NULL pool and pages,
 *          to make a real allocation and copy. internal modules (such as parser)
 *          will call directly to this function, with the appropriate pool and page,
 *          to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader        - Handle to the Allow-events header object.
 *        strBadSyntax   - Text string giving the bad-syntax to be set in the header.
 *        hPool          - The pool on which the string lays (if relevant).
 *        hPage          - The page on which the string lays (if relevant).
 *        strBadSyntaxOffset - Offset of the bad-syntax string (if relevant).
 ***************************************************************************/
RvStatus SipPChargingFunctionAddressesHeaderSetStrBadSyntax(
                                  IN RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);

/***************************************************************************
 * SipPChargingFunctionAddressesListElementGetStrValue
 * ------------------------------------------------------------------------
 * General:This method gets the value in the PChargingFunctionAddressesListElement object.
 * Return Value: value offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingFunctionAddressesListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingFunctionAddressesListElementGetStrValue(IN RvSipPChargingFunctionAddressesListElemHandle hElement);

/***************************************************************************
 * SipPChargingFunctionAddressesListElementSetStrValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Value in the
 *          PChargingFunctionAddressesListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hElement - Handle of the PChargingFunctionAddressesListElement object.
 *            pValue - The Value to be set in the PChargingFunctionAddressesListElement.
 *                          If NULL, the existing Value string in the element will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingFunctionAddressesListElementSetStrValue(
                             IN    RvSipPChargingFunctionAddressesListElemHandle	hElement,
                             IN    RvChar *									pValue,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset);


#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPCHARGINGFUNCTIONADDRESSESHEADER_H */


