/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipAdrress.h
 *
 * file contains 'internal APIs' of Addrees object.
 *
 *
 *      Author           Date
 *     ------           ------------
 *      Ofra             Nov.2000
 ******************************************************************************/



#ifndef SIPADDRESS_H
#define SIPADDRESS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * SipAddrGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the address object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAddrGetPool(RvSipAddressHandle hAddr);

/***************************************************************************
 * SipAddrGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the address object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAddrGetPage(RvSipAddressHandle hAddr);

/***************************************************************************
 * SipAddrUrlGetStrMethod
 * ------------------------------------------------------------------------
 * General: Gets the method string.
 *          If this function RvSipAddrUrlGetMethodType returns RVSIP_METHOD_OTHER,
 *          call this function to get the actual method in a string.
 *          format.
 * Return Value: Returns the offset of the actual method from the URL address object.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr   - Handle to the URL address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetStrMethod(IN  RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetMethod
 * ------------------------------------------------------------------------
 * General:This method sets the method type in the URL address object. If eMethodType
 *         is RVSIP_METHOD_OTHER, the pMethodTypeStr will be copied to the header,
 *         otherwise it will be ignored.
 *         the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoide unneeded coping.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr      - Handle of the URL address object.
 *  eMethodType   - The method type to be set in the object.
 *	strMethodType - text string giving the method type to be set in the object.
 *                  This argument is needed when eMethodType is RVSIP_METHOD_OTHER.
 *                  otherwise it may be NULL.
 *  strMethodOffset - Offset of a string on the page  (if relevant).
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetMethod(   IN    RvSipAddressHandle hSipAddr,
									 IN    RvSipMethodType    eMethodType,
                                     IN    RvChar*           strMethodType,
                                     IN    HRPOOL             hPool,
                                     IN    HPAGE              hPage,
                                     IN    RvInt32           strMethodOffset);

/***************************************************************************
 * SipAddrUrlGetStrTransport
 * ------------------------------------------------------------------------
 * General:This method returns the transport protocol string value from the
 *          MsgAddrUrl object.
 * Return Value: String offset of transportType, or UNDEFINED/
 * ------------------------------------------------------------------------
 * Arguments:
 *	input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetTransport
 * ------------------------------------------------------------------------
 * General: This method sets the transport protocol value in the MsgAddrUrl
 *          object.
 *          if eTransport is RVSIP_TRANSPORT_OTHER, then strTransport or the
 *          given pool and page will contain the transportType string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *	input: hSipUrl      - Handle of the address object.
 *         eTransport   - Enumuration value of transportType.
 *         strTransport - String of transportType, in case eTransport is
 *                        RVSIP_TRANSPORT_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strTransportOffset - The offset of the string in the page, when
 *                        attach is RV_TRUE.
 *         strTransportFromMsgOffset - in case the transport is enumeration we save
 *                                     the string exactly as it apeared in the message
 *                                     along with the enumeration.
 ***************************************************************************/
RvStatus SipAddrUrlSetTransport( IN  RvSipAddressHandle hSipAddr,
    							  IN  RvSipTransport     eTransport,
    							  IN  RvChar*           strTransport,
                                  IN  HRPOOL             hPool,
                                  IN  HPAGE              hPage,
                                  IN  RvInt32           strTransportOffset,
								  IN  RvInt32           strTransportFromMsgOffset);

/***************************************************************************
 * SipAddrUrlGetUser
 * ------------------------------------------------------------------------
 * General:  This method return the user field from the URL object.
 * Return Value: user name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetUser(IN RvSipAddressHandle hSipUrl);


/***************************************************************************
 * SipAddrUrlGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the name of the host, from the URL object.
 * Return Value: host name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetHost(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetHost
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the host in the url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl - Handle of the url address object.
 *	strHost - the host value to be set in the object. - if NULL, the exist strHost
 *            will be removed.
 *  strHostOffset - The offset of the host string - in case of ToAttach RV_TRUE.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetHost(IN    RvSipAddressHandle hSipAddr,
						    IN    RvChar*           strHost,
                            IN    HRPOOL             hPool,
                            IN    HPAGE              hPage,
                            IN    RvInt32           strHostOffset);

/***************************************************************************
 * SipAddrUrlGetMaddrParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Maddr parameter value.
 * Return Value: Maddr param offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetMaddrParam(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetMaddrParam
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the maddr param in the
 *			sip url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl       - Handle of the url address object.
 *	strMaddrParam - The maddr parameter to be set in the object. - if NULL, the
 *                  exist strMaddrParam be removed.
 *  strMaddrOffset - The string offset in case ToAttach = RV_TRUE.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetMaddrParam(IN     RvSipAddressHandle hSipAddr,
    							  IN    RvChar*           strMaddrParam,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32           strMaddrOffset);

/***************************************************************************
 * SipAddrUrlGetUserParam
 * ------------------------------------------------------------------------
 * General:This method returns the user param string value from the
 *		   MsgAddrUrl object.
 * Return Value: user param offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	input:  hSipAddr     - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetStrUserParam(IN  RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetUser
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the user in the
 *			sip url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl - Handle of the url address object.
 *	strUser - The user value to be set in the object - if NULL, the exist strUser
 *            will be removed.
 *  strUserOffset - The offset of the string in the page, when ToAttach is RV_TRUE.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetUser(IN    RvSipAddressHandle hSipAddr,
						    IN    RvChar*           strUser,
                            IN    HRPOOL             hPool,
                            IN    HPAGE              hPage,
                            IN    RvInt32           strUserOffset);

/***************************************************************************
 * SipAddrUrlSetUserParam
 * ------------------------------------------------------------------------
 * General: This method sets the user param value within the MsgAddrUrl object.
 *          if eUserParam is RVSIP_USERPARAM_OTHER, then the string in
 *          strUserParam will be set in strUserParam, else, strUserParam should
 *          be ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 *	input:  hUrl         - Handle of the url address object.
 *          eUserParam   - enum value of the userParam.
 *          strUserParam - string with the user param, in case that eUserParam
 *                         is RVSIP_USERPARAM_OTHER.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          offset - The string offset.
 ***************************************************************************/
RvStatus SipAddrUrlSetUserParam(IN RvSipAddressHandle hSipAddr,
							    IN    RvSipUserParam     eUserParam,
							    IN    RvChar*           strUserParam,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32           offset);

/***************************************************************************
 * SipAddrUrlSetTokenizedByParam
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the TokenizedBy param in the
 *            sip url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle of the url address object.
 *    strTokenizedByParam - The TokenizedBy parameter to be set in the object. - if NULL, the
 *                  exist strTokenizedByParam be removed.
 *  strTokenizedByOffset - The string offset.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetTokenizedByParam(IN     RvSipAddressHandle hSipAddr,
                                  IN    RvChar*           strTokenizedByParam,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32           strTokenizedByOffset);

/***************************************************************************
 * SipAddrUrlGetTokenizedByParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the TokenizedBy parameter value.
 * Return Value: string provides the server address to be contacted for this user
 *                 or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetTokenizedByParam(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlGetUrlParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the UrlParams parameter value.
 * Return Value: offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetUrlParams(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetUrlParams
 * ------------------------------------------------------------------------
 * General: This function is used to set the UrlParams value of the MsgAddrUrl
 *          object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl       - Handle of the url address object.
 *	strUrlParams  - The UrlParams parameter to be set in the object. - if NULL,
 *                  the exist strUrlParams be removed.
 *  strUrlOffset  - The offset of the string, in case ToAttach = RV_TRUE.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetUrlParams( IN    RvSipAddressHandle hSipAddr,
					    		  IN    RvChar*           strUrlParams,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32           strUrlOffset);

/***************************************************************************
 * SipUrlGetHeaders
 * ------------------------------------------------------------------------
 * General: This method is used to get the headers parameter value.
 * Return Value: offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetHeaders(RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetHeaders
 * ------------------------------------------------------------------------
 * General: This function is used to set the Headers value of the maddr param
 *          object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipUrl       - Handle of the url address object.
 *	strHeaders    - The headers parameter to be set in the object. - if NULL,
 *                  the exist strHeaders be removed.
 *  strHeadersOffset - offset of the string, in case ToAttach = RV_TRUE.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetHeaders(IN    RvSipAddressHandle hSipAddr,
					           IN    RvChar*           strHeaders,
                               IN    HRPOOL             hPool,
                               IN    HPAGE              hPage,
                               IN    RvInt32           strHeadersOffset);

/***************************************************************************
 * SipAddrUrlGetStrCompParam
 * ------------------------------------------------------------------------
 * General:This method returns the compression type string value from the
 *          MsgAddrUrl object.
 * Return Value: String offset of compression Type, or UNDEFINED/
 * ------------------------------------------------------------------------
 * Arguments:
 *	input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetStrCompParam(
                             IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetCompParam
 * ------------------------------------------------------------------------
 * General: This method sets the compression parameter value in the MsgAddrUrl
 *          object.
 *          if eComp is RVSIP_COMP_OTHER, then strCompParam or the
 *          given pool and page will contain the compression Type string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *	input: hSipUrl      - Handle of the address object.
 *         eComp        - Enumuration value of compression Type.
 *         strCompParam - String of compression Type, in case eComp is
 *                        RVSIP_COMP_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strCompOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrUrlSetCompParam(IN  RvSipAddressHandle hSipAddr,
    							 IN  RvSipCompType      eComp,
    							 IN  RvChar*           strCompParam,
                                 IN  HRPOOL             hPool,
                                 IN  HPAGE              hPage,
                                 IN  RvInt32           strCompOffset);

/***************************************************************************
 * SipAddrUrlSetSigCompIdParam
 * ------------------------------------------------------------------------
 * General: This method sets the sigcomp-id parameter value in the MsgAddrUrl
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hSipUrl      - Handle of the address object.
 *         strSigCompIdParam - String of URN Scheme
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strSigCompIdOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrUrlSetSigCompIdParam(IN  RvSipAddressHandle hSipAddr,
                                 IN  RvChar*           strSigCompIdParam,
                                 IN  HRPOOL             hPool,
                                 IN  HPAGE              hPage,
                                 IN  RvInt32           strSigCompIdOffset);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RvStatus SipAddrUrlSetSigCompIdParamOverWrite(IN  RvSipAddressHandle hSipAddr,
                                 IN  HRPOOL            sourcePool,
                                 IN  HPAGE             sourcePage,
                                 IN  RvInt32           strSigCompIdOffset);
                
RvInt32 SipAddrUrlIsCompParam     (IN  RvSipAddressHandle hSipAddr);
void    SipAddrUrlResetCompIdParam(IN  RvSipAddressHandle hSipAddr);

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */
/***************************************************************************
 * SipAddrUrlGetSigCompIdParam
 * ------------------------------------------------------------------------
 * General: This method returns the sigcomp-id string value from the
 *          MsgAddrUrl object.
 * Return Value: String of sigcomp-id
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetSigCompIdParam(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrAbsUriGetScheme
 * ------------------------------------------------------------------------
 * General:  This method return the shceme field from the Address object.
 * Return Value: scheme or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrAbsUriGetScheme(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrAbsUriSetScheme
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the scheme in the
 *			sip Abs-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr        - Handle of the address object.
 *	strScheme       - The scheme value to be set in the object - if NULL, the exist strScheme
 *                    will be removed.
 *  strSchemeOffset - The offset of the string in the page.
 *  hPool           - The pool on which the string lays (if relevant).
 *  hPage           - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrAbsUriSetScheme(IN    RvSipAddressHandle hSipAddr,
						         IN    RvChar*           strScheme,
                                 IN    HRPOOL             hPool,
                                 IN    HPAGE              hPage,
                                 IN    RvInt32           strSchemeOffset);

/***************************************************************************
 * SipAddrAbsUriGetIdentifier
 * ------------------------------------------------------------------------
 * General:  This method return the shceme field from the Address object.
 * Return Value: Identifier or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrAbsUriGetIdentifier(IN RvSipAddressHandle hSipAddr);


/***************************************************************************
 * SipAddrAbsUriSetIdentifier
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Identifier in the
 *			sip Abs-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr        - Handle of the address object.
 *	strIdentifier       - The Identifier value to be set in the object - if NULL, the exist strIdentifier
 *                    will be removed.
 *  strIdentifierOffset - The offset of the string in the page.
 *  hPool           - The pool on which the string lays (if relevant).
 *  hPage           - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrAbsUriSetIdentifier(IN    RvSipAddressHandle hSipAddr,
						             IN    RvChar*           strIdentifier,
                                     IN    HRPOOL             hPool,
                                     IN    HPAGE              hPage,
                                     IN    RvInt32           strIdentifierOffset);


/***************************************************************************
 * SipAddrGetAddressTypeFromString
 * ------------------------------------------------------------------------
 * General: get the address type from an address in a string format.
 * Return Value:RvSipAddressType - if RVSIP_ADDRTYPE_UNDEFINED then the function
 *                                 failed or we are in extra lean.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsgMgr   - Handle to the message manager.
 *         strAddr - The address in a string format.
 ***************************************************************************/
