/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                     SipPartyHeader.h
 *
 * The file contains 'internal APIs' of party header.
 *
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/


#ifndef SIPPARTYHEADER_H
#define SIPPARTYHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RV_SIP_LIGHT
    
/***************************************************************************
 * SipPartyHeaderConstructAndCopy
 * ------------------------------------------------------------------------
 * General: Constructs a party header and fill it with information from another 
 *          party header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Party header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipPartyHeaderConstructAndCopy(
                                           IN  RvSipMsgMgrHandle       hMsgMgr,
                                           IN  HRPOOL                  hPool,
                                           IN  HPAGE                   hPage,
                                           IN  RvSipPartyHeaderHandle  hSource,
                                           IN  RvSipHeaderType         eType,
                                           OUT RvSipPartyHeaderHandle* phNewHeader);

/***************************************************************************
 * SipPartyHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the party header object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hParty - The party header to take the page from.
 ***************************************************************************/
HRPOOL SipPartyHeaderGetPool(RvSipPartyHeaderHandle hParty);

/***************************************************************************
 * SipPartyHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the party header object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hParty - The party header to take the page from.
 ***************************************************************************/
HPAGE SipPartyHeaderGetPage(RvSipPartyHeaderHandle hParty);

/***************************************************************************
 * SipPartyHeaderGetTag
 * ------------------------------------------------------------------------
 * General: This method retrieves the tag field.
 * Return Value: Tag value string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetTag(IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * SipPartyHeaderSetTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strTag in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader - Handle of the party header object
 *			strTag  - The tag string to be set - if Null, the exist tag in the
 *                    object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strTagOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetTag(IN    RvSipPartyHeaderHandle hHeader,
    						   IN    RvChar *              strTag,
                               IN    HRPOOL                 hPool,
                               IN    HPAGE                  hPage,
                               IN    RvInt32               strTagOffset);

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipPartyHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: This method retrieves the display name field.
 * Return Value: Display name string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetDisplayName(IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * SipPartyHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DisplayName in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader - Handle of the party header object
 *			strDisplayName - The display name string to be set - if Null,
 *                    the exist DisplayName in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetDisplayName(IN    RvSipPartyHeaderHandle hHeader,
							           IN    RvChar *              strDisplayName,
                                       IN    HRPOOL                 hPool,
                                       IN    HPAGE                  hPage,
                                       IN    RvInt32               strOffset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * SipPartyHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: This method retrieves the OtherParams.
 * Return Value: Other params string or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the party header object
 ***************************************************************************/
RvInt32 SipPartyHeaderGetOtherParams(IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * SipPartyHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strOtherParams in the
 *          PartyHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hHeader        - Handle of the party header object
 *      	strOtherParams - The OtherParams string to be set - if Null, the exist
 *                           strOtherParams in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipPartyHeaderSetOtherParams(  IN    RvSipPartyHeaderHandle hHeader,
    									 IN    RvChar *              strOtherParams,
                                         IN    HRPOOL                 hPool,
                                         IN    HPAGE                  hPage,
                                         IN    RvInt32               strOtherOffset);

/***************************************************************************
 * SipPartyHeaderSetType
 * ------------------------------------------------------------------------
 * General: Sets the type of a party header: RVSIP_HEADERTYPE_TO / RVSIP_HEADERTYPE_FROM
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader        - the handle to the party header object.
 *  type           - The type to be set in the header object (to / from)
 ***************************************************************************/
RvStatus SipPartyHeaderSetType(IN RvSipPartyHeaderHandle hHeader,
                                   IN RvSipHeaderType type);

/***************************************************************************
 * SipPartyHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipPartyHeaderGetStrBadSyntax(
									IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * SipPartyHeaderCorrectUrl
 * ------------------------------------------------------------------------
 * General: The SIP-URL of the party header MUST NOT contain the "transport-param",
 *          "maddr-param", "ttl-param", or "headers" elements.
 *          this function removes these parameters.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader   - Handle to the Party header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipPartyHeaderCorrectUrl(IN RvSipPartyHeaderHandle hHeader);

#ifdef RFC_2543_COMPLIANT
/***************************************************************************
 * SipPartyHeaderSetEmptyTag
 * ------------------------------------------------------------------------
 * General: Sets the empty tag boolean to TRUE
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 ***************************************************************************/
RvStatus SipPartyHeaderSetEmptyTag(IN RvSipPartyHeaderHandle hHeader);

/***************************************************************************
 * SipPartyHeaderGetEmptyTag
 * ------------------------------------------------------------------------
 * General: Gets the empty tag boolean.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader        - the handle to the party header object.
 ***************************************************************************/
RvBool SipPartyHeaderGetEmptyTag(IN RvSipPartyHeaderHandle hHeader);
#endif /*#ifdef RFC_2543_COMPLIANT*/

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif
