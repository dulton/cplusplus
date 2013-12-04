/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           RvSipAddress.h                                   *
 *                                                                            *
 * The file defines the methods of the address object: construct, destruct,   *
 * copy, encode, parse and the ability to access and change it's parameters.  *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/



#ifndef RVSIPADDRESS_H
#define RVSIPADDRESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "rpool_API.h"
#include "RvSipCommonList.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/
/*
 * RvSipAddressStringName
 * ----------------------
 * Defines all Address object fields that are kept in the object in a string format.
 */
typedef enum
{
    RVSIP_ADDRESS_ABS_URI_SCHEME,
    RVSIP_ADDRESS_ABS_URI_IDENTIFIER,
    RVSIP_ADDRESS_USER,
    RVSIP_ADDRESS_HOST,
    RVSIP_ADDRESS_TRANSPORT,
    RVSIP_ADDRESS_MADDR_PARAM,
    RVSIP_ADDRESS_USER_PARAM,
    RVSIP_ADDRESS_COMP_PARAM,
	RVSIP_ADDRESS_SIGCOMPID_PARAM,
	RVSIP_ADDRESS_TOKENIZED_BY_PARAM,
    RVSIP_ADDRESS_URL_OTHER_PARAMS,
    RVSIP_ADDRESS_HEADERS,
    RVSIP_ADDRESS_METHOD,
	RVSIP_ADDRESS_TEL_URI_NUMBER,
    RVSIP_ADDRESS_TEL_URI_EXTENSION,
    RVSIP_ADDRESS_TEL_URI_CONTEXT,
    RVSIP_ADDRESS_TEL_URI_ISDN_SUBS,
    RVSIP_ADDRESS_TEL_URI_POST_DIAL,
    RVSIP_ADDRESS_TEL_URI_OTHER_PARAMS,
	RVSIP_ADDRESS_DISPLAY_NAME,
    RVSIP_ADDRESS_CPC,
    RVSIP_ADDRESS_GR,
    RVSIP_ADDRESS_TEL_URI_CPC,
    RVSIP_ADDRESS_TEL_URI_RN,
    RVSIP_ADDRESS_TEL_URI_RN_CONTEXT,
    RVSIP_ADDRESS_TEL_URI_CIC,
    RVSIP_ADDRESS_TEL_URI_CIC_CONTEXT,
    RVSIP_ADDRESS_DIAMETER_URI_HOST,
    RVSIP_ADDRESS_DIAMETER_URI_TRANSPORT,
    RVSIP_ADDRESS_DIAMETER_URI_OTHER_PARAMS
}RvSipAddressStringName;


#ifdef RV_SIP_JSR32_SUPPORT
/*
 * RvSipAddressFieldName
 * ----------------------------
 * This enum defines all the Address fields.
 * It is used for getting and setting via RvSipAddress interface.
 */
typedef enum
{
	RVSIP_ADDRESS_FIELD_DISPLAY_NAME                 = 0,
	RVSIP_ADDRESS_SIP_URL_FIELD_USER                 = 1,
	RVSIP_ADDRESS_SIP_URL_FIELD_HOST	             = 2,
	RVSIP_ADDRESS_SIP_URL_FIELD_PORT                 = 3,
	RVSIP_ADDRESS_SIP_URL_FIELD_TTL                  = 4,
	RVSIP_ADDRESS_SIP_URL_FIELD_LR                   = 5,           
	RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE            = 6,
	RVSIP_ADDRESS_SIP_URL_FIELD_METHOD               = 7,
	RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT            = 8,           	
	RVSIP_ADDRESS_SIP_URL_FIELD_MADDR                = 9,           
	RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM           = 10,           
	RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS	     = 11,           
	RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS			     = 12,
	RVSIP_ADDRESS_SIP_URL_FIELD_COMP			     = 13,
	RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME			     = 14,
	RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER		     = 15,
	RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM  = 16,
	RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM            = 17,
	RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION            = 18,
	RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR        = 19,
	RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL            = 20,	
	RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT              = 21,	
	RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI               = 22,	
	RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS         = 23,
	RVSIP_ADDRESS_SIP_URL_FIELD_SIGCOMPID            = 24
}RvSipAddressFieldName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/*
 * There are several functions for constructing url:
 * 1. stand alone url (RvSipAddrConstruct) -
 *                          will be copied on the page that is given as argument.
 *
 * 2. url that is part of the requestLine of the msg (RvSipAddrConstructInStartLine) -
 *                          url will be construct on the given msg's page.
 *
 * 3. url that is part of XXX header ( RvSipAddrConstructInXXXHeader ) -
 *                          url will be construct on the given header's page.
 *
 */
/***************************************************************************
 * RvSipAddrConstructInStartLine
 * ------------------------------------------------------------------------
 * General: Constructs an address object inside the Request line of a given message. The
 *          new address is set as the request URI of the Request line. The handle of the
 *          address object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg   - Handle to the SIP message.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly created address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInStartLine(
                                            IN  RvSipMsgHandle      hSipMsg,
                                            IN  RvSipAddressType    eAddrType,
                                            OUT RvSipAddressHandle* hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPartyHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given To or From party
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the party header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPartyHeader(
                                          IN  RvSipPartyHeaderHandle hHeader,
                                          IN  RvSipAddressType       eAddrType,
                                          OUT RvSipAddressHandle*    hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInRouteHopHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given RouteHop
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Route Hop header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInRouteHopHeader(
                                          IN  RvSipRouteHopHeaderHandle hHeader,
                                          IN  RvSipAddressType       eAddrType,
                                          OUT RvSipAddressHandle*    hSipAddr);
#ifndef RV_SIP_PRIMITIVES

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RvSipAddrConstructInReferToHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given Refer-To
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Refer-To header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInReferToHeader(
                                          IN  RvSipReferToHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT RvSipAddressHandle*      hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInReferredByHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given Referred-By
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Referred-By header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInReferredByHeader(
                                        IN  RvSipReferredByHeaderHandle hHeader,
                                        IN  RvSipAddressType            eAddrType,
                                        OUT RvSipAddressHandle*         hSipAddr);
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipAddrConstructInAuthorizationHeader
 * ------------------------------------------------------------------------
 * General: Constructs the digest URI address object inside a given Authorization header.
 *          The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Authorization header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInAuthorizationHeader(
                                          IN   RvSipAuthorizationHeaderHandle hHeader,
                                          IN   RvSipAddressType               eAddrType,
                                          OUT  RvSipAddressHandle            *hSipAddr);
#endif /* #ifdef RV_SIP_AUTH_ON */



/***************************************************************************
 * RvSipAddrConstructInContactHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given contact header. The
 *          address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the contact header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInContactHeader(
                                          IN  RvSipContactHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr);

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrConstructInInfoHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec object inside a given Info 
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Info header.
 *         eAddrType - The type of address to construct.
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInInfoHeader(
                                          IN   RvSipInfoHeaderHandle    hHeader,
                                          IN   RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*      hAddr);

/***************************************************************************
 * RvSipAddrConstructInReplyToHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec object inside a given Reply-To 
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Reply-To header.
 *         eAddrType - The type of address to construct.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInReplyToHeader(
                                          IN   RvSipReplyToHeaderHandle hHeader,
                                          IN   RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*      hSipAddr);

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */										  


#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrConstructInPUriHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PUri header. The
 *          address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PUri header.
 *         eAddrType - The type of address to construct.
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPUriHeader(
                                          IN  RvSipPUriHeaderHandle	   hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPProfileKeyHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PProfileKey header. The
 *          address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PProfileKey header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPProfileKeyHeader(
                                          IN  RvSipPProfileKeyHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPUserDatabaseHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PUserDatabase header. The
 *          address handle is returned.
 *          Note: This address must be of type DiameterURI.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PUserDatabase header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPUserDatabaseHeader(
                                          IN  RvSipPUserDatabaseHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPServedUserHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PServedUser header. The
 *          address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PServedUser header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPServedUserHeader(
                                          IN  RvSipPServedUserHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */	

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrConstructInPDCSTracePartyIDHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PDCSTracePartyID
 *			header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PDCSTracePartyID header.
 *         eAddrType - The type of address to construct.
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPDCSTracePartyIDHeader(
                                          IN  RvSipPDCSTracePartyIDHeaderHandle hHeader,
                                          IN  RvSipAddressType					eAddrType,
                                         OUT  RvSipAddressHandle*				hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPDCSBillingInfoHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PDCSBillingInfo
 *			header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:	hHeader   - Handle to the PDCSBillingInfo header.
 *			eAddrType - The type of address to construct.
 *			ePDCSBillingInfoAddrType - Enumeration for the field in which to put the address.
 * output:	hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPDCSBillingInfoHeader(
                                          IN  RvSipPDCSBillingInfoHeaderHandle	hHeader,
                                          IN  RvSipAddressType					eAddrType,
										  IN  RvSipPDCSBillingInfoAddressType	ePDCSBillingInfoAddrType,
                                         OUT  RvSipAddressHandle*				hSipAddr);

/***************************************************************************
 * RvSipAddrConstructInPDCSRedirectHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PDCSRedirect
 *			header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:	hHeader   - Handle to the PDCSRedirect header.
 *			eAddrType - The type of address to construct.
 *			ePDCSRedirectAddrType - Enumeration for the field in which to put the address.
 * output:	hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPDCSRedirectHeader(
                                  IN  RvSipPDCSRedirectHeaderHandle	hHeader,
                                  IN  RvSipAddressType				eAddrType,
								  IN  RvSipPDCSRedirectAddressType	ePDCSRedirectAddrType,
                                 OUT  RvSipAddressHandle*			hSipAddr);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipAddrConstructInContentTypeHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address inside a given Content-Type header. 
 *          The new address will be set as a parameter within the 
 *          Content-Type header. 
 *          The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Content-Type header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInContentTypeHeader(
                                          IN  RvSipContentTypeHeaderHandle hHeader,
                                          IN  RvSipAddressType			   eAddrType,
                                          OUT RvSipAddressHandle		  *hSipAddr); 

/***************************************************************************
 * RvSipAddrConstructInContentIDHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address inside a given Content-ID header. 
 *          The new address will be set as a parameter within the 
 *          Content-ID header. 
 *          The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Content-ID header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInContentIDHeader(
                                          IN  RvSipContentIDHeaderHandle    hHeader,
                                          IN  RvSipAddressType              eAddrType,
                                          OUT RvSipAddressHandle           *hSipAddr);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * RvSipAddrConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Address object. The object is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool     - Handle to the memory pool. that the object uses.
 *         hPage     - Handle to the memory page that the object uses.
 *         eAddrType - The type of address URL to construct.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstruct(IN  RvSipMsgMgrHandle   hMsgMgr,
                                              IN  HRPOOL              hPool,
                                              IN  HPAGE               hPage,
                                              IN  RvSipAddressType    eAddrType,
                                              OUT RvSipAddressHandle* hSipAddr);

/***************************************************************************
 * RvSipAddrCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Address object to a destination Address object.
 *         You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Address object.
 *    hSource      - Handle to the source Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrCopy(INOUT    RvSipAddressHandle hDestination,
                                        IN       RvSipAddressHandle hSource);

/***************************************************************************
 * RvSipAddrEncode
 * ------------------------------------------------------------------------
 * General: Encodes an Address object to a textual address object. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle to the Address object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded object.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrEncode(IN  RvSipAddressHandle hSipAddr,
                                           IN  HRPOOL             hPool,
                                           OUT HPAGE*             hPage,
                                           OUT RvUint32*         length);

/***************************************************************************
 * RvSipAddrParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Address object, such as, "sip:bob@radvision.com",
 *         into an Address object .
 *         All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A handle to the Address object.
 *    buffer    - Buffer containing a textual Address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrParse(
                                     IN    RvSipAddressHandle hSipAddr,
                                     IN    RvChar*           buffer);


/***************************************************************************
 * RvSipAddrConstructAndParse
 * ------------------------------------------------------------------------
 * General: Constructs and parses a stand-alone Address object. The object is
 *          constructed on a given page taken from a specified pool. The type
 *          of the constructed address and the value of its fields is determined 
 *          by parsing the given buffer. The handle to the new header object is 
 *          returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool     - Handle to the memory pool that the object uses.
 *         hPage     - Handle to the memory page that the object uses.
 *         buffer    - Buffer containing a textual Address.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructAndParse(
											  IN  RvSipMsgMgrHandle   hMsgMgr,
                                              IN  HRPOOL              hPool,
                                              IN  HPAGE               hPage,
                                              IN  RvChar*             buffer,
                                              OUT RvSipAddressHandle* hSipAddr);

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipAddrGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the address fields are kept in a string format, such as the address host
 *          name. In order to get such a field from the address object, your application
 *          should supply an adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hSipAddr   - Handle to the Address object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAddrGetStringLength(
                                      IN  RvSipAddressHandle     hSipAddr,
                                      IN  RvSipAddressStringName stringName);

/***************************************************************************
 * RvSipAddrGetAddrType
 * ------------------------------------------------------------------------
 * General: Gets the address type of an address object.
 * Return Value: Returns RvSipAddressType-for more information see RvSipAddressType
 *               in the Enumerations.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSipAddr - Handle to the address object.
 ***************************************************************************/
RVAPI RvSipAddressType RVCALLCONV RvSipAddrGetAddrType(IN RvSipAddressHandle hSipAddr);


/***************************************************************************
 * RvSipAddrGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the address into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hAddr - Handle to the address object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Via header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrGetRpoolString(
                             IN    RvSipAddressHandle      hAddr,
                             IN    RvSipAddressStringName  eStringName,
                             INOUT RPOOL_Ptr                 *pRpoolPtr);

/***************************************************************************
 * RvSipAddrSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the address
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hAddr         - Handle to the address object.
 *           eStringName   - The string the user wish to set
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrSetRpoolString(
                             IN    RvSipAddressHandle      hAddr,
                             IN    RvSipAddressStringName  eStringName,
                             IN    RPOOL_Ptr               *pRpoolPtr);

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipAddrGetDisplayName
 * ------------------------------------------------------------------------
 * General:  Copies the display-name field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains
 *           the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hAddr     - Handle to the address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL character at the end
 *                    of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrGetDisplayName(
											   IN RvSipAddressHandle hAddr,
                                               IN RvChar*            strBuffer,
                                               IN RvUint             bufferLen,
                                               OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrSetDisplayName
 * ------------------------------------------------------------------------
 * General: Sets the display-name string value in the Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAddr           - Handle to the Address object.
 *    strDisplayName  - The display-name value to be set in the object.
 &                      If NULL is supplied, the existing strDisplayName
 *                      is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrSetDisplayName(
											   IN RvSipAddressHandle hAddr,
                                               IN RvChar*            strDisplayName);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */



/*-----------------------------------------------------------------------*/
/*                  ADDRESS COMPARSION FUNCTIONS                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* RvSipAddrIsEqual
* ------------------------------------------------------------------------
* General: Compares two SIP addresses, as specified in RFC 3261 (except for
*			""%" HEX HEX" encoding)
* Return Value: Returns RV_TRUE if the addresses are equal.
*               Otherwise, the function returns RV_FALSE.
* ------------------------------------------------------------------------
* Arguments:
*    hAddress      - Handle to the first address object.
*    hOtherAddress - Handle to the second address object with which a
*                    comparison is being made.
***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrIsEqual(IN RvSipAddressHandle hAddress,
                                         IN RvSipAddressHandle hOtherAddress);

/***************************************************************************
 * RvSipAddr3261IsEqual
 * ------------------------------------------------------------------------
 * General: Compares two SIP addresses, as specified in RFC 3261.
 * Return Value: Returns RV_TRUE if the addresses are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hAddress      - Handle to the first address object.
 *    hOtherAddress - Handle to the second address object with which a
 *                  comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddr3261IsEqual(IN RvSipAddressHandle hAddress,
											 IN RvSipAddressHandle hOtherAddress); 


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * RvSipAddrUrlCompare
 * ------------------------------------------------------------------------
 * General: Compares two SIP URLs, as specified in RFC 2543.
 * Return Value: Returns 0 if the URLs are equal. Otherwise, the function returns
 *               non-zero value: <0 if first URL is less, or >0 if first URL is more
 *               as the comparison result of a first non-equal URL string element.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader      - Handle to the URL object.
 *	hOtherHeader - Handle to the URL object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvInt8 RVCALLCONV RvSipAddrUrlCompare(IN RvSipAddressHandle hHeader,
                                             IN RvSipAddressHandle hOtherHeader);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipAddrUrlIsEqual
 * ------------------------------------------------------------------------
 * General: OBSOLETE function. use RvSipAddrIsEqual().
 * Return Value: Returns RV_TRUE if the URLs are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the URL object.
 *    hOtherHeader - Handle to the URL object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlIsEqual(IN RvSipAddressHandle hHeader,
                                             IN RvSipAddressHandle hOtherHeader);

/*-----------------------------------------------------------------------
                   SIP URL - GET AND SET FUNCTIONS
  -----------------------------------------------------------------------*/
/***************************************************************************
 * RvSipAddrUrlGetUser
 * ------------------------------------------------------------------------
 * General:  Copies the user field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL
 *                    character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetUser(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);


/***************************************************************************
 * RvSipAddrUrlSetUser
 * ------------------------------------------------------------------------
 * General: Sets the user string value in the Address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the Address object.
 *    strUser  - The user value to be set in the object. If NULL is supplied,
 *               the existing strUser is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetUser(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strUser);

/***************************************************************************
 * RvSipAddrUrlGetHost
 * ------------------------------------------------------------------------
 * General:  Copies the host field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen -  The length of the requested parameter + 1, to include NULL
 *                     character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHost(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);


/***************************************************************************
 * RvSipAddrUrlSetHost
 * ------------------------------------------------------------------------
 * General: Sets the host in the URL object or removes the existing host from the URL
 *          object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    strHost  - The host value to be set in the object.
 *               If NULL is supplied, the existing host is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHost(
                                           IN    RvSipAddressHandle hSipAddr,
                                           IN    RvChar*           strHost);

/***************************************************************************
 * RvSipAddrUrlGetPortNum
 * ------------------------------------------------------------------------
 * General:Gets the port number from the URL object.
 * Return Value: Returns the port number, or UNDEFINED if the port is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAddrUrlGetPortNum(IN RvSipAddressHandle hSipUrl);


/***************************************************************************
 * RvSipAddrUrlSetPortNum
 * ------------------------------------------------------------------------
 * General:  Sets the port number in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 *    portNum - The port number value to be set in the object. In order to remove the port
 *            number from the address object, you can set this parameter to UNDEFINED.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetPortNum(IN RvSipAddressHandle hSipUrl,
                                                  IN RvInt32           portNum);

/***************************************************************************
 * RvSipAddrUrlGetTtlNum
 * ------------------------------------------------------------------------
 * General: Gets the ttl number from the URL object.
 * Return Value: Returns the ttl number, or UNDEFINED if ttl is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipAddrUrlGetTtlNum(IN RvSipAddressHandle hSipAddr);


/***************************************************************************
 * RvSipAddrUrlSetTtlNum
 * ------------------------------------------------------------------------
 * General:  Sets the ttl number in the address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    ttlNum   - The ttl number value to be set in the object.
 *               In order to remove the ttl number from the address object,
 *               you can set this parameter to UNDEFINED.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetTtlNum(
                                          IN RvSipAddressHandle hSipAddr,
                                          IN RvInt16           ttlNum);

/***************************************************************************
 * RvSipAddrUrlGetLrParam
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE if the lr param exists in the address.
 * Return Value: Returns RV_TRUE if the lr param exists in the address.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/

RVAPI RvBool RVCALLCONV RvSipAddrUrlGetLrParam(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetLrParamType
 * ------------------------------------------------------------------------
 * General: Returns the type of the lr param exists in the address.
 *          (lr; lr-true; lr=1;)
 * Return Value: RvSipUrlLrType.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipUrlLrType RVCALLCONV RvSipAddrUrlGetLrParamType(
                                              IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlSetLrParam
 * ------------------------------------------------------------------------
 * General:  Sets the lr parameter in the address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    bLrParam   - RV_TRUE specifies that the lr exists in the address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetLrParam(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvBool            bLrParam);

/***************************************************************************
 * RvSipAddrUrlSetLrParamType
 * ------------------------------------------------------------------------
 * General:  Sets the lr parameter type in the address URL object.
 *          (lr; lr-true; lr=1;)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *    eLrParamType - Type of the lr parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetLrParamType(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvSipUrlLrType     eLrParamType);

/***************************************************************************
 * RvSipAddrUrlIsSecure
 * ------------------------------------------------------------------------
 * General: Defines if the url address scheme is secure or not.
 *          (does the url is secures - with "sips:" prefix,
 *          or not secured - with "sip:" prefix)
 * Return Value: is secure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlIsSecure(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlSetIsSecure
 * ------------------------------------------------------------------------
 * General:  Sets the address URL object to be secured or not.
 *          (sets the url to be secured - with "sips:" prefix,
 *          or not - with "sip:" prefix)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr    - Handle to the URL address object.
 *    bIsSecure   - indicates the secure / not secured
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetIsSecure(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvBool             bIsSecure);

/***************************************************************************
 * RvSipAddrUrlGetMethod
 * ------------------------------------------------------------------------
 * General: Gets the method type enumeration from the URL address object.
 *          If this function returns RVSIP_METHOD_OTHER, call
 *          RvSipAddrUrlGetStrMethod() to get the actual method in a string
 *          format.
 * Return Value: Returns the method type enumeration from the URL address object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipMethodType RVCALLCONV RvSipAddrUrlGetMethod(
                                          IN  RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetStrMethod
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type string value from the
 *          URL address object.
 *          If the bufferLen is big enough, the function will copy the parameter,
 *          into the strBuffer. Else, it will return RV_ERROR_INSUFFICIENT_BUFFER, and
 *          the actualLen param will contain the needed buffer length.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the URL address object.
 *        strBuffer - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter + 1 for null in the end.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrMethod(
                                          IN  RvSipAddressHandle hSipAddr,
                                          IN  RvChar*           strBuffer,
                                          IN  RvUint            bufferLen,
                                          OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrUrlSetMethod
 * ------------------------------------------------------------------------
 * General:Sets the method type in the URL address object.
 *         If eMethodType is RVSIP_METHOD_OTHER, strMethodType is copied to the
 *         header. Otherwise strMethodType is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the URL address object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - You can use this parameter only if the eMethodType parameter is set to
 *                  RVSIP_METHOD_OTHER. In this case, you can supply the method as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetMethod(
                                             IN    RvSipAddressHandle hSipAddr,
                                             IN    RvSipMethodType    eMethodType,
                                             IN    RvChar*           pMethodTypeStr);

/***************************************************************************
 * RvSipAddrUrlGetTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport protocol enumeration value from the URL object. If the
 *          returned Transport enumeration is RVSIP_TRANSPORT_OTHER, you can get
 *          the transport protocol string by calling RvSipAddrUrlGetStrTransport().
 *          If this function returns RVSIP_METHOD_OTHER, call
 *          RvSipAddrUrlGetStrTransport().
 * Return Value: Returns the transportType enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipTransport RVCALLCONV RvSipAddrUrlGetTransport(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetStrTransport
 * ------------------------------------------------------------------------
 * General: Copies the transport protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrUrlGetTransport() returns
 *          RVSIP_TRANSPORT_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include NULL character at the end
 *                         of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetTransport
 * ------------------------------------------------------------------------
 * General:  Sets the transport protocol value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *    eTransport   - The transport protocol enumeration value to be set in the object.
 *  strTransport - You can use this parameter only if the eTransport parameter is set to
 *                 RVSIP_TRANSPORT_OTHER. In this case you can supply the transport
 *                 protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetTransport(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipTransport     eTransport,
                                            IN    RvChar*           strTransport);

/***************************************************************************
 * RvSipAddrUrlGetMaddrParam
 * ------------------------------------------------------------------------
 * General: Copies the maddr parameter from the address object into a given buffer. If the
 *          bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetMaddrParam(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetMaddrParam
 * ------------------------------------------------------------------------
 * General: Sets the value of the maddr parameter in the URL object or removes the the
 *          maddr parameter from the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle to the URL address object.
 *    strMaddrParam - The maddr parameter to be set in the object. If NULL is supplied, the maddr
 *                  parameter is removed from the URL object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetMaddrParam(IN RvSipAddressHandle hSipUrl,
                                                     IN RvChar*       strMaddrParam);


/***************************************************************************
 * RvSipAddrUrlGetUserParam
 * ------------------------------------------------------------------------
 * General:Gets the user parameter enumeration value from the URL object. If eUserParam
 *         is RVSIP_USERPARAM_OTHER, then you can get the user parameter string
 *         value using RvSipAddrUrlGetStrUserParam().
 * Return Value: The enumeration value of the requested userParam.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipUserParam RVCALLCONV RvSipAddrUrlGetUserParam(
                                            IN  RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetStrUserParam
 * ------------------------------------------------------------------------
 * General: Copies the user param string from the URL address object into a given buffer.
 *          Use this function if RvSipAddrUrlGetUserParam() returns
 *          RVSIP_USERPARAM_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr   - Handle to the URL address object.
 *          strBuffer  - Buffer to fill with the requested parameter.
 *          bufferLen  - The length of the buffer.
 *  output: actualLen - The length of the requested parameter + 1, to include a NULL
 *                       value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrUserParam(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetUserParam
 * ------------------------------------------------------------------------
 * General: Sets the user param value within the URL object. If eUserParam is set to
 *          RVSIP_USERPARAM_OTHER, then the string in strUserParam is set to the
 *          URL user parameter. Otherwise, strUserParam is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          eUserParam   - Enumeration value of the userParam.
 *          strUserParam - String containing the user param when eUserParam value is
 *                         RVSIP_USERPARAM_OTHER.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetUserParam(IN RvSipAddressHandle hSipAddr,
                                                    IN RvSipUserParam     eUserParam,
                                                    IN RvChar*           strUserParam);


/***************************************************************************
 * RvSipAddrUrlGetTokenizedByParam
 * ------------------------------------------------------------------------
 * General: Copies the TokenizedBy parameter from the address object into a given buffer. If the
 *          bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetTokenizedByParam(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetTokenizedByParam
 * ------------------------------------------------------------------------
 * General: Sets the value of the TokenizedBy parameter in the URL object or removes the the
 *          TokenizedBy parameter from the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl				- Handle to the URL address object.
 *    strTokenizedBy		- The TokenizedBy parameter to be set in the object. If NULL is 
 *							  supplied, the TokenizedBy parameter is removed from the URL object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetTokenizedByParam(
                                         IN    RvSipAddressHandle	hSipAddr,
                                         IN    RvChar*				strTokenizedBy);

/***************************************************************************
 * RvSipAddrUrlGetOrigParam
 * ------------------------------------------------------------------------
 * General:Gets the Orig param from the URL object.
 * Return Value: Boolean value indicating whether or not the orig parameter exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetOrigParam(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlSetOrigParam
 * ------------------------------------------------------------------------
 * General:  Sets the OrigParam in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 *    bOrigParam - The OrigParam value to be set in the object. In order to remove the port
 *            number from the address object, you can set this parameter to RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetOrigParam(
                                          IN    RvSipAddressHandle	hSipAddr,
                                          IN    RvBool				bOrigParam);

/***************************************************************************
 * RvSipAddrUrlGetOldAddrSpec
 * ------------------------------------------------------------------------
 * General:Gets the OldAddrSpec boolean from the URL object.
 * Return Value: Boolean value indicating whether or not the url should be 
 *               encoded as old addr-spec as defined in RFC 822.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetOldAddrSpec(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlSetOldAddrSpec
 * ------------------------------------------------------------------------
 * General:  Sets the OldAddrSpec boolean in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 *    bOldAddrSpec -  Boolean for whether or not the url should be 
 *                    encoded as old addr-spec as defined in RFC 822.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetOldAddrSpec(
                                          IN    RvSipAddressHandle  hSipAddr,
                                          IN    RvBool              bOldAddrSpec);

/***************************************************************************
 * RvSipAddrUrlGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the URL other params field of the Address object into a given buffer.
 *          Not all the address parameters have separated fields in the address object.
 *          Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form:
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen  - The length of the requested parameter + 1, to include a
 *                     NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetOtherParams(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);


/***************************************************************************
 * RvSipAddrUrlSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the address object.
 *          Not all the address parameters have separated fields in the address object.
 *          Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form:
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle to the URL address object.
 *    strUrlParams  - The extended parameters field to be set in the Address object.
 *                    If NULL is supplied, the existing extended parameters field
 *                    is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetOtherParams(
                                            IN RvSipAddressHandle hSipUrl,
                                            IN RvChar*           strUrlParams);

/***************************************************************************
 * RvSipAddrUrlGetHeaders
 * ------------------------------------------------------------------------
 * General: Copies the headers field of the Address object into a given buffer.
 *          If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a
 *                     NULL value at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHeaders(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);


/***************************************************************************
 * RvSipAddrUrlSetHeaders
 * ------------------------------------------------------------------------
 * General: Used to set the headers parameter in the URL address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl    - Handle to the URL address object.
 *    strHeaders - The headers parameter to be set in the object. If NULL is
 *                 supplied, the existing headers parameter is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHeaders(
                                              IN RvSipAddressHandle hSipUrl,
                                              IN RvChar*           strHeaders);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipAddrUrlGetHeadersList
 * ------------------------------------------------------------------------
 * General: The function takes the url headers parameter as string, and parse
 *          it into a list of headers.
 *          Application must supply a list structure, and a consecutive buffer
 *          with url headers parameter string.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the address object.
 *    hHeadersList    - A handle to the Address object.
 *    pHeadersBuffer  - Buffer containing a textual url headers parameter string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHeadersList(
                                     IN    RvSipAddressHandle    hSipAddr,
                                     IN    RvSipCommonListHandle hHeadersList,
                                     IN    RvChar*               pHeadersBuffer);

/***************************************************************************
 * RvSipAddrUrlSetHeadersList
 * ------------------------------------------------------------------------
 * General: This function is used to set the Headers param from headers list.
 *          The function encode all headers. during encoding each header
 *          coverts reserved characters to it's escaped form. each header
 *          also set '=' instead of ':' after header name.
 *          This function also sets '&' between headers.
 *          If you wish the headers in the header list to take compact form
 *          you should mark each of the headers to be compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the address object.
 *    hHeadersList    - A handle to the headers list object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHeadersList(
                                     IN    RvSipAddressHandle    hSipAddr,
                                     IN    RvSipCommonListHandle hHeadersList);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * RvSipAddrUrlGetCompParam
 * ------------------------------------------------------------------------
 * General: Gets the compression type enumeration value from the URL object. If the
 *          returned compression enumeration is RVSIP_COMP_OTHER, you can get
 *          the compression type string by calling RvSipAddrUrlGetStrCompParam().
 * Return Value: Returns the compression Type enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipCompType RVCALLCONV RvSipAddrUrlGetCompParam(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetStrCompParam
 * ------------------------------------------------------------------------
 * General: Copies the compression type string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrUrlGetCompParam() returns
 *          RVSIP_COMP_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include NULL character at the end
 *                         of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrCompParam(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetCompParam
 * ------------------------------------------------------------------------
 * General:  Sets the compression type value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *    eComp        - The compression type enumeration value to be set in the object.
 *  strCompParam - You can use this parameter only if the eComp parameter is set to
 *                 RVSIP_COMP_OTHER. In this case you can supply the compression
 *                 type as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetCompParam(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipCompType     eComp,
                                            IN    RvChar*           strCompParam);

/***************************************************************************
 * RvSipAddrUrlSetSigCompIdParam
 * ------------------------------------------------------------------------
 * General:  Sets the sigcomp-id value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *  strSigCompIdParam - The sigcomp-id string
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetSigCompIdParam(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strSigCompIdParam);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RVAPI RvInt32  RVCALLCONV RvSipAddrUrlIsCompParam(IN    RvSipAddressHandle hSipAddr);
 
RVAPI RvInt32  RVCALLCONV RvSipAddrUrlGetSigCompIdParamOffset(IN RvSipAddressHandle hSipAddr);

RVAPI RvStatus RVCALLCONV RvSipAddrUrlRpSigCompIdParam(IN    RvSipAddressHandle hSipAddr,
														   IN    RvChar*    strSigCompIdParam, RvInt32 size);

RVAPI RvStatus RvSipAddrUrlSetSigCompIdParamOverWrite(IN  RvSipAddressHandle hSipAddr,
                                 IN  HRPOOL            sourcePool,
                                 IN  HPAGE             sourcePage,
                                 IN  RvInt32           strSigCompIdOffset);
#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */


/***************************************************************************
 * RvSipAddrUrlGetSigCompIdParam
 * ------------------------------------------------------------------------
 * General: Copies the sigcomp-id parameter from the address object into a given buffer. If the
 *          bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetSigCompIdParam(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrUrlGetCPC
 * ------------------------------------------------------------------------
 * General: Gets the CPC enumeration value from the URL object. If the
 *          returned CPC enumeration is RVSIP_CPC_TYPE_OTHER, you can get
 *          the CPC protocol string by calling RvSipAddrUrlGetStrCPC().
 *          If this function returns RVSIP_CPC_TYPE_OTHER, call
 *          RvSipAddrUrlGetStrCPC().
 * Return Value: Returns the CPCType enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipUriCPCType RVCALLCONV RvSipAddrUrlGetCPC(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlGetStrCPC
 * ------------------------------------------------------------------------
 * General: Copies the CPC protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrUrlGetCPC() returns
 *          RVSIP_CPC_TYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include NULL character at the end
 *                         of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrCPC(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlSetCPC
 * ------------------------------------------------------------------------
 * General:  Sets the CPC value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *    eCPC   - The CPC enumeration value to be set in the object.
 *  strCPC - You can use this parameter only if the eCPC parameter is set to
 *                 RVSIP_CPC_TYPE_OTHER. In this case you can supply the CPC
 *                 protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetCPC(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipUriCPCType     eCPC,
                                            IN    RvChar*           strCPC);

/***************************************************************************
 * RvSipAddrUrlGetStrGr
 * ------------------------------------------------------------------------
 * General: Copies the Gr parameter from the address object into a given buffer. If the
 *          bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrGr(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrUrlGetGr
 * ------------------------------------------------------------------------
 * General:Gets the Gr param from the URL object.
 * Return Value: Returns the Gr param. RV_FALSE if missing.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetGr(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrUrlSetGr
 * ------------------------------------------------------------------------
 * General: Sets the value of the Gr parameter in the URL object or removes the
 *          Gr parameter from the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl	- Handle to the URL address object.
 *    bGrActive - Boolean stating if the gr parameter exists (could be empty)
 *    strGr	    - The Gr parameter to be set in the object. If NULL is 
 *				  supplied, the Gr parameter is removed from the URL object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetGr(
                                         IN    RvSipAddressHandle	hSipAddr,
                                         IN    RvBool               bGrActive,
                                         IN    RvChar*				strGr);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/*-----------------------------------------------------------------------
                  ABS URI - GET AND SET FUNCTIONS
   -----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAddrAbsUriGetScheme
 * ------------------------------------------------------------------------
 * General:  Copies the scheme field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrAbsUriGetScheme(IN RvSipAddressHandle hSipAddr,
                                                    IN RvChar*           strBuffer,
                                                    IN RvUint            bufferLen,
                                                    OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrAbsUriSetScheme
 * ------------------------------------------------------------------------
 * General: Sets the scheme string in the absolute URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the absolute URI Address object.
 *    strScheme  - The scheme to be set in the object. If NULL is supplied,
 *             the existing scheme is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrAbsUriSetScheme(IN    RvSipAddressHandle hSipAddr,
                                                    IN    RvChar*           strScheme);


/***************************************************************************
 * RvSipAddrAbsUriGetIdentifier
 * ------------------------------------------------------------------------
 * General:  Copies the Identifier field from the absolute URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and
 *           actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  absolute URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL character at the end
 *                    of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrAbsUriGetIdentifier(IN RvSipAddressHandle hSipAddr,
                                                        IN RvChar*           strBuffer,
                                                        IN RvUint            bufferLen,
                                                        OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrAbsUriSetIdentifier
 * ------------------------------------------------------------------------
 * General: Sets the Identifier string in the absolute URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the absolute URI Address object.
 *    strIdentifier  - The identifier string to be set in the object. If NULL is supplied,
 *             the existing identifier
 *             is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrAbsUriSetIdentifier(IN    RvSipAddressHandle hSipAddr,
                                                        IN    RvChar*           strIdentifier);
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * RvSipAddrUrlReplaceUser
 * ------------------------------------------------------------------------
 * General:  Replace the address object's user field with a given buffer.
 *           This replacement will be successful only if the allocated user field in address object
 *           is adquate. Otherwise, the function returns RV_InsufficientBuffer.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlReplaceUser(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer);

/***************************************************************************
 * RvSipAddrFree
 * ------------------------------------------------------------------------
 * General:  Free memory for the address.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrFree(IN RvSipAddressHandle hSipAddr);

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipAddrGetAddressTypeFromString
 * ------------------------------------------------------------------------
 * General: Get the address type of an address in a string format.
 * Return Value: The type of the given address, or RVSIP_ADDRTYPE_UNDEFINED 
 *               in case of failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsgMgr - Handle to the message manager.
 *         strAddr - The address in a string format.
 ***************************************************************************/
RVAPI RvSipAddressType RVCALLCONV RvSipAddrGetAddressTypeFromString(
												IN  RvSipMsgMgrHandle   hMsgMgr,
												IN  RvChar             *strAddr);

/*-----------------------------------------------------------------------*/
/*                  TEL URI - GET AND SET FUNCTIONS                      */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_TEL_URI_SUPPORT

/***************************************************************************
 * RvSipAddrTelUriSetIsGlobalPhoneNumber
 * ------------------------------------------------------------------------
 * General:  Sets the indication whether this TEL URL address is a global
 *           phone number.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr               - Handle to the TEL URI address object.
 *    bIsGlobalPhoneNumber   - RV_TRUE indicates that this TEL URI represents
 *                             a global phone number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetIsGlobalPhoneNumber(
                                  IN    RvSipAddressHandle hSipAddr,
                                  IN    RvBool             bIsGlobalPhoneNumber);

/***************************************************************************
 * RvSipAddrTelUriGetIsGlobalPhoneNumber
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE if this address indicates a global phone number.
 * Return Value: Returns RV_TRUE if this address indicates a global phone number.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the TEL URI address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrTelUriGetIsGlobalPhoneNumber(
												IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrTelUriSetPhoneNumber
 * ------------------------------------------------------------------------
 * General: Sets the phone-number string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strPhoneNumber  - The phone-number string to be set in the object.
 *                      If NULL is supplied, the existing phone-number
 *						is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetPhoneNumber(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strPhoneNumber);

/***************************************************************************
 * RvSipAddrTelUriGetPhoneNumber
 * ------------------------------------------------------------------------
 * General:  Copies the phone-number field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetPhoneNumber(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetExtension
 * ------------------------------------------------------------------------
 * General: Sets the extension string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strExtension    - The extension string to be set in the object.
 *                      If NULL is supplied, the existing extension
 *						is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetExtension(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strExtension);

/***************************************************************************
 * RvSipAddrTelUriGetExtension
 * ------------------------------------------------------------------------
 * General:  Copies the extension field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetExtension(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetIsdnSubAddr
 * ------------------------------------------------------------------------
 * General: Sets the Isdn-sub-address string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strIsdnSubAddr  - The Isdn-sub-address string to be set in the object.
 *                      If NULL is supplied, the existing Isdn-sub-address
 *						is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetIsdnSubAddr(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strIsdnSubAddr);

/***************************************************************************
 * RvSipAddrTelUriGetIsdnSubAddr
 * ------------------------------------------------------------------------
 * General:  Copies the Isdn-sub-address field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetIsdnSubAddr(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetPostDial
 * ------------------------------------------------------------------------
 * General: Sets the post-dial string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strPostDial     - The post-dial string to be set in the object.
 *                      If NULL is supplied, the existing post-dial
 *						is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetPostDial(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strPostDial);

/***************************************************************************
 * RvSipAddrTelUriGetPostDial
 * ------------------------------------------------------------------------
 * General:  Copies the post-dial field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetPostDial(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetContext
 * ------------------------------------------------------------------------
 * General: Sets the context string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strContext      - The context string to be set in the object.
 *                      If NULL is supplied, the existing context
 *						is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetContext(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strContext);

/***************************************************************************
 * RvSipAddrTelUriGetContext
 * ------------------------------------------------------------------------
 * General:  Copies the context field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetContext(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetEnumdiParamType
 * ------------------------------------------------------------------------
 * General:  Sets the enumdi parameter type in a TEL URI address object.
 *          (undefined or exists and empty)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the TEL URI address object.
 *    eEnumdiType  - Type of the enumdi parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetEnumdiParamType(
                                          IN    RvSipAddressHandle      hSipAddr,
                                          IN    RvSipTelUriEnumdiType   eEnumdiType);

/***************************************************************************
 * RvSipAddrTelUriGetEnumdiParamType
 * ------------------------------------------------------------------------
 * General: Returns the type of the enumdi param in a TEL URI address.
 *          (undefined or exists and empty)
 * Return Value: RvSipTelUriEnumdiType.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the TEL URI address object.
 ***************************************************************************/
RVAPI RvSipTelUriEnumdiType RVCALLCONV RvSipAddrTelUriGetEnumdiParamType(
                                              IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrTelUriSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the TEL URI address object.
 *          Not all the address parameters have separated fields in the address
 *          object. Parameters with no specific fields are referred to as other
 *          params. They are kept in the object in one concatenated string in
 *          the form name=value;name=value...
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the TEL URI Address object.
 *    strOtherParams  - The other-params string to be set in the object.
 *                      If NULL is supplied, the existing other-params
 *						are removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetOtherParams(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strOtherParams);

/***************************************************************************
 * RvSipAddrTelUriGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the URL other params field of the Address object into a
 *          given buffer.
 *          Not all the address parameters have separated fields in the address
 *          object. Parameters with no specific fields are referred to as other
 *          params. They are kept in the object in one concatenated string in
 *          the form name=value;name=value...
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetOtherParams(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrTelUriGetCPC
 * ------------------------------------------------------------------------
 * General: Gets the CPC enumeration value from the URL object. If the
 *          returned CPC enumeration is RVSIP_CPC_TYPE_OTHER, you can get
 *          the CPC protocol string by calling RvSipAddrTelUriGetStrCPC().
 *          If this function returns RVSIP_CPC_TYPE_OTHER, call
 *          RvSipAddrTelUriGetStrCPC().
 * Return Value: Returns the CPCType enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipUriCPCType RVCALLCONV RvSipAddrTelUriGetCPC(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrTelUriGetStrCPC
 * ------------------------------------------------------------------------
 * General: Copies the CPC protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrTelUriGetCPC() returns
 *          RVSIP_CPC_TYPE_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include NULL character at the end
 *                         of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetStrCPC(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetCPC
 * ------------------------------------------------------------------------
 * General:  Sets the CPC value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the URL address object.
 *    eCPC   - The CPC enumeration value to be set in the object.
 *  strCPC - You can use this parameter only if the eCPC parameter is set to
 *                 RVSIP_CPC_TYPE_OTHER. In this case you can supply the CPC
 *                 protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetCPC(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipUriCPCType     eCPC,
                                            IN    RvChar*           strCPC);

/***************************************************************************
 * RvSipAddrTelUriSetRn
 * ------------------------------------------------------------------------
 * General: Sets the Rn string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the TEL URI Address object.
 *    strRn      - The Rn string to be set in the object.
 *                 If NULL is supplied, the existing Rn
 *                 is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetRn(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strRn);

/***************************************************************************
 * RvSipAddrTelUriGetRn
 * ------------------------------------------------------------------------
 * General:  Copies the Rn field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetRn(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetRnContext
 * ------------------------------------------------------------------------
 * General: Sets the RnContext string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the TEL URI Address object.
 *    strRnContext      - The RnContext string to be set in the object.
 *                 If NULL is supplied, the existing RnContext
 *                 is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetRnContext(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strRnContext);

/***************************************************************************
 * RvSipAddrTelUriGetRnContext
 * ------------------------------------------------------------------------
 * General:  Copies the RnContext field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetRnContext(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetCic
 * ------------------------------------------------------------------------
 * General: Sets the Cic string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the TEL URI Address object.
 *    strCic      - The Cic string to be set in the object.
 *                 If NULL is supplied, the existing Cic
 *                 is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetCic(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strCic);

/***************************************************************************
 * RvSipAddrTelUriGetCic
 * ------------------------------------------------------------------------
 * General:  Copies the Cic field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetCic(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriSetCicContext
 * ------------------------------------------------------------------------
 * General: Sets the CicContext string in the TEL URI Address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the TEL URI Address object.
 *    strCicContext      - The CicContext string to be set in the object.
 *                 If NULL is supplied, the existing CicContext
 *                 is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetCicContext(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strCicContext);

/***************************************************************************
 * RvSipAddrTelUriGetCicContext
 * ------------------------------------------------------------------------
 * General:  Copies the CicContext field from the TEL URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter
 *           into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER
 *           and actualLen contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  TEL URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriGetCicContext(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);

/***************************************************************************
 * RvSipAddrTelUriGetNpdi
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE if the Npdi parameter exists in the address.
 * Return Value: Returns RV_TRUE if the Npdi parameter exists in the address.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the TEL URI address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrTelUriGetNpdi(
												IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrTelUriSetNpdi
 * ------------------------------------------------------------------------
 * General:  Sets the Npdi parameter of the address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the TEL URI address object.
 *    bNpdi    - RV_TRUE indicates that the Npdi parameter exists in 
 *               the address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrTelUriSetNpdi(
                                  IN    RvSipAddressHandle hSipAddr,
                                  IN    RvBool             bNpdi);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/*-----------------------------------------------------------------------*/
/*                  DIAMETER URI - GET AND SET FUNCTIONS                 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipAddrDiameterUriIsSecure
 * ------------------------------------------------------------------------
 * General: Defines if the url address scheme is secure or not.
 *          (does the url has "aaas://" or "aaa://" prefix)
 * Return Value: is secure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrDiameterUriIsSecure(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrDiameterUriSetIsSecure
 * ------------------------------------------------------------------------
 * General:  Sets the secure flag in the address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    bIsSecure   - indicates the secure (e.g sips)/ not secured
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetIsSecure(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvBool             bIsSecure);

/***************************************************************************
 * RvSipAddrDiameterUriSetProtocolType
 * ------------------------------------------------------------------------
 * General:  Sets the Protocol type in a DIAMETER URI address object.
 *          (undefined or exists and empty)
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr     - Handle to the DIAMETER URI address object.
 *    eProtocolType  - Type of the Protocol parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetProtocolType(
                                          IN    RvSipAddressHandle      hSipAddr,
                                          IN    RvSipDiameterProtocol   eProtocolType);

/***************************************************************************
 * RvSipAddrDiameterUriGetProtocolType
 * ------------------------------------------------------------------------
 * General: Returns the type of the Protocol in a DIAMETER URI address.
 *          (undefined or exists and empty)
 * Return Value: RvSipDiameterProtocol.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the DIAMETER URI address object.
 ***************************************************************************/
RVAPI RvSipDiameterProtocol RVCALLCONV RvSipAddrDiameterUriGetProtocolType(
                                              IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrDiameterUriGetHost
 * ------------------------------------------------------------------------
 * General:  Copies the host field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains
 *           the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL character at the end
 *                     of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriGetHost(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrDiameterUriSetHost
 * ------------------------------------------------------------------------
 * General: Sets the host in the URL object or removes the existing host from the URL
 *          object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    strHost  - The host value to be set in the object. If NULL is supplied, the existing host is
 *             removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetHost(
                                           IN    RvSipAddressHandle hSipAddr,
                                           IN    RvChar*           strHost);

/***************************************************************************
 * RvSipAddrDiameterUriGetPortNum
 * ------------------------------------------------------------------------
 * General:Gets the port number from the DiameterUri object.
 * Return Value: Returns the port number, or UNDEFINED if the port is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the DiameterUri address object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAddrDiameterUriGetPortNum(IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrDiameterUriSetPortNum
 * ------------------------------------------------------------------------
 * General:  Sets the port number in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    portNum  - The port number value to be set in the object. In order to remove the port
 *               number from the address object, you can set this parameter to UNDEFINED.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetPortNum(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvInt32           portNum);

/***************************************************************************
 * RvSipAddrDiameterUriGetTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport protocol enumeration value from the URL object. If the
 *          returned Transport enumeration is RVSIP_TRANSPORT_OTHER, you can get
 *          the transport protocol string by calling RvSipAddrDiameterUriGetStrTransport().
 *          If this function returns RVSIP_METHOD_OTHER, call
 *          RvSipAddrDiameterUriGetStrTransport().
 * Return Value: Returns the transportType enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipTransport RVCALLCONV RvSipAddrDiameterUriGetTransport(
                                           IN RvSipAddressHandle hSipAddr);

/***************************************************************************
 * RvSipAddrDiameterUriGetStrTransport
 * ------------------------------------------------------------------------
 * General: Copies the transport protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrDiameterUriGetTransport() returns
 *          RVSIP_TRANSPORT_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include NULL character at the end
 *                         of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen);

/***************************************************************************
 * RvSipAddrDiameterUriSetTransport
 * ------------------------------------------------------------------------
 * General:  Sets the transport protocol value in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the URL address object.
 *    eTransport - The transport protocol enumeration value to be set in the object.
 *  strTransport - You can use this parameter only if the eTransport parameter is set to
 *                 RVSIP_TRANSPORT_OTHER. In this case you can supply the transport
 *                 protocol as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetTransport(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipTransport     eTransport,
                                            IN    RvChar*           strTransport);

/***************************************************************************
 * RvSipAddrDiameterUriSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the DIAMETER URI address object.
 *          Not all the address parameters have separated fields in the address
 *          object. Parameters with no specific fields are referred to as other
 *          params. They are kept in the object in one concatenated string in
 *          the form name=value;name=value...
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle to the DIAMETER URI Address object.
 *    strOtherParams  - The other-params string to be set in the object.
 *                      If NULL is supplied, the existing other-params
 *						are removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetOtherParams(
											IN    RvSipAddressHandle hSipAddr,
                                            IN    RvChar*            strOtherParams);

/***************************************************************************
 * RvSipAddrDiameterUriGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the URL other params field of the Address object into a
 *          given buffer.
 *          Not all the address parameters have separated fields in the address
 *          object. Parameters with no specific fields are referred to as other
 *          params. They are kept in the object in one concatenated string in
 *          the form name=value;name=value...
 *          If the bufferLen is adequate, the function copies the requested
 *          parameter into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the  DIAMETER URI address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include
 *                    NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriGetOtherParams(
												IN RvSipAddressHandle hSipAddr,
                                                IN RvChar*            strBuffer,
                                                IN RvUint             bufferLen,
                                                OUT RvUint*           actualLen);



#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif


#endif /* RVSIPADDRESS_H */