RvSipAddressType RVCALLCONV SipAddrGetAddressTypeFromString(
												IN  RvSipMsgMgrHandle   hMsgMgr,
												IN  RvChar             *strAddr);

/***************************************************************************
 * SipAddrEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *			Url as encoded buffer (string) onto it.
 *			The user should check the return code.  If the size of
 *			the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *			and the user should send bigger buffer for the encoded message.
 * Return Value: RV_OK, RV_ERROR_INSUFFICIENT_BUFFER, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle of the url address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bAddBrackets - Indicates whether the address should be wrapped by
 *                       brackets (<>). If the address has a display name (jsr32)
 *                       then it will use brackets anyway.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus SipAddrEncode(IN  RvSipAddressHandle hSipAddr,
                       IN  HRPOOL             hPool,
                       IN  HPAGE              hPage,
                       IN  RvBool             bAddBrackets,
					   OUT RvUint32*          length);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * SipAddrUrlGetStrCPC
 * ------------------------------------------------------------------------
 * General: This method returns the CPC string value from the
 *          MsgAddrUrl object.
 * Return Value: String of CPCType
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetStrCPC(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetCPC
 * ------------------------------------------------------------------------
 * General: This method sets the cpc value in the MsgAddrUrl
 *          object.
 *          if eCPC is RVSIP_CPC_TYPE_OTHER, then strCPC or the
 *          given pool and page will contain the transportType string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hUrl      - Handle of the address object.
 *         eCPC   - Enumeration value of cpc Type.
 *         strCPC - String of cpc, in case eCPC is
 *                        RVSIP_CPC_TYPE_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strCPCOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrUrlSetCPC( IN  RvSipAddressHandle hSipAddr,
                                  IN  RvSipUriCPCType   eCPC,
                                  IN  RvChar*           strCPC,
                                  IN  HRPOOL            hPool,
                                  IN  HPAGE             hPage,
                                  IN  RvInt32           strCPCOffset);

/***************************************************************************
 * SipAddrUrlGetStrGr
 * ------------------------------------------------------------------------
 * General: This method is used to get the Gr parameter value.
 * Return Value: string provides the server address to be contacted for this user
 *                 or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetStrGr(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrUrlSetGr
 * ------------------------------------------------------------------------
 * General: This method sets the Gr value in the MsgAddrUrl
 *          object.
 *          if bGrActive is true, then strGr or the
 *          given pool and page will contain the gr string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hUrl      - Handle of the address object.
 *         bGrActive   - boolean stating if the gr parameter exists (could be empty).
 *         strGr - String of Gr, in case gr is empty, NULL will be placed here.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strGrOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrUrlSetGr( IN  RvSipAddressHandle hSipAddr,
                          IN  RvBool             bGrActive,
                          IN  RvChar*            strGr,
                          IN  HRPOOL             hPool,
                          IN  HPAGE              hPage,
                          IN  RvInt32            strGrOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

#ifdef RV_SIP_OTHER_URI_SUPPORT
/***************************************************************************
 * SipAddrConvertOtherAddrToUrl
 * ------------------------------------------------------------------------
 * General: Converts im/pres address to url address
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSipAddr - Handle to the address object.
 ***************************************************************************/
RvStatus RVCALLCONV SipAddrConvertOtherAddrToUrl(IN RvSipAddressHandle hSipAddr);
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
/*-----------------------------------------------------------------------*/
/*                        TEL URI GET/SET METHODS                        */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_TEL_URI_SUPPORT

