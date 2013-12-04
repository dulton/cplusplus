/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                   SipViaHeader.h
 *
 * This file contains 'internal APIs' of Via header.
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/

#ifndef SIPVIAHEADER_H
#define SIPVIAHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * SipViaHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Via object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The via header to take the pool from.
 ***************************************************************************/
HRPOOL SipViaHeaderGetPool(RvSipViaHeaderHandle hHeader);

/***************************************************************************
 * SipViaHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Via object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The via header to take the page from.
 ***************************************************************************/
HPAGE SipViaHeaderGetPage(RvSipViaHeaderHandle hHeader);

/***************************************************************************
 * SipViaHeaderGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the host name in the MsgViaHeader.
 * Return Value: host name string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetHost(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetHost
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strHost in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipViaHeader - Handle of the Via header object.
 *			strHost       - The host value to be set in the object. If Null, the
 *                          exist host string in the object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetHost(IN    RvSipViaHeaderHandle hSipViaHeader,
						      IN    RvChar*             strHost,
                              IN    HRPOOL               hPool,
                              IN    HPAGE                hPage,
                              IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetMaddrParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Maddr parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetMaddrParam(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetMaddrParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the MaddrParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipViaHeader - Handle of the Via header object.
 *			strMaddrParam - The maddr parameter to be set in the object.
 *                          If Null, the exist MsddrParam string in the
 *                          object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetMaddrParam(IN    RvSipViaHeaderHandle hSipViaHeader,
						            IN    RvChar*             strMaddrParam,
                                    IN    HRPOOL               hPool,
                                    IN    HPAGE                hPage,
                                    IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetReceivedParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Received parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetReceivedParam(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetReceivedParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ReceivedParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipViaHeader    - Handle of the Via header object.
 *			strReceivedParam - The Received parameter to be set in the object.
 *                             If Null, the exist ReceivedParam string in the
 *                             object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetReceivedParam(IN    RvSipViaHeaderHandle hSipViaHeader,
						               IN    RvChar*             strReceivedParam,
                                       IN    HRPOOL               hPool,
                                       IN    HPAGE                hPage,
                                       IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetBranchParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Branch parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetBranchParam(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetBranchParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the BranchParam in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipViaHeader  - Handle of the Via header object.
 *			strBranchParam - The Branch parameter to be set in the object.
 *                           If Null, the exist BranchParam string in the
 *                           object will be removed.
 *          strOffset      - Offset of a string on the same page to use in an attach case.
 *                           (else - UNDEFINED)
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetBranchParam( IN    RvSipViaHeaderHandle hSipViaHeader,
						              IN    RvChar*             strBranchParam,
                                      IN    HRPOOL               hPool,
                                      IN    HPAGE                hPage,
                                      IN    RvInt32             strOffset);


/***************************************************************************
 * SipViaHeaderGetStrCompParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the 'comp' parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrCompParam(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetCompParam
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the comp in the MsgViaHeader
 *           object. User can set a comp enum value (in eComp) and no string,
 *           or to set eComp to be RVSIP_COMP_OTHER, and then put the
 *           comp value in strComp. the API will call it with NULL pool
 *           and page, to make a real allocation and copy. internal modules
 *           (such as parser) will call directly to this function, with valid
 *           pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 *	eComp         - The enum compression type value to be set in the object.
 *  strComp       - String of compType, in case eCompType is
 *                  RVSIP_COMP_OTHER. may be NULL.
 *  hPool         - The pool on which the string lays (if relevant).
 *  hPage         - The page on which the string lays (if relevant).
 *  strOffset     - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetCompParam(IN    RvSipViaHeaderHandle hSipViaHeader,
								  IN    RvSipCompType        eComp,
                                  IN    RvChar*             strComp,
                                  IN    HRPOOL               hPool,
                                  IN    HPAGE                hPage,
                                  IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetStrSigCompIdParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the 'SigCompId' parameter value.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrSigCompIdParam(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetSigCompIdParam
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the sigcomp-id in the MsgViaHeader
 *           object. User can set the sigcomp-id value in strSigCompId. the API will 
 *			 call it with NULL pool and page, to make a real allocation and copy. 
 *			 internal modules (such as parser) will call directly to this function, with valid
 *           pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *  hSipViaHeader - Handle of the Via header object.
 *  strSigCompId  - String of SigCompId
 *  hPool         - The pool on which the string lays (if relevant).
 *  hPage         - The page on which the string lays (if relevant).
 *  strOffset     - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetSigCompIdParam(IN    RvSipViaHeaderHandle hSipViaHeader,
                                   IN    RvChar*	strSigCompId,
                                   IN    HRPOOL		hPool,
                                   IN    HPAGE		hPage,
                                   IN    RvInt32	strOffset);

/***************************************************************************
 * SipViaHeaderGetStrTransport
 * ------------------------------------------------------------------------
 * General: This method returns the transport protocol string value from the
 *          MsgViaHeader object.
 * Return Value: string offset or UNDEFINED
 * ------------------------------------------------------------------------
 * Arguments:
 *	input: hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrTransport(IN  RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetTransport
 * ------------------------------------------------------------------------
 * General:  This is an internal function that sets the transport in the MsgViaHeader
 *           object. User can set a transport enum value (in eTransport) and no string,
 *           or to set eTransport to be RVSIP_TRANSPORT_OTHER, and then put the
 *           transport value in strTransport. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 *	eTransport    - The enum transport protocol value to be set in the object.
 *  strTransport  - String of transportType, in case eTransport is
 *                  RVSIP_TRANSPORT_OTHER. may be NULL.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetTransport(IN    RvSipViaHeaderHandle hSipViaHeader,
								   IN    RvSipTransport       eTransport,
                                   IN    RvChar*             strTransport,
                                   IN    HRPOOL               hPool,
                                   IN    HPAGE                hPage,
                                   IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetViaParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the ViaParams value.
 * Return Value: branch string offset or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipViaHeader - Handle of the Via header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetViaParams(IN RvSipViaHeaderHandle hSipViaHeader);

/***************************************************************************
 * SipViaHeaderSetViaParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ViaParams in the
 *          ViaHeader object. the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hSipViaHeader - Handle of the Via header object.
 *	        strViaParams  - The via parameter to be set in the object.
 *                          If Null, the exist ViaParam string in the
 *                          object will be removed.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 ***************************************************************************/
