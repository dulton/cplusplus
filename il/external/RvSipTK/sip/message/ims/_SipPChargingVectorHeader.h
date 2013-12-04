 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipPChargingVectorHeader.h
 *
 * The file contains 'internal API' for P-Charging-Vector header.
 *
 *      Author           Date
 *     ------           ------------
 *      Mickey           Nov.2005
 ******************************************************************************/
#ifndef SIPPCHARGINGVECTORHEADER_H
#define SIPPCHARGINGVECTORHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

/***************************************************************************
 * SipPChargingVectorHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the PChargingVector object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipPChargingVectorHeaderGetPool(RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the PChargingVector object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipPChargingVectorHeaderGetPage(RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrIcidValue
 * ------------------------------------------------------------------------
 * General:This method gets the IcidValue in the PChargingVector header object.
 * Return Value: IcidValue offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrIcidValue(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrIcidValue
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IcidValue in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pIcidValue - The IcidValue to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrIcidValue(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidValue,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General:This method gets the IcidGenAddr in the PChargingVector header object.
 * Return Value: IcidGenAddr offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrIcidGenAddr(
                                            IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrIcidGenAddr
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IcidGenAddr in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pIcidGenAddr - The IcidGenAddr to be set in the PChargingVector header.
 *                          If NULL, the existing IcidGenAddr string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrIcidGenAddr(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pIcidGenAddr,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrOrigIoi
 * ------------------------------------------------------------------------
 * General:This method gets the OrigIoi in the PChargingVector header object.
 * Return Value: OrigIoi offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrOrigIoi(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrOrigIoi
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OrigIoi in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pIcidValue - The IcidValue to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrOrigIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pOrigIoi,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrTermIoi
 * ------------------------------------------------------------------------
 * General:This method gets the TermIoi in the PChargingVector header object.
 * Return Value: IcidValue offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrTermIoi(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrTermIoi
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the IcidValue in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pTermIoi - The IcidValue to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrTermIoi(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pTermIoi,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrGgsn
 * ------------------------------------------------------------------------
 * General:This method gets the Ggsn in the PChargingVector header object.
 * Return Value: Ggsn offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrGgsn(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrGgsn
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Ggsn in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pGgsn - The Ggsn to be set in the PChargingVector header. If NULL,
 *                           the existing Ggsn string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrGgsn(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pGgsn,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General:This method gets the GprsAuthToken in the PChargingVector header object.
 * Return Value: GprsAuthToken offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrGprsAuthToken(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrGprsAuthToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the GprsAuthToken in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pGprsAuthToken - The GprsAuthToken to be set in the PChargingVector header. If NULL,
 *                           the existing GprsAuthToken string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrGprsAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pGprsAuthToken,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrBras
 * ------------------------------------------------------------------------
 * General:This method gets the Bras in the PChargingVector header object.
 * Return Value: Bras offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBras(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBras
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Bras in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pBras - The Bras to be set in the PChargingVector header. If NULL,
 *				    the existing Bras string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrBras(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pBras,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General:This method gets the XdslAuthToken in the PChargingVector header object.
 * Return Value: XdslAuthToken offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrXdslAuthToken(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrXdslAuthToken
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Bras in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hHeader  - Handle of the PChargingVector header object.
 *			pXdslAuthToken - The Bras to be set in the PChargingVector header. If NULL,
 *				    the existing Bras string in the header will be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrXdslAuthToken(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *							pXdslAuthToken,
                                     IN    HRPOOL							hPool,
                                     IN    HPAGE							hPage,
                                     IN    RvInt32							strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrBCid
 * ------------------------------------------------------------------------
 * General:This method gets the BCid in the PChargingVector header object.
 * Return Value: BCid offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBCid(IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBCid
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BCid in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the PChargingVector header object.
 *            pBCid - The BCid to be set in the PChargingVector header.
 *                          If NULL, the existing IcidValue string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetStrBCid(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pBCid,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General:This method gets the P-Charging-Vector Params in the PChargingVector header object.
 * Return Value: PChargingVector param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the PChargingVector header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetOtherParams(
                                            IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          PChargingVectorHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hHeader - Handle of the PChargingVector header object.
 *          pOtherParams - The P-Charging-Vector Params to be set in the PChargingVector header.
 *                          If NULL, the exist P-Charging-VectorParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorHeaderSetOtherParams(
                                     IN    RvSipPChargingVectorHeaderHandle hHeader,
                                     IN    RvChar *                pPChargingVectorParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset);

/***************************************************************************
 * SipPChargingVectorHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *  hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPChargingVectorHeaderGetStrBadSyntax(
                                    IN RvSipPChargingVectorHeaderHandle hHeader);

/***************************************************************************
 * SipPChargingVectorHeaderSetStrBadSyntax
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
RvStatus SipPChargingVectorHeaderSetStrBadSyntax(
                                  IN RvSipPChargingVectorHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset);

/***************************************************************************
 * SipPChargingVectorInfoListElementGetStrCid
 * ------------------------------------------------------------------------
 * General:This method gets the cid in the PChargingVectorInfoListElement object.
 * Return Value: cid offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingVectorInfoListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorInfoListElementGetStrCid(IN RvSipPChargingVectorInfoListElemHandle hElement);

/***************************************************************************
 * SipPChargingVectorInfoListElementSetStrCid
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Cid in the
 *          PChargingVectorInfoListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hElement - Handle of the PChargingVectorInfoListElement object.
 *            pCid - The Cid to be set in the PChargingVectorInfoListElement.
 *                          If NULL, the existing Cid string in the element will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorInfoListElementSetStrCid(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pCid,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset);

/***************************************************************************
 * SipPChargingVectorInfoListElementGetStrFlowID
 * ------------------------------------------------------------------------
 * General:This method gets the FlowID in the PChargingVectorInfoListElement object.
 * Return Value: FlowID offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hElement - Handle of the PChargingVectorInfoListElement header object..
 ***************************************************************************/
RvInt32 SipPChargingVectorInfoListElementGetStrFlowID(IN RvSipPChargingVectorInfoListElemHandle hElement);

/***************************************************************************
 * SipPChargingVectorInfoListElementSetStrFlowID
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the FlowID in the
 *          PChargingVectorInfoListElement object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hElement - Handle of the PChargingVectorInfoListElement object.
 *			pFlowID	 - The FlowID to be set in the PChargingVectorInfoListElement.
 *						If NULL, the existing FlowID string in the element will
 *						be removed.
 *          strOffset	- Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipPChargingVectorInfoListElementSetStrFlowID(
                             IN    RvSipPChargingVectorInfoListElemHandle	hElement,
                             IN    RvChar *									pFlowID,
                             IN    HRPOOL									hPool,
                             IN    HPAGE									hPage,
                             IN    RvInt32									strOffset);



#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif /* SIPPCHARGINGVECTORHEADER_H */