/***************************************************************************
 * SipAddrTelUriSetPhoneNumber
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the phone-number in the
 *          Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strPhoneNumber       - The phone-number value to be set in the object - if
 *                           NULL, the existing strPhoneNumber will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strPhoneNumberOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetPhoneNumber(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strPhoneNumber,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strPhoneNumberOffset);

/***************************************************************************
 * SipAddrTelUriSetExtension
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Extension in the
 *          Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strExtension         - The Extension value to be set in the object - if
 *                           NULL, the existing strExtension will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strExtensionOffset   - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetExtension(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strExtension,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strExtensionOffset);

/***************************************************************************
 * SipAddrTelUriSetIsdnSubAddr
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Isdn sub address in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strIsdnSubAddr       - The Isdn sub address value to be set in the object -
 *                           if NULL, the existing strIsdnSubAddr will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strIsdnSubAddrOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetIsdnSubAddr(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strIsdnSubAddr,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strIsdnSubAddrOffset);

/***************************************************************************
 * SipAddrTelUriSetPostDial
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the post dial in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strPostDial          - The post dial value to be set in the object -
 *                           if NULL, the existing strPostDial will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strPostDialOffset    - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetPostDial(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strPostDial,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strPostDialOffset);

/***************************************************************************
 * SipAddrTelUriSetContext
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the context in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strContext           - The context value to be set in the object -
 *                           if NULL, the existing strContext will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strContextOffset     - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetContext(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strContext,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strContextOffset);

/***************************************************************************
 * SipAddrTelUriSetOtherParams
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the other parameters in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strOtherParams       - The other params value to be set in the object -
 *                           if NULL, the existing strOtherParams will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strOtherParamsOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetOtherParams(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strOtherParams,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strOtherParamsOffset);

/***************************************************************************
 * SipAddrTelUriGetPhoneNumber
 * ------------------------------------------------------------------------
 * General:  This method returns the phone-number field from the Address object.
 * Return Value: phone-number offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetPhoneNumber(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriGetExtension
 * ------------------------------------------------------------------------
 * General:  This method returns the Extension field from the Address object.
 * Return Value: Extension offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetExtension(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriGetIsdnSubAddr
 * ------------------------------------------------------------------------
 * General:  This method returns the Isdn sub address field from the Address
 *           object.
 * Return Value: Isdn sub address offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetIsdnSubAddr(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriGetPostDial
 * ------------------------------------------------------------------------
 * General:  This method returns the post-dial field from the Address object.
 * Return Value: Post-dial offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetPostDial(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriGetContext
 * ------------------------------------------------------------------------
 * General:  This method returns the context field from the Address object.
 * Return Value: Context offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetContext(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriGetOtherParams
 * ------------------------------------------------------------------------
 * General:  This method returns the other-parameters field from the Address
 *           object.
 * Return Value: Other-parameters offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetOtherParams(IN RvSipAddressHandle hSipAddr);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * SipAddrTelUriGetStrCPC
 * ------------------------------------------------------------------------
 * General: This method returns the CPC string value from the
 *          MsgTelUri object.
 * Return Value: String of CPCType
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrTelUriGetStrCPC(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriSetCPC
 * ------------------------------------------------------------------------
 * General: This method sets the cpc value in the MsgAddrTelUri
 *          object.
 *          if eCPC is RVSIP_CPC_TYPE_OTHER, then strCPC or the
 *          given pool and page will contain the transportType string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hUrl      - Handle of the address object.
 *         eCPC   - Enumeration value of cpc Type.
 *         strCPC - String of cpc, in case eCPC is
 *                        RVSIP_CPC_TYPE_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strCPCOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetCPC( IN  RvSipAddressHandle hSipAddr,
                                  IN  RvSipUriCPCType   eCPC,
                                  IN  RvChar*           strCPC,
                                  IN  HRPOOL            hPool,
                                  IN  HPAGE             hPage,
                                  IN  RvInt32           strCPCOffset);

/***************************************************************************
 * SipAddrTelUriGetRn
 * ------------------------------------------------------------------------
 * General:  This method returns the Rn field from the Address object.
 * Return Value: Rn offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetRn(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriSetRn
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Rn in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strRn           - The Rn value to be set in the object -
 *                      if NULL, the existing strRn will be removed.
 *    hPool           - The pool on which the string lays (if relevant).
 *    hPage           - The page on which the string lays (if relevant).
 *    strRnOffset     - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetRn(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strRn,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strRnOffset);

/***************************************************************************
 * SipAddrTelUriGetRnContext
 * ------------------------------------------------------------------------
 * General:  This method returns the RnContext field from the Address object.
 * Return Value: RnContext offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetRnContext(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriSetRnContext
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the RnContext in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strRnContext    - The RnContext value to be set in the object -
 *                      if NULL, the existing strRnContext will be removed.
 *    hPool           - The pool on which the string lays (if relevant).
 *    hPage           - The page on which the string lays (if relevant).
 *    strRnContextOffset     - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetRnContext(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strRnContext,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strRnContextOffset);

/***************************************************************************
 * SipAddrTelUriGetCic
 * ------------------------------------------------------------------------
 * General:  This method returns the Cic field from the Address object.
 * Return Value: Cic offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetCic(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriSetCic
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Cic in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strCic          - The Cic value to be set in the object -
 *                      if NULL, the existing strCic will be removed.
 *    hPool           - The pool on which the string lays (if relevant).
 *    hPage           - The page on which the string lays (if relevant).
 *    strCicOffset    - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetCic(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strCic,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strCicOffset);

/***************************************************************************
 * SipAddrTelUriGetCicContext
 * ------------------------------------------------------------------------
 * General:  This method returns the CicContext field from the Address object.
 * Return Value: CicContext offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetCicContext(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrTelUriSetCicContext
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the CicContext in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strCicContext   - The CicContext value to be set in the object -
 *                      if NULL, the existing strCicContext will be removed.
 *    hPool           - The pool on which the string lays (if relevant).
 *    hPage           - The page on which the string lays (if relevant).
 *    strCicContextOffset     - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrTelUriSetCicContext(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strCicContext,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strCicContextOffset);

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
/*-----------------------------------------------------------------------*/
/*                  GENERAL ADDRESS GET AND SET FUNCTIONS                */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipAddrGetDisplayName
 * ------------------------------------------------------------------------
 * General:  This method returns the display name field from the address object.
 * Return Value: display name or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrGetDisplayName(IN RvSipAddressHandle hAddr);