RvStatus SipViaHeaderSetViaParams(IN    RvSipViaHeaderHandle hSipViaHeader,
						           IN    RvChar*             strViaParams,
                                   IN    HRPOOL               hPool,
                                   IN    HPAGE                hPage,
                                   IN    RvInt32             strOffset);

/***************************************************************************
 * SipViaHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipViaHeaderGetStrBadSyntax(
									IN RvSipViaHeaderHandle hHeader);

/***************************************************************************
 * SipViaHeaderIsEqual
 * ------------------------------------------------------------------------
 * General: Used to compare to two via headers
 * Return Value: TRUE if the keys are equal, FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hNewVia  - first via header.
 *          hOldVia  - secound via header.
 ***************************************************************************/
RvBool SipViaHeaderIsEqual(IN RvSipViaHeaderHandle hNewVia,
							IN RvSipViaHeaderHandle hOldVia);

/***************************************************************************
 * SipViaHeaderCompareBranchParams
 * ------------------------------------------------------------------------
 * General: Compares two branches
 * Return Value: RV_TRUE if the branches are the same, RV_FALSE - otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 *	Input: hViaHeader      - Handle to the first Via header object.
 *	       hOtherViaHeader - Handle to the second Via header object.
 ***************************************************************************/
RvBool SipViaHeaderCompareBranchParams(IN RvSipViaHeaderHandle hViaHeader,
                                        IN RvSipViaHeaderHandle hOtherViaHeader);

/***************************************************************************
 * SipViaHeaderCompareSentByParams
 * ------------------------------------------------------------------------
 * General: Compares two sent-by params
 * Return Value: RV_TRUE if the sent-by params are the same, RV_FALSE - otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 *	Input: hViaHeader      - Handle to the first Via header object.
 *	       hOtherViaHeader - Handle to the second Via header object.
 ***************************************************************************/
RvBool SipViaHeaderCompareSentByParams(IN RvSipViaHeaderHandle hViaHeader,
                                        IN RvSipViaHeaderHandle hOtherViaHeader);

#ifdef __cplusplus
}
#endif

#endif