/***************************************************************************
 * SipAddrSetDisplayName
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the display-name in the
 *          address object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAddr          - Handle of the address object.
 *    strDisplayName - The display-name value to be set in the object - if NULL,
 *                     the existing display-name will be removed.
 *    hPool          - The pool on which the string lays (if relevant).
 *    hPage          - The page on which the string lays (if relevant).
 *	  strDisplayNameOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrSetDisplayName(IN  RvSipAddressHandle hAddr,
                               IN  RvChar*            strDisplayName,
                               IN  HRPOOL             hPool,
                               IN  HPAGE              hPage,
                               IN  RvInt32            strDisplayNameOffset);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                        DIAMETER URI GET/SET METHODS                   */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * SipAddrDiameterUriGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the name of the host, from the URL object.
 * Return Value: host name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrDiameterUriGetHost(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrDiameterUriSetHost
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the host in the url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDiameterUri - Handle of the url address object.
 *    strHost - the host value to be set in the object. - if NULL, the exist strHost
 *            will be removed.
 *  strHostOffset - The offset of the host string.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetHost(IN    RvSipAddressHandle hSipAddr,
                            IN    RvChar*           strHost,
                            IN    HRPOOL             hPool,
                            IN    HPAGE              hPage,
                            IN    RvInt32           strHostOffset);

/***************************************************************************
 * SipAddrDiameterUriGetStrTransport
 * ------------------------------------------------------------------------
 * General:This method returns the transport protocol string value from the
 *          MsgAddrDiameterUri object.
 * Return Value: String offset of transportType, or UNDEFINED/
 * ------------------------------------------------------------------------
 * Arguments:
 *	input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrDiameterUriGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrDiameterUriSetTransport
 * ------------------------------------------------------------------------
 * General: This method sets the transport protocol value in the MsgAddrDiameterUri
 *          object.
 *          if eTransport is RVSIP_TRANSPORT_OTHER, then strTransport or the
 *          given pool and page will contain the transportType string. else,
 *          they are ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *	input: hSipUrl      - Handle of the address object.
 *         eTransport   - Enumeration value of transportType.
 *         strTransport - String of transportType, in case eTransport is
 *                        RVSIP_TRANSPORT_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strTransportOffset - The offset of the string in the page, when
 *                        attach is RV_TRUE.
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetTransport( IN  RvSipAddressHandle hSipAddr,
    							  IN  RvSipTransport     eTransport,
    							  IN  RvChar*           strTransport,
                                  IN  HRPOOL             hPool,
                                  IN  HPAGE              hPage,
                                  IN  RvInt32           strTransportOffset);

/***************************************************************************
 * SipAddrDiameterUriGetOtherParams
 * ------------------------------------------------------------------------
 * General:  This method returns the other-parameters field from the Address
 *           object.
 * Return Value: Other-parameters offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrDiameterUriGetOtherParams(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * SipAddrDiameterUriSetOtherParams
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the other parameters in
 *          the Tel-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr             - Handle of the address object.
 *    strOtherParams       - The other params value to be set in the object -
 *                           if NULL, the existing strOtherParams will be removed.
 *    hPool                - The pool on which the string lays (if relevant).
 *    hPage                - The page on which the string lays (if relevant).
 *    strOtherParamsOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetOtherParams(
								IN    RvSipAddressHandle hSipAddr,
								IN    RvChar*            strOtherParams,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                IN    RvInt32            strOtherParamsOffset);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

#endif
